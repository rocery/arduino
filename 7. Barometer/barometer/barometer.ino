/*
  V. 0.0.2 Alfa
  Update Terakhir : 16-04-2024

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
#include <Wire.h>
#include <Arduino.h>
#include <TM1637Display.h>

String ESPName = "Barometer | Teknik-Kerupuk";
String deviceID = "IoT-251-TK0532";

// ===== PRESSURE SENSOR =====
const int pressureInput = 13;    // Pin pada mikrokontroller yang digunakan
const int pressureZero = 102.4;  // Nilai analogread() pada kondisi 0 psi
const int pressureMax = 921.6;   // Nilai analogread() pada kondisi maksimum psi
const int pressuremaxPSI = 100;  // Nilai maksimal sensor yang digunakan
float pressureValue = 0;
float barValue = 0;
const float psiToBar = 14.5037738;
int readPressureCoutner = 0;

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
int sendCounter = 0;
String postData, api = "http://192.168.7.223/barometer_api/save_tekanan.php";

// ===== TM1637 =====
// Module connection pins (Digital Pins)
#define CLK 5
#define DIO 18
TM1637Display tm1637(CLK, DIO);

void readPressure() {
  pressureValue = analogRead(pressureInput);
  pressureValue = ((pressureValue - pressureZero) * pressuremaxPSI) / (pressureMax - pressureZero);
  barValue = pressureValue / psiToBar;
  Serial.print(pressureValue, 1);
  Serial.println("psi");
  Serial.print(barValue, 1);
  Serial.println("bar");
  readPressureCoutner++;
}

void testTM1637() {
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };
  tm1637.setBrightness(0x0f);

  tm1637.setSegments(data);
  delay(1000);
  tm1637.clear();
}

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

void sendLogData() {
  /* Mengirim data ke local server
  Ganti isi variabel api sesuai dengan form php
  */
  HTTPClient http;
  http.begin(api);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("ERROR ON SENDING POST");
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  sendStatus = false;
  getStatus = false;
  ntpStatus = false;

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  Serial.println("Try Connect to WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Connecting..");
  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  } else {
    Serial.println("STA Success");
  }

  wifiMulti.run();

  // NTP
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);

  if (wifiMulti.run() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("WiFi Fail");
    lcd.setCursor(0, 2);
    lcd.print("Date/Time Error");
    delay(3000);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected");

    int tryNTP = 0;
    while (ntpStatus == false && tryNTP <= 2) {
      getLocalTime();
      tryNTP++;
      delay(50);
      lcd.setCursor(0, 2);
      lcd.print("Getting Time");
    }
  }

  testTM1637();
  lcd.clear();
}

void loop() {
  readPressure();

  if (readPressureCoutner % 30 == 0) {
    sendLogData();
  }

  if (sendCounter % 60 == 0) {
    ESP.reset();
  }

  delay(1000);
}
