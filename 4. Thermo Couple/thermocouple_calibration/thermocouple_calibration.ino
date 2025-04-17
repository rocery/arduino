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
float tempReal;

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
   const char* ssid_a = "STTB1";
   const char* password_a = "Si4nt4r321";
   const char* ssid_b = "Amano2";
   const char* password_b = "Si4nt4r321";
   const char* ssid_c = "MT1";
   const char* password_c = "siantar321";
   const char* ssid_it = "Tester_ITB";
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
    
}