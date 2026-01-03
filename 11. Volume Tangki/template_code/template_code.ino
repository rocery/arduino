/*
 * ESP32 + A01NYUB V2 RS485
 * FIXED VERSION - All bugs corrected
 */

#include <HardwareSerial.h>

// ========== KONFIGURASI ==========
#define RS485_RX 16  // Pin RX ESP32
#define RS485_TX 17  // Pin TX ESP32

// Dimensi Tank
const float TANK_HEIGHT = 600.0;      // cm
const float TANK_RADIUS = 150.0;      // cm (diameter 3m / 2)
const float SENSOR_OFFSET = 0.0;      // cm, jarak sensor dari puncak tank

// Batas pengukuran sensor
const int MIN_DISTANCE = 28;          // cm (dead zone sensor)
const int MAX_DISTANCE = 750;         // cm (max range sensor)

// Interval pembacaan
const unsigned long READ_INTERVAL = 2000;  // ms

// ========== VARIABEL GLOBAL ==========
HardwareSerial rs485(1);  // FIXED: Tidak pakai underscore ganda
unsigned long lastReadTime = 0;
float currentDistance = 0;
float currentOilHeight = 0;
float currentVolume = 0;

void setup() {
  Serial.begin(115200);
  rs485.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  
  Serial.println("\n========================================");
  Serial.println("ESP32 Oil Tank Volume Monitor");
  Serial.println("Sensor: A01NYUB V2 RS485");
  Serial.println("========================================");
  Serial.print("Tank Height: ");
  Serial.print(TANK_HEIGHT);
  Serial.println(" cm");
  Serial.print("Tank Diameter: ");
  Serial.print(TANK_RADIUS * 2);
  Serial.println(" cm");
  Serial.println("========================================\n");
  
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
      
      // Validasi range
      if (currentOilHeight < 0) currentOilHeight = 0;
      if (currentOilHeight > TANK_HEIGHT) currentOilHeight = TANK_HEIGHT;
      
      // Hitung volume minyak
      float heightM = currentOilHeight / 100.0;
      float radiusM = TANK_RADIUS / 100.0;
      currentVolume = PI * radiusM * radiusM * heightM;  // m³
      
      // Tampilkan hasil
      Serial.println("----------------------------------------");
      Serial.print("Jarak Sensor: ");
      Serial.print(currentDistance, 1);
      Serial.println(" cm");
      
      Serial.print("Tinggi Minyak: ");
      Serial.print(currentOilHeight, 1);
      Serial.print(" cm (");
      Serial.print((currentOilHeight / TANK_HEIGHT) * 100, 1);
      Serial.println("%)");
      
      Serial.print("Volume Minyak: ");
      Serial.print(currentVolume, 2);
      Serial.print(" m³ (");
      Serial.print(currentVolume * 1000, 0);
      Serial.println(" liter)");
      Serial.println("----------------------------------------\n");
      
    } else {
      Serial.println("⚠ Waiting... (check wiring if no data for >10s)");
    }
  }
}

// FIXED: Fungsi readDistance dengan logic yang benar
float readDistance() {
  // Clear buffer lama
  while (rs485.available()) {
    rs485.read();
  }
  
  // Tunggu data baru (timeout 1000ms)
  unsigned long startTime = millis();
  
  while (millis() - startTime < 1000) {
    // Tunggu minimal 4 byte
    if (rs485.available() >= 4) {
      byte header = rs485.read();
      
      // Cek header 0xFF
      if (header == 0xFF) {
        byte highByte = rs485.read();
        byte lowByte = rs485.read();
        byte checksum = rs485.read();
        
        // Validasi checksum
        byte calcChecksum = (0xFF + highByte + lowByte) & 0xFF;
        
        if (calcChecksum == checksum) {
          // Hitung jarak dalam mm
          int distanceMm = (highByte << 8) | lowByte;
          
          // FIXED: Konversi mm ke cm
          float distanceCm = distanceMm / 10.0;
          
          // Validasi range
          if (distanceCm >= MIN_DISTANCE && distanceCm <= MAX_DISTANCE) {
            return distanceCm;
          } else {
            Serial.print("⚠ Out of range: ");
            Serial.print(distanceCm);
            Serial.println(" cm");
          }
        } else {
          Serial.println("⚠ Checksum error");
          Serial.print("Expected: 0x");
          Serial.print(calcChecksum, HEX);
          Serial.print(", Got: 0x");
          Serial.println(checksum, HEX);
        }
      }
    }
    delay(10);  // Small delay untuk efisiensi CPU
  }
  
  // Timeout - tidak ada data valid
  return -1;
}

// ========== FUNGSI DEBUG (optional) ==========
// Uncomment jika perlu troubleshooting
/*
void debugRawData() {
  Serial.println("\n=== DEBUG MODE ===");
  Serial.println("Raw bytes from sensor:");
  
  unsigned long start = millis();
  while (millis() - start < 2000) {
    if (rs485.available()) {
      byte b = rs485.read();
      Serial.print("0x");
      if (b < 16) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    delay(10);
  }
  Serial.println("\n==================\n");
}
*/