/*
  V. 1.1.0
  Update Terakhir : 13-01-2025

  PENTING = Harus menggunakan Dual Core Micro Controller/Microprocessor
  Komponen:
  1. Micro Controller : ESP32
  2. LCD I2C 20x4                               (3.3v, GND, I2C (22 = SCL, 21 = SDA))
  3. RTC DS3231                                 (3.3v, GND, I2C (22 = SCL, 21 = SDA))
  4. IR Sensor                                  (5v/Vin, GND, 13)
  5. Module SD Card + SD Card (FAT32 1-16 GB)   (3.3v, GND, SPI(Mosi 23, Miso 19, CLK 18, CS 5))
  6. Tacticle Button 1x1 cm @3                  (3.3v, GND, 34, 35, 25)
  -- Belum diimplementasikan --
  7. Active Buzzer 3-5 v                        (3.3v, 26)
  8. Fan 5V                                     (5v/Vin, GND)

  Program ini berfungsi untuk melakukan penghitungan barang pada conveyor.
  Penjelasan program terdapat pada comment baris pada program.

  Semua fungsi Serial.print() pada program ini sebenarnya bisa dihapus/di-comment,
  masih dipertahankan untuk fungsi debuging. Akan di-comment/dihapus pada saat final
  program sudah tercapai demi menghemat resource pada ESP32.
  
*/

