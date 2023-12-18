#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

// == WiFi Config ==
ESP8266WiFiMulti wifiMulti;
const char* ssid_a = "STTB1";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "MT1";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
const char* ssid_d = "STTB4";
const char* password_d = "siantar123";
const char* ssid_it = "STTB5";
const char* password_it = "siantar123";

// Set your Static IP address
IPAddress staticIP(192, 168, 15, 215);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

void setup() {
  Serial.begin(9600);

  wifiMulti.addAP("ssid_a", "password_a");
  wifiMulti.addAP("ssid_b", "password_b");
  wifiMulti.addAP("ssid_c", "password_c");
  wifiMulti.addAP("ssid_d", "password_d");
  wifiMulti.addAP("ssid_it", "password_it");

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
  }
}

void loop() {
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("Connected");
  } else {
    Serial.println("Disconnected");
  }
}
