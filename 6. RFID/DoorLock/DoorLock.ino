#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define PCA9548A_address 0x70

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];

void setup() {
  Serial.begin(115200);
  Wire.begin();

  TCA9548A(0);
  nfc.begin();

  TCA9548A(1);
  nfc.begin();
}

void loop() {
  Serial.println("Multi 0");
  TCA9548A(0);
  Serial.println("RFID 0");
  readNFC();
  delay(100);

  Serial.println("Multi 1");
  TCA9548A(1);
  Serial.println("RFID 1");
  readNFC();
  delay(100);
}

// Select I2C BUS PCA9548A
void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(PCA9548A_address);
  Wire.write(1 << bus);
  Wire.endTransmission();
  Serial.println(bus);
}

void readNFC() {
  nfc.setPassiveActivationRetries(0x11);
  if (!nfc.tagPresent()) {
    Serial.println("False");
  } else {
    NfcTag tag = nfc.read();
    // tag.print();
    tagId = tag.getUidString();
    Serial.println(tagId);
  }
}
