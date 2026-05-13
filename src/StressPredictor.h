#ifndef STRESS_PREDICTOR_H
#define STRESS_PREDICTOR_H

#include <Arduino.h>

class StressPredictor {
  public:
    StressPredictor();
    
    /**
     * Predict stress level from HRV features.
     * @param rmssd RMSSD value (ms)
     * @param sdnn  SDNN value (ms)
     * @param bpm   Heart rate (beats per minute)
     * @return 0: Low, 1: Medium, 2: High stress
     */
    int predict(float rmssd, float sdnn, float bpm);
};

#endif
