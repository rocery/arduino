# Projek IoT STTB
info : sastranuralamsyah@gmail.com
```
1. 192.168.15.211 - ESP8266 - IoT-211-Downtime_Molding_1
2. 192.168.15.212
3. 192.168.15.213 - ESP32 - IoT-213-Counter_Kerupuk
4. 192.168.15.214
5. 192.168.15.215 - ESP8266 - IoT-215-Stuffle_Mie_Biskuit
6. 192.168.15.216 - ESP8266 - IoT-216-Kerupuk-Gudang_Jadi
7. 192.168.15.217 - ESP32 - IoT-217-Suhu_Oven_30
8. 192.168.15.218 - ESP32 - IoT-218-Counter_Biskuit

192.168.7.251 - ESP32 - IoT-251-TK0532 - Pipa Angin Teknik - Kerupuk
192.168.7.252 - ESP32 - IoT-252-TK0532 - Pipa Angin Teknik - Biskuit
192.168.7.253 - ESP32 - IoT-253-KR0532 - Pipa Angin Kerupuk - Teknik
192.168.7.254 - ESP32 - IoT-254-BM0532 - Pipa Angin Biskuit - Teknik
```

## Used IP:
```
- 223	= Server (Tidak Boleh diganti)
- 10	= Admin (C-010-IT)
- 243	= Test Address
- 1	= Reserved IP
- 211	= IoT-211-BM0466
- 213	= IoT-213-KR0132
- 215	= IoT-215-BM0366
- 216	= IoT-216-KR0366
- 217	= IoT-217-KR0232
- 218	= IoT-218-BM0132
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
