/**
 * TCA9548 I2CScanner.ino -- I2C bus scanner for Arduino
 *
 * Based on https://playground.arduino.cc/Main/I2cScanner/
 *
 */

// #include "Wire.h"

// #define TCAADDR 0x70

// void tcaselect(uint8_t i) {
//   if (i > 7) return;

//   Wire.beginTransmission(TCAADDR);
//   Wire.write(1 << i);
//   Wire.endTransmission();
// }


// // standard Arduino setup()
// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   Wire.begin();

//   Serial.begin(115200);
//   Serial.println("\nTCAScanner ready!");

//   for (uint8_t t = 0; t < 8; t++) {
//     tcaselect(t);
//     Serial.print("TCA Port #");
//     Serial.println(t);

//     for (uint8_t addr = 0; addr <= 127; addr++) {
//       if (addr == TCAADDR) continue;

//       Wire.beginTransmission(addr);
//       if (!Wire.endTransmission()) {
//         Serial.print("Found I2C 0x");
//         Serial.println(addr, HEX);
//       }
//     }
//   }
//   Serial.println("\ndone");
// }

// void loop() {
// }





#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];

void setup() {
  Serial.begin(9600);
  nfc.begin();
}

void loop() {
  readNFC();
}

void readNFC() {
  if(nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tag.print();
    tagId = tag.getUidString();
  }
  delay(5000);
}

// LCD Data

// sendData
