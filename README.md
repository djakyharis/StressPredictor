# StressPredictor

An Arduino library for **real-time stress level prediction** based on Heart Rate Variability (HRV) features. Uses a lightweight Random Forest classifier optimized for microcontrollers — no internet or server required.

## Features

-  Predicts stress level from **RMSSD**, **SDNN**, and **BPM**
-  Powered by a **Random Forest model** (10 trees, trained in Python)
-  Compatible with **any Arduino-based board**

## Stress Level Output

| Value | Label        | Description                         |
|-------|--------------|-------------------------------------|
| `0`   | Stres Rendah | Low stress — HRV is healthy         |
| `1`   | Stres Sedang | Moderate stress                     |
| `2`   | Stres Tinggi | High stress — HRV is suppressed     |

## Installation

1. Download this repository as a `.zip` file
2. In Arduino IDE: **Sketch → Include Library → Add .ZIP Library...**
3. Select the downloaded `.zip` file
4. Done — the library is ready to use

## Dependencies

Install these libraries via Arduino Library Manager:

- [`SparkFun MAX3010x Pulse and Proximity Sensor Library`](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library) — for MAX30102 sensor (optional, only for realtime examples)

## Usage

### Basic Prediction (Static Input)

```cpp
#include <StressPredictor.h>

StressPredictor stressModel;

void setup() {
  Serial.begin(115200);

  float rmssd = 45.61;
  float sdnn  = 32.18;
  float bpm   = 98.23;

  int result = stressModel.predict(rmssd, sdnn, bpm);

  if (result == 0)      Serial.println("Stres Rendah");
  else if (result == 1) Serial.println("Stres Sedang");
  else if (result == 2) Serial.println("Stres Tinggi");
}

void loop() {}
```

### Real-time with MAX30102 Sensor

See [`examples/MAX30102_RealtimePrediction/`](examples/MAX30102_RealtimePrediction/) for a complete example that:
- Reads raw IR signal from MAX30102
- Computes RR intervals between heartbeats
- Calculates SDNN and RMSSD from a 15-beat window
- Predicts stress level in real-time

## API Reference

### `StressPredictor()`
Constructor — no initialization required.

### `int predict(float rmssd, float sdnn, float bpm)`
Runs the Random Forest model and returns the predicted stress class.

| Parameter | Type    | Unit | Description                        |
|-----------|---------|------|------------------------------------|
| `rmssd`   | `float` | ms   | Root Mean Square of Successive Differences |
| `sdnn`    | `float` | ms   | Standard Deviation of NN Intervals |
| `bpm`     | `float` | bpm  | Heart Rate in Beats Per Minute     |

**Returns:** `0` (Low), `1` (Medium), or `2` (High)

## How It Works

```
MAX30102 Sensor
      │
      ▼
checkForBeat() → RR interval (ms)
      │
      ▼
Buffer 15 RR intervals
      │
      ├── computeSDNN()
      ├── computeRMSSD()
      └── BPM = 60000 / RR
      │
      ▼
stressModel.predict(rmssd, sdnn, bpm)
      │
      ▼
Z-Score Normalization (Auto-Scaled)
      │
      ▼
Random Forest (10 trees) → majority vote
      │
      ▼
0 / 1 / 2
```

The model was trained in Python using `scikit-learn` and exported to C++ using [`micromlgen`](https://github.com/eloquentarduino/micromlgen). The full decision tree logic lives in `src/model.h`.

### Custom Model Training & Evaluation

The library includes Python scripts for training, evaluation, and data distribution visualization inside the [`python/`](python/) directory:

- **[`train_model.py`](python/train_model.py)**: Melatih model Random Forest menggunakan data `DATA TRAINING.csv` (ditemukan secara dinamis di root project) dan secara otomatis meng-ekspor model baru ke [`src/model.h`](src/model.h).
- **[`evaluate_model.py`](python/evaluate_model.py)**: Mengevaluasi akurasi model dan menghasilkan plot Confusion Matrix serta Feature Importance di folder `python/`.
- **[`plot_distribution.py`](python/plot_distribution.py)**: Memvisualisasikan distribusi data sebelum dan sesudah normalisasi dalam bentuk plot `distribution_plot.png`.

*Catatan: File dataset `DATA TRAINING.csv` diletakkan di root project (di luar folder library). `.gitignore` telah dikonfigurasi untuk mengecualikan file `.csv`, visualisasi `.png`, dan virtual environment agar data tidak masuk ke dalam git repository library.*

## Model Performance

| Metric              | Value    |
|---------------------|----------|
| Accuracy (test set) | 98.97%   |
| Preprocessing       | StandardScaler (Z-Score Normalization) |
| Trees               | 10       |
| Max depth           | 5        |
| Input features      | 3 (RMSSD, SDNN, BPM) |

*Note: The C++ library automatically normalizes the incoming features in real-time using mean and scale constants exported from Python during the training phase.*

## Examples

| Example | Description |
|---------|-------------|
| [`BasicPrediction`](examples/BasicPrediction/) | Static test with hardcoded values |
| [`MAX30102_RealtimePrediction`](examples/MAX30102_RealtimePrediction/) | Real-time prediction using MAX30102 sensor |

## License

MIT License — free to use, modify, and distribute.
