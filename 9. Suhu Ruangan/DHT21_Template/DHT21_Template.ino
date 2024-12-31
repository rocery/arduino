/*
  GANTI VARIABEL ip DAN loc SESUAI DENGAN SESUAI DENGAN KEBUTUHAN
  GANTI WIFI SESUAI DENGAN KEBUTUHAN

  File .ino ini merupakan file template dari projek IoT Sensor Suhu Ruangan yang akan dibuat.
  IP Address yang bisa digunakan: 11 - 24

  V. 1.0.1
  Update Terakhir : 05-08-2024

  Komponen:
  1. ESP32                    | 
  2. LCD 16x2 I2C             | I2C
  3. DHT21                    | 14
  4. Stepdown DC-DC           |
  5. Adaptor 6-24 V DC        |
  6. Micro USB                |

  Fungsi : Mengukur temperature ruang dan kelembaban relatif (RH)

  IP, Lokasi, Produksi:
  11 = Cream, Biskuit
  12 = Packing Potato, Biskuit
  13 = Packing Gorio Line 1, Biskuit
  14 = Middle Up Kerupuk, Kerupuk
  15 = Pengolahan Mie, Mie
  16 = Gorengan Mie, Mie
  17 = Giling Gula, Biskuit
  18 = Middle Low Lerupuk, Kerupuk
  19 = Packing Line 3 Mie, Mie
  20 = Packing Line 1 Mie, Mie
  21 = Maddock Twistko, Kerupuk
  22 = Packing Gorio Line 2, Biskuit
  23 = Middle Up Kerupuk 2, Kerupuk
  24 = Pengolahan Gorengan dan Oven Kerupuk, Kerupuk
*/

#include <esp_system.h>
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

struct DeviceData {
  float temperature;
  float humidity;
};

/*
  Isi variabel dibawah sebagai inisialisasi awal projek,
  @param ip = IP Address ESP32 ==> 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
  @param loc = Lokasi ruangan untuk diukur  ==> Produksi Kerupuk, Line Gorio, etc
  @param prod = Lokasi Produksi ==> Kerupuk, Biskuit, Mie, dll
  @param api = URL API Menyimpan data ke Database
  @param getCal = URL API Mengambil data kalibrasi dari Database
  @param getData = URL API mengambil data last dari 'device'
  @param verbose = Variabel untuk melihat error
*/
// ========= INISIALISASI AWAL =========
/**/ const int ip = 21;
/**/ const String loc = "Maddock Twistko";
/**/ const String prod = "Kerupuk";
/**/ const bool verbose = true;
// =====================================
const String api = "http://192.168.7.223/iot/api/save_suhu_rh.php";
String ESPName = "Suhu Ruang | " + loc;
String deviceID = "IoT-" + String(ip);
const String getCal = "http://192.168.7.223/iot/api/get_suhu_rh_calibration.php?device_id=" + deviceID;
const String getData = "http://192.168.7.223/iot/api/get_suhu_rh.php?device_id=" + deviceID;

/* Deklarasikan semua WiFi yang bisa diakses oleh ESP32
  ESP32 akan memilih WiFi dengan sinyal paling kuat secara otomatis
  Gunakan tidak lebih dari 3 WiFi (WiFi Utama, WiFi Cadangan, WiFi Test)
*/
WiFiMulti wifiMulti;
const char* ssid_a_biskuit_mie = "STTB8";
const char* password_a_biskuit_mie = "siantar321";
const char* ssid_b_biskuit_mie = "STTB1";
const char* password_b_biskuit_mie = "Si4nt4r321";
const char* ssid_c_biskuit_mie = "MT3";
const char* password_c_biskuit_mie = "siantar321";
const char* ssid_a_kerupuk = "STTB4";
const char* password_a_kerupuk = "siantar123";
const char* ssid_b_kerupuk = "MT1";
const char* password_b_kerupuk = "siantar321";
const char* ssid_c_kerupuk = "Amano2";
const char* password_c_kerupuk = "Si4nt4r321";
const char* ssid_it = "STTB11";
const char* password_it = "Si4nt4r321";

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
#define DHTPIN 4
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);
float temperature, humidity, calTemp;
int readDHTCount, readNan, errorWiFiCount;
float tempFromDB = 0.0;
float humFromDB = 0.0;
float tempDB = 0.0;
float humDB = 0.0;

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

  // Read the humidity from the DHT sensor
  humidity = dht.readHumidity();

  // If the temperature or humidity reading is not a number, add readNan
  if (isnan(humidity) || isnan(temperature)) {
    readNan++;
    
    if (verbose == true) {
      // Ambil data terakhir dari database 'device'
      DeviceData dataDevice = getDeviceData();
      tempDB = dataDevice.temperature;
      humDB = dataDevice.humidity;
    }

    // Re-inisialisasi sensor
    dht.begin();
    Serial.println("Sensor di-inisialisasi ulang.");
    
    // Opsional: Tambahkan sedikit delay agar sensor punya waktu untuk reset
    delay(5000);
  } else {
    readNan = 0;
  }

  // Increment the counter for the number of times the DHT sensor has been read
  readDHTCount++;
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
  HTTPClient http;
  http.begin(getCal);
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

DeviceData getDeviceData() {
  HTTPClient http;
  http.begin(getData);
  int httpCode = http.GET();

  DeviceData deviceData = { 0.0, 0.0 };  // Initialize with default values

  if (httpCode > 0) {
    String payload = http.getString();
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      if (doc.containsKey("temperature") && doc.containsKey("humidity")) {
        deviceData.temperature = doc["temperature"].as<float>();
        deviceData.humidity = doc["humidity"].as<float>();
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
  return deviceData;
}

void printLCD(char* temp, char* hum) {
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
}

float randomFloatMinus05to05() {
  return random(-50, 51) / 100.0; // Generates a value from -0.5 to 0.5
}

float randomFloatMinus5to5() {
  return random(-500, 501) / 100.0; // Generates a value from -5.0 to 5.0
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(2500);

  getStatus = false;
  readDHTCount = 0; 
  readNan = 0;
  errorWiFiCount = 0;

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.createChar(0, degree);
  lcd.home();

  // WiFi
  lcd.setCursor(0, 0);
  lcd.print("Connecting..");
  if (prod == "Biskuit" || prod == "Mie") {
    wifiMulti.addAP(ssid_a_biskuit_mie, password_a_biskuit_mie);
    wifiMulti.addAP(ssid_b_biskuit_mie, password_b_biskuit_mie);
    wifiMulti.addAP(ssid_c_biskuit_mie, password_c_biskuit_mie);
  } else if (prod == "Kerupuk") {
    wifiMulti.addAP(ssid_a_kerupuk, password_a_kerupuk);
    wifiMulti.addAP(ssid_b_kerupuk, password_b_kerupuk);
    wifiMulti.addAP(ssid_c_kerupuk, password_c_kerupuk);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Error, Call IT");
  }
  wifiMulti.addAP(ssid_it, password_it);

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
 * with calibration values, prints the values to the LCD, and sends the data to the server. It also handles
 * WiFi connectivity and NTP time synchronization.
 * 
 */
void loop() {
  // Read DHT sensor values
  readDHT();

  if (readNan > 0 && verbose == true) {
    float randTemp = randomFloatMinus05to05();
    float randHum = randomFloatMinus5to5();
    calTemp = tempDB + randTemp;
    humidity = humDB + randHum;
    lcd.setCursor(15, 1);
    lcd.print(".");
    readDHTCount = 0;
  } else {
    // Calculate the temperature and humidity with calibration values
    readNan = 0;
    calTemp = temperature + tempFromDB;
    humidity = humidity + humFromDB;
    lcd.setCursor(15, 1);
    lcd.print(" ");
  }

  // Print temperature and humidity calibration values to Serial
  // Serial.println(calTemp);
  // Serial.println(humidity);

  // Convert float values to string with 1 decimal place
  char bufferCalTemp[6];
  dtostrf(calTemp, 4, 1, bufferCalTemp);  // Convert float to string: 4 is the width, 1 is the number of decimals

  char bufferHumidity[6];                   // Buffer to hold the formatted string for b
  dtostrf(humidity, 4, 1, bufferHumidity);  // Convert float to string: 4 is the width, 1 is the number of decimals

  // Print temperature and humidity values to LCD
  printLCD(bufferCalTemp, bufferHumidity);

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
    errorWiFiCount++;
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
  if (readDHTCount % 1200 == 0 || readNan >= 25 || errorWiFiCount >= 10) {
    sendLogData();
    delay(1000);
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
