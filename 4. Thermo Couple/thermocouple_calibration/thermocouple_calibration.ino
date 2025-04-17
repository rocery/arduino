/*
    V. 0.0.1 Alpha
    17-04-2025

    Versi ini dibuat dengan alasan:
    1. Tidak menggunakan potesiometer sebagai alat kalibrasi
    2. Kalibrasi dilakukan memlalui value yang diatur database

*/

// == Library ==
#include <max6675.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

