"""
PartsDB ML Service
==================

FastAPI service for ML inference endpoints.
Provides image classification, similarity search, and anomaly detection.
"""

import io
import os
from typing import List, Optional
from pathlib import Path

import numpy as np
from fastapi import FastAPI, File, UploadFile, HTTPException, Query
from fastapi.responses import JSONResponse
from pydantic import BaseModel

try:
    import onnxruntime as ort
    ONNX_AVAILABLE = True
except ImportError:
    ONNX_AVAILABLE = False

try:
    from PIL import Image
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False

try:
    import faiss
    FAISS_AVAILABLE = True
except ImportError:
    FAISS_AVAILABLE = False


# Configuration
MODEL_DIR = Path(os.getenv("MODEL_DIR", "models"))
CLASSIFIER_PATH = MODEL_DIR / "classifier.onnx"
EMBEDDER_PATH = MODEL_DIR / "embeddings.onnx"
LABELS_PATH = MODEL_DIR / "classifier_labels.txt"
INDEX_PATH = MODEL_DIR / "parts.index"

# ImageNet normalization
IMAGENET_MEAN = np.array([0.485, 0.456, 0.406])
IMAGENET_STD = np.array([0.229, 0.224, 0.225])


# Response models
class ClassificationResult(BaseModel):
    label: str
    confidence: float


class ClassificationResponse(BaseModel):
    predictions: List[ClassificationResult]


class SimilarityResult(BaseModel):
    part_id: int
    similarity: float


class SimilarityResponse(BaseModel):
    results: List[SimilarityResult]


class HealthResponse(BaseModel):
    status: str
    models_loaded: dict


# Initialize FastAPI
app = FastAPI(
    title="PartsDB ML Service",
    description="Machine learning inference for parts database",
    version="1.0.0"
)


# Global model instances
classifier_session: Optional[ort.InferenceSession] = None
embedder_session: Optional[ort.InferenceSession] = None
labels: List[str] = []
faiss_index = None
index_part_ids: List[int] = []


def softmax(x: np.ndarray) -> np.ndarray:
    """Compute softmax probabilities."""
    exp_x = np.exp(x - np.max(x))
    return exp_x / exp_x.sum()


def preprocess_image(image: Image.Image, size: int = 224) -> np.ndarray:
    """
    Preprocess image for model input.

    Applies resizing, normalization with ImageNet stats.
    """
    # Resize
    image = image.convert('RGB')
    image = image.resize((size, size), Image.Resampling.LANCZOS)

    # Convert to numpy and normalize
    x = np.array(image).astype(np.float32) / 255.0
    x = (x - IMAGENET_MEAN) / IMAGENET_STD

    # Channels first: (H, W, C) -> (C, H, W)
    x = x.transpose(2, 0, 1)

    # Add batch dimension: (C, H, W) -> (1, C, H, W)
    x = x[np.newaxis, ...]

    return x.astype(np.float32)


@app.on_event("startup")
async def load_models():
    """Load models on startup."""
    global classifier_session, embedder_session, labels, faiss_index, index_part_ids

    if not ONNX_AVAILABLE:
        print("WARNING: onnxruntime not available")
        return

    # Load classifier
    if CLASSIFIER_PATH.exists():
        try:
            classifier_session = ort.InferenceSession(
                str(CLASSIFIER_PATH),
                providers=['CPUExecutionProvider']
            )
            print(f"Loaded classifier: {CLASSIFIER_PATH}")

            # Load labels
            if LABELS_PATH.exists():
                labels = LABELS_PATH.read_text().strip().split('\n')
                print(f"Loaded {len(labels)} labels")
        except Exception as e:
            print(f"Failed to load classifier: {e}")
    else:
        print(f"Classifier not found: {CLASSIFIER_PATH}")

    # Load embedder
    if EMBEDDER_PATH.exists():
        try:
            embedder_session = ort.InferenceSession(
                str(EMBEDDER_PATH),
                providers=['CPUExecutionProvider']
            )
            print(f"Loaded embedder: {EMBEDDER_PATH}")
        except Exception as e:
            print(f"Failed to load embedder: {e}")

    # Load FAISS index
    if FAISS_AVAILABLE and INDEX_PATH.exists():
        try:
            faiss_index = faiss.read_index(str(INDEX_PATH))
            # Load metadata
            import pickle
            with open(str(INDEX_PATH) + '.meta', 'rb') as f:
                meta = pickle.load(f)
                index_part_ids = meta.get('part_ids', [])
            print(f"Loaded FAISS index: {faiss_index.ntotal} vectors")
        except Exception as e:
            print(f"Failed to load FAISS index: {e}")


@app.get("/health", response_model=HealthResponse)
async def health_check():
    """Health check endpoint."""
    return HealthResponse(
        status="healthy",
        models_loaded={
            "classifier": classifier_session is not None,
            "embedder": embedder_session is not None,
            "similarity_index": faiss_index is not None
        }
    )


@app.post("/classify", response_model=ClassificationResponse)
async def classify_image(
    file: UploadFile = File(...),
    top_k: int = Query(default=5, ge=1, le=20)
):
    """
    Classify a part image.

    Returns top-k predictions with confidence scores.
    """
    if classifier_session is None:
        raise HTTPException(
            status_code=503,
            detail="Classifier model not loaded"
        )

    if not PIL_AVAILABLE:
        raise HTTPException(
            status_code=503,
            detail="PIL not available"
        )

    try:
        # Read and preprocess image
        image_data = await file.read()
        image = Image.open(io.BytesIO(image_data))
        x = preprocess_image(image)

        # Run inference
        input_name = classifier_session.get_inputs()[0].name
        output_name = classifier_session.get_outputs()[0].name

        outputs = classifier_session.run([output_name], {input_name: x})
        logits = outputs[0][0]

        # Get probabilities
        probs = softmax(logits)

        # Get top-k predictions
        top_indices = np.argsort(probs)[::-1][:top_k]

        predictions = []
        for idx in top_indices:
            label = labels[idx] if idx < len(labels) else f"class_{idx}"
            predictions.append(ClassificationResult(
                label=label,
                confidence=float(probs[idx])
            ))

        return ClassificationResponse(predictions=predictions)

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/embed")
async def get_embedding(file: UploadFile = File(...)):
    """
    Get embedding vector for an image.

    Returns 512-dimensional feature vector.
    """
    if embedder_session is None:
        raise HTTPException(
            status_code=503,
            detail="Embedder model not loaded"
        )

    try:
        image_data = await file.read()
        image = Image.open(io.BytesIO(image_data))
        x = preprocess_image(image)

        input_name = embedder_session.get_inputs()[0].name
        output_name = embedder_session.get_outputs()[0].name

        outputs = embedder_session.run([output_name], {input_name: x})
        embedding = outputs[0][0]

        # L2 normalize
        embedding = embedding / np.linalg.norm(embedding)

        return {"embedding": embedding.tolist()}

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/similar", response_model=SimilarityResponse)
async def find_similar(
    file: UploadFile = File(...),
    k: int = Query(default=5, ge=1, le=50)
):
    """
    Find similar parts by image.

    Returns k most similar parts from the index.
    """
    if embedder_session is None:
        raise HTTPException(
            status_code=503,
            detail="Embedder model not loaded"
        )

    if faiss_index is None:
        raise HTTPException(
            status_code=503,
            detail="Similarity index not loaded"
        )

    try:
        # Get query embedding
        image_data = await file.read()
        image = Image.open(io.BytesIO(image_data))
        x = preprocess_image(image)

        input_name = embedder_session.get_inputs()[0].name
        output_name = embedder_session.get_outputs()[0].name

        outputs = embedder_session.run([output_name], {input_name: x})
        embedding = outputs[0][0]
        embedding = embedding / np.linalg.norm(embedding)
        embedding = embedding.reshape(1, -1).astype('float32')

        # Search index
        similarities, indices = faiss_index.search(embedding, k)

        results = []
        for sim, idx in zip(similarities[0], indices[0]):
            if idx >= 0 and idx < len(index_part_ids):
                results.append(SimilarityResult(
                    part_id=index_part_ids[idx],
                    similarity=float(sim)
                ))

        return SimilarityResponse(results=results)

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/models")
async def list_models():
    """List available models and their status."""
    return {
        "classifier": {
            "loaded": classifier_session is not None,
            "path": str(CLASSIFIER_PATH),
            "labels_count": len(labels)
        },
        "embedder": {
            "loaded": embedder_session is not None,
            "path": str(EMBEDDER_PATH)
        },
        "similarity_index": {
            "loaded": faiss_index is not None,
            "path": str(INDEX_PATH),
            "vector_count": faiss_index.ntotal if faiss_index else 0
        }
    }


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
