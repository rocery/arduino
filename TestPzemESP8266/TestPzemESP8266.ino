#include "SoftwareSerial.h"
#include <PZEM004Tv30.h>

PZEM004Tv30 pzem(12, 13);  // 12=D6 (Rx), 13=D7 (Tx)
void setup() {
  Serial.begin(9600);
}

uint8_t addr = 0x09; //THIS IS THE ADDRESS

void loop() {
    // pzem.setAddress(addr);
    Serial.print("Current address: 0x");
    Serial.println(pzem.getAddress(), HEX);
    Serial.println();
    delay(1000);
}