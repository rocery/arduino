#include <PZEM004Tv30.h>

PZEM004Tv30 pzem(5,4);

void setup() {
  Serial.begin(115200);
}

uint8_t addr = 0x09; //THIS IS THE ADDRESS

void loop() {
    pzem.setAddress(addr);
    Serial.print("Current address:");
    Serial.println(pzem.getAddress(), HEX);
    Serial.println();
    delay(1000);
}
