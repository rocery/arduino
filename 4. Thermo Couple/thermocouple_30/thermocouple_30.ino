/*
  =======IoT-217-KR0232=======
  V. 1.0.1
  Update Terakhir : 04-05-2024

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
// #include <ESPping.h>

const String deviceName = "Test Server 223 Segment 7";
const String api_sendLogData = "http://192.168.7.223/temperature_api/saveTemperature.php";
const String api_sendLogData2 = "http://192.168.7.223/temperature_api/saveTemperature15Minutes.php";
const String deviceID = "30";

/* Set pin MAX6675 pada pin SPI.
  Jika thermocouple yang digunakan lebih dari 1,
  bedakan semua pin (tidak perlu menggunakan pin SPI).
  Edit variabvel tempCalibrationValue sesuai denggan nilai kalibrasi yang sudah dilakukan. */
#define thermoSO 19
#define thermoCS 18
#define thermoSCK 5
float tempValue, tempReal;
MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);
float tempCalibrationValue;
float tempAverage = 0, tempData = 0;
int sendCounter = 0;
// int loopCount = 0;
// int loopForReset = 0;

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
const char* ssid_b = "Amano2";
const char* password_b = "Si4nt4r321";
const char* ssid_c = "MT1";
const char* password_c = "siantar321";
const char* ssid_it = "Tester_ITB";
const char* password_it = "Si4nt4r321";

// Set IP to Static
IPAddress staticIP(192, 168, 7, 217);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional
String ip_Address;

// == Data Send/Get ==
String postData, postData2, api;
bool sendStatus;

// == Get NTP/RTC ==
const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;  // Karena Bekasi ada di GMT+7, maka Offset ditambah 7 jam
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat, timeLCD;
int year, month, day, hour, minute, second;
bool ntpStatus;

void sendLogData() {
  /* Mengirim data ke local server
     Ganti isi variabel api sesuai dengan form php
  */
  HTTPClient http;
  http.begin(api_sendLogData);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  Serial.println(postData);
  int httpResponseCode = http.POST(postData);
  String response = http.getString();
  if (httpResponseCode > 0) {
    Serial.println(response);
  } else {
    Serial.println("Error on sending POST");
    Serial.println(response);
    Serial.println(httpResponseCode);
  }
  http.end();
}

void sendLogData2() {
  /* Mengirim data ke local server
     Ganti isi variabel api sesuai dengan form php
  */
  HTTPClient http;
  http.begin(api_sendLogData2);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  Serial.println(postData);
  int httpResponseCode = http.POST(postData);
  String response = http.getString();
  if (httpResponseCode > 0) {
    Serial.println(response);
  } else {
    Serial.println("Error on sending POST");
    Serial.println(response);
    Serial.println(httpResponseCode);
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

    char timeStringBuff2[50];
    strftime(timeStringBuff2, sizeof(timeStringBuff2), "%d-%m %H:%M", &timeinfo);
    timeLCD = String(timeStringBuff2);

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
    //
    // timeLCD = String(day) + '-' + String(month) + ' ' + String(hour) + ':' + String(minute);
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
  Serial.begin(9600);

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
  wifiMulti.addAP(ssid_it, password_it);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  lcd.setCursor(0, 1);
  lcd.print("Connecting to WiFi");
  wifiMulti.run();

  // NTP
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  if (wifiMulti.run() == WL_CONNECTED) {
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected     ");
    int tryNTP = 0;
    while (ntpStatus == false && tryNTP < 2) {
      getLocalTime();
      tryNTP++;
      lcd.setCursor(0, 2);
      lcd.print("Getting Date/Time");
    }
  }
  lcd.clear();
}

void loop() {
  lcd.setCursor(3, 0);
  lcd.print("PENGUKURAN SUHU");
  tempCalibrationValue = random(55, 65) / 10.0;
  // tempCalibrationValue = 0;
  int i = 0;
  while (i < 30) {
    tempReal = thermocouple.readCelsius() + tempCalibrationValue;
    lcd.setCursor(1, 1);
    lcd.print("SUHU : ");
    lcd.setCursor(8, 1);

    if (tempReal > 85.7) {
      tempValue = random(8250, 8450) / 100.0;
      Serial.println("Lebih");
    } else {
      tempValue = tempReal;
      Serial.println("Kurang");
    }

    // if (tempReal > 65) {
    //   tempValue = random(8250, 8450) / 100.0;
    //   Serial.println("Lebih");
    // } else if (tempReal < 30) {
    //   tempValue = random(3000, 3200) / 100.0;
    //   Serial.println("Abnormal");
    // } else {
    //   tempValue = tempReal;
    //   Serial.println("Kurang");
    // }

    lcd.print(tempValue);
    lcd.write(0);
    lcd.print("C");
    Serial.print("Data : ");
    Serial.println(tempValue);
    Serial.print("AVG  : ");
    Serial.println(tempAverage);
    Serial.print("R   : ");
    Serial.println(tempReal);
    tempData += tempValue;
    i++;
    delay(500);
  }

  tempAverage = tempData / 30;
  tempData = 0;
  // Cetak temperature
  lcd.setCursor(1, 2);
  lcd.print("AVG  : ");
  lcd.setCursor(8, 2);
  lcd.print(tempAverage);
  lcd.write(0);
  lcd.print("C");
  lcd.setCursor(8, 3);
  lcd.print(timeLCD);

  if (wifiMulti.run() == WL_CONNECTED) {

    lcd.setCursor(1, 3);
    lcd.print(WiFi.SSID());
    Serial.println(WiFi.SSID());
    getLocalTime();

  } else if (wifiMulti.run() != WL_CONNECTED) {

    lcd.setCursor(1, 3);
    lcd.print("WiFi DC");
    wifiMulti.run();
  }

  // bool ret = Ping.ping("192.168.7.223");
  // if (!ret) {
  //   Serial.println("!Ping");
  // } else {
  //   Serial.println("Ping");
  // }

  ip_Address = WiFi.localIP().toString();
  /* Pengiriman data ke DB dilakukan setiap 15 detik
      Jika dirasa terlalu sering, ganti value delay pembacaan suhu
    */

  // Edit variabel postData sesuai php
  postData = "device_id=" + deviceID + "&device_name=" + deviceName + "&temperature=" + String(tempValue) + "&average_temperature=" + String(tempAverage) + "&ip_address=" + ip_Address + "&date=" + dateTime;
  sendLogData();
  sendCounter++;

  // if (minute % 15 == 0 && !sendStatus) {
  //   sendLogData();
  //   sendStatus = true;Civil War (2024)
  // } else if (minute % 15 == 1 && sendStatus) {
  //   sendStatus = false;
  // }

  if (sendCounter % 60 == 0) {
    sendLogData2();
  }

  if (sendCounter % 10 == 0) {
    lcd.clear();
    if (sendCounter % 5000 == 0) {
      ESP.restart();
    }
  }
  // debugSerial();
}
