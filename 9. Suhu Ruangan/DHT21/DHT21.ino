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
  Isi variabel dibawah sebagai inisialisasi awal projek
*/
const int ip = 11;
const String loc = "Kerupuk";
const String api = "http://192.168.7.223/iot/api/save_suhu_rh.php";


String ESPName = "Suhu Ruang | " + loc;
String deviceID = "IoT-" + String(ip);

#define DHTPIN 14
#define DHTTYPE DHT21

float temperature, humidity;

DHT dht(DHTPIN, DHTTYPE);

void readDHT() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  readDHT();
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C ");
  delay(2000);
}