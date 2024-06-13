#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String payloadAsString, tagId;

bool readRFID() {
  payloadAsString = "";
  Serial.println("\nLetakan kartu RFID/NFC pada sensor");
  if (nfc.tagPresent(20)) {
    NfcTag tag = nfc.read();
    // Serial.println("NFC Tag found:");
    // // Serial.print("Tag Type: ");
    // // Serial.println(tag.getTagType());
    // Serial.print("UID: ");
    tagId = tag.getUidString();
    // tag.print();

    if (tag.hasNdefMessage()) {
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
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("NFC Tag Reader");
  nfc.begin();
}

void loop(void) {
  readRFID();
  if (payloadAsString == "ACC" || payloadAsString == "enACC") {
    Serial.println("OKE");
  } else {
    Serial.println("FALSE");
  }
}
