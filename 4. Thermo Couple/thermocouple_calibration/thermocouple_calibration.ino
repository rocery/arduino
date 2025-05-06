/*
    V. 0.0.1 Alpha
    17-04-2025

    Versi ini dibuat dengan alasan:
    1. Tidak menggunakan potesiometer sebagai alat kalibrasi
    2. Kalibrasi dilakukan memlalui value yang diatur database

*/

// == Library ==
#include <max6675.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const String deviceName = "Suhu Oven 27 - Kerupuk";
const String api_sendLogData = "http://192.168.7.223/temperature_api/saveTemperature.php";
const String api_sendLogData2 = "http://192.168.7.223/temperature_api/saveTemperature15Minutes.php";
const String api_getCalibrationValue = "http://192.168.7.223/temperature_api/getCalibrationValue.php";
const String deviceID = "27";

/*
    Thermocouple Type K
    MAX6675 Pin declaration
*/
#define thermoSO 19
#define thermoCS 18
#define thermoSCK 5
MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);
float temperature, calibrationValue, tempValue;
bool calibrationStatus = false;

/* Mendeklarasikan LCD dengan alamat I2C 0x27
   Total kolom 20
   Total baris 4 */
   LiquidCrystal_I2C lcd(0x27, 16, 2);
   uint8_t degree[8] = {
     0x08,
     0x14,
     0x14,
     0x08,
     0x00,
     0x00,
     0x00,
     0x00
   };

// == WiFi Config ==
/* Deklarasikan semua WiFi yang bisa diakses oleh ESP32
   ESP32 akan memilih WiFi dengan sinyal paling kuat secara otomatis */
   WiFiMulti wifiMulti;
   const char* ssid_a = "STTB4";
   const char* password_a = "siantar123";
   const char* ssid_b = "Amano2";
   const char* password_b = "Si4nt4r321";
   const char* ssid_c = "MT1";
   const char* password_c = "siantar321";
   const char* ssid_it = "STTB11";
   const char* password_it = "Si4nt4r321";

// Set IP to Static
IPAddress staticIP(192, 168, 7, 127);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
String ip_Address;

// == Data Send/Get ==
String postData;
bool sendStatus;

// == Get NTP/RTC ==
const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;  // Karena Bekasi ada di GMT+7, maka Offset ditambah 7 jam
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat, timeLCD;
int year, month, day, hour, minute, second;
bool ntpStatus;

void sendLogData() {
    HTTPClient http;
    http.begin(api_sendLogData);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    Serial.println(postData);
    int httpResponseCode = http.POST(postData);
    String response = http.getString();

    if (httpResponseCode > 0) {
      Serial.println(response);
    } else {
      Serial.println("Error Sending POST");
      Serial.println(response);
      Serial.println(httpResponseCode);
    }

    http.end();
}

void getLocalTime() {
  /* Fungsi bertujuan menerima update waktu
     lokal dari ntp.pool.org */
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    ntpStatus = false;
  } else {
    ntpStatus = true;

    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    dateTime = String(timeStringBuff);

    char timeStringBuff2[50];
    strftime(timeStringBuff2, sizeof(timeStringBuff2), "%d-%m %H:%M", &timeinfo);
    timeLCD = String(timeStringBuff2);

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
    timeFormat = String(hour) + ':' + String(minute);
    //
    // timeLCD = String(day) + '-' + String(month) + ' ' + String(hour) + ':' + String(minute);
  }
}

void setup() {
  Serial.begin(115200);

  sendStatus = false;
  ntpStatus = false;
  calibrationValue = 0;

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.createChar(0, degree);
  lcd.home();

  // WiFi
  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);
  wifiMulti.addAP(ssid_it, password_it);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  wifiMulti.run();
  lcd.setCursor(0, 0);
  lcd.print("Connecting");
  
  // NTP
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  if (wifiMulti.run() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    int tryNTP = 0;
    while (ntpStatus == false && tryNTP < 2) {
      lcd.setCursor(0, 1);
      lcd.print("Getting Date");
      getLocalTime();
      tryNTP++;
    }
  }
  lcd.clear();
}

void loop() {
  // Read calibration value 1x
  if (calibrationStatus && wifiMulti.run() == WL_CONNECTED) {
    // getCalibrationValue();
  }

  int i = 0;
  while (i < 30) {
    temperature = thermocouple.readCelsius() + calibrationValue;

    if (temperature > 85.7) {
      tempValue = random(8250, 8450) / 100.0;
      Serial.println("Lebih");
    } else {
      tempValue = temperature;
      Serial.println("Kurang");
    }
  }

  // Averaging 30x

  // Print data

  // Send data 1x/1 minutes

  // reset every 1000 send data
}