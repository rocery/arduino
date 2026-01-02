/*
 * ESP32 Oil Tank Volume Monitor
 * Sensor: A01NYUB Waterproof Ultrasonic
 * Tank: Height 6m, Diameter 3m
 */

#include <HardwareSerial.h>

// ========== KONFIGURASI ==========
#define SENSOR_RX 16  // Pin RX ESP32 ke TX sensor
#define SENSOR_TX 17  // Pin TX ESP32 ke RX sensor

// Dimensi Tank
const float TANK_HEIGHT = 600.0;      // cm
const float TANK_RADIUS = 150.0;      // cm (diameter 3m / 2)
const float SENSOR_OFFSET = 0.0;      // cm, jarak sensor dari puncak tank

// Batas pengukuran sensor
const int MIN_DISTANCE = 30;          // cm (dead zone sensor)
const int MAX_DISTANCE = 750;         // cm (max range sensor)

// Interval pembacaan
const unsigned long READ_INTERVAL = 2000;  // ms

// ========== VARIABEL GLOBAL ==========
HardwareSerial sensorSerial(1);  // UART1 untuk sensor
unsigned long lastReadTime = 0;
float currentDistance = 0;
float currentOilHeight = 0;
float currentVolume = 0;

// ========== FUNGSI: INISIALISASI ==========
void setup() {
  Serial.begin(115200);
  sensorSerial.begin(9600, SERIAL_8N1, SENSOR_RX, SENSOR_TX);
  
  Serial.println("========================================");
  Serial.println("ESP32 Oil Tank Volume Monitor");
  Serial.println("Sensor: A01NYUB Waterproof Ultrasonic");
  Serial.println("========================================");
  
  displayTankInfo();
  delay(1000);
}

// ========== FUNGSI: LOOP UTAMA ==========
void loop() {
  unsigned long currentTime = millis();
  
  // Baca sensor sesuai interval
  if (currentTime - lastReadTime >= READ_INTERVAL) {
    lastReadTime = currentTime;
    
    // Baca jarak dari sensor
    int distance = readSensorDistance();
    
    if (distance > 0) {
      currentDistance = distance;
      
      // Hitung tinggi minyak
      currentOilHeight = calculateOilHeight(distance);
      
      // Hitung volume minyak
      currentVolume = calculateVolume(currentOilHeight);
      
      // Tampilkan hasil
      displayMeasurement();
    } else {
      Serial.println("Error: Gagal membaca sensor");
    }
  }
  
  // Tambahkan fitur lain di sini
  // Contoh: checkAlarm(), sendToCloud(), updateDisplay(), dll
}

// ========== FUNGSI: BACA DATA SENSOR ==========
int readSensorDistance() {
  unsigned char data[4] = {};
  int distance = -1;
  
  // Cek apakah ada data dari sensor
  if (sensorSerial.available() >= 4) {
    // Cari header 0xFF
    if (sensorSerial.read() == 0xFF) {
      data[0] = 0xFF;
      
      // Baca 3 byte berikutnya
      for (int i = 1; i < 4; i++) {
        data[i] = sensorSerial.read();
      }
      
      // Validasi checksum
      unsigned char checksum = (data[0] + data[1] + data[2]) & 0x00FF;
      
      if (data[3] == checksum) {
        // Hitung jarak (mm ke cm)
        distance = (data[1] << 8) + data[2];
        distance = distance / 10;  // Konversi mm ke cm
        
        // Validasi range
        if (distance < MIN_DISTANCE || distance > MAX_DISTANCE) {
          Serial.print("Warning: Jarak di luar range valid (");
          Serial.print(distance);
          Serial.println(" cm)");
          return -1;
        }
      } else {
        Serial.println("Error: Checksum tidak valid");
        return -1;
      }
    }
  }
  
  return distance;
}

// ========== FUNGSI: HITUNG TINGGI MINYAK ==========
float calculateOilHeight(float sensorDistance) {
  // Tinggi minyak = tinggi tank - jarak sensor - offset
  float oilHeight = TANK_HEIGHT - sensorDistance - SENSOR_OFFSET;
  
  // Pastikan tidak negatif
  if (oilHeight < 0) {
    oilHeight = 0;
  }
  
  // Pastikan tidak melebihi tinggi tank
  if (oilHeight > TANK_HEIGHT) {
    oilHeight = TANK_HEIGHT;
  }
  
  return oilHeight;
}

// ========== FUNGSI: HITUNG VOLUME ==========
float calculateVolume(float oilHeightCm) {
  // Konversi cm ke meter
  float heightM = oilHeightCm / 100.0;
  float radiusM = TANK_RADIUS / 100.0;
  
  // Volume silinder: V = π × r² × h
  float volumeM3 = PI * radiusM * radiusM * heightM;
  
  return volumeM3;
}

// ========== FUNGSI: HITUNG PERSENTASE ==========
float calculatePercentage(float oilHeightCm) {
  float percentage = (oilHeightCm / TANK_HEIGHT) * 100.0;
  
  if (percentage > 100) percentage = 100;
  if (percentage < 0) percentage = 0;
  
  return percentage;
}

// ========== FUNGSI: KONVERSI VOLUME KE LITER ==========
float volumeToLiters(float volumeM3) {
  return volumeM3 * 1000.0;  // 1 m³ = 1000 liter
}

// ========== FUNGSI: TAMPILKAN INFO TANK ==========
void displayTankInfo() {
  Serial.println("\n--- Informasi Tank ---");
  Serial.print("Tinggi Tank: ");
  Serial.print(TANK_HEIGHT);
  Serial.println(" cm");
  
  Serial.print("Diameter Tank: ");
  Serial.print(TANK_RADIUS * 2);
  Serial.println(" cm");
  
  float maxVol = calculateVolume(TANK_HEIGHT);
  Serial.print("Kapasitas Maksimal: ");
  Serial.print(maxVol, 2);
  Serial.print(" m³ (");
  Serial.print(volumeToLiters(maxVol), 0);
  Serial.println(" liter)");
  
  Serial.println("----------------------\n");
}

// ========== FUNGSI: TAMPILKAN HASIL PENGUKURAN ==========
void displayMeasurement() {
  Serial.println("========================================");
  
  Serial.print("Jarak Sensor: ");
  Serial.print(currentDistance, 1);
  Serial.println(" cm");
  
  Serial.print("Tinggi Minyak: ");
  Serial.print(currentOilHeight, 1);
  Serial.print(" cm (");
  Serial.print(calculatePercentage(currentOilHeight), 1);
  Serial.println("%)");
  
  Serial.print("Volume Minyak: ");
  Serial.print(currentVolume, 2);
  Serial.print(" m³ (");
  Serial.print(volumeToLiters(currentVolume), 0);
  Serial.println(" liter)");
  
  // Bar indicator
  displayProgressBar(calculatePercentage(currentOilHeight));
  
  Serial.println("========================================\n");
}

// ========== FUNGSI: TAMPILKAN BAR INDICATOR ==========
void displayProgressBar(float percentage) {
  Serial.print("Level: [");
  
  int bars = (int)(percentage / 5);  // 20 karakter untuk 100%
  
  for (int i = 0; i < 20; i++) {
    if (i < bars) {
      Serial.print("=");
    } else {
      Serial.print(" ");
    }
  }
  
  Serial.print("] ");
  Serial.print(percentage, 1);
  Serial.println("%");
}

// ========== FUNGSI TAMBAHAN: CEK ALARM ==========
// Uncomment untuk mengaktifkan alarm
/*
void checkAlarm() {
  float percentage = calculatePercentage(currentOilHeight);
  
  if (percentage < 10) {
    Serial.println("⚠️ ALARM: Level minyak sangat rendah!");
  } else if (percentage < 20) {
    Serial.println("⚠️ WARNING: Level minyak rendah");
  } else if (percentage > 95) {
    Serial.println("⚠️ WARNING: Level minyak hampir penuh");
  }
}
*/

// ========== FUNGSI TAMBAHAN: KIRIM DATA KE CLOUD ==========
// Contoh fungsi untuk mengirim data (implementasi sesuai platform)
/*
void sendToCloud() {
  // Kirim data ke server/cloud
  // Implementasi: HTTP POST, MQTT, dll
  
  Serial.println("Mengirim data ke cloud...");
  // Contoh payload JSON:
  // {"distance": currentDistance, "height": currentOilHeight, "volume": currentVolume}
}
*/

// ========== FUNGSI TAMBAHAN: KALIBRASI ==========
/*
float calibrateSensor(int numReadings) {
  Serial.println("Memulai kalibrasi...");
  float sum = 0;
  int validReadings = 0;
  
  for (int i = 0; i < numReadings; i++) {
    int distance = readSensorDistance();
    if (distance > 0) {
      sum += distance;
      validReadings++;
    }
    delay(100);
  }
  
  if (validReadings > 0) {
    float average = sum / validReadings;
    Serial.print("Kalibrasi selesai. Rata-rata: ");
    Serial.print(average);
    Serial.println(" cm");
    return average;
  }
  
  return -1;
}
*/