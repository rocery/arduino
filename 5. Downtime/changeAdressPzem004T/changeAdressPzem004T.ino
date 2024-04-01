/*
  Copyright (c) 2021 Jakub Mandula

  Example of using one PZEM module with Software Serial interface.
  ================================================================

  If only RX and TX pins are passed to the constructor, software
  serial interface will be used for communication with the module.

*/

#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
// Digunakan pin 12 dan 13 (6, 7) sebagai pin UART (RX, TX)
// Pin TX dari Pzem dihubungkan ke pin (6) ESP8266
// Pin RX dari Pzem dihubungkan ke pin (7) ESP8266


PZEM004Tv30 pzem(12, 13);
// SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
// PZEM004Tv30 pzem(pzemSWSerial);
// Ganti Adress sesuai nilai yang dibutuhkan
#define SET_ADDRESS 0x14



void setup() {
  /* Debugging serial */
  Serial.begin(115200);
  // pzemSWSerial.begin(9600);
}

void loop() {
  pzem.setAddress(SET_ADDRESS);
  Serial.print("Custom address: 0x");
  Serial.println(pzem.readAddress(), HEX);
  delay(1000);
  float Energy = pzem.voltage();
  // Serial.print("Pzem New Address: 0x");
  Serial.println(Energy);
  // Serial.println();
  // delay(2000);
}
