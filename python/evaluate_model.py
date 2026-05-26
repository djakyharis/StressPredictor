import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
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

df = pd.read_csv(csv_path)
features = ['RMSSD (ms)', 'SDNN (ms)', 'BPM']
X = df[features]
y = df['Label Stres']

label_map = {'stres rendah': 0, 'stres sedang': 1, 'stres tinggi': 2}
y = y.map(label_map)

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

clf = RandomForestClassifier(n_estimators=10, max_depth=5, random_state=42)
clf.fit(X_train, y_train)
y_pred = clf.predict(X_test)

acc = accuracy_score(y_test, y_pred)
report = classification_report(y_test, y_pred, target_names=['stres rendah', 'stres sedang', 'stres tinggi'])
print(f"Akurasi Model Evaluasi: {acc * 100:.2f}%")
print("\nLaporan Klasifikasi:")
print(report)

# Confusion Matrix
cm = confusion_matrix(y_test, y_pred)
plt.figure(figsize=(8, 6))
sns.heatmap(cm, annot=True, fmt='d', cmap='Blues', xticklabels=label_map.keys(), yticklabels=label_map.keys())
plt.title(f'Confusion Matrix (Accuracy: {acc*100:.2f}%)')
plt.ylabel('Actual')
plt.xlabel('Predicted')
plt.tight_layout()

cm_path = os.path.join(script_dir, 'confusion_matrix.png')
plt.savefig(cm_path)
print(f"Confusion Matrix disimpan ke: {cm_path}")
plt.close()

# Feature Importance
importances = clf.feature_importances_
plt.figure(figsize=(8, 6))
sns.barplot(x=features, y=importances, hue=features, legend=False, palette='viridis')
plt.title('Feature Importances in Random Forest')
plt.ylabel('Importance')
plt.xlabel('Feature')
plt.tight_layout()

fi_path = os.path.join(script_dir, 'feature_importance.png')
plt.savefig(fi_path)
print(f"Feature Importance disimpan ke: {fi_path}")
plt.close()
