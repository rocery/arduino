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
PZEM004Tv30 pzem12(pzemSWSerial, 0x12);
PZEM004Tv30 pzem13(pzemSWSerial, 0x13);
#define SET_ADDRESS 0x13



void setup() {
  /* Debugging serial */
  Serial.begin(115200);
  pzemSWSerial.begin(9600);
}

void loop() {
  //  pzem13.setAddress(SET_ADDRESS);
  //  Serial.print("Custom address:    0x");
  //  Serial.println(pzem13.readAddress(), HEX);

  Serial.print("Pzem12 Address: 0x");
  Serial.println(pzem12.readAddress(), HEX);
  Serial.print("Pzem13 Address: 0x");
  Serial.println(pzem13.readAddress(), HEX);
  //
  // Read the data from the sensor
  float voltage12 = pzem12.voltage();
  float current12 = pzem12.current();
  float power12 = pzem12.power();
  float energy12 = pzem12.energy();
  float frequency12 = pzem12.frequency();
  float pf12 = pzem12.pf();
  //
  float voltage13 = pzem13.voltage();
  float current13 = pzem13.current();
  float power13 = pzem13.power();
  float energy13 = pzem13.energy();
  float frequency13 = pzem13.frequency();
  float pf13 = pzem13.pf();

  //  Print the values to the Serial console
  Serial.print("Voltage12: ");      Serial.print(voltage12);      Serial.println("V");
  Serial.print("Current12: ");      Serial.print(current12);      Serial.println("A");
  Serial.print("Power12: ");        Serial.print(power12);        Serial.println("W");
  Serial.print("Energy12: ");       Serial.print(energy12, 3);     Serial.println("kWh");
  Serial.print("Frequency12: ");    Serial.print(frequency12, 1); Serial.println("Hz");
  Serial.print("PF12: ");           Serial.println(pf12);
  
  Serial.print("Voltage13: ");      Serial.print(voltage13);      Serial.println("V");
  Serial.print("Current13: ");      Serial.print(current13);      Serial.println("A");
  Serial.print("Power13: ");        Serial.print(power13);        Serial.println("W");
  Serial.print("Energy13: ");       Serial.print(energy13, 3);     Serial.println("kWh");
  Serial.print("Frequency13: ");    Serial.print(frequency13, 1); Serial.println("Hz");
  Serial.print("PF13: ");           Serial.println(pf13);

  Serial.println();
  delay(2000);
}
