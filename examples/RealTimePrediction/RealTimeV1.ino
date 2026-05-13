#include <Wire.h>
#include "MAX30105.h"   // Requires: SparkFun MAX3010x Pulse and Proximity Sensor Library
#include "heartRate.h"  // Part of SparkFun MAX3010x library
#include <StressPredictor.h>

MAX30105 particleSensor;
StressPredictor stressModel;

const byte RATE_SIZE = 15; // Number of RR intervals to collect before prediction
float rrIntervals[RATE_SIZE];
byte beatCount = 0;
unsigned long lastBeatTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MAX30102...");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("Sensor not found. Check SDA/SCL connections!");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A); // Red LED on (indicates active)
  particleSensor.setPulseAmplitudeGreen(0);  // Green LED off

  Serial.println("MAX30102 ready. Place your finger on the sensor...");
}

void loop() {
  long irValue = particleSensor.getIR();

  // Check for heartbeat (R-peak detection)
  if (checkForBeat(irValue)) {
    unsigned long currentTime = millis();
    unsigned long rr = currentTime - lastBeatTime;
    lastBeatTime = currentTime;

    // Accept RR intervals within 300–2000 ms (30–200 BPM)
    if (rr > 300 && rr < 2000) {
      rrIntervals[beatCount] = (float)rr;
      beatCount++;

      Serial.print("Beat ["); Serial.print(beatCount);
      Serial.print("/");      Serial.print(RATE_SIZE);
      Serial.print("] RR: "); Serial.print(rr);
      Serial.println(" ms");
    }

    if (beatCount >= RATE_SIZE) {
      computeAndPredict();
      beatCount = 0; // Reset window
    }
  }

  // Optional: reset on finger removal (irValue < 50000)
}

void computeAndPredict() {
  Serial.println("\n--- HRV Feature Extraction ---");

  // Mean RR
  float sumRR = 0;
  for (int i = 0; i < RATE_SIZE; i++) sumRR += rrIntervals[i];
  float meanRR = sumRR / RATE_SIZE;

  // BPM
  float bpm = 60000.0 / meanRR;

  // SDNN
  float sumSqDiff = 0;
  for (int i = 0; i < RATE_SIZE; i++) sumSqDiff += pow(rrIntervals[i] - meanRR, 2);
  float sdnn = sqrt(sumSqDiff / RATE_SIZE);

  // RMSSD
  float sumSqDiffSucc = 0;
  for (int i = 1; i < RATE_SIZE; i++) sumSqDiffSucc += pow(rrIntervals[i] - rrIntervals[i-1], 2);
  float rmssd = sqrt(sumSqDiffSucc / (RATE_SIZE - 1));

  Serial.print("BPM   : "); Serial.println(bpm);
  Serial.print("SDNN  : "); Serial.println(sdnn);
  Serial.print("RMSSD : "); Serial.println(rmssd);

  int result = stressModel.predict(rmssd, sdnn, bpm);

  Serial.print(">>> PREDICTION: ");
  if      (result == 0) Serial.println("Low Stress");
  else if (result == 1) Serial.println("Medium Stress");
  else if (result == 2) Serial.println("High Stress");
  else                  Serial.println("Unknown");

  Serial.println("------------------------------\n");
}
