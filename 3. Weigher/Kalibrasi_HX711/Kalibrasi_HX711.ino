/*
  V. 0.0.1 Beta
  Update terakhir : 22-12-2023
  Last Change Log {

  }
  Perhatikan spesifikasi Load Cell yan digunakan, tika maksimal berart adalah 5 KG,
  jangan mengukur berat objek lebih dari 5 KG.

  Program ini hanya digunakan dalam proses kalibrasi,
  tidak untuk mengukur berat objek.

  Langkah Kalibrasi:
  1. Kosongkan timbangan, diatas load cell hanya ada tempat menyimpan barang.
  2. Setelah 5 detik, timbang barang yang sudah diketahui beratnya, misal 100 g
  3. Catat nilai dari hasil timbangan, biasanya bernilai minus.
  4. Calibrarion Factor = Nilai Hasil Timbang/Berat Barang
    Contoh : -141449/300 = -471.479
    Maka nilai kalibrasi adalah -471.479, cacat nilai ini untuk dimasukan
    ke program Weigher
*/

#include "HX711.h"
#include "LiquidCrystal_I2C.h"

// HX711 Wiring
const int LOADCELL_DOUT_PIN = 26;  // Pin RX2
const int LOADCELL_SCK_PIN = 27;

HX711 scale;
float beratBarang = 20000;  // Contoh 300 g, sesuaikan dengan barang yang ada
float calibrationFactor;
LiquidCrystal_I2C lcd(0x27, 20, 4);
#define buttonUp 34
#define buttonDown 35
#define buttonSelect 32
int buttonUpState = 0;
int buttonDownState = 0;
int buttonSelectState = 0;

bool isCalibrating = false;
bool isSelecting = false;


void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);
  pinMode(buttonSelect, INPUT);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare();
}

void loop() {
  if (digitalRead(buttonUp) && digitalRead(buttonDown))
  { 
  while (digitalRead(buttonUp) == HIGH && digitalRead(buttonDown) == HIGH);
  calibrationProcess();
  
  } else {
    lcd.setCursor(0, 0);
    lcd.print(calibrationFactor);
    delay(100);
    // if (!isCalibrated) {
    // scale.set_scale(calibrationFactor);                      // this value is obtained by calibrating the scale with known weights; see the README for details
    // scale.tare();
    // }
  }

  // if (scale.is_ready()) {
  //   scale.set_scale();
  //   Serial.println("Kosongkan timbangan");
  //   delay(5000);
  //   scale.tare();
  //   Serial.println("Tare done...");
  //   Serial.print("Taruh barang yang sudah diketahui beratnya");
  //   delay(5000);
  //   long reading = scale.get_units(10);
  //   Serial.print("Hasil : ");
  //   Serial.println(reading);
  //   calibrationFactor = reading / beratBarang;
  //   Serial.print("Faktor Kalibrasi: ");
  //   Serial.println(calibrationFactor);
  // } else {
  //   Serial.println("HX711 not found.");
  // }
  // delay(5000);
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