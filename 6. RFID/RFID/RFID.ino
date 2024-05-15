#include <Wire.h>
#include <Adafruit_PN532.h>
#include <PN532_SWHSU.h>

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
String uidString = "";

bool setupPN532(uint8_t bus_TCA9548A) {
  TCA9548A(bus_TCA9548A);
  Serial.print("Inisialisasi PN532 bus : ");
  Serial.println(bus_TCA9548A);
  nfc.begin();
  nfc.setPassiveActivationRetries(0x11);

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    // Serial.println("PN532 Tidak Terdeteksi");
    return false;
  } else {
    Serial.print("PN532 Firmware Version: ");
    Serial.println(versiondata);
    return true;
  }
  // Configure board to read RFID tags
  nfc.SAMConfig();
  delay(500);
}

void readRFID(uint8_t bus_TCA9548A) {
  TCA9548A(bus_TCA9548A);
  Serial.print("Read RFID : ");
  Serial.println(bus_TCA9548A);
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
    delay(1000);
  } else {
    // Put a delay to avoid too much serial output
    delay(100);
  }
}

void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(PCA9548A_address);
  Wire.write(1 << bus);
  Wire.endTransmission();
  // Serial.println(bus);
}

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
    while (1) {
      Serial.print(".");
      // ledFDIDFailSetup();
    }
  }
}

void loop() {
  readRFID(0);
  delay(500);
  readRFID(1);
  delay(500);
}

