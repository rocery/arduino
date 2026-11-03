# Algoritma Weigher ESP32 (WiFi, Keypad 1x4)

## Komponen

1. HX711 + Load Cell
2. ESP32 DOIT DevKit V1 30 Pin
3. Modul Micro SD Card (SPI)
4. Modul RTC DS3231 (I2C)
5. Membrane Keypad 1x4
6. LCD 20x4 I2C

## Koneksi Pin ke ESP32

| Perangkat | Pin ESP32 | Keterangan |
| --- | --- | --- |
| HX711 | SCK = 27, DOUT = 26 | Load cell terhubung ke HX711 |
| Micro SD (VSPI) | SCK = 18, MISO = 19, MOSI = 23, CS = 5 | SPI |
| RTC DS3231 | SDA = 21, SCL = 22 | I2C, alamat 0x68 |
| LCD 20x4 | SDA = 21, SCL = 22 | I2C, alamat 0x27 (satu bus dengan RTC) |
| Tombol 1 | GPIO 32 | `INPUT_PULLUP`, aktif LOW |
| Tombol 2 | GPIO 33 | `INPUT_PULLUP`, aktif LOW |
| Tombol 3 | GPIO 25 | `INPUT_PULLUP`, aktif LOW |
| Tombol 4 | GPIO 13 | `INPUT_PULLUP`, aktif LOW |

Catatan pin:

- GPIO 34 dan 35 diganti karena hanya input dan tidak punya pull-up internal.
- Keempat tombol memakai pin yang punya pull-up internal, satu kaki tombol ke GND. Logika seragam: ditekan = LOW.
- Hindari GPIO 0, 2, 12, 15 (strapping pin) untuk tombol.

## Fungsi Tombol

| Tombol | Layar utama | Menu pilih produk | Kalibrasi |
| --- | --- | --- | --- |
| 1 | Tahan > 3 detik: kirim log | Turun (bawah) | Kalibrasi ulang dari awal |
| 2 | Ganti produk | Naik (atas) | Naik (ubah nilai) |
| 3 | Tare | Tidak dipakai | Lanjut |
| 4 | Timbang (tahan > 200 ms) | Tidak dipakai | Konfirmasi selesai |

Saat boot, tombol 1 + 2 ditekan bersamaan masuk ke kalibrasi.

## Algoritma

### 1. Inisialisasi (`setup()`)

1. Mulai Serial, LCD, EEPROM/Preferences, HX711, RTC, dan SD Card.
2. Jika tombol 1 + 2 ditekan saat boot, masuk **Kalibrasi** (bagian 2).
3. Jika tidak, baca `calibrationFactor` dan `digitScale` dari EEPROM, lalu `set_scale()` dan `tare()`.
4. Cek SD Card:
   - Gagal: `sdStatus = false`, tampilkan peringatan di LCD. Timbangan tetap bisa dipakai, data hanya dikirim ke database.
   - Berhasil: `sdStatus = true`, baca counter terakhir.
5. Hubungkan WiFi (bagian 3). Jika terhubung, sinkronkan RTC dengan NTP.
6. Tampilkan menu **Pilih Produk** (bagian 4).
7. Buat mutex untuk SD, LCD, dan WiFiClient.
8. Buat task WiFi/pengiriman di core 0 (`xTaskCreatePinnedToCore`). `loop()` di core 1 menangani timbang dan tampilan.

### 2. Kalibrasi

Tombol: 1 = ulang dari awal, 2 = naik, 3 = lanjut, 4 = konfirmasi selesai.

1. Pilih berat acuan (1/2/5/10/20/40 kg) dengan tombol 2, lanjut dengan tombol 3.
2. LCD meminta timbangan dikosongkan, hitung mundur lalu `tare()`. Lanjut dengan tombol 3.
3. Letakkan beban acuan. LCD menampilkan `calibrationFactor = pembacaan / (berat × 1000)` secara langsung. Lanjut dengan tombol 3.
4. Pilih jumlah digit desimal (0-4) dengan tombol 2, lanjut dengan tombol 3.
5. Tombol 4 menyimpan `calibrationFactor` dan `digitScale` ke EEPROM (hanya jika nilainya berubah) lalu selesai. Tombol 1 mengulang dari langkah 1.

### 3. Koneksi WiFi (auto connect, IP statis)

1. Simpan daftar SSID dan password dalam array berurutan prioritas.
2. Panggil `WiFi.config(ip, gateway, subnet, dns)` sebelum `WiFi.begin()`. Semua WiFi satu subnet, jadi satu konfigurasi IP cukup.
3. Scan jaringan, pilih SSID dari daftar yang terdeteksi dengan sinyal terkuat.
4. Coba sambung dengan timeout (misalnya 10 detik). Jika gagal, coba SSID berikutnya.
5. Task WiFi memeriksa status setiap beberapa detik. Jika putus, ulangi langkah 3-4 tanpa mengganggu `loop()`.
6. Saat WiFi tersambung, sinkronkan RTC dengan NTP secara berkala. RTC tetap sumber waktu utama.
7. LCD menampilkan `W` (terhubung) atau `X` (putus), beserta SSID.

### 4. Pilih produk

Daftar produk ditulis langsung di kode (hardcoded).

1. Tombol 1 menggeser pilihan ke bawah, tombol 2 ke atas. Tombol 3 dan 4 tidak dipakai.
2. Hasil pilihan disimpan di `productSelected`.
3. Dari layar utama, tombol 2 membuka kembali menu ini.

### 5. Loop utama

1. Baca `get_units(5)`, ubah ke kg, format sesuai `digitScale` dengan `dtostrf()`.
2. LCD 20x4 menampilkan produk, berat, status WiFi + SSID, IP, status SD, counter berhasil/gagal, dan jam (format `%02d`).
3. Tombol 3: jalankan `tare()`.
4. Tombol 4 ditahan > 200 ms: proses timbang (bagian 6).
5. Tombol 2: buka menu ganti produk.
6. Tombol 1 ditahan > 3 detik: kirim log (bagian 7). LCD menampilkan hitung mundur saat ditahan.
7. Gunakan debounce dan `vTaskDelay`/`delay` kecil agar tidak memicu watchdog.

### 6. Proses timbang (tombol 4 > 200 ms)

1. Ambil berat saat itu. Jika berat 0 (atau di bawah batas minimum), tidak melakukan apa-apa.
2. Ambil waktu dari RTC, buat ID unik (`deviceID + epoch + nomor urut`).
3. Susun satu baris data: `id, deviceID, produk, berat, waktu, IP, SSID`.
4. **Simpan ke SD** (`weigherLog.txt`). Jika gagal, tampilkan "DATA GGL DISAVE" dan naikkan counter gagal SD.
5. **Kirim ke database** via HTTP POST ke API (tabel `weigher_realtime`). Jika WiFi putus atau respons gagal, tampilkan "GAGAL KIRIM".
6. Naikkan counter berhasil atau gagal, simpan counter ke SD, lalu tampilkan hasil di LCD.
7. Satu data timbang sengaja diproses dua kali (database dan log SD). Server menolak duplikat berdasarkan ID unik pada tabel yang sama.

### 7. Kirim log (tombol 1 > 3 detik)

1. Cek WiFi dan file log. Jika WiFi putus atau file kosong, tampilkan pesan lalu batal.
2. Hitung jumlah baris file log.
3. Kirim file ke API (tabel `weigher_log`) per chunk (512 byte).
4. Baca respons HTTP dan parse JSON (`status`, `jumlah_data`).
5. Jika `status == "success"` dan `jumlah_data` sama dengan jumlah baris file:
   - Hapus file log dan reset counter.
   - Simpan `jumlah_data` sebagai "terkirim terakhir".
6. Jika gagal, file tidak dihapus dan LCD menampilkan "GAGAL".

## Catatan perancangan

- SD, LCD, dan WiFiClient diakses dari dua core, jadi lindungi dengan mutex.
- Hapus log hanya setelah server mengonfirmasi jumlah data.
- Format waktu memakai nol di depan (`%02d`).
- RTC DS3231 sebagai sumber waktu utama, NTP hanya untuk koreksi.

## Poin yang masih perlu diputuskan

1. **Konfirmasi pilihan produk:** tombol 3/4 tidak dipakai, sehingga perlu cara menyelesaikan pemilihan. Usulan: pilihan otomatis terkunci setelah 3 detik tanpa tekanan, atau tombol 1 + 2 bersamaan.
2. **Batas minimum berat:** apakah "0" berarti persis 0, atau di bawah batas tertentu (misalnya < 0,05 kg) agar noise tidak tersimpan?
