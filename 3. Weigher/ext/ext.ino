#include <HX711.h>
#include <LiquidCrystal_I2C.h>

// HX711 Wiring
const int LOADCELL_DOUT_PIN = 26;  // DOUT pin
const int LOADCELL_SCK_PIN = 27;   // SCK pin

// Button Pins
const int buttonUp = 34;    // Button to increase weight
const int buttonDown = 35;  // Button to decrease weight
const int buttonSelect = 32; // Button to select weight

// LCD I2C Address (sesuaikan dengan alamat I2C LCD Anda)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Ganti 0x27 dengan alamat I2C yang sesuai jika perlu

HX711 scale;
float calibrationFactor;  // Variable to hold the calibration factor
int selectedWeightIndex = 0; // Index for selected weight
const float weights[] = {1.0, 5.0, 10.0, 20.0}; // Available weights in kg
const char* weightLabels[] = {"1 kg", "5 kg", "10 kg", "20 kg"};
bool inCalibrationMode = false; // Flag to indicate if in calibration mode

void setup() {
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  // Setup button pins
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonSelect, INPUT_PULLUP);
  
  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.print("Weight: ");
  
  // Tare the scale initially
  scale.tare();
}

void loop() {
  // Check for button presses
  if (digitalRead(buttonUp) == LOW || digitalRead(buttonDown) == LOW) {
    inCalibrationMode = true; // Masuk ke mode kalibrasi
    selectedWeightIndex = 0; // Reset index ke 0
    updateLCD();
    delay(300); // Debounce delay
  }
  
  if (inCalibrationMode) {
    // Menu kalibrasi
    if (digitalRead(buttonUp) == LOW) {
      selectedWeightIndex = (selectedWeightIndex + 1) % 4; // Cycle through weights
      updateLCD();
      delay(300); // Debounce delay
    }
    
    if (digitalRead(buttonDown) == LOW) {
      selectedWeightIndex = (selectedWeightIndex - 1 + 4) % 4; // Cycle through weights
      updateLCD();
      delay(300); // Debounce delay
    }
    
    if (digitalRead(buttonSelect) == LOW) {
      calibrateScale(weights[selectedWeightIndex]);
      inCalibrationMode = false; // Keluar dari mode kalibrasi
      delay(300); // Debounce delay
    }
  } else {
    // Normal weighing mode
    if (scale.is_ready()) {
      long weight = scale.get_units(10);  // Get the weight
      float weightInKg = weight / 1000.0; // Convert to kg
      lcd.setCursor(0, 0);
      lcd.print("Weight: ");
      lcd.print(weightInKg, 2); // Tampilkan dengan 2 angka di belakang koma
      lcd.print(" kg   ");  // Output in kg
    } else {
      lcd.setCursor(0, 0);
      lcd.print("HX711 not found.");
    }
  }
  
  delay(100);  // Delay for readability
}

void updateLCD() {
  lcd.setCursor(0, 1);
  lcd.print("Select: ");
  lcd.print(weightLabels[selectedWeightIndex]);
}

void calibrateScale(float knownWeight) {
  lcd.clear();
  lcd.print("Remove weight");
  delay(2000); // Tampilkan pesan selama 2 detik
  scale.tare(); // Tare the scale
  
  lcd.clear();
  lcd.print("Place ");
  lcd.print(knownWeight);
  lcd.print(" kg");
  delay(2000); // Tampilkan pesan selama 2 detik
  
  long reading = scale.get_units(10);  // Get average reading
  float calibrationFactor = reading / (knownWeight * 1000);  // Calculate calibration factor
  scale.set_scale(calibrationFactor);  // Set the calibration factor
  
  Serial.print("Calibration factor set to: ");
  Serial.println(calibrationFactor);
  lcd.clear();
  lcd.print("Calibrated to: ");
  lcd.print(knownWeight);
  lcd.print(" kg   ");
  delay(2000); // Show calibration message for 2 seconds
}