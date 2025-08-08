/*
  V. 1.1.0
  Update Terakhir : 13-01-2025

  Versi Board ESP32 : V. 3.1.1

  PENTING = Harus menggunakan Dual Core Micro Controller/Microprocessor
  Komponen:
  1. Micro Controller : ESP32
  2. LCD I2C 20x4                               (3.3v, GND, I2C (22 = SCL, 21 = SDA))
  3. RTC DS3231                                 (3.3v, GND, I2C (22 = SCL, 21 = SDA))
  4. IR Sensor                                  (5v/Vin, GND, 13)
  5. Module SD Card + SD Card (FAT32 1-16 GB)   (3.3v, GND, SPI(Mosi 23, Miso 19, CLK 18, CS 5))
  6. Tacticle Button 1x1 cm @3                  (3.3v, GND, 34, 35, 25)

  Semua fungsi Serial.print() pada program ini sebenarnya bisa dihapus/di-comment,
  masih dipertahankan untuk fungsi debuging. Akan di-comment/dihapus pada saat final
  program sudah tercapai demi menghemat resource pada ESP32.
  
*/

// ===== Add All the Libraries
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <LiquidCrystal_I2C.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <RTClib.h>
#include "time.h"

// ===== Device Identity
const String deviceName = "Counter - Tic Tic";
const int deviceID = 213;

/* ==== LCD I2C 0x27 
  Column 20
  Row 4 */
LiquidCrystal_I2C lcd(0x27, 20, 4);

/* ==== Button 
  There are 3 buttons => up, down and select
*/
#define upButton 34
#define downButton 35
#define selectButton 32

/* IR Sensor 
  There are 2 sensors (counter and reject).
  You can choose use either 1 or 2 sensors.
*/
#define sensorPin 14
#define rejectPin 27

