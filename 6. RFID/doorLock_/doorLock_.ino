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
#include <LiquidCrystal_I2C.h>

const String deviceName = "Door Lock Sports Park";
const String deviceId = "RFID_200";
int programLoop = 0;

/* Mendeklarasikan LCD dengan alamat I2C 0x27
   Total kolom 20
   Total baris 4 */
LiquidCrystal_I2C lcd(0x27, 20, 4);

// == PN532 ==
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

// Variabel tag RFID
String payloadAsString, tagId;

// == Relay/LED/IR ==
#define irPin 34
#define relayPin 33
#define led1 25
#define led2 26

// == WiFi/Network ==
/*  Deklarasikan semua WiFi yang bisa diakses oleh ESP32
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

IPAddress staticIP(192, 168, 7, 200);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
String ip_Address;

// == Get NTP ==
const char* ntpServer = "192.168.7.223";
// Karena Bekasi ada di GMT+7, maka Offset ditambah 7 jam
const long gmtOffsetSec = 7 * 3600;
const int daylightOffsetSec = 0;
String dateTime, dateLCD;
int year, month, day, hour, minute, second;

// == SD Card ==
String logIn = "/logRFID.txt";
String logFail = "/logRFID.txt";
String logData, uid_RFID_SD, datetime_SD;

// == Data Send/Get ==
String postData;
const String api = "http://192.168.7.223/rfid_api/sendDataRFID.php";
const String apiFail = "http://192.168.7.223/rfid_api/sendDataRFIDFail.php";

bool setupPN532() {
  Serial.println("== Inisialisasi PN532 ==");
  nfc.begin();
  if (!nfc.status()) {
    return false;
  } else {
    return true;
  }
}

bool readRFID() {
  Serial.println("\nLetakan kartu RFID/NFC pada sensor");
  if (nfc.tagPresent(20)) {
    NfcTag tag = nfc.read();
    Serial.println("NFC Tag found:");
    // Serial.print("Tag Type: ");
    // Serial.println(tag.getTagType());
    Serial.print("UID: ");
    tagId = tag.getUidString();
    Serial.println(tagId);
    // tag.print();

    payloadAsString = "";
    if (tag.hasNdefMessage()) {  // Check if the tag has an NDEF message
      NdefMessage message = tag.getNdefMessage();
      Serial.print("Message: ");
      for (int i = 0; i < message.getRecordCount(); i++) {
        NdefRecord record = message.getRecord(i);
        int payloadLength = record.getPayloadLength();
        byte payload[payloadLength];
        record.getPayload(payload);

        for (int c = 1; c < payloadLength; c++) {
          payloadAsString += (char)payload[c];
        }
        Serial.println(payloadAsString);
      }
    }

    return true;
  } else {
    Serial.println("\nKatru RFID/NFC tidak terbaca");
    return false;
  }
  // delay(1000);
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

    dateLCD = String(day) + "-" + String(month) + " " + String(hour) + ":" + String(minute);

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
    Serial.println("WiFi Disconnected  ");
    Serial.println("Date/Time Error    ");
    lcd.setCursor(1, 2);
    lcd.print("WiFi Error     ");
    delay(5000);

  } else {
    lcd.setCursor(1, 2);
    lcd.print("WiFi Normal        ");
    Serial.println(WiFi.SSID());
    int tryNTP = 0;
    while (!getLocalTime() && tryNTP <= 2) {
      tryNTP++;
      delay(50);
      Serial.println("Getting Date/Time");
      lcd.setCursor(1, 3);
    }
    if (getLocalTime()) {
      lcd.setCursor(1, 3);
      lcd.print("Proses NTP Normal ");
    }
  }
}

bool insertLastLineLog(String path, String data) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Gagal membuka file");
    return false;
  } else if (file) {
    if (file.println(data)) {
      Serial.println("Data disimpan");
      Serial.print("Data ke SD : ");
      Serial.println(data);
      return true;
    } else {
      Serial.println("Data gagal disimpan");
      return false;
    }

    file.close();
  }
}

bool readLastLineSDCard(String path) {
  File file = SD.open(path);
  String line;
  if (!file || file.isDirectory()) {
    Serial.println("Gagal membuka file");
    return false;
  } else if (file || file.isDirectory()) {
    while (file.available()) {
      line = file.readStringUntil('\n');
    }
    file.close();
    Serial.print("Data dari SD : ");
    Serial.println(line);
    int firstCommaIndex = line.indexOf(',');
    String productSelectedSD = line.substring(0, firstCommaIndex);

    line = line.substring(firstCommaIndex + 1);
    int secondCommaIndex = line.indexOf(',');
    uid_RFID_SD = line.substring(0, secondCommaIndex);

    line = line.substring(secondCommaIndex + 1);
    int thirdCommaIndex = line.indexOf(',');
    datetime_SD = line.substring(0, thirdCommaIndex);

    String ipAddressSD = line.substring(thirdCommaIndex + 1);

    return true;
  }
}

void sendLogData(String API, String data) {
  /* Mengirim data ke local server
     Ganti isi variabel api sesuai dengan form php
  */
  HTTPClient http;
  http.begin(API);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(data);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST");
  }
  http.end();
}

void openKey() {
  digitalWrite(relayPin, HIGH);
  delay(5050);
  digitalWrite(relayPin, LOW);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  pinMode(irPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  lcd.setCursor(1, 0);
  lcd.print("Inisialisasi RFID");
  lcd.setCursor(1, 1);
  lcd.print("Memuat SDCard");
  lcd.setCursor(1, 2);
  lcd.print("Koneksi WiFi");
  lcd.setCursor(1, 3);
  lcd.print("Akses NTP");

  // Setup RFID
  if (!setupPN532()) {
    Serial.println("Inisialisasi PN532 Gagal");
    Serial.println("Alat Tidak Bisa Digunakan");
    Serial.println("Periksa Sensor PN532");
    lcd.setCursor(1, 0);
    lcd.print("RFID Error        ");
  } else {
    lcd.setCursor(1, 0);
    lcd.print("RFID Normal       ");
    Serial.println("RFID Normal");
  }

  // Setup SD
  if (!SD.begin()) {
    Serial.println("Inisialisasi SD Card Gagal");
    lcd.setCursor(1, 1);
    lcd.print("SDCard Error       ");
  } else {
    lcd.setCursor(1, 1);
    lcd.print("SDCard Normal      ");
  }

  // Setup WiFi
  wifiNtpSetup();

  delay(3000);
  lcd.clear();
}

void loop() {
  lcd.setCursor(2, 0);
  lcd.print("== SPORTS PARK ==");
  lcd.setCursor(1, 1);
  lcd.print("Letakan Kartu Pada");
  lcd.setCursor(0, 2);
  lcd.print("       Sensor       ");

  /* Jika WiFi status WiFi tidak terkoneksi,
     coba ulang koneksi
  */
  if (wifiMulti.run() == WL_CONNECTED) {
    lcd.setCursor(0, 3);
    lcd.print(WiFi.SSID());
    getLocalTime();
  } else if (wifiMulti.run() != WL_CONNECTED) {
    lcd.setCursor(0, 3);
    lcd.print("WiFi DC");
    wifiMulti.run();
  }

  // Print Waktu hh:mm:ss
  lcd.setCursor(9, 3);
  lcd.print(dateLCD);

  ip_Address = WiFi.localIP().toString();

  if (readRFID()) {
    if (payloadAsString == "ACC") {
      Serial.println("Akses diterima");
      lcd.setCursor(0, 1);
      lcd.print("   Akses Diterima   ");
      lcd.setCursor(0, 2);
      lcd.print("                ");

      openKey();

      // Save Log
      Serial.println("Proses simpan log");
      logData = deviceName + "," + tagId + "," + dateTime + "," + ip_Address;

      if (insertLastLineLog(logIn, logData)) {
        if (readLastLineSDCard(logIn)) {
          postData = "device_name=" + deviceName + "&tag_id=" + uid_RFID_SD + "&date=" + datetime_SD + "&ip_address=" + ip_Address;
        } else {
          postData = "device_name=" + deviceName + "&tag_id=" + tagId + "&date=" + dateTime + "&ip_address=" + ip_Address;
        }
      } else {
        postData = "device_name=" + deviceName + "&tag_id=" + tagId + "&date=" + dateTime + "&ip_address=" + ip_Address;
      }
      sendLogData(api, postData);
    } else {
      Serial.println("Akses ditolak");
      lcd.setCursor(1, 1);
      lcd.print("   Akses Ditolak   ");
      lcd.setCursor(0, 2);
      lcd.print("Kartu Tidak Dikenali");

      Serial.println("Proses simpan log");
      logData = deviceName + "," + tagId + "," + dateTime + "," + ip_Address;

      if (insertLastLineLog(logFail, logData)) {
        if (readLastLineSDCard(logFail)) {
          postData = "device_name=" + deviceName + "&tag_id=" + uid_RFID_SD + "&date=" + datetime_SD + "&ip_address=" + ip_Address;
        } else {
          postData = "device_name=" + deviceName + "&tag_id=" + tagId + "&date=" + dateTime + "&ip_address=" + ip_Address;
        }
      } else {
        postData = "device_name=" + deviceName + "&tag_id=" + tagId + "&date=" + dateTime + "&ip_address=" + ip_Address;
      }
      sendLogData(apiFail, postData);
      delay(3000);
    }
  }

  if (digitalRead(irPin) == LOW) {
    openKey();
  }

  programLoop++;
  if (programLoop % 10 == 0) {
    lcd.clear();
    if (programLoop % 20000 == 0) {
      ESP.restart();
    }
  }
}
