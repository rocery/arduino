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
#define PZEM_ADDRESS 0x14

// Device Config
#define DEVICE_NAME "Molding Line 4 - Biskuit"
#define DEVICE_ID  194 // Isi dengan IP

// WiFi Configuration
const char* WIFI_NETWORKS[3][2] = {
  {"STTB1", "Si4nt4r321"},
  {"STTB2", "Si4nt4r321"},
  {"MT3", "siantar321"}
};
const int WIFI_NETWORKS_COUNT = 3;

// Network Configuration
IPAddress STATIC_IP(192, 168, 7, DEVICE_ID);
IPAddress GATEWAY(192, 168, 15, 250);
IPAddress SUBNET(255, 255, 0, 0);
IPAddress PRIMARY_DNS(8, 8, 8, 8);
IPAddress SECONDARY_DNS(8, 8, 4, 4);

// Server Configuration
const char* API_HOST = "192.168.7.223";
const char* API_STATUS_ENDPOINT = "/molding_api/saveStatus.php";
const char* API_LOG_ENDPOINT = "/molding_api/createFile.php";
const int API_PORT = 80;

// Timing Configuration
const unsigned long PZEM_READ_INTERVAL = 100;
const unsigned long DATA_SEND_INTERVAL = 15000;
const unsigned int RESET_COUNTER_LIMIT = 480;
const unsigned long WIFI_RETRY_TIMEOUT = 500;
const int WIFI_RETRY_LIMIT = 15;

// Voltage Threshold (dalam Volt)
const float VOLTAGE_THRESHOLD = 0.0;

// ============================================
// GLOBAL VARIABLES
// ============================================

SoftwareSerial pzemSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSerial, PZEM_ADDRESS);

struct ElectricalData {
  float voltage;
  float current;
  float power;
  float energy;
};

ElectricalData pzemData = {0, 0, 0, 0};
unsigned int sendDataCounter = 0;
unsigned long lastReadTime = 0;
unsigned long lastSendTime = 0;
String deviceIP = "";

// ============================================
// FUNCTION: LED Control
// ============================================

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