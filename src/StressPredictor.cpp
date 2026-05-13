#include "StressPredictor.h"
#include "model.h"

StressPredictor::StressPredictor() {
    // Constructor tidak memerlukan inisialisasi khusus untuk model ini
}

int StressPredictor::predict(float rmssd, float sdnn, float bpm) {
    // Urutan fitur harus sama persis dengan saat training di Python:
    // 1. RMSSD (ms)
    // 2. SDNN (ms)
    // 3. BPM
    float features[3] = {rmssd, sdnn, bpm};
    
    // Inisialisasi classifier Random Forest
    Eloquent::ML::Port::StressPredictorRF clf;
    
    // Melakukan prediksi 
    // Return: 0 (Stres Rendah), 1 (Stres Sedang), 2 (Stres Tinggi)
    return clf.predict(features);
}
