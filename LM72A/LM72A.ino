//---------------------------------------------------------------------------------------------
//
// Basic usage example for the M2M_LM75A Arduino library.
//
// Copyright 2016-2017, M2M Solutions AB
// Written by Jonny Bergdahl, 2016-11-18
//
// Licensed under the MIT license, see the LICENSE.txt file.
//
//---------------------------------------------------------------------------------------------
// 2017-04-19 Added begin() and end() functions for Wire handling.
// 2017-04-19 Fixed a code merge problem
//
////////////////////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include <M2M_LM75A.h>

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Global variables
//
M2M_LM75A lm75a;
float suhu;

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Setup
//
void setup() {
  lm75a.begin();
  Serial.begin(115200);
  Serial.println(F("M2M_LM75A - Basic usage"));
  Serial.println(F("==========================================="));
  Serial.println("");
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Loop
//
void loop() {
  // Temperature
  suhu = lm75a.getTemperature();
  Serial.print(F("Temperature : "));
  Serial.print(suhu);
  Serial.println(F(" *C"));

  lm75a.shutdown();
  delay(2000);
  lm75a.wakeup();
  delay(2000);
}