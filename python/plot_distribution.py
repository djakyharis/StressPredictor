import pandas as pd
import matplotlib.pyplot as plt
from sklearn.preprocessing import StandardScaler
import seaborn as sns
import os

# Mendapatkan direktori skrip dijalankan
script_dir = os.path.dirname(os.path.abspath(__file__)) if '__file__' in locals() else os.getcwd()

# Mencari DATA TRAINING.csv secara dinamis
csv_path = 'DATA TRAINING.csv'
if not os.path.exists(csv_path):
    # Coba satu level di atas
    parent_path = os.path.join(script_dir, '..', 'DATA TRAINING.csv')
    if os.path.exists(parent_path):
        csv_path = parent_path
    else:
        # Coba dua level di atas
        grandparent_path = os.path.join(script_dir, '..', '..', 'DATA TRAINING.csv')
        if os.path.exists(grandparent_path):
            csv_path = grandparent_path

if not os.path.exists(csv_path):
    print(f"Error: File 'DATA TRAINING.csv' tidak ditemukan. Harap letakkan file tersebut di root project.")
    exit(1)

print(f"Menggunakan file dataset dari: {os.path.abspath(csv_path)}")

# Load Data
df = pd.read_csv(csv_path)
features = ['RMSSD (ms)', 'SDNN (ms)', 'BPM']
X = df[features]

# Normalize
scaler = StandardScaler()
X_scaled = pd.DataFrame(scaler.fit_transform(X), columns=features)

# Plot
fig, axes = plt.subplots(2, 3, figsize=(15, 10))
fig.suptitle('Distribusi Data: Sebelum vs Sesudah Normalisasi (StandardScaler)', fontsize=16)

colors = ['#1f77b4', '#ff7f0e', '#2ca02c']

for i, feature in enumerate(features):
    # Sebelum Normalisasi
    sns.histplot(X[feature], kde=True, ax=axes[0, i], color=colors[i])
    axes[0, i].set_title(f'Original - {feature}')
    
    # Sesudah Normalisasi
    sns.histplot(X_scaled[feature], kde=True, ax=axes[1, i], color=colors[i])
    axes[1, i].set_title(f'Normalized - {feature}')

plt.tight_layout(rect=[0, 0.03, 1, 0.95])

plot_path = os.path.join(script_dir, 'distribution_plot.png')
plt.savefig(plot_path)
print(f"Plot saved to {plot_path}")
