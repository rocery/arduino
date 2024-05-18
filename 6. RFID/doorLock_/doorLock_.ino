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

const String deviceName = "Door Lock Sports Park";
const String deviceId = "RFID_200";

// == PN532 ==
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

// Variabel tag RFID
String payloadAsString = "";

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
String dateTime;

// == SD Card ==
String logIn = "/logRFID.txt";
String logFail = "/logRFID.txt";
String uid_RFID_SD, datetime_SD;

// == Data Send/Get ==
String postData;
const String api = "http://192.168.7.223/rfid_api/sendDataRFID.php";

bool setupPN532() {
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
    Serial.print("Tag Type: ");
    Serial.println(tag.getTagType());
    Serial.print("UID: ");
    Serial.println(tag.getUidString());
    tag.print();

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
        if (payloadAsString == "ACC") {
          Serial.println("Success");
        }
      }
    }

    return true;
  } else {
    Serial.println("\nKatru RFID/NFC tidak terbaca");
    return false;
  }
  // delay(1000);
}

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
