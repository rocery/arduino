/*
  V. 0.0.1
  Update Terakhir : 20-07-2024
  FILE INI MERUPAKAN TEMPLATE JIKA ADA PENAMBAHAN PROJECT/ALAT SENSOR SUHU RUANGAN

  Komponen:
  1. ESP32
  2. LCD 16x2 I2C
  3. Potensiometer 100K Ohm
  4. DHT21

  PP = 18 Juli 2024

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
const int ip = 11;
const String loc = "Kerupuk";
const String api = "http://192.168.7.223/iot/api/save_suhu_rh.php";

String ESPName = "Suhu Ruang | " + loc;
String deviceID = "IoT-" + String(ip);

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
float temperature, humidity, calTemp;
int readDHTCount;

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
String ip_Address, postData;

// ===== NTP =====
const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat, lcdFormat;
int year, month, day, hour, minute, second;
bool ntpStatus;

/**
 * @brief Reads the temperature and humidity data from the DHT sensor.
 * 
 * This function reads the temperature and humidity values from the DHT sensor
 * and updates the temperature and humidity variables. It also increments the
 * readDHTCount variable.
 */
void readDHT() {
  // Read the temperature from the DHT sensor
  temperature = dht.readTemperature();

  // Read the humidity from the DHT sensor
  humidity = dht.readHumidity();

  // Increment the readDHTCount variable
  readDHTCount++;
}

/**
 * @brief Reads the potentiometer value from the POTPIN pin.
 * 
 * This function reads the analog value from the POTPIN pin and maps it
 * to a value between 0 and 10. The mapped value is stored in the mappedPotValue
 * variable.
 */
void readPot() {
  // Read the analog value from the POTPIN pin
  potValue = analogRead(POTPIN);

  // Map the analog value to a value between 0 and 10
  mappedPotValue = map(potValue, 0, 4095, 0, 10);
}

bool getLocalTime() {
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

/**
 * Sends log data to the local server.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
void sendLogData() {
  /* Mengirim data ke local server
  Ganti isi variabel api sesuai dengan form php
  */
  // Begin HTTP client
  HTTPClient http;
  http.begin(api);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(postData);

  // Add header for content type
  if (httpResponseCode > 0) {
    // Send POST request with data
    String response = http.getString();
    // Check if request is successful
    Serial.println(response);
    // Get response from server
  } else {
    Serial.print("ERROR ON SENDING POST");
  }
  // Print error message if request fails
  http.end();
}

// End HTTP client

void printLCD(float temp, float hum) {
  /*
   * Print temperature and humidity data to LCD display.
   *
   * @param temp float temperature value
   * @param hum float humidity value
   */
  // Temperature
  lcd.setCursor(0, 0);  // Set cursor to position (0, 0)
  lcd.print("T: ");     // Print "T: "
  lcd.setCursor(3, 0);  // Set cursor to position (3, 0)
  lcd.print(temp);      // Print temperature value
  lcd.write(0);         // Print degree symbol (°C)
  lcd.print("C");       // Print "C"

  // Humidity
  lcd.setCursor(0, 1);  // Set cursor to position (0, 1)
  lcd.print("H: ");     // Print "H: "
  lcd.setCursor(3, 1);  // Set cursor to position (3, 1)
  lcd.print(hum);       // Print humidity value
  lcd.print("%");       // Print "%"
}

void setup() {
  /*
   * Initialize serial communication.
   * The baud rate is set to 115200.
   */
  Serial.begin(115200);
  dht.begin();

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

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  } else {
    Serial.println("STA Success");
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
      getLocalTime();
      tryNTP++;
      delay(50);
      lcd.setCursor(0, 1);
      lcd.print("Getting Time");
    }
  }

  lcd.clear();
}

/**
 * The loop function to run in the ESP32
 * This function will check if the ESP32 is connected to the WiFi network.
 * If it is, it will collect data from the DHT21 and potentiometer.
 * It will then calculate the temperature with the potentiometer value.
 * After that, it will print the data to the LCD.
 * If it's not connected to the WiFi, it will print error message to the LCD.
 * It will also check if the NTP is connected correctly or not.
 * If it is, it will print the time to the LCD.
 * It will also send the data to the API if the time is up.
 * And restart the ESP32 after every 1200 readings.
 */
void loop() {
  static unsigned long lastSend = 0;
  static const unsigned long sendInterval = 30000;  // 30 seconds

  // Read data from DHT21 and potentiometer
  readDHT();
  readPot();
  calTemp = temperature + mappedPotValue;
  printLCD(calTemp, humidity);

  // Calculate temperature with potentiometer value
  if (wifiMulti.run() == WL_CONNECTED) {
    // Print data to LCD
    if (millis() - lastSend >= sendInterval) {
      // Check if WiFi is connected
      lastSend = millis();
      // If it is, check if the time is up
      getLocalTime();
      // If it is, get local time
      ip_Address = WiFi.localIP().toString();
      postData = "device_id=" + deviceID + "&device_name=" + ESPName + "&temp=" + String(calTemp) + "&hum=" + String(humidity) + "&date=" + dateTime + "&ip_address=" + ip_Address;
      sendLogData();
      lcd.clear();
    }

    // Check if NTP is connected correctly or not
    // Check if the reading count is a multiple of 1200
    // If it is, restart the ESP32
    // Delay for 2 seconds
    if (ntpStatus == false) {
      lcd.setCursor(11, 1);
      // Set up data to send to API
      lcd.print("Error");
      // Send data to API
    } else {
      // Clear LCD
      lcd.setCursor(12, 0);
      // Print error message to LCD
      lcd.print("Error");
      lcd.setCursor(11, 1);
      lcd.print(lcdFormat);
    }

    if (readDHTCount % 1200 == 0) {
      ESP.restart();
    }
    delay(2000);
  }
}