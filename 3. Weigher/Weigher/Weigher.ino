/*
  V. 0.0.1 Beta
  Update Terakhir : 22-12-2023
  Last Change Log {

  }

  Komponen :
  1. ESP32
  2. Load Cell 5 Kg
  3. HX711
  4. LCD I2C 20x4

  Program ini berfungsi mengukur berat objek dengan maksimal 5 Kg
  Output dari program ini adalah rejector*

  Tidak dibuat tim IT
  Fitur:
  1. Pilih Produk
  2. Kalibrasi
  3. 
*/

// == Deklarasi semua Library yang digunakan ==
#include <Arduino.h>
#include "soc/rtc.h"
#include "HX711.h"

// HX711 Wiring
const int LOADCELL_DOUT_PIN = 16;  // Pin RX2
const int LOADCELL_SCK_PIN = 4;

HX711 scale;

void setup() {
  Serial.begin(115200);

  /* Dikarenakan CPU ESP32 yang terlalu cepat, maka frekuensinya harus dikurangi
  Dalam beberapa kasus, hal ini tidak perlu dilakukan, maka comment baris program ini
  */
  rtc_cpu_freq_config_t config;
  rtc_clk_cpu_freq_get_config(&config);
  rtc_clk_cpu_freq_to_config(RTC_CPU_FREQ_80M, &config);
  rtc_clk_cpu_freq_set_config_fast(&config);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // print a raw reading from the ADC
  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());

  // print the average of 20 readings from the ADC
  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));

  // print the average of 5 readings from the ADC minus the tare weight (not set yet)
  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));

  // print the average of 5 readings from the ADC minus tare weight (not set) divided
  // by the SCALE parameter (not set yet)
  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);


  // this value is obtained by calibrating the scale with known weights; see the README for details
  // scale.set_scale(-471.497);
  // scale.set_scale(INSERT YOUR CALIBRATION FACTOR);
  scale.tare();  // reset the scale to 0

  Serial.println("After setting up the scale:");
  // print a raw reading from the ADC
  Serial.print("read: \t\t");
  Serial.println(scale.read());

  // print the average of 20 readings from the ADC
  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));

  // print the average of 5 readings from the ADC minus the tare weight, set with tare()
  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));

  // print the average of 5 readings from the ADC minus tare weight, divided
  // by the SCALE parameter set with set_scale
  Serial.print("get units: \t\t");


  
  Serial.println(scale.get_units(5), 1);

  Serial.println("Readings:");
}

void loop() {
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 5);

  scale.power_down();  // put the ADC in sleep mode
  delay(5000);
  scale.power_up();
}
