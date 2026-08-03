/*
  V 0.9.1
  Update Terakhir : 03-08-2026
  Last Change Log {
    1. 
  }

  Komponen:
  1. NodeMCU ESP8266 V.3
  2. PZEM-004T V 3.0
  3. LED

  Program ini berfungsi menghitung instrumen listrik pada kabel.
  Projek ini mengunakan ESP8266 sebagai micro controller karena memiliki fitur SerialSoftware, sehingga komunikasi UART bisa digunakan sebanyak mungkin selama address-nya berbeda dan daya dari ESP8266 kuat.

  Proses koneksi WiFi tidak disarakn menggunakan Library MultiWiFiESP8266.h, kecuali Micon yang digunakan ESP32
*/

/*
  Template ESP8266 + PZEM-004T v3.0
  - Koneksi WiFi dengan multiple SSID
  - Pembacaan daya, energi, voltase, arus
  - Pengiriman data ke server via HTTP POST
  - LED indikator status
  - Watchdog reset otomatis setelah 2 jam (atau sesuai kebutuhan)
*/

#include <PZEM004Tv30.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <math.h>

// ===== KONFIGURASI PIN =====
#define PZEM_RX_PIN 12   // D6 (RX ke TX PZEM)
#define PZEM_TX_PIN 13   // D7 (TX ke RX PZEM)
#define LED_PIN     5    // D1 (LED indikator)

// ===== KONFIGURASI WIFI =====
// Daftar SSID & password (coba secara berurutan)
const char* ssids[] = {
  "STTB1",
  "STTB11",
  "MT3"
};
const char* passwords[] = {
  "Si4nt4r321",
  "Si4nt4r321",
  "siantar321"
};
const int NUM_NETWORKS = sizeof(ssids) / sizeof(ssids[0]);

// Konfigurasi IP statis
IPAddress staticIP(192, 168, 7, 193);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// ===== KONFIGURASI SERVER =====
const char* serverURL_save = "http://192.168.7.223/molding_api/saveStatus.php";
const char* serverURL_log   = "http://192.168.7.223/molding_api/createFile.php";

// ===== IDENTITAS PERANGKAT =====
String deviceName = "Roller Line 2 - Mie"; // Nama unik untuk ESP ini

// ===== VARIABEL GLOBAL =====
SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial, 0x15); // Alamat sensor (sesuai hasil perubahan)

float voltage, current, power, energy;
String ipAddress;

unsigned long previousMillis = 0;
const long intervalSend = 15000;      // Kirim data tiap 15 detik
const long intervalRead = 500;        // Baca sensor tiap 500 ms (tapi diambil saat kirim)

int sendCounter = 0;
const int maxSendCount = 480;         // Reset setelah ~2 jam (480 * 15 detik)

bool wifiConnected = false;

// ===== PROTOTIPE FUNGSI =====
bool connectToWiFi();
void readPZEM();
bool sendData(float voltage, float current, float power, float energy, String ip, String device, const char* url, bool isLog);
void blinkLED(int times, int onTime, int offTime);

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  pzemSWSerial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Konfigurasi IP statis
  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Gagal konfigurasi IP statis!");
  }

  // Koneksi WiFi
  wifiConnected = connectToWiFi();
  if (wifiConnected) {
    ipAddress = WiFi.localIP().toString();
    Serial.print("IP Address: ");
    Serial.println(ipAddress);
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("WiFi gagal terhubung, coba ulang nanti.");
    blinkLED(5, 200, 200);
  }
}

// ===== LOOP =====
void loop() {
  unsigned long currentMillis = millis();

  // Cek koneksi WiFi secara berkala (setiap 5 detik)
  if (currentMillis - previousMillis >= 5000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi terputus, mencoba koneksi ulang...");
      wifiConnected = connectToWiFi();
      if (wifiConnected) {
        ipAddress = WiFi.localIP().toString();
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
        blinkLED(3, 300, 300);
      }
    }
    previousMillis = currentMillis; // update untuk pengecekan WiFi
  }

  // Kirim data setiap intervalSend
  static unsigned long lastSendTime = 0;
  if (currentMillis - lastSendTime >= intervalSend) {
    lastSendTime = currentMillis;

    // Baca sensor (sekaligus)
    readPZEM();

    // Tampilkan ke Serial
    Serial.println("=== Pembacaan Sensor ===");
    Serial.print("Voltage: "); Serial.print(voltage); Serial.println(" V");
    Serial.print("Current: "); Serial.print(current); Serial.println(" A");
    Serial.print("Power: ");   Serial.print(power);   Serial.println(" W");
    Serial.print("Energy: ");  Serial.print(energy);  Serial.println(" kWh");
    Serial.println();

    // Kirim data jika WiFi terhubung dan pembacaan valid
    if (wifiConnected && !isnan(voltage) && voltage > 0) {
      // Kirim ke log (createFile.php)
      bool successLog = sendData(voltage, current, power, energy, ipAddress, deviceName, serverURL_log, true);
      // Kirim ke status (saveStatus.php)
      bool successStatus = sendData(voltage, current, power, energy, ipAddress, deviceName, serverURL_save, false);

      if (successLog && successStatus) {
        digitalWrite(LED_PIN, HIGH);
        sendCounter++;
        Serial.print("Counter: "); Serial.println(sendCounter);
      } else {
        Serial.println("Gagal mengirim data, coba lagi nanti.");
        blinkLED(2, 100, 100);
      }
    } else {
      Serial.println("WiFi tidak terhubung atau data sensor tidak valid, lewati pengiriman.");
      blinkLED(4, 150, 150);
    }

    // Reset ESP setelah mencapai batas kirim
    if (sendCounter >= maxSendCount) {
      Serial.println("Mencapai batas kirim, mereset ESP...");
      ESP.reset();
    }
  }

  // Tugas lain bisa ditambahkan di sini (non-blocking)
}

// ===== FUNGSI KONEKSI WIFI =====
bool connectToWiFi() {
  WiFi.mode(WIFI_STA);
  int attempt = 0;
  const int maxAttempts = 3; // coba 3 kali untuk setiap SSID

  for (int i = 0; i < NUM_NETWORKS; i++) {
    Serial.print("Mencoba SSID: ");
    Serial.println(ssids[i]);
    WiFi.begin(ssids[i], passwords[i]);

    int tries = 0;
    while (tries < maxAttempts * 5) { // timeout ~15 detik (3 kali * 5 * 1000ms)
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Berhasil terhubung!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
      }
      delay(1000);
      tries++;
      Serial.print(".");
    }
    Serial.println();
    Serial.println("Gagal konek ke SSID ini, coba berikutnya...");
  }

  Serial.println("Semua SSID gagal.");
  return false;
}

// ===== FUNGSI BACA SENSOR =====
void readPZEM() {
  voltage = pzem.voltage();
  current = pzem.current();
  power   = pzem.power();
  energy  = pzem.energy();

  // Jika voltase > 0, data dianggap valid, LED menyala.
  // Jika voltase 0 atau NaN, LED berkedip dan set nilai ke NAN (opsional).
  if (isnan(voltage) || voltage <= 0) {
    Serial.println("Pembacaan PZEM gagal (voltase tidak valid).");
    // Set semua ke NAN untuk menghindari pengiriman data salah
    voltage = NAN;
    current = NAN;
    power   = NAN;
    energy  = NAN;
    blinkLED(2, 200, 200);
  } else {
    // LED menyala normal saat data valid
    digitalWrite(LED_PIN, HIGH);
  }
}

// ===== FUNGSI KIRIM DATA =====
bool sendData(float voltage, float current, float power, float energy,
              String ip, String device, const char* url, bool isLog) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi tidak terhubung, tidak bisa kirim data.");
    return false;
  }

  HTTPClient http;
  WiFiClient client;
  String postData;

  if (isLog) {
    // Untuk createFile.php
    postData = "power=" + String(power) +
               "&energy=" + String(energy, 4) +
               "&voltage=" + String(voltage) +
               "&current=" + String(current) +
               "&ip_address=" + ip;
  } else {
    // Untuk saveStatus.php
    postData = "voltage=" + String(voltage) +
               "&device_name=" + device +
               "&ip_address=" + ip;
  }

  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(postData);
  Serial.print("POST ke "); Serial.println(url);
  Serial.println("Data: " + postData);

  bool success = false;
  if (httpCode == 200) {
    Serial.println("HTTP 200 OK");
    String response = http.getString();
    Serial.println("Respon: " + response);
    success = true;
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpCode);
  }

  http.end();
  return success;
}

// ===== FUNGSI BLINK LED =====
void blinkLED(int times, int onTime, int offTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(onTime);
    digitalWrite(LED_PIN, LOW);
    delay(offTime);
  }
}