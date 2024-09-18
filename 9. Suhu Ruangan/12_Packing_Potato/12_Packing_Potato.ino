/*
  GANTI VARIABEL ip DAN loc SESUAI DENGAN SESUAI DENGAN KEBUTUHAN

  File .ino ini merupakan file template dari projek IoT Sensor Suhu Ruangan yang akan dibuat.
  IP Address yang bisa digunakan: 11 - 25

  V. 1.0.1
  Update Terakhir : 05-08-2024

  Komponen:
  1. ESP32                    | 
  2. LCD 16x2 I2C             | I2C
  3. Potensiometer 100K Ohm   | 34 3V
  4. DHT21                    | 14
  5. Stepdown DC-DC           |
  6. Adaptor 6-24 V DC        |
  7. Micro USB                |

  Fungsi : Mengukur temperature ruang dan kelembaban relatif (RH)
*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// Struct to hold the calibration data
struct CalibrationData {
  float temperature;
  float humidity;
};

/*
  Isi variabel dibawah sebagai inisialisasi awal projek,
  @param ip = IP Address ESP32
  @param loc = Lokasi ruangan untuk diukur
  @param api = URL API Menyimpan data ke Database
  @param getData = URL API Mengambil data kalibrasi dari Database
*/
// ========= INISIALISASI AWAL =========
/**/ const int ip = 12;             /**/
/**/ const String loc = "Packing Potato";  /**/
// =====================================
const String api = "http://192.168.7.223/iot/api/save_suhu_rh.php";
String ESPName = "Suhu Ruang | " + loc;
String deviceID = "IoT-" + String(ip);
const String getData = "http://192.168.7.223/iot/api/get_suhu_rh_calibration.php?device_id=" + deviceID;

/* Mendeklarasikan DHT21
  @param DHTPIN = Pin DHT
  @param DHTTYPE = Tipe DHT
  @param dht = Variabel DHT
  @param temperature = Variabel Suhu
  @param humidity = Variabel Kelembaban
  @param calTemp = Variabel Kalibrasi
  @param readDHTCount = Variabel Hitung
  @param tempFromDB = Variabel Kalibrasi Suhu dari DB
  @param humFromDB = Variabel Kalibrasi Kelembaban dari DB
*/
#define DHTPIN 33
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);
float temperature, humidity, calTemp;
int readDHTCount, readNan;
float tempFromDB = 0.0;
float humFromDB = 0.0;

/* Mendeklarasikan Potensiometer
  @param potPin = Pin Potensiometer
  @param potValue = Nilai Potensiometer
  @param mappedPotValue = Nilai Mapped Potensiometer
*/
#define POTPIN 34
int potValue = 0;
int mappedPotValue = 0;

/* Mendeklarasikan LCD dengan alamat I2C 0x27
  @param LCDADDR = Alamat I2C
  @param lcd = Variabel LCD
*/
#define LCDADDR 0x27
LiquidCrystal_I2C lcd(LCDADDR, 16, 2);
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

/* Deklarasikan semua WiFi yang bisa diakses oleh ESP32
  ESP32 akan memilih WiFi dengan sinyal paling kuat secara otomatis
  Gunakan tidak lebih dari 3 WiFi (WiFi Utama, WiFi Cadangan, WiFi Test)
*/
WiFiMulti wifiMulti;
const char* ssid_a = "STTB8";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "MT3";
const char* password_b = "siantar321";
const char* ssid_c = "STTB11";
const char* password_c = "Si4nt4r321";
const char* ssid_d = "STTB4";
const char* password_d = "Si4nt4r321";
const char* ssid_e = "MT1";
const char* password_e = "siantar321";
const char* ssid_f = "STTB1";
const char* password_f = "Si4nt4r321";

// Set IP to Static
IPAddress staticIP(192, 168, 7, ip);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional
String ip_Address, postData;

// ===== NTP =====
const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat, lcdFormat;
int year, month, day, hour, minute, second;
bool ntpStatus, getStatus;

/**
 * @brief Reads the temperature and humidity from the DHT sensor.
 * If the temperature or humidity reading is not a number, it sets the value to 950.
 * Increments the counter for the number of times the DHT sensor has been read.
 */
void readDHT() {
  // Read the temperature from the DHT sensor
  temperature = dht.readTemperature();

  // If the temperature reading is not a number, set it to 950
  if (isnan(temperature)) {
    temperature = 950;
    readNan++;
  }

  // Read the humidity from the DHT sensor
  humidity = dht.readHumidity();

  // If the humidity reading is not a number, set it to 950
  if (isnan(humidity)) {
    humidity = 950;
    readNan++;
  }

  // Increment the counter for the number of times the DHT sensor has been read
  readDHTCount++;
}

/**
 * @brief Reads the analog value from the potentiometer and maps it to a temperature range.
 * 
 * This function reads the analog value from the potentiometer using the `analogRead()` function.
 * The read value is then mapped to a temperature range using the `map()` function.
 * The temperature range is [-5, 5] degrees Celsius.
 * The `POTPIN` constant is used to specify the analog pin connected to the potentiometer.
 * 
 * The `potValue` variable stores the raw analog value read from the potentiometer.
 * The `mappedPotValue` variable stores the mapped temperature value.
 */
void readPot() {
  // Read the analog value from the potentiometer
  potValue = analogRead(POTPIN);

  // Map the analog value to a temperature range
  // mappedPotValue = map(potValue, 0, 4095, -5, 5);
  mappedPotValue = 0;
}

void getLocalTime() {
  /* Fungsi bertujuan menerima update waktu
  lokal dari ntpServer */
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    ntpStatus = false;
  } else {
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

    ntpStatus = true;
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

CalibrationData getCalibrationData() {
  /* Untung mendapatkan data terakhir dari DB, 
  saat ini tidak digunakan karena sudah menggunakan SD Card
  Kode dibawah mohon untuk tidak dihapus.
  */
  HTTPClient http;
  http.begin(getData);
  int httpCode = http.GET();

  CalibrationData calibrationData = { 0.0, 0.0 };  // Initialize with default values

  if (httpCode > 0) {
    String payload = http.getString();
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      if (doc.containsKey("temperature") && doc.containsKey("humidity")) {
        calibrationData.temperature = doc["temperature"].as<float>();
        calibrationData.humidity = doc["humidity"].as<float>();
      } else {
        Serial.println("Keys 'temperature' and/or 'humidity' not found in JSON");
      }
    } else {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("Error get log: ");
    Serial.println(httpCode);
  }

  http.end();
  return calibrationData;
}

void printLCD(char* temp, char* hum, int pot) {
  // Temperature
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.setCursor(2, 0);
  lcd.print(temp);
  lcd.write(0);
  lcd.print("C");

  // Humidity
  lcd.setCursor(0, 1);
  lcd.print("H: ");
  lcd.setCursor(2, 1);
  lcd.print(hum);
  lcd.print("%");

  // Print mapped potentiometer value to LCD
  // lcd.setCursor(14, 1);
  // lcd.print(pot);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  getStatus = false;

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.createChar(0, degree);
  lcd.home();

  // WiFi
  lcd.setCursor(0, 0);
  lcd.print("Connecting..");
  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);
  wifiMulti.addAP(ssid_d, password_d);
  wifiMulti.addAP(ssid_e, password_e);
  wifiMulti.addAP(ssid_f, password_f);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  wifiMulti.run();

  // NTP
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  if (wifiMulti.run() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Fail");
    lcd.setCursor(0, 1);
    lcd.print("Date/Time Error");
    delay(3000);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");

    int tryNTP = 0;
    while (ntpStatus == false && tryNTP <= 2) {
      lcd.setCursor(0, 1);
      lcd.print("Getting Time");
      getLocalTime();
      tryNTP++;
      delay(50);
    }
  }

  lcd.clear();
}

/**
 * @brief The main loop function
 * 
 * This function reads the potentiometer value, reads the DHT sensor, calculates the temperature and humidity
 * with calibration values, prints the values to the LCD, and sends the data to the server. It also handles
 * WiFi connectivity and NTP time synchronization.
 * 
 */
void loop() {
  // Read potentiometer value
  readPot();

  // Print mapped potentiometer value to Serial
  // Serial.print("Mapped Value: ");
  // Serial.println(mappedPotValue);

  // Read DHT sensor values
  readDHT();

  // Calculate the temperature and humidity with calibration values
  calTemp = temperature + mappedPotValue + tempFromDB;
  humidity = humidity + humFromDB;

  // Print temperature and humidity calibration values to Serial
  // Serial.println(calTemp);
  // Serial.println(humidity);

  // Convert float values to string with 1 decimal place
  char bufferCalTemp[6];
  dtostrf(calTemp, 4, 1, bufferCalTemp);  // Convert float to string: 4 is the width, 1 is the number of decimals

  char bufferHumidity[6];                   // Buffer to hold the formatted string for b
  dtostrf(humidity, 4, 1, bufferHumidity);  // Convert float to string: 4 is the width, 1 is the number of decimals

  // Print temperature and humidity values to LCD
  printLCD(bufferCalTemp, bufferHumidity, mappedPotValue);

  // Handle WiFi connectivity
  if (wifiMulti.run() == WL_CONNECTED) {
    lcd.setCursor(10, 0);
    lcd.print(WiFi.SSID());
    getLocalTime();

    // Get calibration data from server if not already retrieved
    if (getStatus == false) {
      CalibrationData data = getCalibrationData();
      tempFromDB = data.temperature;
      humFromDB = data.humidity;
      getStatus = true;
    }

  } else if (wifiMulti.run() != WL_CONNECTED) {
    lcd.setCursor(10, 0);
    lcd.print("Error");
    wifiMulti.run();
  }

  // Handle NTP time synchronization
  if (ntpStatus == false) {
    lcd.setCursor(10, 1);
    lcd.print("Error");
  } else {
    lcd.setCursor(10, 1);
    lcd.print(lcdFormat);
  }

  // Get local IP address
  ip_Address = WiFi.localIP().toString();

  // Prepare data to be sent to the server
  postData = "device_id=" + deviceID + "&device_name=" + ESPName + "&temp=" + String(calTemp) + "&hum=" + String(humidity) + "&date=" + dateTime + "&ip_address=" + ip_Address;

  // Restart the device every 1200 readings of the DHT sensor
  if (readDHTCount % 1200 == 0 || readNan >= 10) {
    ESP.restart();
  }

  // Send data to the server every 30 readings of the DHT sensor
  delay(5000);
  if (readDHTCount % 30 == 0) {
    sendLogData();
    getStatus = false;
    lcd.clear();
  }
}
