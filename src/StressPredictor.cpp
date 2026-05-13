#include "StressPredictor.h"
#include "model.h"

StressPredictor::StressPredictor() {
    // No special initialization needed
}

int StressPredictor::predict(float rmssd, float sdnn, float bpm) {
    // Feature order must match training: RMSSD, SDNN, BPM
    float features[3] = {rmssd, sdnn, bpm};

    Eloquent::ML::Port::StressPredictorRF clf;

    // Returns: 0 = Low, 1 = Medium, 2 = High stress
    return clf.predict(features);
}
