"""
Visual Similarity Search
========================

Extract embeddings from images and perform similarity search using FAISS.
Based on techniques from Géron Ch. 17 and fast.ai representation learning.
"""

from pathlib import Path
from typing import List, Dict, Optional, Tuple
import numpy as np
import pickle

try:
    from fastai.vision.all import load_learner, PILImage
    import torch
    import torch.nn as nn
    FASTAI_AVAILABLE = True
except ImportError:
    FASTAI_AVAILABLE = False

try:
    import faiss
    FAISS_AVAILABLE = True
except ImportError:
    FAISS_AVAILABLE = False


class PartEmbedder:
    """
    Extract visual embeddings from part images for similarity search.

    Usage:
        # Build index
        embedder = PartEmbedder('classifier.pkl')
        embedder.build_index(image_paths, part_ids)
        embedder.save_index('parts.index')

        # Search
        embedder = PartEmbedder('classifier.pkl')
        embedder.load_index('parts.index')
        results = embedder.search_similar('query.jpg', k=5)
    """

    def __init__(self, model_path: str):
        if not FASTAI_AVAILABLE:
            raise ImportError("fastai not installed")
        if not FAISS_AVAILABLE:
            raise ImportError("faiss not installed. Run: pip install faiss-cpu")

        self.learn = load_learner(model_path)

        # Extract encoder (remove classification head)
        # ResNet: children are [conv1, bn1, relu, maxpool, layer1-4, avgpool, fc]
        # We want everything except fc
        self.encoder = nn.Sequential(*list(self.learn.model.children())[:-1])
        self.encoder.eval()

        self.embedding_dim = self._get_embedding_dim()
        self.index = None
        self.part_ids: List[int] = []
        self.image_paths: List[str] = []

    def _get_embedding_dim(self) -> int:
        """Determine embedding dimension from model."""
        with torch.no_grad():
            dummy = torch.randn(1, 3, 224, 224)
            out = self.encoder(dummy)
            return out.shape[1]

    def get_embedding(self, image_path: str) -> np.ndarray:
        """
        Extract embedding vector from a single image.

        Returns:
            Normalized embedding vector of shape (embedding_dim,)
        """
        img = PILImage.create(image_path)

        # Use the learner's transforms for preprocessing
        x = self.learn.dls.test_dl([img]).one_batch()[0]

        with torch.no_grad():
            emb = self.encoder(x)
            emb = emb.squeeze()  # Remove spatial dimensions

            # L2 normalize for cosine similarity
            emb = emb / emb.norm()

        return emb.cpu().numpy()

    def get_embeddings_batch(
        self,
        image_paths: List[str],
        batch_size: int = 32,
        show_progress: bool = True
    ) -> np.ndarray:
        """
        Extract embeddings for multiple images efficiently.

        Returns:
            Array of shape (n_images, embedding_dim)
        """
        embeddings = []

        if show_progress:
            from tqdm import tqdm
            iterator = tqdm(range(0, len(image_paths), batch_size), desc="Extracting embeddings")
        else:
            iterator = range(0, len(image_paths), batch_size)

        for i in iterator:
            batch_paths = image_paths[i:i + batch_size]
            batch_imgs = [PILImage.create(p) for p in batch_paths]

            # Create batch tensor
            dl = self.learn.dls.test_dl(batch_imgs)
            batch_tensor = dl.one_batch()[0]

            with torch.no_grad():
                batch_emb = self.encoder(batch_tensor)
                batch_emb = batch_emb.squeeze(-1).squeeze(-1)  # Remove spatial dims

                # L2 normalize
                batch_emb = batch_emb / batch_emb.norm(dim=1, keepdim=True)

            embeddings.append(batch_emb.cpu().numpy())

        return np.vstack(embeddings).astype('float32')

    def build_index(
        self,
        image_paths: List[str],
        part_ids: List[int],
        index_type: str = 'flat'
    ):
        """
        Build FAISS index from images.

        Args:
            image_paths: Paths to images
            part_ids: Corresponding part IDs
            index_type: 'flat' for exact search, 'ivf' for approximate (faster)
        """
        if len(image_paths) != len(part_ids):
            raise ValueError("image_paths and part_ids must have same length")

        print(f"Building index for {len(image_paths)} images...")

        # Extract all embeddings
        embeddings = self.get_embeddings_batch(image_paths)

        # Build FAISS index
        d = embeddings.shape[1]

        if index_type == 'flat':
            # Exact search with inner product (cosine similarity for normalized vectors)
            self.index = faiss.IndexFlatIP(d)

        elif index_type == 'ivf':
            # Approximate search for large datasets (Géron recommends)
            nlist = min(100, len(embeddings) // 10)  # Number of clusters
            quantizer = faiss.IndexFlatIP(d)
            self.index = faiss.IndexIVFFlat(quantizer, d, nlist)

            # IVF requires training
            self.index.train(embeddings)

        else:
            raise ValueError(f"Unknown index_type: {index_type}")

        self.index.add(embeddings)
        self.part_ids = list(part_ids)
        self.image_paths = list(image_paths)

        print(f"Index built: {self.index.ntotal} vectors")

    def search_similar(
        self,
        query_image: str,
        k: int = 5,
        threshold: Optional[float] = None
    ) -> List[Dict]:
        """
        Find k most similar parts to query image.

        Args:
            query_image: Path to query image
            k: Number of results
            threshold: Minimum similarity score (0-1)

        Returns:
            List of {part_id, similarity, image_path}
        """
        if self.index is None:
            raise ValueError("Index not built. Call build_index() first.")

        # Get query embedding
        query_emb = self.get_embedding(query_image).reshape(1, -1).astype('float32')

        # Search
        similarities, indices = self.index.search(query_emb, k)

        results = []
        for sim, idx in zip(similarities[0], indices[0]):
            if idx < 0:  # FAISS returns -1 for empty slots
                continue
            if threshold and sim < threshold:
                continue

            results.append({
                'part_id': self.part_ids[idx],
                'similarity': float(sim),
                'image_path': self.image_paths[idx]
            })

        return results

    def search_by_embedding(
        self,
        embedding: np.ndarray,
        k: int = 5
    ) -> List[Dict]:
        """Search using pre-computed embedding."""
        if self.index is None:
            raise ValueError("Index not built")

        query = embedding.reshape(1, -1).astype('float32')
        similarities, indices = self.index.search(query, k)

        return [
            {
                'part_id': self.part_ids[idx],
                'similarity': float(sim),
                'image_path': self.image_paths[idx]
            }
            for sim, idx in zip(similarities[0], indices[0])
            if idx >= 0
        ]

    def save_index(self, path: str):
        """Save index and metadata to disk."""
        if self.index is None:
            raise ValueError("No index to save")

        # Save FAISS index
        faiss.write_index(self.index, path)

        # Save metadata
        metadata_path = path + '.meta'
        with open(metadata_path, 'wb') as f:
            pickle.dump({
                'part_ids': self.part_ids,
                'image_paths': self.image_paths,
                'embedding_dim': self.embedding_dim
            }, f)

        print(f"Index saved to {path}")

    def load_index(self, path: str):
        """Load index and metadata from disk."""
        self.index = faiss.read_index(path)

        metadata_path = path + '.meta'
        with open(metadata_path, 'rb') as f:
            metadata = pickle.load(f)
            self.part_ids = metadata['part_ids']
            self.image_paths = metadata['image_paths']

        print(f"Index loaded: {self.index.ntotal} vectors")

    def add_to_index(self, image_path: str, part_id: int):
        """Add a new image to existing index."""
        if self.index is None:
            raise ValueError("No index. Call build_index() first.")

        embedding = self.get_embedding(image_path).reshape(1, -1).astype('float32')
        self.index.add(embedding)
        self.part_ids.append(part_id)
        self.image_paths.append(image_path)

    def export_embeddings(self, output_path: str):
        """Export all embeddings as numpy array for external use."""
        if self.index is None:
            raise ValueError("No index built")

        # Reconstruct embeddings from index
        embeddings = np.zeros((self.index.ntotal, self.embedding_dim), dtype='float32')
        for i in range(self.index.ntotal):
            embeddings[i] = self.index.reconstruct(i)

        np.savez(
            output_path,
            embeddings=embeddings,
            part_ids=np.array(self.part_ids),
            image_paths=np.array(self.image_paths)
        )
        print(f"Embeddings exported to {output_path}")
