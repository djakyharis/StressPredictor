#include <StressPredictor.h>

StressPredictor stressModel;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("=== Stress Prediction System ===");
  delay(1000);

  // Test cases: predict(RMSSD, SDNN, BPM)
  float rmssd_1 = 207.69, sdnn_1 = 167.53, bpm_1 = 74.89; // Expected: Low
  float rmssd_2 = 67.38,  sdnn_2 = 60.82,  bpm_2 = 79.71; // Expected: Medium
  float rmssd_3 = 45.61,  sdnn_3 = 32.18,  bpm_3 = 98.23; // Expected: High

  Serial.println("\nRunning prediction tests...");
  predictAndPrint("Sample 1 (Expected: Low)",    rmssd_1, sdnn_1, bpm_1);
  predictAndPrint("Sample 2 (Expected: Medium)", rmssd_2, sdnn_2, bpm_2);
  predictAndPrint("Sample 3 (Expected: High)",   rmssd_3, sdnn_3, bpm_3);
}

void loop() {
  // In real use: read from HR sensor, compute SDNN/RMSSD every 10–60s, then call predict()
  delay(5000);
}

void predictAndPrint(String label, float rmssd, float sdnn, float bpm) {
  Serial.print(label);
  Serial.print(" | RMSSD: "); Serial.print(rmssd);
  Serial.print(", SDNN: ");   Serial.print(sdnn);
  Serial.print(", BPM: ");    Serial.print(bpm);

  int result = stressModel.predict(rmssd, sdnn, bpm);

  Serial.print(" => RESULT: ");
  if      (result == 0) Serial.println("Low Stress (0)");
  else if (result == 1) Serial.println("Medium Stress (1)");
  else if (result == 2) Serial.println("High Stress (2)");
  else                  Serial.println("Unknown");
}
