/*
 * ESP32 + MAX485 (4 pin auto direction) + A01NYUB V2 RS485
 * Code MINIMAL untuk test pengukuran tinggi
 * TANPA pin DE/RE (auto direction control)
 */

#include <HardwareSerial.h>

// Pin Configuration (HANYA RX dan TX)
#define RS485_RX 16
#define RS485_TX 17

// Tank Configuration
const float TANK_HEIGHT = 600.0;  // cm

// Global Variables
HardwareSerial rs485(1);
float distance = 0;
float height = 0;

void setup() {
  Serial.begin(115200);
  
  // Setup RS485 - TANPA DE/RE karena auto direction
  rs485.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  
  Serial.println("\n=== ESP32 RS485 Height Sensor ===");
  Serial.println("Modul: MAX485 4-pin (auto direction)");
  Serial.println("Waiting for sensor data...\n");
  
  delay(2000);
}

void loop() {
  distance = readDistance();
  
  if (distance > 0) {
    height = TANK_HEIGHT - distance;
    
    if (height < 0) height = 0;
    if (height > TANK_HEIGHT) height = TANK_HEIGHT;
    
    // Print hasil
    Serial.print("Jarak: ");
    Serial.print(distance, 1);
    Serial.print(" cm  |  Tinggi Minyak: ");
    Serial.print(height, 1);
    Serial.print(" cm  |  ");
    Serial.print((height/TANK_HEIGHT)*100, 1);
    Serial.println(" %");
    
  } else {
    Serial.println("Waiting... (check wiring if no data)");
  }
  
  delay(1000);
}

float readDistance() {
  // Clear buffer
  while (rs485.available()) {
    rs485.read();
  }
  
  // Tunggu data (sensor akan auto transmit setiap ~120ms)
  unsigned long start = millis();
  
  while (millis() - start < 500) {
    if (rs485.available() >= 4) {
      byte header = rs485.read();
      
      if (header == 0xFF) {
        byte dataH = rs485.read();
        byte dataL = rs485.read();
        byte checksum = rs485.read();
        
        // Validasi checksum
        byte calcChecksum = (0xFF + dataH + dataL) & 0xFF;
        
        if (checksum == calcChecksum) {
          // Hitung jarak (mm to cm)
          int distMm = (dataH << 8) | dataL;
          float distCm = distMm / 10.0;
          
          // Validasi range
          if (distCm >= 30 && distCm <= 750) {
            return distCm;
          } else {
            Serial.print("Out of range: ");
            Serial.println(distCm);
          }
        } else {
          Serial.println("Checksum error");
        }
      }
    }
    delay(10);
  }
  
  return -1;  // Timeout
}

// Fungsi DEBUG - uncomment jika perlu troubleshooting
/*
void debugRawData() {
  Serial.println("\n--- DEBUG MODE ---");
  Serial.println("Reading raw RS485 data:");
  
  for (int i = 0; i < 20; i++) {
    if (rs485.available()) {
      byte b = rs485.read();
      Serial.print("0x");
      if (b < 16) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    delay(50);
  }
  Serial.println("\n------------------\n");
}
*/