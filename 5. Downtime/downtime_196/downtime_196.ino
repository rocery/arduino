/*
  V 0.9.3
  Update Terakhir : 19-08-2026
  Last Change Log {
    1. Added machine state change detection for immediate data sending
  }

  Komponen:
  1. NodeMCU ESP8266 V.3
  2. PZEM-004T V 3.0
  3. LED @1

  Program ini berfungsi menghitung instrumen listrik pada kabel.
  Projek ini mengunakan ESP8266 sebagai microcontroller karena memiliki fitur SerialSoftware,
  sehingga komunikasi UART bisa digunakan sebanyak mungkin selama address-nya berbeda dan daya
  dari ESP8266 kuat.

  Proses koneksi WiFi tidak disarakn menggunakan Library MultiWiFiESP8266.h, kecuali Micon yang digunakan ESP32
*/

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
// Pin LED VCC ke pin 1
#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 13
#define PZEM_ADDRESS 0x15
#define LED_PIN 5

// Device Config
#define DEVICE_NAME "Downtime 196"
#define DEVICE_ID  196 // Isi dengan IP

// WiFi Configuration
const char* WIFI_NETWORKS[3][2] = {
  {"STTB4", "siantar123"},
  {"MT3", "Si4rt4r321"},
  {"STTB2", "siantar321"}
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

bool machineWasOn = false;
bool machineIsOn = false;
unsigned long stateChangeDebounceTime = 0;
const unsigned long STATE_CHANGE_DEBOUNCE = 2000;

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

// ============================================
// FUNCTION: Read PZEM Sensors
// ============================================

bool readPZEMData() {
  pzemData.voltage = pzem.voltage();
  pzemData.current = pzem.current();
  pzemData.power = pzem.power();
  pzemData.energy = pzem.energy();

  if (isnan(pzemData.voltage) || isnan(pzemData.current) || 
      isnan(pzemData.power) || isnan(pzemData.energy)) {
    Serial.println("[ERROR] PZEM read failed");
    return false;
  }

  return true;
}

// ============================================
// FUNCTION: Print PZEM Data
// ============================================

void printPZEMData() {
  Serial.println("=== PZEM Data ===");
  
  Serial.print("Voltage: ");
  Serial.print(pzemData.voltage, 2);
  Serial.println(" V");
  
  Serial.print("Current: ");
  Serial.print(pzemData.current, 2);
  Serial.println(" A");
  
  Serial.print("Power: ");
  Serial.print(pzemData.power, 2);
  Serial.println(" W");
  
  Serial.print("Energy: ");
  Serial.print(pzemData.energy, 4);
  Serial.println(" kWh");
  
  Serial.println();
}

// ============================================
// FUNCTION: HTTP POST Request
// ============================================

bool sendHTTPRequest(const char* endpoint, const String& postData) {
  HTTPClient http;
  WiFiClient client;

  String url = String("http://") + API_HOST + endpoint;
  
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setTimeout(5000);

  int httpCode = http.POST(postData);
  
  Serial.print("[HTTP] POST to ");
  Serial.print(endpoint);
  Serial.print(" - Response: ");
  Serial.println(httpCode);

  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.print("[HTTP] Response: ");
    Serial.println(response);
    http.end();
    return true;
  } else {
    Serial.print("[ERROR] HTTP Error: ");
    Serial.println(httpCode);
    http.end();
    return false;
  }
}

// ============================================
// FUNCTION: Send Status Data
// ============================================

bool sendStatusData() {
  String postData = "voltage=" + String(pzemData.voltage, 2) +
                    "&device_name=" + String(DEVICE_NAME) +
                    "&ip_address=" + deviceIP;

  Serial.println("[SEND] Status Data: " + postData);
  return sendHTTPRequest(API_STATUS_ENDPOINT, postData);
}

// ============================================
// FUNCTION: Send Log Data
// ============================================

bool sendLogData() {
  String postData = "power=" + String(pzemData.power, 2) +
                    "&energy=" + String(pzemData.energy, 4) +
                    "&voltage=" + String(pzemData.voltage, 2) +
                    "&current=" + String(pzemData.current, 2) +
                    "&ip_address=" + deviceIP;

  Serial.println("[SEND] Log Data: " + postData);
  return sendHTTPRequest(API_LOG_ENDPOINT, postData);
}

// ============================================
// FUNCTION: Detect Machine State Change
// ============================================

bool detectMachineStateChange() {
  unsigned long currentTime = millis();
  
  machineIsOn = (pzemData.voltage > VOLTAGE_THRESHOLD);
  
  if (machineIsOn != machineWasOn) {
    if (currentTime - stateChangeDebounceTime >= STATE_CHANGE_DEBOUNCE) {
      machineWasOn = machineIsOn;
      stateChangeDebounceTime = currentTime;
      
      Serial.print("[STATE] Machine ");
      Serial.println(machineIsOn ? "TURNED ON" : "TURNED OFF");
      
      return true;
    }
  }
  
  return false;
}

// ============================================
// FUNCTION: WiFi Connection
// ============================================

bool connectToWiFi() {
  Serial.println("\n[WIFI] Starting connection...");

  if (!WiFi.config(STATIC_IP, GATEWAY, SUBNET, PRIMARY_DNS, SECONDARY_DNS)) {
    Serial.println("[ERROR] Failed to configure static IP");
    return false;
  }

  WiFi.mode(WIFI_STA);
  int scanResult = WiFi.scanNetworks();

  if (scanResult == 0) {
    Serial.println("[ERROR] No WiFi networks found");
    return false;
  }

  int retries = 0;
  int networkIndex = 0;

  while ((WiFi.status() != WL_CONNECTED) && (retries < WIFI_RETRY_LIMIT)) {
    for (int i = 0; i < scanResult; i++) {
      String scannedSSID = WiFi.SSID(i);
      
      for (int j = 0; j < WIFI_NETWORKS_COUNT; j++) {
        if (scannedSSID == WIFI_NETWORKS[j][0]) {
          Serial.print("[WIFI] Attempting to connect to: ");
          Serial.println(WIFI_NETWORKS[j][0]);
          
          WiFi.begin(WIFI_NETWORKS[j][0], WIFI_NETWORKS[j][1]);
          
          for (int k = 0; k < 20; k++) {
            if (WiFi.status() == WL_CONNECTED) {
              break;
            }
            delay(250);
            Serial.print(".");
          }
          
          if (WiFi.status() == WL_CONNECTED) {
            break;
          }
        }
      }
    }

    if (WiFi.status() != WL_CONNECTED) {
      retries++;
      delay(WIFI_RETRY_TIMEOUT);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    deviceIP = WiFi.localIP().toString();
    Serial.println("\n[WIFI] Connected!");
    Serial.print("[WIFI] IP Address: ");
    Serial.println(deviceIP);
    blinkLED(3, 200);
    return true;
  } else {
    Serial.println("\n[ERROR] WiFi connection failed");
    blinkLED(5, 100);
    return false;
  }
}

// ============================================
// FUNCTION: System Check
// ============================================

void systemCheck() {
  Serial.println("\n=== SYSTEM CHECK ===");
  Serial.print("Device Name: ");
  Serial.println(DEVICE_NAME);
  Serial.print("Device ID: ");
  Serial.println(DEVICE_ID);
  Serial.print("PZEM Address: 0x");
  Serial.println(PZEM_ADDRESS, HEX);
  Serial.print("API Host: ");
  Serial.println(API_HOST);
  Serial.print("WiFi Status: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  Serial.println();
}

// ============================================
// SETUP
// ============================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP8266 PZEM Template Starting ===");
  
  pzemSerial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  setLED(false);

  systemCheck();

  if (!connectToWiFi()) {
    Serial.println("[WARNING] Proceeding without WiFi");
  }

  delay(1000);
}

// ============================================
// LOOP
// ============================================

void loop() {
  unsigned long currentTime = millis();

  // Read PZEM data
  if (currentTime - lastReadTime >= PZEM_READ_INTERVAL) {
    lastReadTime = currentTime;
    
    if (readPZEMData()) {
      printPZEMData();
      
      if (pzemData.voltage > VOLTAGE_THRESHOLD) {
        setLED(true);
      } else {
        setLED(false);
      }
      
      // Detect machine state change and send immediately
      if (detectMachineStateChange()) {
        if (WiFi.status() == WL_CONNECTED) {
          bool statusSent = sendStatusData();
          delay(500);
          bool logSent = sendLogData();
          
          if (statusSent && logSent) {
            sendDataCounter++;
            Serial.print("[SYNC] State change data sent. Counter: ");
            Serial.println(sendDataCounter);
          }
        } else {
          Serial.println("[WARNING] WiFi disconnected, attempting reconnection...");
          connectToWiFi();
        }
      }
    }
  }

  // Send data to server at regular interval
  if (currentTime - lastSendTime >= DATA_SEND_INTERVAL) {
    lastSendTime = currentTime;

    if (WiFi.status() == WL_CONNECTED) {
      bool statusSent = sendStatusData();
      delay(500);
      bool logSent = sendLogData();
      
      if (statusSent && logSent) {
        sendDataCounter++;
        Serial.print("[SYNC] Interval data sent successfully. Counter: ");
        Serial.println(sendDataCounter);
      }
    } else {
      Serial.println("[WARNING] WiFi disconnected, attempting reconnection...");
      connectToWiFi();
    }

    // Reset system after reaching limit
    if (sendDataCounter >= RESET_COUNTER_LIMIT) {
      Serial.print("[SYSTEM] Resetting after ");
      Serial.print(RESET_COUNTER_LIMIT);
      Serial.println(" successful sends");
      delay(1000);
      ESP.reset();
    }
  }
}
