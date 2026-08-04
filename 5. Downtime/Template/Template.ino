#include <PZEM004Tv30.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

// ============================================
// CONFIGURATION SECTION
// ============================================

// Digunakan pin 12 dan 13 (6, 7) sebagai pin UART (RX, TX)
// Pin TX dari PZEM dihubungkan ke pin (6) ESP8266
// Pin RX dari PZEM dihubungkan ke pin (7) ESP8266
#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 13
#define LED_PIN 2

// PZEM Config