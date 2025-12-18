"""
Part Image Classifier
====================

Transfer learning with fastai for part category classification.
Based on fast.ai Practical Deep Learning course methodology.
"""

from pathlib import Path
from typing import Optional, Tuple, List, Dict
import numpy as np

try:
    from fastai.vision.all import (
        DataBlock, ImageBlock, CategoryBlock,
        get_image_files, RandomSplitter, parent_label,
        Resize, aug_transforms, vision_learner,
        load_learner, PILImage,
        resnet34, resnet50, error_rate, accuracy
    )
    import torch
    FASTAI_AVAILABLE = True
except ImportError:
    FASTAI_AVAILABLE = False


class PartClassifier:
    """
    CNN-based part classifier using transfer learning.

    Usage:
        # Training
        classifier = PartClassifier()
        classifier.train(Path('data/images'), epochs=4)
        classifier.export('models/classifier.pkl')

        # Inference
        classifier = PartClassifier.load('models/classifier.pkl')
        result = classifier.predict('part_image.jpg')
    """

    def __init__(self, model_arch=None):
        if not FASTAI_AVAILABLE:
            raise ImportError("fastai not installed. Run: pip install fastai")

        self.model_arch = model_arch or resnet34
        self.learn = None
        self.labels: List[str] = []

    def prepare_data(
        self,
        data_path: Path,
        valid_pct: float = 0.2,
        img_size: int = 224,
        batch_size: int = 32
    ):
        """
        Prepare DataLoaders from image folder.

        Expected structure:
            data_path/
                category1/
                    img1.jpg
                    img2.jpg
                category2/
                    ...
        """
        parts = DataBlock(
            blocks=(ImageBlock, CategoryBlock),
            get_items=get_image_files,
            splitter=RandomSplitter(valid_pct=valid_pct, seed=42),
            get_y=parent_label,
            item_tfms=Resize(img_size),
            batch_tfms=aug_transforms(mult=2.0)
        )

        self.dls = parts.dataloaders(data_path, bs=batch_size)
        self.labels = list(self.dls.vocab)

        return self.dls

    def train(
        self,
        data_path: Path,
        epochs: int = 4,
        freeze_epochs: int = 1,
        lr: Optional[float] = None,
        img_size: int = 224,
        batch_size: int = 32
    ) -> Dict:
        """
        Train classifier using fast.ai fine_tune method.

        Args:
            data_path: Path to image directory
            epochs: Total training epochs after unfreezing
            freeze_epochs: Epochs with frozen backbone
            lr: Learning rate (auto-detected if None)
            img_size: Input image size
            batch_size: Batch size

        Returns:
            Training metrics dict
        """
        self.prepare_data(data_path, img_size=img_size, batch_size=batch_size)

        self.learn = vision_learner(
            self.dls,
            self.model_arch,
            metrics=[error_rate, accuracy]
        )

        # Find optimal learning rate if not provided
        if lr is None:
            lr_find_result = self.learn.lr_find(suggest_funcs=(valley, slide))
            lr = lr_find_result.valley
            print(f"Using learning rate: {lr:.2e}")

        # Fine-tune: freeze, train head, unfreeze, train all
        self.learn.fine_tune(epochs, base_lr=lr, freeze_epochs=freeze_epochs)

        # Get final metrics
        val_loss, error, acc = self.learn.validate()

        return {
            'val_loss': float(val_loss),
            'error_rate': float(error),
            'accuracy': float(acc),
            'labels': self.labels
        }

    def train_progressive(
        self,
        data_path: Path,
        sizes: List[int] = [128, 224, 384],
        epochs_per_size: int = 3
    ) -> Dict:
        """
        Progressive resizing training (fast.ai technique).
        Train on small images first, then progressively increase.
        """
        metrics = {}

        for i, size in enumerate(sizes):
            print(f"\n{'='*50}")
            print(f"Training at size {size}x{size}")
            print('='*50)

            # Adjust batch size inversely with image size
            batch_size = max(8, 64 // (size // 64))

            if i == 0:
                # First size: fresh training
                result = self.train(
                    data_path,
                    epochs=epochs_per_size,
                    img_size=size,
                    batch_size=batch_size
                )
            else:
                # Subsequent sizes: transfer from previous
                self.prepare_data(data_path, img_size=size, batch_size=batch_size)
                self.learn.dls = self.dls
                self.learn.fine_tune(epochs_per_size)

                val_loss, error, acc = self.learn.validate()
                result = {
                    'val_loss': float(val_loss),
                    'error_rate': float(error),
                    'accuracy': float(acc)
                }

            metrics[f'size_{size}'] = result

        return metrics

    def predict(self, image_path: str) -> Dict:
        """
        Classify a single image.

        Returns:
            {
                'label': predicted category,
                'confidence': probability,
                'all_probs': {label: prob, ...}
            }
        """
        if self.learn is None:
            raise ValueError("Model not trained or loaded")

        img = PILImage.create(image_path)
        pred_class, pred_idx, probs = self.learn.predict(img)

        return {
            'label': str(pred_class),
            'confidence': float(probs[pred_idx]),
            'all_probs': {
                label: float(prob)
                for label, prob in zip(self.labels, probs)
            }
        }

    def predict_batch(self, image_paths: List[str]) -> List[Dict]:
        """Classify multiple images efficiently."""
        return [self.predict(p) for p in image_paths]

    def get_top_losses(self, k: int = 9):
        """
        Get examples with highest loss for debugging.
        fast.ai technique for understanding model failures.
        """
        from fastai.vision.all import ClassificationInterpretation

        interp = ClassificationInterpretation.from_learner(self.learn)
        return interp.plot_top_losses(k, figsize=(15, 10))

    def confusion_matrix(self):
        """Plot confusion matrix."""
        from fastai.vision.all import ClassificationInterpretation

        interp = ClassificationInterpretation.from_learner(self.learn)
        return interp.plot_confusion_matrix(figsize=(10, 10))

    def export(self, path: str):
        """Export model for deployment."""
        if self.learn is None:
            raise ValueError("No model to export")
        self.learn.export(path)
        print(f"Model exported to {path}")

    @classmethod
    def load(cls, path: str) -> 'PartClassifier':
        """Load exported model."""
        instance = cls()
        instance.learn = load_learner(path)
        instance.labels = list(instance.learn.dls.vocab)
        return instance

    def export_onnx(self, path: str, img_size: int = 224):
        """Export to ONNX format for cross-platform inference."""
        import torch.onnx

        model = self.learn.model.eval()
        dummy_input = torch.randn(1, 3, img_size, img_size)

        torch.onnx.export(
            model,
            dummy_input,
            path,
            export_params=True,
            opset_version=14,
            input_names=['image'],
            output_names=['logits'],
            dynamic_axes={
                'image': {0: 'batch_size'},
                'logits': {0: 'batch_size'}
            }
        )

        # Save labels alongside
        labels_path = path.replace('.onnx', '_labels.txt')
        with open(labels_path, 'w') as f:
            f.write('\n'.join(self.labels))

        print(f"ONNX model exported to {path}")
        print(f"Labels saved to {labels_path}")


# Learning rate finder helpers
def valley(lrs, losses):
    """Find the valley in lr vs loss curve."""
    idx = np.argmin(losses)
    return lrs[idx]

def slide(lrs, losses):
    """Find steepest point in lr vs loss curve."""
    gradients = np.gradient(losses)
    idx = np.argmin(gradients)
    return lrs[idx] / 10
