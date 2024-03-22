#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);


void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
}


void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    if (uidLength == 4){
      // We probably have a Mifare Classic card ... 
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
   
      Serial.println("Trying to authenticate block 8 with default KEYA value");
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
     success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 8, 0, keya);
   
      if (success)
      {
        Serial.println("Sector 2 (Blocks 8..11) has been authenticated");
        uint8_t data[16];

        Serial.println("Writing Block 8:...");
        //menulis data
        memcpy(data, (const uint8_t[]){ 'r', 'f', 'i', 'd','1', '3', ',', '5', '6', 'M', 'H', 'z', 0, 0 }, sizeof data);
        success = nfc.mifareclassic_WriteDataBlock (8, data); //blok 8
        Serial.println("Writing Block 8: OK");
        Serial.println("");
        
        // Try to read the contents of block 8
        success = nfc.mifareclassic_ReadDataBlock(8, data);
  
        if (success){
          // Data seems to have been read ... spit it out
          Serial.println("Reading Block 8:");
          nfc.PrintHexChar(data, 16);
          Serial.println("");
    
          // Wait a bit before reading the card again
          delay(1000);
        }
        else{
          Serial.println("Ooops ... unable to read the requested block.  Try another key?");
        }
      }
      else{
        Serial.println("Ooops ... authentication failed: Try another key?");
      }
    }
  }
}
