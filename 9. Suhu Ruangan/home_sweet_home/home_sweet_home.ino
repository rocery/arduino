#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <DHT.h>

// ================== SENSOR ==================
#define DHTPIN 33
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float calTemp = -0.5;
float calHum = 0.0;
float temp, hum;
#define LED 2

// ================== WIFI ==================
WiFiMulti wifiMulti;
const char* ssid_home = "A52s";
const char* password_ssid_home = "Welive99_";
const char* ssid_sttb = "STTB11";
const char* password_ssid_sttb = "Si4nt4r321";

IPAddress local_IP(192, 168, 15, 230);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);

// ================== SERVER ==================
WebServer server(80);

// ================== HTML BUILDER ==================
String rootPage(float temp, float hum) {
  String html = "";

  html += "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<title>&#8451; ESP32 DHT22</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='3'>";
  html += "<meta charset='UTF-8'>";
  html += "<style>";

  html += "body{font-family:Arial;background:#0f172a;color:#e5e7eb;text-align:center;}";
  html += ".card{background:#1e293b;padding:25px;margin:40px auto;width:480px;";
  html += "border-radius:12px;box-shadow:0 0 20px rgba(0,0,0,0.5);}";

  html += "h1{color:#237599;}";
  html += ".value{font-size:32px;font-weight:bold;}";
  html += ".label{font-size:21px;color:#94a3b8;}";
  html += ".ip{font-size:15px;color:#94a3b8;}";

  html += ".parent {display: grid;grid-template-columns: repeat(4, 1fr);grid-template-rows: repeat(5, 1fr);gap: 8px;}";

  html += ".div1{grid-column: span 2 / span 2;grid-row: span 5 / span 5;}";
  html += ".div2{grid-column: span 2 / span 2;grid-row: span 5 / span 5;grid-column-start: 3;}";

  html += "</style>";
  html += "</head>";

  html += "<body>";
  html += "<div class='card'>";
  html += "<h1>ESP32 DHT22</h1>";

  html += "<div class='parent'>";

  html += "<div class='div1'>";
  html += "<p class='label'>Temperature</p>";
  html += "<p class='value'>" + String(temp, 1) + " &#8451;</p>";
  html += "</div>";

  html += "<div class='div2'>";
  html += "<p class='label'>Humidity</p>";
  html += "<p class='value'>" + String(hum, 1) + " %</p>";
  html += "</div>";

  html += "</div>";

  html += "<p class='ip'>" + WiFi.localIP().toString() + "</p>";
  html += "</div>";
  html += "</body>";
  html += "</html>";

  return html;
}

// ================== ROUTE ==================
void handleRoot() {

  // Blink LED ON refresh
  // digitalWrite(LED, HIGH);
  // delay(50);
  digitalWrite(LED, LOW);
  delay(100);

  server.send(200, "text/html", rootPage(temp, hum));
}

void handleData() {
  server.send(200, "text/html", "OK");
}

// ================= READ DHT ================
void readDHT() {
  temp = dht.readTemperature() + calTemp;
  hum = dht.readHumidity() + calHum;

  if (isnan(temp) || isnan(hum)) {
    temp = 0;
    hum = 0;
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LED,OUTPUT);

  WiFi.config(local_IP, gateway, subnet, dns1, dns2);
  wifiMulti.addAP(ssid_home, password_ssid_home);
  wifiMulti.addAP(ssid_sttb, password_ssid_sttb);
  
  while (wifiMulti.run() != WL_CONNECTED) {
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

// ================== LOOP ==================
void loop() {
  readDHT();
  server.handleClient();
  digitalWrite(LED, HIGH);

  while (wifiMulti.run() != WL_CONNECTED) {
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
  }
}
