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
  6. Relay 5v

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
const String deviceID = "RFID_200";
// == PN532 ==
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId;
byte nuidPICC[4];
String master1 = "23 45 B7 15";

// Relay/LED/IR
#define irPin 34
#define relayPin 33
#define led1 25
#define led2 26

// == WiFi Config ==
/* Deklarasikan semua WiFi yang bisa diakses oleh ESP32
ESP32 akan memilih WiFi dengan sinyal paling kuat secara otomatis
*/
WiFiMulti wifiMulti;
const char* ssid_a = "STTB8";
const char* password_a = "siantar123";
const char* ssid_b = "MT4";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
const char* ssid_d = "STTB1";
const char* password_d = "Si4nt4r321";
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

// == SD Card ==
String logName = "/logRFID.txt";
String logNameFail = "/logRFIDFail.txt";
String logOut = "/logRFIDout.txt";
String listDataCard = "/dataCardRFID.txt";
String lineData, logData, uid_RFID_SD, datetime_SD;

// == Data Send/Get ==
bool sendStatus;
String postData;
const String api = "http://192.168.7.223/rfid_api/sendDataRFID.php";
const String apiOut = "http://192.168.7.223/rfid_api/sendDataRFIDout.php";
const String apiFail = "http://192.168.7.223/rfid_api/sendDataRFIDFail.php";

bool setupPN532() {
  nfc.begin();
  if (!nfc.status()) {
    return false;
  } else {
    return true;
  }
  delay(500);
}

bool readRFID() {
  if (!nfc.tagPresent(20)) {
    Serial.println("RFID tidak terbaca");
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

void openKey() {
  digitalWrite(relayPin, HIGH);
  delay(5000);
  digitalWrite(relayPin, LOW);
  delay(500);
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

bool insertCard(String dataFile, String dataCard) {
  File data = SD.open(dataFile, FILE_WRITE);

  if (!data) {
    Serial.println("File data akses RFID tidak ada");
    return false;
  } else {
    if (data.print(dataCard)) {
      Serial.println("Kartu baru berhasil disimpan");
      return true;
    } else {
      Serial.println("Kartu baru berhasil gagal disimpan");
      return false;
    }
  }
}

// void readFile(fs::FS& fs, String path) {
//   Serial.printf("Reading file: %s\n", path);

//   File file = fs.open(path);
//   if (!file) {
//     Serial.println("Failed to open file for reading");
//     return;
//   }

//   Serial.print("Read from file: ");
//   while (file.available()) {
//     Serial.println(file.read());
//   }
//   file.close();
// }

bool checkCard(String dataFile, String dataCard) {
  File data = SD.open(dataFile, FILE_WRITE);
  bool dataRFID = false;

  if (!data) {
    Serial.println("File data akses RFID tidak ada");
    return false;
  } else {
    while (data.available()) {
      String line = data.readStringUntil('\n');
      Serial.println(line);
      if (line == dataCard) {
        Serial.println("Data ditemukan");
        dataRFID = true;
        break;
      }
    }
    data.close();

    if (!dataRFID) {
      Serial.println("Data tidak ditemukan");
      return false;
    } else {
      return true;
    }
  }
}

bool insertLastLineLog(String dataFile, String dataCard) {
  File file = SD.open(dataFile, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  } else if (file) {
    if (file.println(dataCard)) {
      Serial.println("Line written");
      Serial.print("Data ke SD : ");
      Serial.println(dataCard);
      return true;
    } else {
      Serial.println("Write failed");
      return false;
    }
    file.close();
  }
}

bool readLastLineLog(String dataFile) {
  File file = SD.open(dataFile, FILE_READ);
  String line;
  if (!file) {
    Serial.println("Failed to open file for reading");
    return false;
  } else {
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

    // line = line.substring(secondCommaIndex + 1);
    // int thirdCommaIndex = line.indexOf(',');
    // dateTimeSD = line.substring(0, thirdCommaIndex);

    datetime_SD = line.substring(firstCommaIndex + 1);

    return true;
  }
}

void openDoor() {
  Serial.println("Pintu Terbuka");
  digitalWrite(relayPin, HIGH);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  delay(5000);
  digitalWrite(relayPin, LOW);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
}

void sendLogData(String API, String Data) {
  /* Mengirim data ke local server
     Ganti isi variabel api sesuai dengan form php
  */
  // HTTPClient http;
  // http.begin(API);
  // http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // int httpResponseCode = http.POST(Data);

  // if (httpResponseCode > 0) {
  //   String response = http.getString();
  //   Serial.println(response);
  // } else {
  //   Serial.print("Error on sending POST");
  // }
  // http.end();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(irPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  // Setup PN532
  if (!setupPN532()) {
    Serial.println("Inisialisasi PN532 Gagal");
    Serial.println("Alat Tidak Bisa Digunakan");
    Serial.println("Periksa Sensor PN532");
  }

  // Setup SD Card
  if (!SD.begin()) {
    Serial.println("Inisialisasi SD Card Gagal");
  }

  // Setup WiFi // NTP
  wifiNtpSetup();
}

void loop() {
  if (wifiMulti.run() != WL_CONNECTED) {
    wifiMulti.run();
  } else {
    getLocalTime();
  }

  // readFile(SD, listDataCard);

  if (readRFID()) {
    if (tagId == "23 45 B7 15") {
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      delay(2000);

      Serial.println("Proses pendaftaran kartu baru");
      int tryNewInsertCard = 0;
      while (!readRFID() && tryNewInsertCard <= 5) {
        Serial.println("Tempelkan kartu baru");
        tryNewInsertCard++;
        delay(500);
      }

      if (tagId != "23 45 B7 15" && tagId != "") {
        insertCard(listDataCard, tagId);
        // digitalWrite(led1, HIGH);
      } else {
        Serial.println("Kartu tidak terbaca/master");
        // digitalWrite(led2, HIGH);
      }

    } else {
      logData = tagId + "," + String(dateTime);
      if (checkCard(listDataCard, tagId)) {
        // Open Door
        openDoor();

        // Insert Log
        insertLastLineLog(logName, logData);

        // Read Log
        readLastLineLog(logName);

        // Send Log
        // postData = "deviceName=" + deviceName + "&data=" + lineData + "&ip_address=" + ip_Address;
        postData = "deviceName=" + deviceName + "&uid=" + uid_RFID_SD + "&dateTime=" + String(dateTime) + "&ip_address=" + ip_Address;
        sendLogData(api, postData);
      } else {
        Serial.println("Akses Kartu tidak ada");
      }
    }
  }

  // Membaca output Sensor
  int irState = digitalRead(irPin);
  if (irState == LOW) {
    openDoor();
  }
  delay(50);
}