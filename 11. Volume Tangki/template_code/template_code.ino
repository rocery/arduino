/*
    Volume Tangki Minyak
    Menghitung volume tangki minyak berdasarkan dimensi yang diberikan.
    
    Dibuat oleh: [Sastra Nur Alamsyah]
    Tanggal: [02-12-2026]
    
    Deskripsi:
    Program ini menghitung volume tangki minyak berbentuk silinder atau kubus
    berdasarkan input dimensi dari pengguna. Hasil volume ditampilkan dalam liter.
    
    Catatan:
    - Pastikan untuk memasukkan dimensi dalam satuan yang sesuai (cm).
    - Volume dihitung dalam liter (1 liter = 1000 cm³).
*/

#include <HardwareSerial.h>

// ========== KONFIGURASI ==========
#define SENSOR_RX 16  // Pin RX2 ESP32 ke TX sensor
#define SENSOR_TX 17  // Pin TX2 ESP32 ke RX sensor

// Dimensi Tank
const float TANK_HEIGHT = 600.0;      // cm
const float TANK_RADIUS = 150.0;      // cm (diameter 3m / 2)
const float SENSOR_OFFSET = 0.0;      // cm, jarak sensor dari puncak tank

// Batas pengukuran sensor
const int MIN_DISTANCE = 28;          // cm (dead zone sensor)
const int MAX_DISTANCE = 750;         // cm (max range sensor based on datasheet)

// Interval pembacaan
const unsigned long READ_INTERVAL = 2000;  // ms

