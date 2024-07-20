/*
  V. 0.0.1
  Update Terakhir : 19-07-2024

  Komponen:
  1. ESP32
  2. LCD 16x2 I2C
  3. Potensiometer 100K Ohm
  4. DHT21

  Fungsi : Mengukur temperatur ruang dan kelembaban relatif (RH)
*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

/*
  Isi variabel dibawah sebagai inisialisasi awal projek,
  @param ip = IP Address ESP32
  @param loc = Lokasi ruangan untuk diukur
  @param api = URL API
*/
const String ip = "11";
const String loc = "Kerupuk";
const String api = "http://192.168.7.223/iot/api/save_suhu_rh.php";

String ESPName = "Suhu Ruang | " + loc;
String deviceID = "IoT-" + ip;

/* Mendeklarasikan DHT21
  @param DHTPIN = Pin DHT
  @param DHTTYPE = Tipe DHT
  @param dht = Variabel DHT
  @param temperature = Variabel Suhu
  @param humidity = Variabel Kelembaban
*/
#define DHTPIN 34
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);
float temperature, humidity, calTemp, calHum;

/* Mendeklarasikan Potensiometer
  @param POTPIN = Pin Potensiometer
  @param potValue = Nilai Potensiometer
  @param mappedPotValue = Nilai Mapped Potensiometer
*/
#define POTPIN 15
int potValue, mappedPotValue;

/* Mendeklarasikan LCD dengan alamat I2C 0x27
  @param LCDADDR = Alamat I2C
  @param lcd = Variabel LCD
*/
#define LCDADDR 0x27
LiquidCrystal_I2C lcd(LCDADDR, 16, 2);

/* Deklarasikan semua WiFi yang bisa diakses oleh ESP32
  ESP32 akan memilih WiFi dengan sinyal paling kuat secara otomatis
  Gunakan tidak lebih dari 3 WiFi (WiFi Utama, WiFi Cadangan, WiFi Test)
*/
WiFiMulti wifiMulti;
const char* ssid_a = "STTB1";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "MT3";
const char* password_b = "siantar321";
const char* ssid_c = "STTB11";
const char* password_c = "Si4nt4r321";
const char* ssid_d = "Tester_ITB";
const char* password_d = "Si4nt4r321";

// Set IP to Static
IPAddress staticIP(192, 168, 7, ip);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional
String ip_Address;

// ===== NTP =====
const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat, lcdFormat;
int year, month, day, hour, minute, second;
bool ntpStatus;

void readDHT() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

void readPot() {
  potValue = analogRead(POTPIN);
  mappedPotValue = map(potValue, 0, 4095, 0, 10);
}

void getLocalTime() {
  /* Fungsi bertujuan menerima update waktu
  lokal dari ntpServer */
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
    timeFormat = String(hour) + ':' + String(minute) + ':' + String(second);
    // hh:mm
    lcdFormat = String(hour) + ':' + String(minute);
  }
}

void sendLogData() {
  /* Mengirim data ke local server
  Ganti isi variabel api sesuai dengan form php
  */
  HTTPClient http;
  http.begin(api);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("ERROR ON SENDING POST");
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  dht.begin();


}

void loop() {
  readDHT();
  readPot();

  calTemp = temperature + mappedPotValue;

}