// work, tapi bukan dari kode original paper

#include "MAX30105.h"
#include "heartRate.h"
#include <StressPredictor.h>
#include <Wire.h>
// #include "../../Model/rf_model.h"

MAX30105 particleSensor;

// ===== HRV CONFIG =====
const byte RR_SIZE = 15;
float rrList[RR_SIZE];
byte rrCount = 0;

unsigned long lastBeat = 0;
float lastBPM = -1.0;
const float BPM_THRESHOLD = 25.0;
const byte CALIBRATION_BEATS = 5;
float calibrationBpmSum = 0;
byte calibrationBpmCount = 0;
bool isCalibrating = true;
float restingBPM = -1.0;

// Stress Predictor
StressPredictor stressModel;
// RFModel stressModel;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("=== HRV + Prediksi Stres ===");
  Serial.println("Inisialisasi sensor MAX30102...");

  // Fix #4: pin I2C eksplisit untuk ESP32 (SDA=21, SCL=22)
  Wire.begin(21, 22);
  Wire.setClock(400000);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("Sensor tidak ditemukan!");
    while (1)
      ;
  }

  // Fix #3: konfigurasi sensor eksplisit untuk sinyal yang lebih bersih
  // brightness=60(~11.8mA), avg=4, Red+IR, 400sps, pulseWidth=411us, adc=4096
  particleSensor.setup(60, 4, 2, 400, 411, 4096);
  particleSensor.setPulseAmplitudeGreen(0); // Matikan LED hijau

  Serial.println("Sensor siap.");
}

float computeSDNN(float *rr, int n) {
  if (n < 2)
    return 0;

  float mean = 0;
  for (int i = 0; i < n; i++)
    mean += rr[i];
  mean /= n;

  float sumSq = 0;
  for (int i = 0; i < n; i++) {
    float diff = rr[i] - mean;
    sumSq += diff * diff;
  }

  return sqrt(sumSq / (n - 1));
}

float computeRMSSD(float *rr, int n) {
  if (n < 2)
    return 0;

  float sumSq = 0;
  int count = 0;

  for (int i = 1; i < n; i++) {
    float diff = rr[i] - rr[i - 1];
    sumSq += diff * diff;
    count++;
  }

  return sqrt(sumSq / count);
}

void loop() {
  long irValue = particleSensor.getIR();

  if (irValue < 50000) {
    Serial.println("Tempelkan jari...");
    rrCount = 0;
    lastBeat =
        0; // Fix #1: reset agar RR pertama tidak dihitung dari timestamp lama
    lastBPM = -1.0;
    calibrationBpmSum = 0;
    calibrationBpmCount = 0;
    isCalibrating = true;
    restingBPM = -1.0;
    delay(200);
    return;
  }

  if (checkForBeat(irValue)) {
    unsigned long now = millis();
    unsigned long rr = now - lastBeat;

    float bpm = 60000.0 / rr;

    if (bpm < 35 || bpm > 220) {
      if (bpm < 35)
        lastBeat =
            now; // Jika terlalu lambat (missed beat long gap), reset timer
      return;    // Jika terlalu cepat (>220), ini pasti noise. Jangan update
                 // lastBeat!
    }

    if (isCalibrating) {
      lastBeat = now;
      calibrationBpmSum += bpm;
      calibrationBpmCount++;

      Serial.print("Kalibrasi BPM: ");
      Serial.print(calibrationBpmCount);
      Serial.print("/");
      Serial.println(CALIBRATION_BEATS);

      if (calibrationBpmCount < CALIBRATION_BEATS) {
        return;
      }

      restingBPM = calibrationBpmSum / calibrationBpmCount;
      lastBPM = restingBPM;
      isCalibrating = false;

      Serial.print("Resting BPM terkalibrasi: ");
      Serial.println(restingBPM, 1);
      return;
    }

    // Filter Outlier Anti-Noise
    if (lastBPM >= 0) {
      if (bpm > lastBPM + BPM_THRESHOLD) {
        // PREMATURE BEAT (Noise). Jangan update lastBeat, lewati saja.
        return;
      }
      if (bpm < lastBPM - BPM_THRESHOLD) {
        // MISSED BEAT (Gap). Jangan hitung ke HRV, tapi UPDATE lastBeat untuk
        // siklus berikutnya.
        lastBeat = now;
        return;
      }
    }

    // Jika sampai di sini, detak valid!
    lastBeat = now;
    lastBPM = bpm;

    if (rrCount < RR_SIZE) {
      rrList[rrCount++] = rr;
    }

    Serial.print("BPM: ");
    Serial.print(bpm);

    if (rrCount >= RR_SIZE) {
      float sdnn = computeSDNN(rrList, RR_SIZE);
      float rmssd = computeRMSSD(rrList, RR_SIZE);

      // Fix #2: hitung mean BPM dari window RR, bukan BPM instan beat terakhir
      float sumRR = 0;
      for (int i = 0; i < RR_SIZE; i++)
        sumRR += rrList[i];
      float meanBPM = 60000.0 / (sumRR / RR_SIZE);

      Serial.print(" | SDNN: ");
      Serial.print(sdnn, 2);

      Serial.print(" | RMSSD: ");
      Serial.print(rmssd, 2);

      int stress = stressModel.predict(rmssd, sdnn, meanBPM);

      // Fix #5: tampilkan label stres dalam satu baris
      Serial.print(" | STRESS: ");
      Serial.print(stress);
      if (stress == 0)
        Serial.println(" (Rendah)");
      else if (stress == 1)
        Serial.println(" (Sedang)");
      else if (stress == 2)
        Serial.println(" (Tinggi)");
      else
        Serial.println(" (Unknown)");

      rrCount = 0;
    } else {
      Serial.print(" | Mengumpulkan RR: ");
      Serial.println(rrCount);
    }
  }
}