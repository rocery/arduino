#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// SD Card
#define CS_PIN D8
File logFile;

void setup() {
  Serial.begin(9600);
  // SD Card 
  Serial.println("Initializing SD Card...");
  if (!SD.begin(CS_PIN)) {
    Serial.println("Initializing SD Card Failed");
  }
}

void loop() {
  Serial.println("Initializing SD Card...");
  SD.begin(CS_PIN);
  if (!SD.begin(CS_PIN)) {
    Serial.println("Initializing SD Card Failed");
  } else { 
    Serial.println("Initializing SD Card Success");
  }
}
