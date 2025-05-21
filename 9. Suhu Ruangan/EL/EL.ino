#include <DHT.h>
#include <WiFi.h>

#define ledPin 27
#define buzzPin 33

#define DHTPIN 25
#define DHTTYPE DHT21
#define tempThreshold 30
DHT dht(DHTPIN, DHTTYPE);
float temperature, humidity;

const char* ssid = "STTB11";
const char* password = "Si4nt4r321";

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