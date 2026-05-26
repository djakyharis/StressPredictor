import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, classification_report
from micromlgen import port
import os

# Mendapatkan direktori skrip dijalankan
script_dir = os.path.dirname(os.path.abspath(__file__)) if '__file__' in locals() else os.getcwd()

# Mencari DATA TRAINING.csv secara dinamis
csv_path = 'DATA TRAINING.csv'
if not os.path.exists(csv_path):
    # Coba satu level di atas (jika dijalankan dari StressPredictor)
    parent_path = os.path.join(script_dir, '..', 'DATA TRAINING.csv')
    if os.path.exists(parent_path):
        csv_path = parent_path
    else:
        # Coba dua level di atas (jika dijalankan dari StressPredictor/python)
        grandparent_path = os.path.join(script_dir, '..', '..', 'DATA TRAINING.csv')
        if os.path.exists(grandparent_path):
            csv_path = grandparent_path

if not os.path.exists(csv_path):
    print(f"Error: File 'DATA TRAINING.csv' tidak ditemukan. Harap letakkan file tersebut di root project.")
    exit(1)

print(f"Menggunakan file dataset dari: {os.path.abspath(csv_path)}")

# 1. Muat Data
df = pd.read_csv(csv_path)

# 2. Persiapan Fitur dan Target
# RMSSD, SDNN, dan BPM
features = ['RMSSD (ms)', 'SDNN (ms)', 'BPM']
X = df[features]
y = df['Label Stres']

# Encoding target ke integer
# stres rendah = 0, stres sedang = 1, stres tinggi = 2
label_map = {'stres rendah': 0, 'stres sedang': 1, 'stres tinggi': 2}
y = y.map(label_map)

# Membagi data menjadi data latih dan uji
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Normalisasi Data
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

# 3. Latih Model Random Forest
clf = RandomForestClassifier(n_estimators=10, max_depth=5, random_state=42)
clf.fit(X_train_scaled, y_train)

# 4. Evaluasi Model
y_pred = clf.predict(X_test_scaled)
acc = accuracy_score(y_test, y_pred)
print(f"Akurasi Model: {acc * 100:.2f}%")
print("\nLaporan Klasifikasi:")
print(classification_report(y_test, y_pred, target_names=['stres rendah', 'stres sedang', 'stres tinggi']))

# 5. Ekspor ke Header C++
print("Mengekspor model ke model.h...")
c_code = port(clf, classname='StressPredictorRF')

scaler_code = f"""
// Normalization Constants
const float SCALER_MEANS[3] = {{{scaler.mean_[0]}f, {scaler.mean_[1]}f, {scaler.mean_[2]}f}};
const float SCALER_SCALES[3] = {{{scaler.scale_[0]}f, {scaler.scale_[1]}f, {scaler.scale_[2]}f}};

"""

c_code_combined = scaler_code + c_code

# Cari folder StressPredictor/src
src_dir = os.path.abspath(os.path.join(script_dir, '..', 'src'))
if os.path.exists(src_dir):
    dest_path = os.path.join(src_dir, 'model.h')
    with open(dest_path, 'w') as f:
        f.write(c_code_combined)
    print(f"Berhasil! Model diekspor dan disimpan ke {dest_path}")
else:
    # Fallback ke folder saat ini jika dijalankan di luar project structure yang normal
    dest_path = os.path.join(script_dir, 'model.h')
    with open(dest_path, 'w') as f:
        f.write(c_code_combined)
    print(f"Berhasil! Model diekspor ke {dest_path}")
