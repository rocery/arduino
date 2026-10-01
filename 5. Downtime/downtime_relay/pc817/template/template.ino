/*
  V 0.0.2
  Update Terakhir : 01-10-2026
  Last Change Log {
    1. Memperbaiki pembacaan GPIO PC817
    2. Menggunakan array GPIO agar pin tidak diasumsikan berurutan
    3.
    4.
  }

  Komponen:
  1. ESP32 DOIT
  2. Optocoupler PC817 @4
  3. LED @1

  Program ini berfungsi menghitung instrumen listrik pada kabel.
*/

#define PC817_PIN_1 27
#define PC817_PIN_2 26
#define PC817_PIN_3 25
#define PC817_PIN_4 33

const uint8_t PC817_PINS[4] = {
  PC817_PIN_1,
  PC817_PIN_2,
  PC817_PIN_3,
  PC817_PIN_4
};

int pinState[4];

void setup()
{
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(PC817_PINS[i], INPUT_PULLUP);
  }
}

void loop()
{
  // Baca semua input PC817
  for (int i = 0; i < 4; i++) {
    pinState[i] = digitalRead(PC817_PINS[i]);
  }

  // Tampilkan status
  for (int i = 0; i < 4; i++) {
    Serial.print(pinState[i]);

    if (i < 3) {
      Serial.print(" ");
    }
  }

  Serial.println();

  delay(100);
}
