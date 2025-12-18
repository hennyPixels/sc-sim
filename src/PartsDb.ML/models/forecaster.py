"""
Demand Forecasting
==================

Time series forecasting for part demand using scikit-learn and Keras.
Based on techniques from Géron Ch. 15 (RNNs) and Ch. 7 (Ensemble Methods).
"""

from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
import numpy as np
import pandas as pd
from datetime import datetime, timedelta

try:
    from sklearn.ensemble import GradientBoostingRegressor, RandomForestRegressor
    from sklearn.preprocessing import StandardScaler
    from sklearn.model_selection import TimeSeriesSplit
    from sklearn.metrics import mean_absolute_error, mean_squared_error
    SKLEARN_AVAILABLE = True
except ImportError:
    SKLEARN_AVAILABLE = False

try:
    import tensorflow as tf
    from tensorflow.keras.models import Sequential
    from tensorflow.keras.layers import LSTM, Dense, Dropout
    from tensorflow.keras.callbacks import EarlyStopping
    TF_AVAILABLE = True
except ImportError:
    TF_AVAILABLE = False


@dataclass
class ForecastResult:
    """Container for forecast results."""
    dates: List[datetime]
    predictions: List[float]
    lower_bound: Optional[List[float]] = None
    upper_bound: Optional[List[float]] = None
    model_type: str = ""
    metrics: Optional[Dict] = None


class DemandForecaster:
    """
    Demand forecasting using Gradient Boosting with feature engineering.

    Based on Géron's approach in Ch. 7 (Ensemble Methods) with
    time series specific feature engineering.

    Usage:
        forecaster = DemandForecaster()
        forecaster.fit(transactions_df)
        forecast = forecaster.predict(part_id=123, horizon=30)
    """

    def __init__(self, model_type: str = 'gbm'):
        if not SKLEARN_AVAILABLE:
            raise ImportError("scikit-learn not installed")

        self.model_type = model_type
        self.models: Dict[int, object] = {}  # part_id -> model
        self.scalers: Dict[int, StandardScaler] = {}
        self.feature_cols: List[str] = []

    def _create_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """
        Feature engineering for time series data.

        Creates lag features, rolling statistics, and calendar features
        as recommended by Géron for time series problems.
        """
        df = df.copy()
        df['date'] = pd.to_datetime(df['date'])
        df = df.sort_values(['part_id', 'date'])

        # Calendar features
        df['day_of_week'] = df['date'].dt.dayofweek
        df['day_of_month'] = df['date'].dt.day
        df['month'] = df['date'].dt.month
        df['quarter'] = df['date'].dt.quarter
        df['week_of_year'] = df['date'].dt.isocalendar().week.astype(int)
        df['is_weekend'] = df['day_of_week'].isin([5, 6]).astype(int)
        df['is_month_start'] = df['date'].dt.is_month_start.astype(int)
        df['is_month_end'] = df['date'].dt.is_month_end.astype(int)

        # Lag features (critical for time series)
        for lag in [1, 2, 3, 7, 14, 21, 28]:
            df[f'demand_lag_{lag}'] = (
                df.groupby('part_id')['quantity']
                .shift(lag)
            )

        # Rolling statistics
        for window in [7, 14, 28]:
            df[f'demand_rolling_mean_{window}'] = (
                df.groupby('part_id')['quantity']
                .transform(lambda x: x.shift(1).rolling(window, min_periods=1).mean())
            )
            df[f'demand_rolling_std_{window}'] = (
                df.groupby('part_id')['quantity']
                .transform(lambda x: x.shift(1).rolling(window, min_periods=1).std())
            )
            df[f'demand_rolling_max_{window}'] = (
                df.groupby('part_id')['quantity']
                .transform(lambda x: x.shift(1).rolling(window, min_periods=1).max())
            )

        # Exponential moving average
        df['demand_ema_7'] = (
            df.groupby('part_id')['quantity']
            .transform(lambda x: x.shift(1).ewm(span=7).mean())
        )

        # Trend features
        df['demand_diff_1'] = df.groupby('part_id')['quantity'].diff(1)
        df['demand_diff_7'] = df.groupby('part_id')['quantity'].diff(7)

        return df

    def fit(
        self,
        transactions_df: pd.DataFrame,
        target_col: str = 'quantity',
        cv_splits: int = 5
    ) -> Dict:
        """
        Train models for each part with time series cross-validation.

        Args:
            transactions_df: DataFrame with columns [part_id, date, quantity]
            target_col: Target column name
            cv_splits: Number of time series CV splits

        Returns:
            Training metrics
        """
        df = self._create_features(transactions_df)
        df = df.dropna()

        self.feature_cols = [
            c for c in df.columns
            if c not in ['date', 'part_id', target_col, 'id', 'transaction_type']
        ]

        all_metrics = {}

        for part_id in df['part_id'].unique():
            part_df = df[df['part_id'] == part_id].copy()

            if len(part_df) < 30:  # Skip parts with insufficient data
                continue

            X = part_df[self.feature_cols].values
            y = part_df[target_col].values

            # Scale features
            scaler = StandardScaler()
            X_scaled = scaler.fit_transform(X)

            # Time series cross-validation (Géron's recommendation)
            tscv = TimeSeriesSplit(n_splits=cv_splits)
            cv_scores = []

            for train_idx, val_idx in tscv.split(X_scaled):
                X_train, X_val = X_scaled[train_idx], X_scaled[val_idx]
                y_train, y_val = y[train_idx], y[val_idx]

                model = self._create_model()
                model.fit(X_train, y_train)

                y_pred = model.predict(X_val)
                mae = mean_absolute_error(y_val, y_pred)
                cv_scores.append(mae)

            # Final fit on all data
            model = self._create_model()
            model.fit(X_scaled, y)

            self.models[part_id] = model
            self.scalers[part_id] = scaler

            all_metrics[part_id] = {
                'cv_mae_mean': np.mean(cv_scores),
                'cv_mae_std': np.std(cv_scores),
                'n_samples': len(part_df)
            }

        return all_metrics

    def _create_model(self):
        """Create the forecasting model."""
        if self.model_type == 'gbm':
            return GradientBoostingRegressor(
                n_estimators=100,
                max_depth=5,
                learning_rate=0.1,
                subsample=0.8,
                random_state=42
            )
        elif self.model_type == 'rf':
            return RandomForestRegressor(
                n_estimators=100,
                max_depth=10,
                random_state=42,
                n_jobs=-1
            )
        else:
            raise ValueError(f"Unknown model_type: {self.model_type}")

    def predict(
        self,
        part_id: int,
        historical_df: pd.DataFrame,
        horizon: int = 30
    ) -> ForecastResult:
        """
        Generate demand forecast for a specific part.

        Args:
            part_id: Part to forecast
            historical_df: Recent transaction history
            horizon: Days to forecast

        Returns:
            ForecastResult with predictions
        """
        if part_id not in self.models:
            raise ValueError(f"No model trained for part_id {part_id}")

        model = self.models[part_id]
        scaler = self.scalers[part_id]

        # Prepare historical data
        df = self._create_features(historical_df)
        df = df[df['part_id'] == part_id].dropna()

        if len(df) == 0:
            raise ValueError(f"No valid data for part_id {part_id}")

        predictions = []
        dates = []

        last_date = df['date'].max()
        last_row = df.iloc[-1:].copy()

        for i in range(horizon):
            next_date = last_date + timedelta(days=i + 1)
            dates.append(next_date)

            # Update features for next day
            next_row = last_row.copy()
            next_row['date'] = next_date
            next_row['day_of_week'] = next_date.weekday()
            next_row['day_of_month'] = next_date.day
            next_row['month'] = next_date.month
            next_row['is_weekend'] = int(next_date.weekday() in [5, 6])

            # Prepare features
            X = next_row[self.feature_cols].values
            X_scaled = scaler.transform(X)

            pred = model.predict(X_scaled)[0]
            pred = max(0, pred)  # Demand can't be negative
            predictions.append(pred)

            # Update lag features for next iteration
            last_row = next_row.copy()
            last_row['demand_lag_1'] = pred

        return ForecastResult(
            dates=dates,
            predictions=predictions,
            model_type=self.model_type
        )

    def get_feature_importance(self, part_id: int) -> pd.DataFrame:
        """Get feature importance for a part's model."""
        if part_id not in self.models:
            raise ValueError(f"No model for part_id {part_id}")

        model = self.models[part_id]

        importance = pd.DataFrame({
            'feature': self.feature_cols,
            'importance': model.feature_importances_
        })

        return importance.sort_values('importance', ascending=False)


class LSTMForecaster:
    """
    LSTM-based demand forecasting for complex sequential patterns.
    Based on Géron Ch. 15 (RNNs and LSTMs).
    """

    def __init__(self, sequence_length: int = 30, n_features: int = 1):
        if not TF_AVAILABLE:
            raise ImportError("TensorFlow not installed")

        self.sequence_length = sequence_length
        self.n_features = n_features
        self.model = None
        self.scaler = StandardScaler()

    def _build_model(self) -> Sequential:
        """Build LSTM model architecture."""
        model = Sequential([
            LSTM(64, return_sequences=True,
                 input_shape=(self.sequence_length, self.n_features)),
            Dropout(0.2),
            LSTM(32, return_sequences=False),
            Dropout(0.2),
            Dense(16, activation='relu'),
            Dense(1)
        ])

        model.compile(
            optimizer='adam',
            loss='mse',
            metrics=['mae']
        )

        return model

    def _create_sequences(
        self,
        data: np.ndarray
    ) -> Tuple[np.ndarray, np.ndarray]:
        """Create sequences for LSTM training."""
        X, y = [], []

        for i in range(len(data) - self.sequence_length):
            X.append(data[i:i + self.sequence_length])
            y.append(data[i + self.sequence_length])

        return np.array(X), np.array(y)

    def fit(
        self,
        demand_series: np.ndarray,
        epochs: int = 50,
        batch_size: int = 32,
        validation_split: float = 0.1
    ) -> Dict:
        """
        Train LSTM on demand time series.

        Args:
            demand_series: 1D array of demand values
            epochs: Training epochs
            batch_size: Batch size
            validation_split: Validation fraction

        Returns:
            Training history
        """
        # Scale data
        demand_scaled = self.scaler.fit_transform(
            demand_series.reshape(-1, 1)
        )

        # Create sequences
        X, y = self._create_sequences(demand_scaled)
        X = X.reshape(-1, self.sequence_length, self.n_features)

        # Build and train
        self.model = self._build_model()

        early_stop = EarlyStopping(
            monitor='val_loss',
            patience=5,
            restore_best_weights=True
        )

        history = self.model.fit(
            X, y,
            epochs=epochs,
            batch_size=batch_size,
            validation_split=validation_split,
            callbacks=[early_stop],
            verbose=1
        )

        return {
            'train_loss': history.history['loss'][-1],
            'val_loss': history.history['val_loss'][-1],
            'epochs_trained': len(history.history['loss'])
        }

    def predict(
        self,
        recent_demand: np.ndarray,
        horizon: int = 30
    ) -> np.ndarray:
        """
        Forecast demand for next N days.

        Args:
            recent_demand: Last sequence_length days of demand
            horizon: Days to forecast

        Returns:
            Array of predictions
        """
        if self.model is None:
            raise ValueError("Model not trained")

        # Ensure we have enough history
        if len(recent_demand) < self.sequence_length:
            raise ValueError(
                f"Need at least {self.sequence_length} days of history"
            )

        # Use last sequence_length values
        current_sequence = self.scaler.transform(
            recent_demand[-self.sequence_length:].reshape(-1, 1)
        )

        predictions = []

        for _ in range(horizon):
            # Reshape for LSTM: (1, sequence_length, n_features)
            X = current_sequence.reshape(1, self.sequence_length, self.n_features)

            # Predict next value
            pred_scaled = self.model.predict(X, verbose=0)[0, 0]
            pred = self.scaler.inverse_transform([[pred_scaled]])[0, 0]
            pred = max(0, pred)  # Non-negative

            predictions.append(pred)

            # Update sequence
            current_sequence = np.roll(current_sequence, -1, axis=0)
            current_sequence[-1] = pred_scaled

        return np.array(predictions)

    def save(self, path: str):
        """Save model and scaler."""
        import pickle

        self.model.save(path + '_model.keras')

        with open(path + '_scaler.pkl', 'wb') as f:
            pickle.dump(self.scaler, f)

    @classmethod
    def load(cls, path: str) -> 'LSTMForecaster':
        """Load saved model."""
        import pickle

        instance = cls()
        instance.model = tf.keras.models.load_model(path + '_model.keras')

        with open(path + '_scaler.pkl', 'rb') as f:
            instance.scaler = pickle.load(f)

        return instance
