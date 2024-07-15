# Projek IoT STTB

info : sastranuralamsyah@gmail.com

---

## Important!

```
Library ada di folder '/libraries'
Semua file yang berhubungan sudah ada di '/fritzing library' dan '/fritzing schematic'
Arduino IDE yang digunakan versi 2++

Link Board ESP32 dan ESP8266:
http://arduino.esp8266.com/stable/package_esp8266com_index.json
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

## Error Note
```
Errors below can be ignored:  
WARNING: library LiquidCrystal I2C claims to run on avr architecture(s) and 
may be incompatible with your current board which runs on esp32 architecture(s).

In Linux bases OS, in order to upload code, you need to add your user to dialout group.
Just run this command: 
sudo usermod -G dialout $USER

```

---
## Used IP:

```
- 223    = Server (Tidak Boleh diganti)
- 211    = IoT-211-Downtime_Molding_1
- 213    = IoT-213-Counter_Kerupuk
- 215    = IoT-215-Stuffle_Mie_Biskuit
- 216    = IoT-216-Kerupuk-Gudang_Jadi
- 217    = IoT-217-Suhu_Oven_30
- 218    = IoT-218-Counter_Biskuit
- 128    = IoT-128-Suhu_Oven_28
- 129    = IoT-129-Suhu_Oven_29
- 251    = IoT-251-Pipa Angin Teknik - Kerupuk
- 252    = IoT-252-Pipa Angin Teknik - Biskuit
- 253    = IoT-253-Pipa Angin Biskuit - Teknik
- 254    = IoT-254-Pipa Angin Kerupuk - Teknik
- 200    = IoT-200-Doolock_Gym
- 219    = IoT-219-Counter_Mie_Hancur
- 49     = Komputer IT
```

## Aturan Penamaan ID/Host Name/Device Name pada Alat IoT

```
- IoT-
- 3 digit akhir Ip Address
- Kode Lokasi
  - KR Produksi Kerupuk
  - BM Produksi Biskuit/Mie
  - TK Teknik
- Fungsi alat
  - 01 Counter
  - 02 Suhu
  - 03 Monitoring Operasional Conveyor
  - 04 Downtime Alat
  - 05 Barometer
- Jenis Micon
  - 32 ESP32
  - 66 ESP8266
### Contoh:
- IoT-211-BM0466 = Produksi Biskuit/Mie - Downtime Alat - 8266 - Molding Line 1 Biskuit
- IoT-213-KR0132 = Produksi Kerupuk - Counter - ESP32 - Tic-Tic CBP 2000
- IoT-215-BM0366 = Produksi Biskuit/Mie - Monitoring Operasional Conveyor - 8266 - Stuffle Mie dan Biskuit
- IoT-216-KR0366 = Produksi Kerupuk - Monitoring Operasional Conveyor - 8266 - Stuffle Kerupuk-Gudang Jadi
- IoT-217-KR0232 = Produksi Kerupuk - Suhu - ESP32 - Oven 30
- IoT-218-BM0132 = Produksi Biskuit/Mie - Counter - ESP32 - Goriorio Isi 2 - FC:B4:67:51:47:C0
```

Catatan:
Kuning - Putih Data
Merah - Ungu VCC
Hitam - Biru GND
