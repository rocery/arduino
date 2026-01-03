/*
  Volume Tangki Minyak
  Menghitung volume tangki minyak berdasarkan dimensi yang diberikan.

  Dibuat oleh: [Sastra Nur Alamsyah]
  Tanggal: [02-12-2026]

  Deskripsi:
  Program ini menghitung volume tangki minyak berbentuk silinder atau kubus
  berdasarkan input dimensi dari pengguna. Hasil volume ditampilkan dalam liter.

  Catatan:
  - Pastikan untuk memasukkan dimensi dalam satuan yang sesuai (cm).
  - Volume dihitung dalam liter (1 liter = 1000 cm³).
*/

#include <HardwareSerial.h>

// ========== KONFIGURASI ==========
#define RS485_RX 16  // Pin RX2 ESP32 ke TX sensor
#define RS485_TX 17  // Pin TX2 ESP32 ke RX sensor

// Dimensi Tank
const float TANK_HEIGHT = 600.0;  // cm
const float TANK_RADIUS = 150.0;  // cm (diameter 3m / 2)
const float SENSOR_OFFSET = 0.0;  // cm, jarak sensor dari puncak tank

// Batas pengukuran sensor
const int MIN_DISTANCE = 28;   // cm (dead zone sensor)
const int MAX_DISTANCE = 750;  // cm (max range sensor based on datasheet)

// Interval pembacaan
const unsigned long READ_INTERVAL = 2000;

// ========== VARIABEL GLOBAL ==========
HardwareSerial rs485(1);
unsigned long lastReadTime = 0;
float currentDistance = 0;
float currentOilHeight = 0;
float currentVolume = 0;

void setup() {
  Serial.begin(115200);
  rs485.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);

  Serial.println("========================================");
  Serial.println("ESP32 Oil Tank Volume Monitor");
  Serial.println("Sensor: A01NYUB Waterproof Ultrasonic");
  Serial.println("========================================");

  delay(1000);
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime >= READ_INTERVAL) {
    lastReadTime = currentTime;

    float distance = readDistance();

    if (distance > 0) {
      currentDistance = distance;

      // Hitung tinggi minyak
      currentOilHeight = TANK_HEIGHT - currentDistance - SENSOR_OFFSET;
      if (currentOilHeight < 0) currentOilHeight = 0;
      if (currentOilHeight > TANK_HEIGHT) currentOilHeight = TANK_HEIGHT;

      // Hitung volume minyak
      float heightM = currentOilHeight / 100.0;
      float radiusM = TANK_RADIUS / 100.0;
      currentVolume = PI * radiusM * radiusM * heightM;  // m³

      // Tampilkan hasil
      Serial.print("Jarak Sensor: ");
      Serial.print(currentDistance, 1);
      Serial.print(" cm | Tinggi Minyak: ");
      Serial.print(currentOilHeight, 1);
      Serial.print(" cm | Volume Minyak: ");
      Serial.print(currentVolume, 2);
      Serial.print(" m³ (");
      Serial.print(currentVolume * 1000, 0);
      Serial.println(" liter)");
    } else {
      Serial.println("Waiting... (check wiring if no data)");
    }
  }
}


float readDistance() {

  while (rs485.available()) {
    rs485.read();
  }


  unsigned long currentTime = millis();

  return 0;
}