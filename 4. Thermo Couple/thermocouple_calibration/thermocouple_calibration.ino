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

const String deviceName = "Suhu Oven 27 - Kerupuk";
const String api_sendLogData = "http://192.168.7.223/temperature_api/saveTemperature.php";
const String api_sendLogData2 = "http://192.168.7.223/temperature_api/saveTemperature15Minutes.php";
const String api_getCalibrationValue = "http://192.168.7.223/temperature_api/getCalibrationValue.php";
const String deviceID = "27";

/*
    Thermocouple Type K
    MAX6675 Pin declaration
*/
#define thermoSO 19
#define thermoCS 18
#define thermoSCK 5
float tempReal;

/* Mendeklarasikan LCD dengan alamat I2C 0x27
   Total kolom 20
   Total baris 4 */
   LiquidCrystal_I2C lcd(0x27, 16, 2);
   uint8_t degree[8] = {
     0x08,
     0x14,
     0x14,
     0x08,
     0x00,
     0x00,
     0x00,
     0x00
   };