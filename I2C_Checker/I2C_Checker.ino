/*
  Program ini bertujuan mencari address I2C pada sensor atau komponen yang ada.
  Sambungkan VCC GND SCL SDA secara benar.
  address ini nantinya bisa digunakan untuk melakukan pembacaan sensor/output.
  Program ini bisa digunakan pada seluruh board yang didukung oleh Arduino IDE.
*/
#include <Wire.h>

// Set I2C bus to use: Wire, Wire1, etc.
#define WIRE Wire

void setup() {
  WIRE.begin();

  Serial.begin(115200);
  while (!Serial)
    delay(10);
  Serial.println("\nI2C Scanner");
}

/**
 * Scans the I2C bus for devices and prints the addresses of those found.
 * 
 * This function uses the I2C bus to scan for devices and prints the addresses
 * of those that respond to the scan request. Devices that respond with an
 * error code of 0 indicate that they are present at the specified address.
 * Devices that respond with an error code of 4 indicate that an unknown error
 * occurred when attempting to communicate with that address.
 * 
 * This function will continue to scan for devices every 5 seconds until it is
 * stopped manually.
 */
void loop() {
  // Initialize variables
  byte error;  // Error code returned by the I2C bus
  byte address;  // Address of the device being scanned
  int nDevices;  // Number of devices found

  // Print initial message
  Serial.println("Scanning...");

  // Set initial value for number of devices found
  nDevices = 0;

  // Loop through all possible addresses on the I2C bus
  for (address = 1; address < 127; address++) {
    // Try to communicate with the device at the current address
    WIRE.beginTransmission(address);
    error = WIRE.endTransmission();

    // Check the error code returned by the I2C bus
    if (error == 0) {
      // Device responded successfully
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      // Increment the number of devices found
      nDevices++;
    } else if (error == 4) {
      // Unknown error occurred
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  // Print final message based on number of devices found
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("done\n");
  }

  // Wait for 5 seconds before starting the next scan
  delay(5000);
}
