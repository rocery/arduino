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

const String deviceName = "Suhu dan Temperature 1";
const String api = "http://192.168.15.223/iot/api/project_universitas/save_data.php";
const String getCalibration = "http://192.168.15.223/iot/api/project_universitas/get_calibration.php";
const String deviceID = "IoT-1";

#define ledPin 27
#define buzzPin 33

#define DHTPIN 25
#define DHTTYPE DHT21
#define tempThreshold 30
DHT dht(DHTPIN, DHTTYPE);
float temperature, humidity, tempCalibration = 0.0, humCalibration = 0.0;
bool getCalibrationStatus;
int readLoop = 0;

const char* ssid = "STTB11";
const char* password = "Si4nt4r321";
String ip_Address, postData;
int WiFiStatus;

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

void sendData() {
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
  http.begin(getCalibration);
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

  getCalibrationStatus = false;

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(ConnectedToAP_Handler, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(GotIP_Handler, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFi_Disconnected_Handler, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  Serial.println("Connecting to WiFi Network ..");
  WiFi.begin(ssid, password);

  WiFiStatus = WiFi.status();
  while (WiFiStatus != WL_CONNECTED) {
    delay(250);
    WiFiStatus = WiFi.status();
  }
  Serial.println("\nConnected To The WiFi Network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // NTP
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  int tryNTP = 1;
  while (!getTime() && tryNTP <= 5) {
    Serial.print("Getting Time : ");
    Serial.println(tryNTP);
    getTime();
    tryNTP++;
    delay(100);
  }
}

void loop() {
  temperature = dht.readTemperature() + tempCalibration;
  humidity = dht.readHumidity() + humCalibration;

  if (temperature > tempThreshold) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzPin, LOW);
  }

  Serial.print("T: ");
  Serial.println(temperature);
  Serial.print("H: ");
  Serial.println(humidity);

  if (getCalibrationStatus == false) {
    CalibrationData data = getCalibrationData();
    tempCalibration = data.temperature;
    humCalibration = data.humidity;
    getCalibrationStatus = true;
  }

  ip_Address = WiFi.localIP().toString();
  postData = "device_id=" + deviceID + "&device_name=" + deviceName + "&temp=" + String(temperature) + "&hum=" + String(humidity) + "&date=" + dateTime + "&ip_address=" + ip_Address;

  delay(5000);

  readLoop++;
  if (readLoop % 30 == 0) {
    sendData();
    getCalibrationStatus = false;
  }
}