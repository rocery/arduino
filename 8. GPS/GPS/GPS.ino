/*
  V. 0.0.1 Alpa
  Update Terakhir : 26-06-2024

  Komponen:
  1. ESP32 Doit V1
  2. SIM 800L
  3. GPS Neo-8M

  Algoritma:
  Setup:
    Setup SIM -> Setup GPS
  Main:
    Pembacaan GPS -> Waktu kirim ? pembacaan GPS, kirim data
*/

// ============ INISIALISASI LIBRARY ============
#include <HardwareSerial.h>
#include <TinyGPS++.h>

// ============ GPS ============
TinyGPSPlus gps;

// ============ Hardware Serial ============
// Gunakan Hardware Serial nomor 2 pada ESP32 yaitu pin 16, 17
HardwareSerial simSerial(2);

/**
 * Sets up the hardware and initializes the SIM800L module.
 *
 * This function initializes the serial communication with the ESP32 and
 * configures the SIM800L module. It sets the data rate to 9600 baud and
 * uses the SERIAL_8N1 mode. The RX pin is connected to pin 17 and the TX pin
 * is connected to pin 16.
 */
void setup() {
  // Initialize serial communication with the ESP32
  Serial.begin(115200);

  // Initialize serial communication with the SIM800L module
  // Set the data rate to 9600 baud and use the SERIAL_8N1 mode
  // Pin RX SIM800L is connected to pin 17
  // Pin TX SIM800L is connected to pin 16
  simSerial.begin(9600, SERIAL_8N1, 16, 17);

  // Initialize the SIM800L module
  sim800Linit();
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

/**
 * Initializes the SIM800L module.
 *
 * This function sends a series of AT commands to the SIM800L module to initialize it.
 * The commands check the SIM card status, registration status, attach status,
 * shut down the IP connection, set the multiplexer mode to 0,
 * set the APN, bring up the IP connection, and get the local IP address.
 */
void sim800lInit() {
  // Check SIM card status
  sendCommand("AT", 1000, "OK");
  // Check SIM card readiness
  sendCommand("AT+CPIN?", 1000, "READY");
  // Check network registration status
  sendCommand("AT+CREG?", 1000, "OK");
  // Check attach status
  sendCommand("AT+CGATT?", 1000, "OK");
  // Shut down the IP connection
  sendCommand("AT+CIPSHUT", 1000, "SHUT OK");
  // Set the multiplexer mode to 0
  sendCommand("AT+CIPMUX=0", 1000, "OK");
  // Set the access point name (APN), username, and password
  sendCommand("AT+CSTT=\"your_apn\",\"your_username\",\"your_password\"", 1000, "OK");
  // Bring up the IP connection
  sendCommand("AT+CIICR", 1000, "OK");
  // Get the local IP address
  sendCommand("AT+CIFSR", 1000, ".");
}
