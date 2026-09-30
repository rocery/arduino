/*
  V 0.0.1
  Update Terakhir : 30-09-2026
  Last Change Log {
    1. 
    2. 
    3. 
    4. 
  }

  Komponen:
  1. ESP32 DOIT
  2. Optocoupler PC817
  3. LED @1

  Program ini berfungsi menghitung instrumen listrik pada kabel.
*/

#define PC8127_PIN_1 27
#define PC8127_PIN_2 26
#define PC8127_PIN_3 25
#define PC8127_PIN_4 33

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  digitalRead(PC8127_PIN_1);
}