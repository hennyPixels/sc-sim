# Machine Learning Architecture Extension

## Overview

This document outlines the ML integration strategy for the Parts Database application, incorporating methodologies from:

- **fast.ai's Practical Deep Learning for Coders** - Top-down, practical approach
- **Hands-On Machine Learning with Scikit-Learn, Keras & TensorFlow** (Géron) - Comprehensive ML/DL foundations

---

## ML Use Cases for Parts Database

| Use Case | Technique | Framework | Priority |
|----------|-----------|-----------|----------|
| Part image classification | CNN, Transfer Learning | fastai/PyTorch | High |
| Visual part search | Embeddings, Similarity | fastai/FAISS | High |
| Demand forecasting | Time Series, ARIMA, LSTM | scikit-learn/Keras | Medium |
| Anomaly detection | Isolation Forest, Autoencoders | scikit-learn/Keras | Medium |
| Part recommendations | Collaborative Filtering | Surprise/TensorFlow | Low |
| OCR for part numbers | CNN + CTC | Tesseract/EasyOCR | Medium |
| Quality inspection | Object Detection | YOLOv8/fastai | Low |

---

## Project Structure Extension

```
sc-sim/
├── src/
│   ├── PartsDb.Shared/
│   ├── PartsDb.Maui/
│   ├── partsdb-cli/
│   │
│   └── PartsDb.ML/                    # New ML module
│       ├── training/
│       │   ├── notebooks/             # Jupyter training notebooks
│       │   │   ├── 01_image_classifier.ipynb
│       │   │   ├── 02_similarity_search.ipynb
│       │   │   ├── 03_demand_forecast.ipynb
│       │   │   └── 04_anomaly_detection.ipynb
│       │   ├── scripts/               # Training scripts
│       │   └── configs/               # Hyperparameter configs
│       │
│       ├── models/                    # Model definitions
│       │   ├── classifier.py          # Part classification
│       │   ├── embeddings.py          # Visual similarity
│       │   ├── forecaster.py          # Demand prediction
│       │   └── anomaly.py             # Anomaly detection
│       │
│       ├── inference/                 # Production inference
│       │   ├── onnx_runtime.py        # ONNX inference
│       │   ├── tflite_runtime.py      # TFLite for mobile
│       │   └── api.py                 # FastAPI endpoints
│       │
│       ├── data/                      # Data processing
│       │   ├── datasets.py            # PyTorch datasets
│       │   ├── transforms.py          # Augmentations
│       │   └── loaders.py             # DataLoaders
│       │
│       └── exports/                   # Exported models
│           ├── classifier.onnx
│           ├── classifier.tflite
│           └── embeddings.onnx
│
├── ml-service/                        # ML microservice
│   ├── Dockerfile
│   ├── requirements.txt
│   └── main.py
│
└── data/
    ├── images/                        # Training images
    │   ├── resistors/
    │   ├── capacitors/
    │   └── ...
    ├── labels/                        # Annotations
    └── splits/                        # Train/val/test splits
```

---

## 1. Part Image Classification

### Approach (fast.ai style - top-down)

Start with a pre-trained model and fine-tune on part images. This follows fast.ai's philosophy of getting results quickly, then iterating.

```python
# src/PartsDb.ML/training/notebooks/01_image_classifier.ipynb

from fastai.vision.all import *

# fast.ai DataBlock API - flexible data loading
parts = DataBlock(
    blocks=(ImageBlock, CategoryBlock),
    get_items=get_image_files,
    splitter=RandomSplitter(valid_pct=0.2, seed=42),
    get_y=parent_label,
    item_tfms=Resize(224),
    batch_tfms=aug_transforms(mult=2)  # Data augmentation
)

dls = parts.dataloaders(path/'images', bs=32)

# Transfer learning with ResNet (Géron Ch. 14)
# Start with smaller model, scale up if needed
learn = vision_learner(dls, resnet34, metrics=error_rate)

# fast.ai learning rate finder
learn.lr_find()

# Fine-tune: freeze base, train head, then unfreeze
learn.fine_tune(4, freeze_epochs=1)

# Interpretation (fast.ai's top losses)
interp = ClassificationInterpretation.from_learner(learn)
interp.plot_confusion_matrix()
interp.plot_top_losses(9)

# Export for production
learn.export('part_classifier.pkl')
```

### Progressive Resizing (fast.ai technique)

Train on small images first, then progressively increase size:

```python
# Start small - faster iteration
dls = parts.dataloaders(path, bs=64, item_tfms=Resize(128))
learn.fine_tune(3)

# Increase size - transfer learning from yourself
dls = parts.dataloaders(path, bs=32, item_tfms=Resize(224))
learn.dls = dls
learn.fine_tune(3)

# Final size for production accuracy
dls = parts.dataloaders(path, bs=16, item_tfms=Resize(384))
learn.dls = dls
learn.fine_tune(2)
```

### Model Selection Guide (Géron Ch. 14)

| Model | Params | Accuracy | Speed | Use Case |
|-------|--------|----------|-------|----------|
| ResNet-18 | 11M | Good | Fast | Embedded/Mobile |
| ResNet-34 | 21M | Better | Medium | General use |
| EfficientNet-B0 | 5M | Good | Fast | Mobile |
| ConvNeXt-Tiny | 28M | Best | Slower | Server |

---

## 2. Visual Similarity Search

### Embedding Extraction (Géron Ch. 17 + fast.ai)

Use CNN as feature extractor for visual search:

```python
# src/PartsDb.ML/models/embeddings.py

from fastai.vision.all import *
import faiss
import numpy as np

class PartEmbedder:
    """Extract visual embeddings for similarity search"""

    def __init__(self, model_path: str):
        self.learn = load_learner(model_path)
        # Remove classification head, keep features
        self.encoder = nn.Sequential(*list(self.learn.model.children())[:-1])
        self.encoder.eval()

    def get_embedding(self, img_path: str) -> np.ndarray:
        """Extract 512-d embedding from image"""
        img = PILImage.create(img_path)
        x = self.learn.dls.test_dl([img]).one_batch()[0]

        with torch.no_grad():
            emb = self.encoder(x).squeeze()
        return emb.cpu().numpy()

    def build_index(self, image_paths: list, part_ids: list):
        """Build FAISS index for fast similarity search"""
        embeddings = []
        for path in image_paths:
            emb = self.get_embedding(path)
            embeddings.append(emb)

        embeddings = np.array(embeddings).astype('float32')

        # L2 normalized for cosine similarity
        faiss.normalize_L2(embeddings)

        # IVF index for large datasets (Géron recommends)
        d = embeddings.shape[1]  # 512
        nlist = min(100, len(embeddings) // 10)

        quantizer = faiss.IndexFlatIP(d)
        self.index = faiss.IndexIVFFlat(quantizer, d, nlist)
        self.index.train(embeddings)
        self.index.add(embeddings)

        self.part_ids = part_ids

    def search_similar(self, img_path: str, k: int = 5) -> list:
        """Find k most similar parts"""
        query = self.get_embedding(img_path).reshape(1, -1).astype('float32')
        faiss.normalize_L2(query)

        distances, indices = self.index.search(query, k)

        return [
            {'part_id': self.part_ids[i], 'similarity': float(d)}
            for i, d in zip(indices[0], distances[0])
        ]
```

### Integration with Database

```sql
-- Add embedding column to parts table
ALTER TABLE part_images ADD COLUMN embedding BLOB;

-- Or separate embeddings table for flexibility
CREATE TABLE part_embeddings (
    id INTEGER PRIMARY KEY,
    part_id INTEGER NOT NULL,
    embedding BLOB NOT NULL,  -- 512 floats = 2048 bytes
    model_version TEXT,
    created_at TEXT DEFAULT (datetime('now')),
    FOREIGN KEY (part_id) REFERENCES parts(id)
);
```

---

## 3. Demand Forecasting

### Time Series with scikit-learn (Géron Ch. 15)

```python
# src/PartsDb.ML/models/forecaster.py

import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestRegressor, GradientBoostingRegressor
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import TimeSeriesSplit

class DemandForecaster:
    """Predict future part demand using transaction history"""

    def __init__(self):
        self.model = GradientBoostingRegressor(
            n_estimators=100,
            max_depth=5,
            learning_rate=0.1,
            random_state=42
        )
        self.scaler = StandardScaler()

    def create_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Feature engineering for time series (Géron Ch. 15)"""
        df = df.copy()
        df['date'] = pd.to_datetime(df['date'])

        # Time-based features
        df['day_of_week'] = df['date'].dt.dayofweek
        df['month'] = df['date'].dt.month
        df['quarter'] = df['date'].dt.quarter
        df['is_weekend'] = df['day_of_week'].isin([5, 6]).astype(int)

        # Lag features (critical for time series)
        for lag in [1, 7, 14, 30]:
            df[f'demand_lag_{lag}'] = df.groupby('part_id')['quantity'].shift(lag)

        # Rolling statistics
        for window in [7, 14, 30]:
            df[f'demand_rolling_mean_{window}'] = (
                df.groupby('part_id')['quantity']
                .transform(lambda x: x.rolling(window, min_periods=1).mean())
            )
            df[f'demand_rolling_std_{window}'] = (
                df.groupby('part_id')['quantity']
                .transform(lambda x: x.rolling(window, min_periods=1).std())
            )

        return df.dropna()

    def train(self, transactions_df: pd.DataFrame):
        """Train model with time series cross-validation"""
        df = self.create_features(transactions_df)

        feature_cols = [c for c in df.columns if c not in
                       ['date', 'part_id', 'quantity', 'id']]

        X = df[feature_cols]
        y = df['quantity']

        X_scaled = self.scaler.fit_transform(X)

        # Time series CV (Géron's recommendation)
        tscv = TimeSeriesSplit(n_splits=5)
        scores = []

        for train_idx, val_idx in tscv.split(X_scaled):
            X_train, X_val = X_scaled[train_idx], X_scaled[val_idx]
            y_train, y_val = y.iloc[train_idx], y.iloc[val_idx]

            self.model.fit(X_train, y_train)
            score = self.model.score(X_val, y_val)
            scores.append(score)

        print(f"CV R² scores: {scores}")
        print(f"Mean R²: {np.mean(scores):.3f} (+/- {np.std(scores):.3f})")

        # Final fit on all data
        self.model.fit(X_scaled, y)
        self.feature_cols = feature_cols

    def predict(self, part_id: int, horizon_days: int = 30) -> pd.DataFrame:
        """Forecast demand for next N days"""
        # Implementation depends on feature availability
        pass
```

### LSTM Alternative (Géron Ch. 15)

```python
# For longer sequences and complex patterns

import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense, Dropout

def build_lstm_model(sequence_length: int, n_features: int):
    """LSTM for sequence-to-sequence forecasting"""
    model = Sequential([
        LSTM(64, return_sequences=True,
             input_shape=(sequence_length, n_features)),
        Dropout(0.2),
        LSTM(32, return_sequences=False),
        Dropout(0.2),
        Dense(16, activation='relu'),
        Dense(1)  # Single output: next day demand
    ])

    model.compile(
        optimizer='adam',
        loss='mse',
        metrics=['mae']
    )
    return model
```

---

## 4. Anomaly Detection

### Isolation Forest (Géron Ch. 9)

```python
# src/PartsDb.ML/models/anomaly.py

from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
import numpy as np

class InventoryAnomalyDetector:
    """Detect unusual inventory patterns"""

    def __init__(self, contamination: float = 0.05):
        self.model = IsolationForest(
            n_estimators=100,
            contamination=contamination,
            random_state=42,
            n_jobs=-1
        )
        self.scaler = StandardScaler()

    def create_features(self, parts_df: pd.DataFrame) -> np.ndarray:
        """Create anomaly detection features"""
        features = parts_df[[
            'quantity',
            'quantity_change_rate',      # Daily change
            'days_since_last_order',
            'order_frequency',
            'avg_order_size',
            'price_per_unit',
            'stock_turnover_ratio'
        ]].values

        return self.scaler.fit_transform(features)

    def fit_predict(self, parts_df: pd.DataFrame) -> np.ndarray:
        """Returns -1 for anomalies, 1 for normal"""
        X = self.create_features(parts_df)
        return self.model.fit_predict(X)

    def get_anomaly_scores(self, parts_df: pd.DataFrame) -> np.ndarray:
        """Lower score = more anomalous"""
        X = self.create_features(parts_df)
        return self.model.decision_function(X)
```

### Autoencoder for Complex Anomalies (Géron Ch. 17)

```python
import tensorflow as tf
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Input, Dense

def build_autoencoder(input_dim: int, encoding_dim: int = 8):
    """
    Autoencoder for anomaly detection
    High reconstruction error = anomaly
    """
    # Encoder
    inputs = Input(shape=(input_dim,))
    encoded = Dense(32, activation='relu')(inputs)
    encoded = Dense(16, activation='relu')(encoded)
    encoded = Dense(encoding_dim, activation='relu')(encoded)

    # Decoder
    decoded = Dense(16, activation='relu')(encoded)
    decoded = Dense(32, activation='relu')(decoded)
    decoded = Dense(input_dim, activation='linear')(decoded)

    autoencoder = Model(inputs, decoded)
    autoencoder.compile(optimizer='adam', loss='mse')

    return autoencoder

class AutoencoderAnomalyDetector:
    def __init__(self, threshold_percentile: float = 95):
        self.autoencoder = None
        self.threshold = None
        self.threshold_percentile = threshold_percentile

    def fit(self, X: np.ndarray, epochs: int = 50):
        self.autoencoder = build_autoencoder(X.shape[1])
        self.autoencoder.fit(
            X, X,
            epochs=epochs,
            batch_size=32,
            validation_split=0.1,
            verbose=0
        )

        # Set threshold from training data
        reconstructions = self.autoencoder.predict(X)
        mse = np.mean(np.power(X - reconstructions, 2), axis=1)
        self.threshold = np.percentile(mse, self.threshold_percentile)

    def predict(self, X: np.ndarray) -> np.ndarray:
        """Returns True for anomalies"""
        reconstructions = self.autoencoder.predict(X)
        mse = np.mean(np.power(X - reconstructions, 2), axis=1)
        return mse > self.threshold
```

---

## 5. Model Export & Deployment

### ONNX Export (Cross-platform inference)

```python
# src/PartsDb.ML/inference/export.py

import torch
import onnx
from fastai.vision.all import load_learner

def export_to_onnx(fastai_model_path: str, output_path: str):
    """Export fastai model to ONNX for .NET inference"""
    learn = load_learner(fastai_model_path)
    model = learn.model.eval()

    # Dummy input
    dummy_input = torch.randn(1, 3, 224, 224)

    torch.onnx.export(
        model,
        dummy_input,
        output_path,
        export_params=True,
        opset_version=14,
        input_names=['image'],
        output_names=['predictions'],
        dynamic_axes={
            'image': {0: 'batch_size'},
            'predictions': {0: 'batch_size'}
        }
    )

    # Validate
    onnx_model = onnx.load(output_path)
    onnx.checker.check_model(onnx_model)
    print(f"Exported to {output_path}")
```

### TensorFlow Lite for Mobile/Embedded

```python
import tensorflow as tf

def export_to_tflite(keras_model, output_path: str, quantize: bool = True):
    """Export Keras model to TFLite for mobile"""
    converter = tf.lite.TFLiteConverter.from_keras_model(keras_model)

    if quantize:
        # int8 quantization for smaller size & faster inference
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        converter.target_spec.supported_types = [tf.int8]

    tflite_model = converter.convert()

    with open(output_path, 'wb') as f:
        f.write(tflite_model)

    print(f"Exported to {output_path} ({len(tflite_model) / 1024:.1f} KB)")
```

### .NET Integration with ONNX Runtime

```csharp
// src/PartsDb.Shared/ML/OnnxClassifier.cs

using Microsoft.ML.OnnxRuntime;
using Microsoft.ML.OnnxRuntime.Tensors;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.Processing;

namespace PartsDb.Shared.ML;

public class PartClassifier : IDisposable
{
    private readonly InferenceSession _session;
    private readonly string[] _labels;

    public PartClassifier(string modelPath, string labelsPath)
    {
        _session = new InferenceSession(modelPath);
        _labels = File.ReadAllLines(labelsPath);
    }

    public async Task<ClassificationResult> ClassifyAsync(Stream imageStream)
    {
        // Preprocess image
        using var image = await Image.LoadAsync<Rgb24>(imageStream);
        image.Mutate(x => x.Resize(224, 224));

        // Convert to tensor [1, 3, 224, 224]
        var tensor = new DenseTensor<float>(new[] { 1, 3, 224, 224 });

        for (int y = 0; y < 224; y++)
        {
            for (int x = 0; x < 224; x++)
            {
                var pixel = image[x, y];
                // Normalize to ImageNet stats
                tensor[0, 0, y, x] = (pixel.R / 255f - 0.485f) / 0.229f;
                tensor[0, 1, y, x] = (pixel.G / 255f - 0.456f) / 0.224f;
                tensor[0, 2, y, x] = (pixel.B / 255f - 0.406f) / 0.225f;
            }
        }

        // Run inference
        var inputs = new List<NamedOnnxValue>
        {
            NamedOnnxValue.CreateFromTensor("image", tensor)
        };

        using var results = _session.Run(inputs);
        var output = results.First().AsTensor<float>();

        // Softmax and get top prediction
        var probabilities = Softmax(output.ToArray());
        var maxIdx = Array.IndexOf(probabilities, probabilities.Max());

        return new ClassificationResult
        {
            Label = _labels[maxIdx],
            Confidence = probabilities[maxIdx],
            AllProbabilities = _labels.Zip(probabilities)
                .ToDictionary(x => x.First, x => x.Second)
        };
    }

    private static float[] Softmax(float[] logits)
    {
        var maxLogit = logits.Max();
        var exp = logits.Select(x => Math.Exp(x - maxLogit)).ToArray();
        var sum = exp.Sum();
        return exp.Select(x => (float)(x / sum)).ToArray();
    }

    public void Dispose() => _session.Dispose();
}

public record ClassificationResult
{
    public string Label { get; init; } = "";
    public float Confidence { get; init; }
    public Dictionary<string, float> AllProbabilities { get; init; } = new();
}
```

---

## 6. ML Microservice Architecture

### FastAPI Service

```python
# ml-service/main.py

from fastapi import FastAPI, File, UploadFile, HTTPException
from fastapi.responses import JSONResponse
import onnxruntime as ort
import numpy as np
from PIL import Image
import io

app = FastAPI(title="PartsDB ML Service")

# Load models at startup
classifier_session = None
embedder_session = None
faiss_index = None

@app.on_event("startup")
async def load_models():
    global classifier_session, embedder_session
    classifier_session = ort.InferenceSession("models/classifier.onnx")
    embedder_session = ort.InferenceSession("models/embeddings.onnx")

@app.post("/classify")
async def classify_part(file: UploadFile = File(...)):
    """Classify a part image"""
    try:
        image_data = await file.read()
        img = Image.open(io.BytesIO(image_data)).convert('RGB')
        img = img.resize((224, 224))

        # Preprocess
        x = np.array(img).astype(np.float32) / 255.0
        x = (x - [0.485, 0.456, 0.406]) / [0.229, 0.224, 0.225]
        x = x.transpose(2, 0, 1)[np.newaxis, ...]

        # Inference
        outputs = classifier_session.run(None, {'image': x})
        probs = softmax(outputs[0][0])

        return {
            'predictions': [
                {'label': LABELS[i], 'confidence': float(p)}
                for i, p in sorted(enumerate(probs),
                                   key=lambda x: x[1], reverse=True)[:5]
            ]
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/similar")
async def find_similar(file: UploadFile = File(...), k: int = 5):
    """Find similar parts by image"""
    # Implementation using embeddings + FAISS
    pass

@app.post("/forecast/{part_id}")
async def forecast_demand(part_id: int, days: int = 30):
    """Forecast demand for a part"""
    pass

@app.get("/anomalies")
async def detect_anomalies():
    """Detect inventory anomalies"""
    pass
```

### Docker Deployment

```dockerfile
# ml-service/Dockerfile

FROM python:3.11-slim

WORKDIR /app

# Install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy models and code
COPY models/ models/
COPY main.py .

# Run with uvicorn
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
```

---

## 7. Training Pipeline (MLOps)

### Data Versioning with DVC

```yaml
# dvc.yaml

stages:
  prepare_data:
    cmd: python scripts/prepare_data.py
    deps:
      - data/raw/
    outs:
      - data/processed/
      - data/splits/

  train_classifier:
    cmd: python scripts/train_classifier.py
    deps:
      - data/splits/
      - src/PartsDb.ML/models/classifier.py
    params:
      - classifier.epochs
      - classifier.lr
    outs:
      - models/classifier.pkl
    metrics:
      - metrics/classifier.json:
          cache: false

  export_onnx:
    cmd: python scripts/export_onnx.py
    deps:
      - models/classifier.pkl
    outs:
      - exports/classifier.onnx
```

### Experiment Tracking

```python
# Using Weights & Biases (fast.ai integrates well)
import wandb
from fastai.callback.wandb import WandbCallback

wandb.init(project="partsdb-classifier")

learn = vision_learner(dls, resnet34, metrics=error_rate,
                       cbs=WandbCallback())
learn.fine_tune(4)

wandb.finish()
```

---

## 8. Learning Resources Mapping

### fast.ai Course Integration

| Lesson | Application |
|--------|-------------|
| Lesson 1: Image Classification | Part category classifier |
| Lesson 2: Deployment | ONNX export, Gradio demo |
| Lesson 3: Neural Net Foundations | Custom architectures |
| Lesson 4: NLP | Part description search |
| Lesson 5: Tabular | Demand forecasting |
| Lesson 6: Collaborative Filtering | Part recommendations |

### Géron Book Chapters

| Chapter | Application |
|---------|-------------|
| Ch. 2: End-to-End ML | Project structure |
| Ch. 5: SVM | Classification fallback |
| Ch. 7: Ensemble | Demand forecasting |
| Ch. 9: Unsupervised | Anomaly detection |
| Ch. 10-11: Neural Networks | Deep learning foundations |
| Ch. 14: CNNs | Image classification |
| Ch. 15: RNNs | Time series forecasting |
| Ch. 17: Autoencoders | Anomaly detection, embeddings |

---

## 9. Implementation Roadmap

### Phase 1: Image Classification (2-3 weeks)
1. Collect/label 100+ images per category
2. Train ResNet-34 with fast.ai
3. Export to ONNX
4. Integrate with .NET app

### Phase 2: Visual Search (2 weeks)
1. Extract embeddings from classifier
2. Build FAISS index
3. Add search API endpoint
4. Integrate with UI

### Phase 3: Demand Forecasting (2-3 weeks)
1. Analyze transaction history
2. Feature engineering
3. Train Gradient Boosting model
4. Add predictions to dashboard

### Phase 4: Anomaly Detection (1-2 weeks)
1. Define anomaly features
2. Train Isolation Forest
3. Add alerts to UI

---

## 10. Hardware Requirements

### Training
- **Minimum**: CPU with 16GB RAM (slow but works)
- **Recommended**: NVIDIA GPU with 8GB+ VRAM (RTX 3060+)
- **Cloud**: Google Colab Pro, AWS p3.2xlarge

### Inference
- **Server**: CPU sufficient for most models
- **Mobile**: TFLite with int8 quantization
- **Embedded**: Consider Edge TPU or ONNX Runtime Mobile

---

## Quick Start

```bash
# Create ML environment
cd src/PartsDb.ML
python -m venv .venv
source .venv/bin/activate  # or .venv\Scripts\activate on Windows

# Install dependencies
pip install fastai torch torchvision
pip install scikit-learn pandas numpy
pip install onnx onnxruntime
pip install faiss-cpu  # or faiss-gpu
pip install fastapi uvicorn

# Start training notebook
jupyter notebook training/notebooks/01_image_classifier.ipynb
```
