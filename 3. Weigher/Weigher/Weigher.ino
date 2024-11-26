/*
 * Normal Mode:
 * - Timbangan 
 * 
 *
*/

#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

const int LOADCELL_DOUT_PIN = 27;
const int LOADCELL_SCK_PIN = 26;
HX711 scale;

LiquidCrystal_I2C lcd(0x27, 20, 4);

const int EEPROM_ADDRESS = 0;

#define buttonUp 34
#define buttonDown 35
#define buttonSelect 32

bool isCalibrating = false;
bool isSelecting = false;

void setup() {
  Serial.begin(9600);
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);
  pinMode(buttonSelect, INPUT);

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  EEPROM.begin(512);

  scale.tare();
}

void loop() {
  if (isButtonPressed(buttonUp) && isButtonPressed(buttonDown)) {
    isCalibrating = true;
  }

  if (isButtonPressed(buttonSelect)) {
    isSelecting = true;
  }

  if (isCalibrating) {
    Serial.println("Kosongkan timbangan");
    delay(5000);
    scale.tare();
    Serial.println("Tare done...");
    Serial.print("Taruh barang yang sudah diketahui beratnya");
    delay(5000);
    long reading = scale.get_units(10);
    Serial.print("Hasil : ");
    Serial.println(reading);
    isCalibrating = false;
  }
  

  if (scale.is_ready()) {
    long reading = scale.get_units(10);
    Serial.print("Hasil : ");
    Serial.println(reading);
  } else {
    Serial.println("HX711 not found.");
  }
  delay(1000);
  Serial.print(isButtonPressed(buttonUp));
}

bool isButtonPressed(int buttonPin) {
  return !digitalRead(buttonPin);
}

void readAllButtons() {
  Serial.print("Button Up: ");
  Serial.println(isButtonPressed(buttonUp));
  Serial.print("Button Down: ");
  Serial.println(isButtonPressed(buttonDown));
  Serial.print("Button Select: ");
  Serial.println(isButtonPressed(buttonSelect));
}

void updateFloatInEEPROM(int address, float newValue) {
  float currentValue = readFloatFromEEPROM(address);

  if (currentValue != newValue) {
    EEPROM.put(address, newValue);
    Serial.println("Value updated in EEPROM.");
  } else {
    Serial.println("Value is already up-to-date, no write needed.");
  }
}

float readFloatFromEEPROM(int address) {
  float value;
  EEPROM.get(address, value);
  return value;
}