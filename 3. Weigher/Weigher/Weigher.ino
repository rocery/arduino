#include <HX711.h>

// HX711 Wiring
const int LOADCELL_DOUT_PIN = 27;
const int LOADCELL_SCK_PIN = 26;
HX711 scale;

#define buttonUp 34
#define buttonDown 35
#define buttonSelect 32

void setup() {
  Serial.begin(9600);
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonSelect, INPUT_PULLUP);

  scale.tare();
}

void loop() {
  if (scale.is_ready()) {
    long reading = scale.get_units(10);
    Serial.print("Hasil : ");
    Serial.println(reading);
  } else {
    Serial.println("HX711 not found.");
  }
  delay(1000);
}

bool isButtonPressed(int buttonPin) {
  return !digitalRead(buttonPin);
}