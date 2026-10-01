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

const int DEVICE_ID = 196;
const String LOCATION = "Mesin 43 - Kerupuk";
const String PRODUCTION = "Kerupuk";

#define PC817_PIN_1 27
#define PC817_PIN_2 26
#define PC817_PIN_3 25
#define PC817_PIN_4 33
#define LED_PIN 34

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

IPAddress STATIC_IP(192, 168, 7, DEVICE_ID);
IPAddress GATEWAY(192, 168, 15, 250);
IPAddress SUBNET(255, 255, 0, 0);
IPAddress PRIMARY_DNS(8, 8, 8, 8);
IPAddress SECONDARY_DNS(8, 8, 4, 4);

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

void setLED(bool state) {
  digitalWrite(LED_PIN, state ? HIGH : LOW);
}

void blinkLED(int times, int duration) {
  for (int i = 0; i < times; i++) {
    setLED(true);
    delay(duration);
    setLED(false);
    delay(duration);
  }
}

bool setupWiFi() {
  if (PRODUCTION == "Biskuit" || PRODUCTION == "Mie") {
    wifiMulti.addAP(ssid_a_biskuit_mie, password_a_biskuit_mie);
    wifiMulti.addAP(ssid_b_biskuit_mie, password_b_biskuit_mie);
    wifiMulti.addAP(ssid_c_biskuit_mie, password_c_biskuit_mie);
  } else if (PRODUCTION == "Kerupuk") {
    wifiMulti.addAP(ssid_a_kerupuk, password_a_kerupuk);
    wifiMulti.addAP(ssid_b_kerupuk, password_b_kerupuk);
    wifiMulti.addAP(ssid_c_kerupuk, password_c_kerupuk);
  } else {
    return false;
  }
  wifiMulti.addAP(ssid_it, password_it);

  if (!WiFi.config(STATIC_IP, GATEWAY, SUBNET, PRIMARY_DNS, SECONDARY_DNS)) {
    Serial.println("STA Failed to configure");
    return false;
  }

  return true;
}

bool connectWiFi() {
    setupWiFi();
    if (wifiMulti.run() != WL_CONNECTED) {
      return false;
    }
    return true;
}

void setup()
{
  Serial.begin(115200);
  setupPin_PC817();
  pinMode(LED_PIN, OUTPUT);

  int tryWiFi = 0;
  while (!connectWiFi() && tryWiFi < 10) {
    tryWiFi++;
    blinkLED(2, 500);
  }

  if (tryWiFi == 10) {
    blinkLED(5, 500);
  } else {
    blinkLED(1, 500);
  }

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
