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

// HX711 Wiring
const int LOADCELL_DOUT_PIN = 26;  // Pin RX2
const int LOADCELL_SCK_PIN = 27;

HX711 scale;
float beratBarang = 20000;  // Contoh 300 g, sesuaikan dengan barang yang ada
float calibrationFactor;

void setup() {
  Serial.begin(9600);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {
  if (scale.is_ready()) {
    scale.set_scale();
    Serial.println("Kosongkan timbangan");
    delay(5000);
    scale.tare();
    Serial.println("Tare done...");
    Serial.print("Taruh barang yang sudah diketahui beratnya");
    delay(5000);
    long reading = scale.get_units(10);
    Serial.print("Hasil : ");
    Serial.println(reading);
    calibrationFactor = reading / beratBarang;
    Serial.print("Faktor Kalibrasi: ");
    Serial.println(calibrationFactor);
  } else {
    Serial.println("HX711 not found.");
  }
  delay(5000);
}