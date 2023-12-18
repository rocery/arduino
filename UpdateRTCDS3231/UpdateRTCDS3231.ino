//#include <LiquidCrystal_I2C.h>
//#include <RTClib.h>
//#include <NTPClient.h>
//#include <WiFi.h>
//#include <WiFiUdp.h>
//#include <HTTPClient.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include "time.h"
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <LiquidCrystal_I2C.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <RTClib.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);


// == WiFi Config ==
WiFiMulti wifiMulti;
const char* ssid_a = "STTB1";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "MT1";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
const char* ssid_d = "Djoksen";
const char* password_d = "Welive99";
const char* ssid_it = "STTB5";
const char* password_it = "siantar123";

// == Get NTP/RTC ==
const char* ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 7 * 3600;  // GMT +7
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat;
int year;
int month;
int day;
int hour;
int minute;
int second;
RTC_DS3231 rtc;
DateTime now;
bool ntpStatus;

void getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    ntpStatus = false;
  } else {
    ntpStatus = true;
  }
  char timeStringBuff[50];  //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  dateTime = String(timeStringBuff);

  // Save time data to variabel
  year = timeinfo.tm_year + 1900;
  month = timeinfo.tm_mon + 1;
  day = timeinfo.tm_mday;
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;

  dateFormat = String(year) + '-' + String(month) + '-' + String(day);
  timeFormat = String(hour) + ':' + String(minute) + ':' + String(second);
}


void setup() {
  Serial.begin(115200);
  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);
  wifiMulti.addAP(ssid_d, password_d);
  wifiMulti.addAP(ssid_it, password_it);
  delay(500);
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  rtc.begin();
  getLocalTime();
}

void loop() {
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    delay(1000);
    wifiMulti.run();
  }
  int tryNTP = 0;
  while (ntpStatus == false && tryNTP <= 20) {
    Serial.println("Connected to SSID: " + WiFi.SSID());
    getLocalTime();
    tryNTP++;
    delay(50);
  }
  getLocalTime();
  now = rtc.now();
  // Adjust RTC berdasarkan NTP
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.adjust(DateTime(2023, 12, 11, hour, minute, second));

  
  // Print Waktu NTP
  Serial.println("Formatted Time: " + dateTime);

  // Print Waktu RTC DS3231
  Serial.print(now.year());
  Serial.print('-');
  Serial.print(now.month());
  Serial.print('-');
  Serial.print(now.day());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.println(now.second());
  delay(1000);
}
