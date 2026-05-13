#include <StressPredictor.h>

// --- KONFIGURASI SENSOR ---
// Sesuaikan dengan pin analog yang Anda gunakan di ESP32
const int SENSOR_PIN = 34; 
// Ambang batas sinyal detak jantung. Sesuaikan nilainya berdasarkan pembacaan sensor Anda
const int THRESHOLD = 2000; 

// --- VARIABEL HRV & DETAK JANTUNG ---
unsigned long lastBeatTime = 0;
// Kita akan menghitung fitur dan memprediksi stres setiap mengumpulkan 15 detak jantung
const int WINDOW_SIZE = 15; 
float rrIntervals[WINDOW_SIZE];
int beatCount = 0;

// Inisialisasi object model Random Forest
StressPredictor stressModel;

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  
  // Berikan waktu sejenak untuk Serial Monitor
  delay(1000);
  Serial.println("\n=== Sistem Prediksi Stres Real-Time ESP32 ===");
  Serial.println("Menunggu detak jantung dari sensor...");
}

void loop() {
  int sensorValue = analogRead(SENSOR_PIN);
  unsigned long currentTime = millis();

  // --- DETEKSI DETAK JANTUNG SEDERHANA (PEAK DETECTION) ---
  // Catatan: Jika Anda menggunakan modul pintar seperti MAX30102, 
  // sangat disarankan menggunakan library bawaan sensor untuk mendeteksi 'beat' 
  // agar jauh lebih akurat. Logika di bawah ini untuk sensor Pulse Analog biasa.
  static bool isBeat = false;
  
  // Cek apakah sinyal melewati batas threshold dan jarak antar detak > 300ms (max 200 BPM)
  if (sensorValue > THRESHOLD && !isBeat && (currentTime - lastBeatTime > 300)) {
    isBeat = true;
    
    // 1. Dapatkan RR Interval (waktu dari detak sebelumnya ke detak sekarang dalam milidetik)
    unsigned long rr = currentTime - lastBeatTime;
    lastBeatTime = currentTime;
    
    // Abaikan pembacaan pertama atau jika RR interval tidak masuk akal 
    // (RR > 2000ms = < 30 BPM, RR < 300ms = > 200 BPM)
    if (rr > 300 && rr < 2000 && lastBeatTime != currentTime) { 
      rrIntervals[beatCount] = (float)rr;
      beatCount++;
      
      Serial.print("Detak ["); Serial.print(beatCount); Serial.print("/"); Serial.print(WINDOW_SIZE);
      Serial.print("] terdeteksi! RR Interval: ");
      Serial.print(rr);
      Serial.println(" ms");
    }

    // 2. Jika sudah mengumpulkan data cukup banyak (WINDOW_SIZE)
    if (beatCount >= WINDOW_SIZE) {
      hitungFiturDanPrediksi();
      
      // Reset counter untuk membaca batch detak jantung berikutnya
      beatCount = 0; 
    }
  }
  
  // Reset status beat jika sinyal sudah turun di bawah threshold
  if (sensorValue < THRESHOLD) {
    isBeat = false;
  }
  
  delay(10); // Loop delay untuk stabilitas ADC ESP32
}

void hitungFiturDanPrediksi() {
  Serial.println("\n--- Ekstraksi Fitur HRV ---");
  
  // 1. Hitung Rata-rata RR (Mean RR)
  float sumRR = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sumRR += rrIntervals[i];
  }
  float meanRR = sumRR / WINDOW_SIZE;
  
  // 2. Hitung BPM (Beats Per Minute)
  float bpm = 60000.0 / meanRR;
  
  // 3. Hitung SDNN (Standard Deviation of NN intervals)
  float sumSqDiff = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sumSqDiff += pow(rrIntervals[i] - meanRR, 2);
  }
  float sdnn = sqrt(sumSqDiff / WINDOW_SIZE);
  
  // 4. Hitung RMSSD (Root Mean Square of Successive Differences)
  float sumSqDiffSucc = 0;
  for (int i = 1; i < WINDOW_SIZE; i++) {
    sumSqDiffSucc += pow(rrIntervals[i] - rrIntervals[i-1], 2);
  }
  float rmssd = sqrt(sumSqDiffSucc / (WINDOW_SIZE - 1));

  // Tampilkan Nilai Fitur yang diekstraksi
  Serial.print("BPM   : "); Serial.println(bpm);
  Serial.print("SDNN  : "); Serial.println(sdnn);
  Serial.print("RMSSD : "); Serial.println(rmssd);

  // Lakukan Prediksi Menggunakan Library Kita
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
