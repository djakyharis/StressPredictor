#ifndef STRESS_PREDICTOR_H
#define STRESS_PREDICTOR_H

#include <Arduino.h>

class StressPredictor {
  public:
    StressPredictor();
    
    /**
     * Prediksi tingkat stres berdasarkan fitur
     * @param rmssd Nilai RMSSD (ms)
     * @param sdnn Nilai SDNN (ms)
     * @param bpm Nilai Detak Jantung (Beats Per Minute)
     * @return 0: Stres Rendah, 1: Stres Sedang, 2: Stres Tinggi
     */
    int predict(float rmssd, float sdnn, float bpm);
};

#endif
