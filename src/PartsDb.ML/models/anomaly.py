"""
Anomaly Detection
=================

Detect unusual patterns in inventory data using Isolation Forest and Autoencoders.
Based on Géron Ch. 9 (Unsupervised Learning) and Ch. 17 (Autoencoders).
"""

from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
import numpy as np
import pandas as pd

try:
    from sklearn.ensemble import IsolationForest
    from sklearn.neighbors import LocalOutlierFactor
    from sklearn.preprocessing import StandardScaler
    from sklearn.decomposition import PCA
    SKLEARN_AVAILABLE = True
except ImportError:
    SKLEARN_AVAILABLE = False

try:
    import tensorflow as tf
    from tensorflow.keras.models import Model, Sequential
    from tensorflow.keras.layers import Input, Dense, Dropout
    from tensorflow.keras.callbacks import EarlyStopping
    TF_AVAILABLE = True
except ImportError:
    TF_AVAILABLE = False


@dataclass
class AnomalyResult:
    """Container for anomaly detection results."""
    part_id: int
    anomaly_score: float
    is_anomaly: bool
    anomaly_type: str
    details: Dict


class InventoryAnomalyDetector:
    """
    Detect anomalies in inventory patterns using Isolation Forest.

    Based on Géron Ch. 9 - Isolation Forest is effective for high-dimensional
    data and doesn't require labeled anomaly examples.

    Usage:
        detector = InventoryAnomalyDetector(contamination=0.05)
        detector.fit(inventory_df)
        anomalies = detector.detect(inventory_df)
    """

    def __init__(self, contamination: float = 0.05):
        if not SKLEARN_AVAILABLE:
            raise ImportError("scikit-learn not installed")

        self.contamination = contamination
        self.model = IsolationForest(
            n_estimators=100,
            contamination=contamination,
            random_state=42,
            n_jobs=-1
        )
        self.scaler = StandardScaler()
        self.feature_cols: List[str] = []
        self.is_fitted = False

    def _create_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """
        Create anomaly detection features from inventory data.

        Features capture various aspects of inventory behavior that
        might indicate anomalies.
        """
        df = df.copy()

        features = pd.DataFrame(index=df.index)

        # Basic inventory metrics
        features['quantity'] = df['quantity']
        features['quantity_zscore'] = (
            (df['quantity'] - df['quantity'].mean()) / df['quantity'].std()
        )

        # Stock level relative to minimum
        if 'min_quantity' in df.columns:
            features['stock_ratio'] = df['quantity'] / (df['min_quantity'] + 1)
            features['below_minimum'] = (df['quantity'] < df['min_quantity']).astype(int)

        # Price anomalies
        if 'unit_price' in df.columns:
            features['unit_price'] = df['unit_price']
            features['price_zscore'] = (
                (df['unit_price'] - df['unit_price'].mean()) /
                (df['unit_price'].std() + 1e-6)
            )

        # Transaction patterns (if available)
        if 'transaction_count' in df.columns:
            features['transaction_count'] = df['transaction_count']

        if 'days_since_last_movement' in df.columns:
            features['days_since_movement'] = df['days_since_last_movement']
            features['stale_inventory'] = (
                df['days_since_last_movement'] > 90
            ).astype(int)

        # Turnover metrics
        if 'turnover_rate' in df.columns:
            features['turnover_rate'] = df['turnover_rate']
            features['low_turnover'] = (df['turnover_rate'] < 0.1).astype(int)

        # Value metrics
        if 'unit_price' in df.columns:
            features['total_value'] = df['quantity'] * df['unit_price']
            features['value_zscore'] = (
                (features['total_value'] - features['total_value'].mean()) /
                (features['total_value'].std() + 1e-6)
            )

        return features.fillna(0)

    def fit(self, inventory_df: pd.DataFrame) -> 'InventoryAnomalyDetector':
        """
        Fit anomaly detector on inventory data.

        Args:
            inventory_df: DataFrame with inventory metrics

        Returns:
            self
        """
        features = self._create_features(inventory_df)
        self.feature_cols = list(features.columns)

        X = features.values
        X_scaled = self.scaler.fit_transform(X)

        self.model.fit(X_scaled)
        self.is_fitted = True

        return self

    def detect(self, inventory_df: pd.DataFrame) -> List[AnomalyResult]:
        """
        Detect anomalies in inventory data.

        Returns:
            List of AnomalyResult for anomalous items
        """
        if not self.is_fitted:
            raise ValueError("Model not fitted. Call fit() first.")

        features = self._create_features(inventory_df)
        X = features[self.feature_cols].values
        X_scaled = self.scaler.transform(X)

        # Predict: -1 for anomaly, 1 for normal
        predictions = self.model.predict(X_scaled)

        # Get anomaly scores (lower = more anomalous)
        scores = self.model.decision_function(X_scaled)

        results = []

        for i, (pred, score) in enumerate(zip(predictions, scores)):
            if pred == -1:  # Anomaly
                row = inventory_df.iloc[i]
                feature_values = features.iloc[i]

                # Determine anomaly type based on feature analysis
                anomaly_type = self._classify_anomaly(feature_values)

                results.append(AnomalyResult(
                    part_id=row.get('id', row.get('part_id', i)),
                    anomaly_score=float(score),
                    is_anomaly=True,
                    anomaly_type=anomaly_type,
                    details={
                        col: float(feature_values[col])
                        for col in self.feature_cols
                    }
                ))

        # Sort by anomaly score (most anomalous first)
        results.sort(key=lambda x: x.anomaly_score)

        return results

    def _classify_anomaly(self, features: pd.Series) -> str:
        """Classify the type of anomaly based on feature values."""
        anomaly_types = []

        if features.get('quantity_zscore', 0) > 3:
            anomaly_types.append('excess_stock')
        elif features.get('quantity_zscore', 0) < -2:
            anomaly_types.append('critically_low')

        if features.get('below_minimum', 0) == 1:
            anomaly_types.append('below_reorder_point')

        if features.get('stale_inventory', 0) == 1:
            anomaly_types.append('stale_inventory')

        if features.get('price_zscore', 0) > 3:
            anomaly_types.append('price_anomaly')

        if features.get('low_turnover', 0) == 1:
            anomaly_types.append('slow_moving')

        return ', '.join(anomaly_types) if anomaly_types else 'general_anomaly'

    def get_anomaly_scores(self, inventory_df: pd.DataFrame) -> np.ndarray:
        """Get anomaly scores for all items (lower = more anomalous)."""
        features = self._create_features(inventory_df)
        X = features[self.feature_cols].values
        X_scaled = self.scaler.transform(X)

        return self.model.decision_function(X_scaled)


class AutoencoderAnomalyDetector:
    """
    Autoencoder-based anomaly detection for complex patterns.

    Based on Géron Ch. 17 - Autoencoders learn to reconstruct normal data.
    High reconstruction error indicates anomalies.

    Usage:
        detector = AutoencoderAnomalyDetector()
        detector.fit(normal_data)
        anomalies = detector.detect(new_data)
    """

    def __init__(
        self,
        encoding_dim: int = 8,
        threshold_percentile: float = 95
    ):
        if not TF_AVAILABLE:
            raise ImportError("TensorFlow not installed")

        self.encoding_dim = encoding_dim
        self.threshold_percentile = threshold_percentile
        self.autoencoder = None
        self.encoder = None
        self.threshold = None
        self.scaler = StandardScaler()
        self.input_dim = None

    def _build_autoencoder(self, input_dim: int) -> Tuple[Model, Model]:
        """
        Build autoencoder architecture.

        Uses a symmetric encoder-decoder structure with bottleneck.
        """
        # Encoder
        inputs = Input(shape=(input_dim,))
        x = Dense(32, activation='relu')(inputs)
        x = Dropout(0.2)(x)
        x = Dense(16, activation='relu')(x)
        encoded = Dense(self.encoding_dim, activation='relu', name='encoding')(x)

        # Decoder (mirror of encoder)
        x = Dense(16, activation='relu')(encoded)
        x = Dropout(0.2)(x)
        x = Dense(32, activation='relu')(x)
        decoded = Dense(input_dim, activation='linear')(x)

        autoencoder = Model(inputs, decoded, name='autoencoder')
        encoder = Model(inputs, encoded, name='encoder')

        autoencoder.compile(
            optimizer='adam',
            loss='mse',
            metrics=['mae']
        )

        return autoencoder, encoder

    def fit(
        self,
        X: np.ndarray,
        epochs: int = 50,
        batch_size: int = 32,
        validation_split: float = 0.1
    ) -> Dict:
        """
        Train autoencoder on normal data.

        Args:
            X: Normal data samples (anomalies should be rare or absent)
            epochs: Training epochs
            batch_size: Batch size
            validation_split: Validation fraction

        Returns:
            Training metrics
        """
        self.input_dim = X.shape[1]

        # Scale data
        X_scaled = self.scaler.fit_transform(X)

        # Build model
        self.autoencoder, self.encoder = self._build_autoencoder(self.input_dim)

        # Train
        early_stop = EarlyStopping(
            monitor='val_loss',
            patience=5,
            restore_best_weights=True
        )

        history = self.autoencoder.fit(
            X_scaled, X_scaled,  # Reconstruct input
            epochs=epochs,
            batch_size=batch_size,
            validation_split=validation_split,
            callbacks=[early_stop],
            verbose=1
        )

        # Set threshold based on training reconstruction errors
        reconstructions = self.autoencoder.predict(X_scaled)
        mse = np.mean(np.power(X_scaled - reconstructions, 2), axis=1)
        self.threshold = np.percentile(mse, self.threshold_percentile)

        return {
            'train_loss': history.history['loss'][-1],
            'val_loss': history.history['val_loss'][-1],
            'threshold': self.threshold,
            'epochs_trained': len(history.history['loss'])
        }

    def detect(self, X: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """
        Detect anomalies based on reconstruction error.

        Returns:
            Tuple of (is_anomaly array, reconstruction_errors array)
        """
        if self.autoencoder is None:
            raise ValueError("Model not trained. Call fit() first.")

        X_scaled = self.scaler.transform(X)
        reconstructions = self.autoencoder.predict(X_scaled)

        # Mean squared error per sample
        mse = np.mean(np.power(X_scaled - reconstructions, 2), axis=1)

        is_anomaly = mse > self.threshold

        return is_anomaly, mse

    def get_reconstruction_errors(self, X: np.ndarray) -> np.ndarray:
        """Get reconstruction error for each sample."""
        X_scaled = self.scaler.transform(X)
        reconstructions = self.autoencoder.predict(X_scaled)
        return np.mean(np.power(X_scaled - reconstructions, 2), axis=1)

    def get_encodings(self, X: np.ndarray) -> np.ndarray:
        """Get latent encodings (useful for visualization)."""
        X_scaled = self.scaler.transform(X)
        return self.encoder.predict(X_scaled)

    def visualize_latent_space(self, X: np.ndarray, labels: Optional[np.ndarray] = None):
        """
        Visualize samples in 2D latent space using PCA.

        Useful for understanding data structure and anomaly patterns.
        """
        import matplotlib.pyplot as plt

        encodings = self.get_encodings(X)

        # Reduce to 2D if needed
        if encodings.shape[1] > 2:
            pca = PCA(n_components=2)
            encodings_2d = pca.fit_transform(encodings)
        else:
            encodings_2d = encodings

        plt.figure(figsize=(10, 8))

        if labels is not None:
            scatter = plt.scatter(
                encodings_2d[:, 0],
                encodings_2d[:, 1],
                c=labels,
                cmap='RdYlGn_r',
                alpha=0.6
            )
            plt.colorbar(scatter, label='Anomaly Score')
        else:
            plt.scatter(encodings_2d[:, 0], encodings_2d[:, 1], alpha=0.6)

        plt.xlabel('Latent Dimension 1')
        plt.ylabel('Latent Dimension 2')
        plt.title('Inventory Latent Space')
        plt.tight_layout()

        return plt.gcf()

    def save(self, path: str):
        """Save model, scaler, and threshold."""
        import pickle

        self.autoencoder.save(path + '_autoencoder.keras')
        self.encoder.save(path + '_encoder.keras')

        with open(path + '_config.pkl', 'wb') as f:
            pickle.dump({
                'scaler': self.scaler,
                'threshold': self.threshold,
                'input_dim': self.input_dim,
                'encoding_dim': self.encoding_dim
            }, f)

    @classmethod
    def load(cls, path: str) -> 'AutoencoderAnomalyDetector':
        """Load saved model."""
        import pickle

        with open(path + '_config.pkl', 'rb') as f:
            config = pickle.load(f)

        instance = cls(encoding_dim=config['encoding_dim'])
        instance.autoencoder = tf.keras.models.load_model(path + '_autoencoder.keras')
        instance.encoder = tf.keras.models.load_model(path + '_encoder.keras')
        instance.scaler = config['scaler']
        instance.threshold = config['threshold']
        instance.input_dim = config['input_dim']

        return instance
