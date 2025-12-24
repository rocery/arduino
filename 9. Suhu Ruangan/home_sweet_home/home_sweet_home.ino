#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <DHT.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// ================== SENSOR ==================
#define DHTPIN 33
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float calTemp = -0.5;
float calHum = 0.0;
float temp, hum;
#define LED 2
float lastTemp = 0;
float lastHum = 0;

// ================== WIFI ==================
WiFiMulti wifiMulti;
const char* ssid_home = "TP-Link_7764";
const char* password_ssid_home = "78384767";
const char* ssid_sttb = "STTB11";
const char* password_ssid_sttb = "Si4nt4r321";

IPAddress local_IP(192, 168, 15, 230);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);

// ================== NTP ==================
const char* ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 7 * 3600;
const int daylightOffsetSec = 0;
bool getLocalTimeStatus = false;
int year, month, day, hour, minute, second;
String dateTime, dateFormat, timeFormat;

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
  html += "<p class='value'>" + String(temp, 1) + " &degC</p>";
  html += "</div>";

  html += "<div class='div2'>";
  html += "<p class='label'>Humidity</p>";
  html += "<p class='value'>" + String(hum, 1) + " %</p>";
  html += "</div>";

  html += "</div>";

  html += "<p class='ip'>" + WiFi.localIP().toString() + " | " + dateTime + "</p>";
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
  String html;

  html += "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<title>Data Harian ESP32 DHT22</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta charset='UTF-8'>";

  // ===== STYLE SAMA =====
  html += "<style>";
  html += "body{font-family:Arial;background:#0f172a;color:#e5e7eb;text-align:center;}";
  html += ".card{background:#1e293b;padding:25px;margin:40px auto;width:90%;max-width:900px;";
  html += "border-radius:12px;box-shadow:0 0 20px rgba(0,0,0,0.5);}";

  html += "h1{color:#237599;margin-bottom:20px;}";

  html += "table{width:100%;border-collapse:collapse;margin-top:10px;}";
  html += "th,td{border:1px solid #334155;padding:8px;font-size:14px;}";
  html += "th{background:#020617;color:#38bdf8;}";
  html += "tr:nth-child(even){background:#020617;}";
  html += "</style>";

  html += "</head><body>";

  html += "<div class='card'>";
  html += "<h1>Data Sensor Harian</h1>";
  html += "<p class='ip'>Tanggal: " + dateFormat + "</p>";

  html += "<table>";
  html += "<tr><th>Tanggal</th><th>Waktu</th><th>Suhu (&#8451;)</th><th>Kelembapan (%)</th></tr>";

  // ===== FILE HARIAN =====
  String dailyPath = "/data/" + dateFormat + ".csv";
  File file = SD.open(dailyPath);

  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) continue;

      line.replace(",", "</td><td>");
      html += "<tr><td>" + line + "</td></tr>";
    }
    file.close();
  } else {
    html += "<tr><td colspan='4'>Data belum tersedia</td></tr>";
  }

  html += "</table>";
  html += "</div>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void downloadCSV() {
  if (!server.hasArg("date")) {
    server.send(400, "text/plain", "Missing '?date=YYYY-MM-DD' or '?date=all'");
    return;
  }

  String date = server.arg("date");
  File file;

  // ===== DOWNLOAD ALL DATA =====
  if (date == "all") {
    file = SD.open("/data_all.csv");
    if (!file) {
      server.send(404, "text/plain", "data_all.csv not found");
      return;
    }

    server.sendHeader(
      "Content-Disposition",
      "attachment; filename=\"data_all.csv\""
    );
    server.streamFile(file, "text/csv");
    file.close();
    return;
  }

  // ===== VALIDATE DATE FORMAT =====
  if (date.length() != 10 || date.charAt(4) != '-' || date.charAt(7) != '-') {
    server.send(400, "text/plain", "Invalid date format (YYYY-MM-DD)");
    return;
  }

  // ===== DAILY FILE =====
  String dailyPath = "/data/" + date + ".csv";
  file = SD.open(dailyPath);
  if (!file) {
    server.send(404, "text/plain", "File not found for selected date");
    return;
  }

  server.sendHeader(
    "Content-Disposition",
    "attachment; filename=\"" + date + ".csv\""
  );
  server.streamFile(file, "text/csv");
  file.close();
}

void handleDownloadPage() {
  String html;

  html += "<!DOCTYPE html><html><head>";
  html += "<title>Download Data ESP32</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta charset='UTF-8'>";

  // ===== STYLE (konsisten dengan UI Anda) =====
  html += "<style>";
  html += "body{font-family:Arial;background:#0f172a;color:#e5e7eb;text-align:center;}";
  html += ".card{background:#1e293b;padding:25px;margin:40px auto;width:90%;max-width:420px;";
  html += "border-radius:12px;box-shadow:0 0 20px rgba(0,0,0,0.5);}";

  html += "h1{color:#237599;}";
  html += "select,button{width:100%;padding:10px;margin-top:15px;";
  html += "border-radius:6px;border:none;font-size:16px;}";
  html += "button{background:#237599;color:white;cursor:pointer;}";
  html += "button:hover{background:#1d4ed8;}";
  html += "</style>";

  html += "</head><body>";
  html += "<div class='card'>";
  html += "<h1>Download Data</h1>";

  html += "<select id='date'>";

  // ===== OPTION ALL DATA =====
  html += "<option value='all'>All Data</option>";

  // ===== READ SD DIRECTORY =====
  File dir = SD.open("/data");
  if (dir) {
    File entry;
    while ((entry = dir.openNextFile())) {
      String name = entry.name();   // e.g. 2025-12-24.csv
      entry.close();

      if (name.endsWith(".csv")) {
        name.replace(".csv", "");
        html += "<option value='" + name + "'>" + name + "</option>";
      }
    }
    dir.close();
  }

  html += "</select>";

  html += "<button onclick='download()'>Download</button>";

  // ===== SCRIPT =====
  html += "<script>";
  html += "function download(){";
  html += "var d=document.getElementById('date').value;";
  html += "window.location='/download_data?date='+d;";
  html += "}";
  html += "</script>";

  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

// =================== DHT ===================
void readDHT() {
  temp = dht.readTemperature() + calTemp;
  hum = dht.readHumidity() + calHum;

  if (isnan(temp) || isnan(hum)) {
    temp = 0;
    hum = 0;
  }
}

void ensureDataDir() {
  if (!SD.exists("/data")) {
    SD.mkdir("/data");
  }
}

void saveToSD() {
  // Tidak ada perubahan
  if (temp == lastTemp && hum == lastHum) {
    return;
  }

  // Validasi waktu
  if (!getLocalTimeStatus) {
    Serial.println("Waktu belum tersedia");
    return;
  }

  // Pastikan folder ada
  ensureDataDir();

  // ===== File ALL DATA =====
  File allFile = SD.open("/data_all.csv", FILE_APPEND);
  if (!allFile) {
    Serial.println("Gagal membuka data_all.csv");
    return;
  }

  // ===== File PER TANGGAL =====
  String dailyPath = "/data/" + dateFormat + ".csv";
  File dailyFile = SD.open(dailyPath, FILE_APPEND);
  if (!dailyFile) {
    allFile.close();
    Serial.println("Gagal membuka file harian");
    return;
  }

  // Format data
  String line = dateFormat + "," + timeFormat + "," + String(temp, 2) + "," + String(hum, 2);

  // Tulis ke kedua file
  allFile.println(line);
  dailyFile.println(line);

  allFile.close();
  dailyFile.close();

  // Update state setelah sukses
  lastTemp = temp;
  lastHum = hum;

  Serial.println("Data tersimpan (global & harian)");
}

// ================== TIME ===================
void getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    getLocalTimeStatus = false;
    return;
  }
  getLocalTimeStatus = true;

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
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LED, OUTPUT);

  WiFi.config(local_IP, gateway, subnet, dns1, dns2);
  wifiMulti.addAP(ssid_home, password_ssid_home);
  wifiMulti.addAP(ssid_sttb, password_ssid_sttb);

  while (wifiMulti.run() != WL_CONNECTED) {
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
  }

  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  getLocalTime();

  if (SD.begin()) {
    Serial.println("SD Card Initialized");
  } else {
    Serial.println("SD Card Initialization Failed");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/download_data", downloadCSV);
  server.on("/download", handleDownloadPage);
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
  getLocalTime();
  saveToSD();
  delay(1000);
}
