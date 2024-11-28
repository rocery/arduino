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
int buttonUpState = 0;
int buttonDownState = 0;
int buttonSelectState = 0;

bool isCalibrating = false;
bool isSelecting = false;

float readFloatFromEEPROM(int address) {
  float value;
  EEPROM.get(address, value);
  return value;
}

// updateFloatInEEPROM(EEPROM_ADDRESS, newValue);
void updateFloatInEEPROM(int address, float newValue) {
  float currentValue = readFloatFromEEPROM(address);

  if (currentValue != newValue) {
    EEPROM.put(address, newValue);
    EEPROM.commit();
    Serial.println("Value updated in EEPROM.");
  } else {
    Serial.println("Value is already up-to-date, no write needed.");
  }
}

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

  scale.set_scale(readFloatFromEEPROM(EEPROM_ADDRESS));
  scale.tare();
}

void loop() {
  lcd.clear();
  if (isButtonPressed(buttonUp) && isButtonPressed(buttonDown)) {
    isCalibrating = true;
  } else {
    isCalibrating = false;
  }

  if (isButtonPressed(buttonSelect)) {
    isSelecting = true;
  }

  if (isCalibrating) {
    calibrationProcess();
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
  Serial.println(buttonPin);
  return digitalRead(buttonPin);
}

void calibrationProcess() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  BERAT BARANG  ");
  
  int values[] = {1, 2, 5, 10, 20, 40}; // Weight options in KG
  int currentValueIndex = 0;

  // Step 1: Select weight
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print("BERAT: ");
    lcd.print(values[currentValueIndex]);
    lcd.print(" KG");
    
    int buttonUpState = digitalRead(buttonUp);
    int buttonDownState = digitalRead(buttonDown);

    // If buttonUp is pressed
    if (buttonUpState == HIGH) {
      while (digitalRead(buttonUp) == HIGH);
      currentValueIndex = (currentValueIndex + 1) % 6; // Update index
      delay(200); // Debounce delay
    }

    // If buttonDown is pressed, move to the next step
    if (buttonDownState == HIGH) {
      while (digitalRead(buttonDown) == HIGH);
      break; // Exit the loop to proceed to the next step
    }
  }

  // Step 2: Empty the scale
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("KOSONGKAN");
  lcd.setCursor(0, 1);
  lcd.print("TIMBANGAN");
  int timerStep2 = 5;
  for (int i = 0; i <= timerStep2; i++) {
    lcd.setCursor(12, 0);
    lcd.print(timerStep2 - i);
    scale.tare();
    delay(800);
  }
  lcd.setCursor(11, 1);
  lcd.print("OKE");
  // Wait for user to empty the scale
  while (true) {
    int buttonDownState = digitalRead(buttonDown);

    // If buttonDown is pressed, move to the next step
    if (buttonDownState == HIGH) {
      while (digitalRead(buttonDown) == HIGH);
      break; // Exit the loop to proceed to the next step
    }
  }

  // Step 3: Weigh the item
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" TIMBANG BARANG");
  lcd.setCursor(1, 1);
  lcd.print(values[currentValueIndex]);
  lcd.print(" KG");
  delay(2000); // Display for 2 seconds

  // Wait for user to finish weighing the item
  while (true) {
    // Tare the scale and get the reading
    long reading = scale.get_units(10); // Get the reading with averaging of 20 samples
    calibrationFactor = (float)reading / (values[currentValueIndex] * 1000); // Calculate calibration factor
    lcd.setCursor(10, 1);
    lcd.print(calibrationFactor);
    scale.power_down();
    delay(1000);
    scale.power_up();

    int buttonDownState = digitalRead(buttonDown);
    int buttonSelectState = digitalRead(buttonSelect);

    // If buttonDown is pressed, move to the next step
    if (buttonDownState == HIGH) {
      while (digitalRead(buttonDown) == HIGH);
      break; // Exit the loop to proceed to the next step
    }
  }

  // Step 4: Confirm calibration
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("KALIBRASI");
  lcd.print("  H <==");
  lcd.setCursor(0, 1);
  lcd.print("SELESAI");
  lcd.print("    M ==>");

  while (true) {
    int buttonSelectState = digitalRead(buttonSelect);
    int buttonDownState = digitalRead(buttonDown);

    // If buttonSelect is pressed, exit the calibration process
    if (buttonSelectState == HIGH) {
      while (digitalRead(buttonSelect) == HIGH);
      isCalibrated = true;
      return; // Exit the function
    }

    // If buttonDown is pressed, restart the calibration
    if (buttonDownState == HIGH) {
      while (digitalRead(buttonDown) == HIGH);
      calibrationProcess(); // Restart the calibration process
      break; // Exit the loop to start over
    }
  }
}