#include "HardwareSerial.h"
#include <LiquidCrystal_I2C.h>

// Create a HardwareSerial object for GM66
HardwareSerial GM66Serial(2);

// Create a LiquidCrystal_I2C object for the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);
  
  // Initialize GM66 serial communication
  // GM66Serial.begin(baud, protocol, RX pin, TX pin)
  GM66Serial.begin(9600, SERIAL_8N1, 16, 17);
  
  Serial.println("GM66 Barcode Scanner Ready");
  Serial.println("Waiting for scans...");

  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.print("Waiting for scans...");
}

void loop() {
  // Check if data is available from GM66
  while (GM66Serial.available()) {
    String barcode = GM66Serial.readString();
    //Serial.println(barcode);
    processBarcode(barcode);
  }
}

void processBarcode(String code) {
  // Add your barcode processing logic here
  Serial.println("Processing: " + code);
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanned: " + code);
  // Example: Send to web server, save to SD card, etc.
}