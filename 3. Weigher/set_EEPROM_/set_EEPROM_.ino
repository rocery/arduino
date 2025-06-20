#include <EEPROM.h>

const int EEPROM_ADDRESS = 0;

void setup() {
  Serial.begin(115200);

  // EEPROM
  EEPROM.begin(512);

  delay(1000);

  float calibrationFactor = readFloatFromEEPROM(EEPROM_ADDRESS);
  int digitScale = readFloatFromEEPROM(EEPROM_ADDRESS + 4);

  Serial.print("C: ");
  Serial.println(calibrationFactor);
  Serial.print("D: ");
  Serial.println(digitScale);
}

void loop() {
  updateFloatInEEPROM(EEPROM_ADDRESS, 105.52);
  updateFloatInEEPROM(EEPROM_ADDRESS + 4, 2);

  int read = 1;
  if (read <= 2) {
    float calibrationFactor = readFloatFromEEPROM(EEPROM_ADDRESS);
    int digitScale = readFloatFromEEPROM(EEPROM_ADDRESS + 4);

    Serial.print("C: ");
    Serial.println(calibrationFactor);
    Serial.print("D: ");
    Serial.println(digitScale);
    read++;
  }
  delay(5000);
}

float readFloatFromEEPROM(int address) {
  float value = EEPROM.get(address, value);
  return value;
}

void updateFloatInEEPROM(int address, float newValue) {
  float currentValue = readFloatFromEEPROM(address);

  if (currentValue != newValue) {
    // If the new value is different from the current value in EEPROM, update it.
    EEPROM.put(address, newValue);
    EEPROM.commit();
    Serial.println("Value updated in EEPROM.");
  } else {
    Serial.println("Value is already up-to-date, no write needed.");
  }
}