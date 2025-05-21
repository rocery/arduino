#include <esp_system.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

struct CalibrationData {
  float temperature;
  float humidity;
};

const deviceName = "Suhu dan Temperature 1";
const String api = "http://192.168.15.223/iot/api/project_universitas/save_data.php";
const String getCalibration = "http://192.168.15.223/iot/api/project_universitas/get_calibration.php";
const String deviceID = "IoT-1";

#define ledPin 27
#define buzzPin 33

#define DHTPIN 25
#define DHTTYPE DHT21
#define tempThreshold 30
DHT dht(DHTPIN, DHTTYPE);
float temperature, humidity;

const char* ssid = "STTB11";
const char* password = "Si4nt4r321";

const char* ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 7 * 3600;
const int daylightOffsetSec = 0;
int year, month, day, hour, minute, second;
String dateTime, dateFormat, timeFormat, lcdFormat;

bool getTime() {
  struct tm timeNTP;
  if (!getLocalTime(&timeNTP)) {
    return false;
  } else {
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeNTP);
    dateTime = String(timeStringBuff);
  
    // Save time data to variabel
    year = timeNTP.tm_year + 1900;
    month = timeNTP.tm_mon + 1;
    day = timeNTP.tm_mday;
    hour = timeNTP.tm_hour;
    minute = timeNTP.tm_min;
    second = timeNTP.tm_sec;
    // YYYY-MM-DD
    dateFormat = String(year) + '-' + String(month) + '-' + String(day);
    // hh:mm:ss
    timeFormat = String(hour) + ':' + String(minute) + ':' + String(second);
    // hh:mm
    lcdFormat = String(hour) + ':' + String(minute);

    return true;
  }
}

void ConnectedToAP_Handler(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
  Serial.println("Connected To The WiFi Network");
}

void GotIP_Handler(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void WiFi_Disconnected_Handler(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
  Serial.println("Disconnected From WiFi Network");
  // Attempt Re-Connection
  WiFi.begin(ssid, password);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(2500);

  pinMode(ledPin, OUTPUT);
  pinMode(buzzPin, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(ConnectedToAP_Handler, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(GotIP_Handler, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFi_Disconnected_Handler, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi Network ..");
}

void loop() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (temperature > tempThreshold) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzPin, LOW);
  }

  Serial.print("T: ");
  Serial.print("H: ");
  Serial.println(humidity);

  delay(2000);
}