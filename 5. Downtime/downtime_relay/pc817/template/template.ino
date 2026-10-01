/*
  V 0.0.2
  Update Terakhir : 01-10-2026
  Last Change Log {
    1. Memperbaiki pembacaan GPIO PC817
    2. Menggunakan array GPIO agar pin tidak diasumsikan berurutan
    3.
    4.
  }

  Komponen:
  1. ESP32 DOIT
  2. Optocoupler PC817 @4
  3. LED @1

  Program ini berfungsi menghitung instrumen listrik pada kabel.
*/

#include <esp_system.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#define PC817_PIN_1 27
#define PC817_PIN_2 26
#define PC817_PIN_3 25
#define PC817_PIN_4 33

const uint8_t PC817_PINS[4] = {
  PC817_PIN_1,
  PC817_PIN_2,
  PC817_PIN_3,
  PC817_PIN_4
};
int pinState[4];

WiFiMulti wifiMulti;
const char* ssid_a_biskuit_mie = "STTB8";
const char* password_a_biskuit_mie = "siantar123";
const char* ssid_b_biskuit_mie = "STTB1";
const char* password_b_biskuit_mie = "Si4nt4r321";
const char* ssid_c_biskuit_mie = "MT3";
const char* password_c_biskuit_mie = "siantar321";
const char* ssid_a_kerupuk = "STTB4";
const char* password_a_kerupuk = "siantar123";
const char* ssid_b_kerupuk = "MT1";
const char* password_b_kerupuk = "siantar321";
const char* ssid_c_kerupuk = "Amano2";
const char* password_c_kerupuk = "Si4nt4r321";
const char* ssid_it = "STTB11";
const char* password_it = "Si4nt4r321";

void setupPin_PC817() {
  for (int i = 0; i < 4; i++) {
    pinMode(PC817_PINS[i], INPUT_PULLUP);
  }
}

int readPin_PC817() {
  for (int i = 0; i < 4; i++) {
    pinState[i] = digitalRead(PC817_PINS[i]);
  }
  return 0;
}

void setup()
{
  Serial.begin(115200);
  setupPin_PC817();
}

void loop()
{
  // Baca semua input PC817
  readPin_PC817();

  // Tampilkan status
  for (int i = 0; i < 4; i++) {
    Serial.print(pinState[i]);

    if (i < 3) {
      Serial.print(" ");
    }
  }

  Serial.println();

  delay(100);
}
