#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define PCA9548A_address 0x70

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc0 = NfcAdapter(pn532_i2c);
NfcAdapter nfc1 = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];

void setup() {
  Serial.begin(115200);
  Wire.begin();

  TCA9548A(0);
  nfc0.begin();

  TCA9548A(1);
  nfc1.begin();
}

void loop() {
  readNFC0();
}

// Select I2C BUS PCA9548A
void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(PCA9548A_address);
  Wire.write(1 << bus);
  Wire.endTransmission();
  // Serial.print(bus);
}

void readNFC0() {
  if (nfc0.tagPresent()) {
    NfcTag tag = nfc0.read();
    tag.print();
    tagId = tag.getUidString();
  }
  delay(5000);
}

void readNFC1() {
  if (nfc1.tagPresent()) {
    NfcTag tag = nfc1.read();
    tag.print();
    tagId = tag.getUidString();
  }
  delay(5000);
}
