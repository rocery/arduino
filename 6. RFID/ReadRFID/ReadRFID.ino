/*
  V. 0.0.3
  Update Terakhir : 15-05-2024

  PENTING
  1. Harus menggunakan Dual Core Micro Controller
  2. Kartu RFID yang digunakan adalah Mifare ISO14443A
  3. Library SD harus dideklarasikan secara lengkap dan pertama dideklarasikan,
     Jika tidak, akan bermasalah dengan library PN532

  Komponen:
  1. Micro Controller : ESP32
  2. LED
  3. SD Card
  4. PN532
  5. PCA9548A

*/
// == Deklarasi semua Library yang digunakan ==
// Library SD Card Deklarasikan pertama sebelum PN532
#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>
#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include "time.h"

const String deviceName = "Door Lock Sport Park";

// == PN532 ==
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];

// Adress PCA9548A (A0, A1, A2 == GND)
#define PCA9548A_address 0x70

// == WiFi Config ==
/* Deklarasikan semua WiFi yang bisa diakses oleh ESP32
ESP32 akan memilih WiFi dengan sinyal paling kuat secara otomatis
*/
WiFiMulti wifiMulti;
const char* ssid_d = "STTB1";
const char* password_d = "Si4nt4r321";
const char* ssid_b = "MT4";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
const char* ssid_a = "STTB8";
const char* password_a = "siantar123";
const char* ssid_it = "Tester_ITB";
const char* password_it = "Si4nt4r321";

// Set IP to Static
IPAddress staticIP(192, 168, 7, 200);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
String ip_Address;

// == Get NTP ==
const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;  // Karena Bekasi ada di GMT+7, maka Offset ditambah 7 jam
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat;
int year, month, day, hour, minute, second;
bool ntpStatus;

// == Data Send/Get ==
bool sendStatus;
String postData;
const String api = "http://192.168.7.223/rfid_api/sendDataRFID.php?";

// == SD Card ==
String logName = "/logRFID.txt";
String listDataCard = "/dataCardRFID.txt";
String line, logData, uid_RFID_SD, datetime_SD;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Setup RFID
  if (setupPN532(0) && setupPN532(1)) {
    Serial.println("Inisialisasi PN532 Berhasil");
  } else {
    Serial.println("Inisialisasi PN532 Gagal");
    Serial.println("Alat Tidak Bisa Digunakan");
    Serial.println("Periksa Sensor PN532");
    // ledFDIDFailSetup();
  }

  //Setup SD Card
  if (!setupSDCard()) {
    Serial.println("Inisialisasi SD Card Gagal");
    // ledSDFail();
  }

  //Setup WiFi
  wifiNtpSetup();
}

void loop() {
  // Jika tag id == master --> insert card
  if (readRFID(0) && tagId == "master") {
    delay(1000);
    // ledInsert
    Serial.println("Proses mendaftarkan kartu baru");
    int tryNewInsertCard = 5;
    while (!readRFID(0) || tryNewInsertCard < 5) {
      Serial.println("Tempelkan kartu baru");
    }
    if (tagId != "master" && tagId != "") {
      insertCard(listDataCard, tagId);
    } else {
      Serial.println("Kartu tidak terbaca");
    }
  } else if (readRFID(0) { 
    // Jika tidak --> cek data di sd card, grand access, masukan log, masukan db

  }
  
  readRFID(1);
  //
}

bool setupPN532(uint8_t bus_TCA9548A) {
  TCA9548A(bus_TCA9548A);
  Serial.print("Inisialisasi PN532 bus : ");
  Serial.println(bus_TCA9548A);
  nfc.begin();
  if (!nfc.status()) {
    return false;
  } else {
    return true;
  }
  delay(500);
}

bool readRFID(uint8_t bus_TCA9548A) {
  TCA9548A(bus_TCA9548A);
  Serial.print("Get tagID dari RFID ");
  Serial.println(bus_TCA9548A);
  if (!nfc.tagPresent(1)) {
    Serial.println("tag RFID Tidak terbaca");
    return false;
  } else {
    NfcTag tag = nfc.read();
    // tag.print();
    tagId = tag.getUidString();
    Serial.println(tagId);
    return true;
  }
  delay(100);
}

void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(PCA9548A_address);
  Wire.write(1 << bus);
  Wire.endTransmission();
  // Serial.println(bus);
}

bool getLocalTime() {
  /* Fungsi bertujuan menerima update waktu
     lokal dari ntp.pool.org */
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("NTP Gagal");
    return false;
  } else {
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

    return true;
  }
}

void wifiNtpSetup() {
  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);
  wifiMulti.addAP(ssid_d, password_d);
  wifiMulti.addAP(ssid_it, password_it);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  wifiMulti.run();

  // NTP
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);

  if (wifiMulti.run() != WL_CONNECTED) {
    wifiMulti.run();
    Serial.println("WiFi Disconnected");
    Serial.println("Date/Time Error");
    delay(5000);

  } else {
    Serial.println(WiFi.SSID());
    int tryNTP = 0;
    while (!getLocalTime() && tryNTP <= 2) {
      tryNTP++;
      delay(50);
      Serial.println("Getting Date/Time");
    }
  }
}

bool setupSDCard() {
  if (!SD.begin()) {
    return false;
  } else {
    File logFile = SD.open(logName);
    File cardFile = SD.open(listDataCard);
    if (!logFile || logFile.isDirectory()) {
      File myLog = SD.open(logName, FILE_WRITE);
      myLog.println("0,0");
      myLog.close();
    } else {
      Serial.println("Data logName berhasil dimuat");
    }

    if (!cardFile || cardFile.isDirectory()) {
      File myCard = SD.open(listDataCard, FILE_WRITE);
      myCard.println("0");
      myCard.close();
    } else {
      Serial.println("Data listDataCard berhasil dimuat");
    }
    logFile.close();
    cardFile.close();

    return true;
  }
}

bool readLastLineLog(String logName) {
  File file = SD.open(logName);

  if (!file || file.isDirectory()) {
    Serial.println("Failed to open file for reading");
    return false;
  } else if (file) {
    while (file.available()) {
      line = file.readStringUntil('\n');
    }
    file.close();

    Serial.print("Data dari SD : ");
    Serial.println(line);
    int firstCommaIndex = line.indexOf(',');
    uid_RFID_SD = line.substring(0, firstCommaIndex);

    // line = line.substring(firstCommaIndex + 1);
    // int secondCommaIndex = line.indexOf(',');
    // counterSD = line.substring(0, secondCommaIndex);

    datetime_SD = line.substring(firstCommaIndex + 1);

    return true;
  }
}

bool insertLastLineLog(String logName, String line) {
  File file = SD.open(logName, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  } else if (file) {
    if (file.println(line)) {
      Serial.println("Line written");
    } else {
      Serial.println("Write failed");
    }
    Serial.print("Data ke SD : ");
    Serial.println(line);
    file.close();

    return true;
  }
}

bool insertCard(String listDataCard, String dataCard) {
  File file = SD.open(listDataCard, FILE_WRITE);

  if (!file) {
    Serial.println("File data akses RFID tidak ada");
    return false;
  } else {
    bool doubleRFID = false;
    while (file.available()) {
      String line = file.readStringUntil('\n');
      if (line == dataCard) {
        Serial.println("Data kartu sudah ada");
        doubleRFID = true;
        return false;
        break;
      }
    }
    if (doubleRFID == false) {
      insertLastLineLog(listDataCard, dataCard);
      Serial.println("Kartu baru berhasil disimpan");
      return true;
    }
    file.close();
  }
}

bool readCard(String listDataCard, String dataCard) {
  File file = SD.open(listDataCard);

  if (!file) {
    Serial.println("File data akses RFID tidak ada");
    return false;
  } else {
    bool dataRFID = false;
    while (file.available()) {
      String line = file.readStringUntil('\n');
      if (line == dataCard) {
        Serial.println("Data ditemukan");
        dataRFID = true;
        return true;
        break;
      }
    }
    if (dataRFID == false) {
      Serial.println("Data akses RFID tidak ada");
      return false;
    }
    file.close();
  }
}