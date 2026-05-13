#include <StressPredictor.h>

// Inisialisasi object dari library
StressPredictor stressModel;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("=== Sistem Prediksi Stres ===");
  Serial.println("Menginisialisasi model Random Forest...");
  delay(1000);
  
  // Contoh Pengujian
  // Format: predict(RMSSD, SDNN, BPM)
  
  // Test Case 1: Harusnya Stres Rendah
  float rmssd_1 = 207.69;
  float sdnn_1  = 167.53;
  float bpm_1   = 74.89;
  
  // Test Case 2: Harusnya Stres Sedang
  float rmssd_2 = 67.38;
  float sdnn_2  = 60.82;
  float bpm_2   = 79.71;

  // Test Case 3: Harusnya Stres Tinggi
  float rmssd_3 = 45.61;
  float sdnn_3  = 32.18;
  float bpm_3   = 98.23;

  Serial.println("\nMemulai Pengujian Prediksi...");
  
  prediksiDanPrint("Data 1 (Ekspektasi: Rendah)", rmssd_1, sdnn_1, bpm_1);
  prediksiDanPrint("Data 2 (Ekspektasi: Sedang)", rmssd_2, sdnn_2, bpm_2);
  prediksiDanPrint("Data 3 (Ekspektasi: Tinggi)", rmssd_3, sdnn_3, bpm_3);
}

void loop() {
  // Dalam aplikasi nyata, Anda akan membaca data dari sensor Heart Rate
  // menghitung SDNN dan RMSSD setiap rentang waktu (misalnya setiap 10-60 detik)
  // lalu memanggil stressModel.predict()
  
  delay(5000); // Tunggu sebelum pengujian berikutnya jika ada
}

void prediksiDanPrint(String label, float rmssd, float sdnn, float bpm) {
  Serial.print(label);
  Serial.print(" | RMSSD: "); Serial.print(rmssd);
  Serial.print(", SDNN: "); Serial.print(sdnn);
  Serial.print(", BPM: "); Serial.print(bpm);
  
  int hasil = stressModel.predict(rmssd, sdnn, bpm); 
  
  Serial.print(" => HASIL: ");
  if (hasil == 0) {
    Serial.println("Stres Rendah (0)");
  } else if (hasil == 1) {
    Serial.println("Stres Sedang (1)");
  } else if (hasil == 2) {
    Serial.println("Stres Tinggi (2)");
  } else {
    Serial.println("Tidak Diketahui");
  }
}
