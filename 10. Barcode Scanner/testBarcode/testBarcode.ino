#include "HardwareSerial.h"

// Create a HardwareSerial object for GM66
HardwareSerial GM66Serial(2);

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);
  
  // Initialize GM66 serial communication
  // GM66Serial.begin(baud, protocol, RX pin, TX pin)
  GM66Serial.begin(9600, SERIAL_8N1, 16, 17);
  
  Serial.println("GM66 Barcode Scanner Ready");
  Serial.println("Waiting for scans...");
}

void loop() {
  // Check if data is available from GM66
  if (GM66Serial.available()) {
    String barcode = GM66Serial.readString();
    barcode.trim(); // Remove whitespace/newlines
    
    if (barcode.length() > 0) {
      Serial.println("Barcode scanned: " + barcode);
      
      // Process your barcode here
      processBarcode(barcode);
    }
  }
  
  delay(100);
}

void processBarcode(String code) {
  // Add your barcode processing logic here
  Serial.println("Processing: " + code);
  
  // Example: Send to web server, save to SD card, etc.
}