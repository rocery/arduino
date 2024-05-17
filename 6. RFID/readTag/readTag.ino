#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

void setup(void) {
  Serial.begin(115200);
  Serial.println("NFC Tag Reader");
  nfc.begin();
}

void loop(void) {
  Serial.println("\nPlace your NFC tag near the reader...");
  if (nfc.tagPresent()) {
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

        // Assuming the payload is ASCII text
        String payloadAsString = "";
        for (int c = 1; c < payloadLength; c++) {
          payloadAsString += (char)payload[c];
        }
        Serial.println(payloadAsString);
        if (payloadAsString == "ACC") {
          Serial.println("Success");
        }
      }
    }
  }
  delay(1000);
}
