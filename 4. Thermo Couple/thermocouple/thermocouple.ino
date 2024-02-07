/*
  V. 0.1.0
  Update Terakhir : 18-01-2024
  Last Change Log {
    2024
    - Januari
    1. Program utama MAX 7765
    2. Tambah fungsi WiFi
    3. Tambah fungsi sendData
    4. Penambahan fungsi debug
  }

  Komponen:
  1. Micro Controller : ESP32
  -- Belum diimplementasikan --
  2. LCD I2C 20x4                               (3.3v, GND, I2C (22 = SCL, 21 = SDA))
  3. Max 6675 + Thermocouple Type K             (3.3v, SPI (Miso = 19, CLK = 18, CS = 5))
  4. Fan 5V                                     (5v/Vin, GND)

  Hasil pembacaan temperature sensor 3. Max 6675 + Thermocouple Type K perlu dikalibrasi
  dengan cara membandingkan dengan termometer yang sudah ada dan diyakini benar pembacaanya.
  Edit variabel tempCalibrationValue sesuai dengan selisih nilai sensor Thermpcouple dan
  termometer. Sensor ini hanya digunakan pada ruangan bertemperatur tinggi, tidak disarankan
  untuk mengukur suhu ruangan.

  Program ini berfungsi untuk mengukur suhu pada tempat panas >70'C.
  Sensor Thermocouple memiliki toleransi akurasi ukur +- 2 C
  Penjelasan program terdapat pada comment baris pada program.

  Semua fungsi Serial.print() pada program ini sebenarnya bisa dihapus/di-comment,
  masih dipertahankan untuk fungsi debuging. Akan di-comment/dihapus pada saat final
  program sudah tercapai demi menghemat rosource pada ESP32.

  Informasi lebih lengkap sensor yang digunakan ada pada link berikut:
  https://learn.adafruit.com/thermocouple/
*/

// == Library ==
#include <max6675.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

/* Set pin MAX6675 pada pin SPI.
  Jika thermocouple yang digunakan lebih dari 1,
  bedakan semua pin (tidak perlu menggunakan pin SPI).
  Edit variabvel tempCalibrationValue sesuai denggan nilai kalibrasi yang sudah dilakukan. */
#define thermoSO 19
#define thermoCS 18
#define thermoSCK 5
float tempValue;
MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);
const float tempCalibrationValue = 0;

/* Mendeklarasikan LCD dengan alamat I2C 0x27
   Total kolom 20
   Total baris 4 */
LiquidCrystal_I2C lcd(0x27, 20, 4);
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

uint8_t plusMinus[8] = {
  0x00,
  0x04,
  0x04,
  0x1F,
  0x04,
  0x04,
  0x00,
  0x1F
};

// == WiFi Config ==
/* Deklarasikan semua WiFi yang bisa diakses oleh ESP32
   ESP32 akan memilih WiFi dengan sinyal paling kuat secara otomatis */
WiFiMulti wifiMulti;
const char* ssid_a = "STTB1";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "MT1";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
const char* ssid_d = "Djoksen";
const char* password_d = "Welive99";
const char* ssid_it = "STTB5";
const char* password_it = "siantar123";

// Set IP to Static
IPAddress staticIP(192, 168, 15, 214);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional
String ip_Address;

// == Data Send/Get ==
String postData, api;
bool sendStatus;

// == Get NTP/RTC ==
const char* ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 7 * 3600;  // Karena Bekasi ada di GMT+7, maka Offset ditambah 7 jam
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat;
int year, month, day, hour, minute, second;
bool ntpStatus;

void sendLogData() {
  /* Mengirim data ke local server
     Ganti isi variabel api sesuai dengan form php
  */
  api = "shttp://192.168.15.221/counter_hsit_api/saveCounter.pshp";
  HTTPClient http;
  http.begin(api);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST");
  }
  http.end();
}

void getLocalTime() {
  /* Fungsi bertujuan menerima update waktu
     lokal dari ntp.pool.org */
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    ntpStatus = false;
  } else {
    ntpStatus = true;

    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    dateTime = String(timeStringBuff);

    // Save time data to variabel
    year = timeinfo.tm_year + 1900;
    month = timeinfo.tm_mon + 1;
    day = timeinfo.tm_mday;
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    second = timeinfo.tm_sec;
    // YYYY-MM-DD
    dateFormat = String(year) + '-' + String(month) + '-' + String(day);
    // hh:mm:ss
    timeFormat = String(hour) + ':' + String(minute);
  }
}

void debugSerial() {
  /* Fungsi ini bertujuan untuk debug project via Serial monitor
     Isi fungsi ini adalah kumpulan Serial.print merujuk pada fungsi lainnya
  */
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi Not Connected");
  }

  if (ntpStatus == false) {
    Serial.println("NTP Fail");
  } else {
    Serial.println(dateTime);
  }

  Serial.println(tempValue);
}

void setup() {
  Serial.begin(115200);

  sendStatus = false;
  ntpStatus = false;

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.createChar(0, degree);
  lcd.createChar(1, plusMinus);
  lcd.home();

  // WiFi
  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);
  wifiMulti.addAP(ssid_d, password_d);
  wifiMulti.addAP(ssid_it, password_it);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  wifiMulti.run();
  lcd.setCursor(0, 1);
  lcd.print("Connecting to WiFi");
  delay(1000);

  // NTP
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  if (wifiMulti.run() == WL_CONNECTED) {
    int tryNTP = 0;
    while (ntpStatus == false && tryNTP <= 5) {
      getLocalTime();
      tryNTP++;
      delay(50);
      lcd.setCursor(0, 2);
      lcd.print("Getting Date/Time");
    }
  }
  lcd.clear();
}

void loop() {
  lcd.setCursor(2, 0);
  lcd.print("PENGUKURAN SUHU");

  tempValue = thermocouple.readCelsius() + tempCalibrationValue;
  lcd.setCursor(1, 1);
  lcd.print("Suhu : ");
  lcd.setCursor(8, 1);
  lcd.print(tempValue);
  lcd.print(" ");
  lcd.write(0);
  lcd.print("C");

  int tryWiFi = 0;
  while (wifiMulti.run() != WL_CONNECTED && tryWiFi <= 5) {
    tryWiFi++;
    lcd.setCursor(1, 2);
    lcd.print("Try " + String(tryWiFi));
    wifiMulti.run();
  }

  if (wifiMulti.run() == WL_CONNECTED) {
    lcd.setCursor(1, 2);
    lcd.print("WiFi : " + WiFi.SSID());

    int tryNTP = 0;
    while (ntpStatus == false && tryNTP <= 2) {
      getLocalTime();
      tryNTP++;
      delay(50);
    }

    getLocalTime();
    // Print Waktu hh:mm:ss
    lcd.setCursor(1, 3);
    lcd.print(dateTime);
  } else if (wifiMulti.run() != WL_CONNECTED) {
    lcd.setCursor(1, 3);
    lcd.print("WiFi DC");
  }

  ip_Address = WiFi.localIP().toString();
  /* Pengiriman data ke DB dilakukan setiap 15 detik
      Jika dirasa terlalu sering, ganti value second sesuai keperluan
      Misal, if ((second == 0 || second == 30) && !sendStatus) --> Akan mengirim data setiap detik 0 dan 30
    */
  if ((second == 15 || second == 30 || second == 45 || second == 0) && !sendStatus) {
    // Edit variabel postData sesuai php
    postData = "&ip_address=" + ip_Address;
    sendLogData();
    sendStatus = true;
    lcd.clear();
  } else if ((second == 1 || second == 28 || second == 43 || second == 58) && sendStatus) {
    sendStatus = false;
  }
  debugSerial();
  // MAX6675 memerlukan delay setidaknya 250ms
  delay(900);
}
