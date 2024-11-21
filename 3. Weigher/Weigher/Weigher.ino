#include <Arduino.h>
#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 26; // Data pin
const int LOADCELL_SCK_PIN = 27;   // Clock pin

HX711 scale;

void setup() {
  Serial.begin(115200);
  Serial.println("HX711 Demo");
  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Display initial readings before calibration
  Serial.println("Before setting up the scale:");
  Serial.print("Raw read: \t\t");
  Serial.println(scale.read()); // Print a raw reading from the ADC

  Serial.print("Average read: \t\t");
  Serial.println(scale.read_average(20)); // Average of 20 readings

  Serial.print("Get value: \t\t");
  Serial.println(scale.get_value(5)); // Average of 5 readings (not calibrated yet)

  Serial.print("Get units: \t\t");
  Serial.println(scale.get_units(5), 1); // Average of 5 readings (not calibrated yet)

  // Set the scale factor (calibration factor)
  scale.set_scale(-459.542); // Replace with your calibration factor
  scale.tare(); // Reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("Raw read: \t\t");
  Serial.println(scale.read()); // Print a raw reading from the ADC

  Serial.print("Average read: \t\t");
  Serial.println(scale.read_average(20)); // Average of 20 readings

  Serial.print("Get value: \t\t");
  Serial.println(scale.get_value(5)); // Average of 5 readings minus tare

  Serial.print("Get units: \t\t");
  Serial.println(scale.get_units(5), 1); // Average of 5 readings divided by scale

  Serial.println("Readings:");
}

void loop() {
  Serial.print("One reading:\t");
  Serial.print(scale.get_units(), 1); // Single reading
  Serial.print("\t| Average:\t");
  Serial.println(scale.get_units(10), 5); // Average of 10 readings

  delay(5000); // Delay for readability
}