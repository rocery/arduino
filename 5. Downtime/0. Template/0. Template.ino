/*
  V 0.9.1
  Update Terakhir : 03-08-2026
  Last Change Log {
    1. 
  }

  Komponen:
  1. NodeMCU ESP8266 V.3
  2. PZEM-004T V 3.0
  3. LED

  Program ini berfungsi menghitung instrumen listrik pada kabel.
  Projek ini mengunakan ESP8266 sebagai micro controller karena memiliki fitur SerialSoftware, sehingga komunikasi UART bisa digunakan sebanyak mungkin selama address-nya berbeda dan daya dari ESP8266 kuat.

  Proses koneksi WiFi tidak disarakn menggunakan Library MultiWiFiESP8266.h, kecuali Micon yang digunakan ESP32
*/