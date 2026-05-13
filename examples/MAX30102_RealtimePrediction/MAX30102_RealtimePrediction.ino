#include <Wire.h>
#include "MAX30105.h"   // Membutuhkan library "SparkFun MAX3010x Pulse and Proximity Sensor Library"
#include "heartRate.h"  // Bagian dari library SparkFun MAX3010x
#include <StressPredictor.h>

MAX30105 particleSensor;

const byte RATE_SIZE = 15; // Jumlah sampel detak jantung yang dikumpulkan sebelum prediksi
float rrIntervals[RATE_SIZE];
byte beatCount = 0;

unsigned long lastBeatTime = 0;

// Inisialisasi object dari library
StressPredictor stressModel;

void setup() {
  Serial.begin(115200);
  Serial.println("Menginisialisasi MAX30102...");

  // Inisialisasi komunikasi I2C dan Sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { // Gunakan port I2C default, kecepatan 400kHz
    Serial.println("Sensor MAX30102 tidak ditemukan. Tolong cek koneksi kabel SDA dan SCL Anda!");
    while (1);
  }

  particleSensor.setup(); // Konfigurasi sensor dengan pengaturan default
  particleSensor.setPulseAmplitudeRed(0x0A); // Nyalakan LED Merah agar terlihat sensor bekerja
  particleSensor.setPulseAmplitudeGreen(0); // Matikan LED Hijau
  
  Serial.println("MAX30102 Terkoneksi! Tempelkan jari Anda pada sensor...");
}

void loop() {
  long irValue = particleSensor.getIR();

  // Memeriksa apakah ada detak jantung (R-Peak) yang terdeteksi
  if (checkForBeat(irValue) == true) {
    unsigned long currentTime = millis();
    unsigned long rr = currentTime - lastBeatTime;
    lastBeatTime = currentTime;

    // Filter nilai RR yang masuk akal (antara 300ms = 200 BPM, hingga 2000ms = 30 BPM)
    if (rr > 300 && rr < 2000 && lastBeatTime != currentTime) {
      rrIntervals[beatCount] = (float)rr;
      beatCount++;
      
      Serial.print("Detak ["); Serial.print(beatCount); Serial.print("/"); Serial.print(RATE_SIZE);
      Serial.print("] terdeteksi! RR Interval: ");
      Serial.print(rr);
      Serial.println(" ms");
    }

    // Jika data RR sudah terkumpul sejumlah RATE_SIZE
    if (beatCount >= RATE_SIZE) {
      hitungFiturDanPrediksi();
      beatCount = 0; // Reset untuk mulai membaca jendela baru
    }
  }

  // Jika jari dilepas dari sensor (nilai IR turun drastis)
  if (irValue < 50000) {
    // Anda dapat mereset hitungan jika jari dilepas, atau sekadar menampilkan notifikasi
    // beatCount = 0; 
  }
}

void hitungFiturDanPrediksi() {
  Serial.println("\n--- Ekstraksi Fitur HRV dari MAX30102 ---");
  
  // 1. Hitung Rata-rata RR
  float sumRR = 0;
  for (int i = 0; i < RATE_SIZE; i++) {
    sumRR += rrIntervals[i];
  }
  float meanRR = sumRR / RATE_SIZE;
  
  // 2. Hitung BPM
  float bpm = 60000.0 / meanRR;
  
  // 3. Hitung SDNN
  float sumSqDiff = 0;
  for (int i = 0; i < RATE_SIZE; i++) {
    sumSqDiff += pow(rrIntervals[i] - meanRR, 2);
  }
  float sdnn = sqrt(sumSqDiff / RATE_SIZE);
  
  // 4. Hitung RMSSD
  float sumSqDiffSucc = 0;
  for (int i = 1; i < RATE_SIZE; i++) {
    sumSqDiffSucc += pow(rrIntervals[i] - rrIntervals[i-1], 2);
  }
  float rmssd = sqrt(sumSqDiffSucc / (RATE_SIZE - 1));

  Serial.print("BPM   : "); Serial.println(bpm);
  Serial.print("SDNN  : "); Serial.println(sdnn);
  Serial.print("RMSSD : "); Serial.println(rmssd);

  // Lakukan Prediksi Menggunakan AI
  int hasil = stressModel.predict(rmssd, sdnn, bpm);
  
  Serial.print(">>> HASIL PREDIKSI: ");
  if (hasil == 0) {
    Serial.println("Stres Rendah");
  } else if (hasil == 1) {
    Serial.println("Stres Sedang");
  } else if (hasil == 2) {
    Serial.println("Stres Tinggi");
  } else {
    Serial.println("Tidak Diketahui");
  }
  Serial.println("----------------------------\n");
}
