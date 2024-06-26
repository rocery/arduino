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

void setup() {
  Serial.begin(115200);

  // Inialisasi Hardware Serial yang digunakan SIM800L (RX, TX)
  // Pin RX SIM800L => 17, Pin TX SIM800L => 16
  simSerial.begin(9600, SERIAL_8N1, 16, 17);
  sim800Linit();
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

void sim800lInit() {
  sendCommand("AT", 1000, "OK");
  sendCommand("AT+CPIN?", 1000, "READY");
  sendCommand("AT+CREG?", 1000, "OK");
  sendCommand("AT+CGATT?", 1000, "OK");
  sendCommand("AT+CIPSHUT", 1000, "SHUT OK");
  sendCommand("AT+CIPMUX=0", 1000, "OK");
  sendCommand("AT+CSTT=\"your_apn\",\"your_username\",\"your_password\"", 1000, "OK");
  sendCommand("AT+CIICR", 1000, "OK");
  sendCommand("AT+CIFSR", 1000, ".");
}
