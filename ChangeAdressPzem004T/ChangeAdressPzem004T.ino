/*
  Copyright (c) 2021 Jakub Mandula

  Example of using one PZEM module with Software Serial interface.
  ================================================================

  If only RX and TX pins are passed to the constructor, software
  serial interface will be used for communication with the module.

*/

#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 13


SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);
// Ganti Adress sesuai nilai yang dibutuhkan
#define SET_ADDRESS 0x13



void setup() {
  /* Debugging serial */
  Serial.begin(115200);
  pzemSWSerial.begin(9600);
}

void loop() {
  pzem.setAddress(SET_ADDRESS);
  Serial.print("Custom address:    0x");
  Serial.println(pzem.readAddress(), HEX);
  delay(1000);
  
  Serial.print("Pzem New Address: 0x");
  Serial.println(pzem.readAddress(), HEX);
  Serial.println();
  delay(2000);
}
