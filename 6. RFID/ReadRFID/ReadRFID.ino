/*
  V. 0.0.2
  Update Terakhir : 13-05-2024

  PENTING
  1. Harus menggunakan Dual Core Micro Controller
  2. Kartu RFID yang digunakan adalah Mifare ISO14443A

  Komponen:
  1. Micro Controller : ESP32
  2. LED
  3. SD Card
  4. PN532
  5. PCA9548A


*/

// == Deklarasi semua Library yang digunakan ==
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "FS.h"
#include "SD.h"
#include "time.h"

const String deviceName = "Door Lock Sport Park";

// PIN I2C PCA9548A
#define SDA_PIN 21
#define SCL_PIN 22

// Adress PCA9548A (A0, A1, A2 == GND)
#define PCA9548A_address 0x70

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

// == PN532 ==
uint8_t rfidReadStatus;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
String uidString = "";                    // String to store UID for further use

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

// == Data Send/Get ==
bool sendStatus;
String postData;
const String api = "http://192.168.7.223/counter_hit_api/getDataLastCounter.php?";

// == SD Card ==
String line, logName, logData;
String dateTimeSD, productSelectedSD, counterSD, ipAddressSD;
bool statusSD, readStatusSD, insertLastLineSDCardStatus;

void setup(void) {
  Serial.begin(115200);
  Wire.begin();

  if(setupPN532(0) && setupPN532(1)) {
    Serial.println("Inisialisasi PN532 berhasil");
  } else {
    Serial.println("Inisialisasi PN532 gagal");
    Serial.println("Alat tidak bisa digunakan");
    Serial.println("Periksa sensor PN532");
    while(1) {
      Serial.print(".");
      // ledFDIDFailSetup();
    }
  }

  Serial.println("Waiting for an NFC Tag...");
}

void loop() {
}

bool setupPN532(int bus_TCA9548A) {
  TCA9548A(bus_TCA9548A);
  Serial.print("Inisialisasi PN532 bus : ");
  Serial.println(bus_TCA9548A);
  nfc.begin();
  nfc.setPassiveActivationRetries(0x10);

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("PN532 Tidak Terdeteksi");
  } else {
    Serial.print("PN532 Firmware Version: ");
    Serial.println(versiondata);
    return true;
  }
  // Configure board to read RFID tags
  nfc.SAMConfig();
  delay(500);
}

void readRFID() {
  // Wait for an ISO14443A type cards (Mifare, etc.). When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if it's a 4-byte or 7-byte UID
  rfidReadStatus = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (rfidReadStatus) {
    Serial.println("Found an NFC tag!");
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(" 0x");
      Serial.print(uid[i], HEX);
      uidString += String(uid[i], HEX);  // Append each byte of the UID to the uidString
    }
    Serial.println("");
    Serial.print("UID String: ");
    Serial.println(uidString);  // Display the complete UID string

    // Wait 1 second before continuing
    delay(500);
  } else {
    // Put a delay to avoid too much serial output
    delay(500);
  }
}

void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(PCA9548A_address);
  Wire.write(1 << bus);
  Wire.endTransmission();
  // Serial.println(bus);
}
