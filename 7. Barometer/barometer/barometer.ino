/*
  V. 0.0.1
  Update Terakhir : 02-04-2024

  Komponen:
  1. ESP32
  2. Sensor Pressure Transmitter 5V DC G 1/4"
  3. LCD I2C 16x2
  4. 7 Segment TM1637

  Program ini  berfungsi untuk mengukur tekanan angin secara real-time.
*/

// ===== LIBRARY ======
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <TM1637Display.h>

String ESPName = "Barometer | Teknik-Kerupuk";
String deviceID = "";

/* Mendeklarasikan LCD dengan alamat I2C 0x27
Total kolom 16
Total baris 2 */
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== NETWORK CONFIG =====
/* Deklarasikan semua WiFi yang bisa diakses oleh ESP32
ESP32 akan memilih WiFi dengan sinyal paling kuat secara otomatis
Gunakan tidak lebih dari 3 WiFi (WiFi Utama, WiFi Cadangan, WiFi Test)
*/
WiFiMulti wifiMulti;
const char* ssid_a = "STTB1";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "MT1";
const char* password_b = "siantar321";
const char* ssid_c = "Tester_ITB";
const char* password_c = "Si4nt4r321";

// Set IP to Static
IPAddress staticIP(192, 168, 7, 251);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional
String ip_Address;

// ===== NTP =====
const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat;
int year, month, day, hour, minute, second;
bool ntpStatus;

// ===== Data Send/Get =====
bool sendStatus, getStatus;
const char* counterFromDB;
int counterValueDB;
String postData, api;

void getLocalTime() {
  /* Fungsi bertujuan menerima update waktu
  lokal dari ntpServer */
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    ntpStatus = false;
  } else {
    ntpStatus = true;

    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    dateTime = String(timeStringBuff);

    // Save time data to variabel
    year = timeinfo.tm_year + 1900;
    month = timeinfo.tm_mon + 1;
    day = timeinfo.tm_mday;
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    second = timeinfo.tm_sec;
    // YYYY-MM-DD
    dateFormat = String(year) + '-' + String(month) + '-' + String(day);
    // hh:mm:ss
    timeFormat = String(hour) + ':' + String(minute) + ':' + String(second);
  }
}

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
