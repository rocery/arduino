#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include "time.h"

const int ip = 31;
const String loc = "Kerupuk 31";
const String prod = "Kerupuk";

const String ESPName = "Weigher | " + loc;
const String deviceID = "IoT-" + String(ip);
const String ipString = "192.168.7." + String(ip);

struct WifiCred {
  const char* ssid;
  const char* pass;
};
const WifiCred wifiList[] = {
  { "SSID_1", "PASSWORD_1" },
  { "SSID_2", "PASSWORD_2" },
  { "SSID_3", "PASSWORD_3" }
};
const int WIFI_COUNT = sizeof(wifiList) / sizeof(wifiList[0]);

IPAddress localIP(192, 168, 7, ip);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress dns1(192, 168, 15, 250);

const char* SERVER_HOST = "192.168.7.223";
const uint16_t DATA_PORT = 5001;
const char* DATA_PATH = "/weigher/save_weigher.php";
const uint16_t LOG_PORT = 5001;
const char* LOG_PATH = "/weigher/upload_log_weigher";
const char* NTP_SERVER_1 = "192.168.7.223";
const char* NTP_SERVER_2 = "pool.ntp.org";
const long GMT_OFFSET = 7 * 3600;

const int LOADCELL_DOUT_PIN = 27;
const int LOADCELL_SCK_PIN = 14;
const uint8_t BTN[4] = { 32, 33, 25, 26 };
#define SD_CS 5

const float MIN_WEIGHT_KG = 0.1f;
const unsigned long WEIGH_HOLD_MS = 200;
const unsigned long SEND_LOG_HOLD_MS = 3000;
const int EEPROM_ADDRESS = 0;

const int values[] = { 1, 2, 5, 10, 20, 40 };
const int digits[] = { 0, 1, 2, 3, 4 };
const String product[] = {
  "TIC TIC CBP",
  "LEANET BBQ",
  "V-TOZ",
  "LEANET TIC2 BWG",
  "TIC TIC SAMCOL",
  "FUJI CHIPS",
  "TIC TIC BALADO",
  "TEST_31"
};
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

HX711 scale;
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS3231 rtc;
Preferences prefs;

float calibrationFactor = 1.0f;
int digitScale = 2;
String productSelected;

String logPathStr;
String logFileName;
bool sdStatus = false;

SemaphoreHandle_t sdMutex;
QueueHandle_t sendQueue;

struct Record {
  char id[48];
  char product[32];
  char weight[12];
  char datetime[20];
};

volatile bool wifiUp = false;
char currentSsid[33] = "";
volatile bool uploading = false;
volatile bool logRequested = false;
volatile int logResult = 0;
volatile uint32_t okSent = 0;
volatile uint32_t failSent = 0;
volatile uint32_t saveCount = 0;
volatile uint32_t saveFailed = 0;
volatile uint32_t lastSent = 0;
uint32_t seqNumber = 0;

volatile bool ntpPending = false;
volatile int ntpY, ntpM, ntpD, ntpH, ntpMin, ntpS;

String msgText = "";
unsigned long msgUntil = 0;

bool btn(int n) {
  return digitalRead(BTN[n - 1]) == LOW;
}

bool pressedOnce(int n) {
  if (btn(n)) {
    delay(25);
    if (btn(n)) return true;
  }
  return false;
}

void waitRelease(int n) {
  while (btn(n)) delay(10);
  delay(30);
}

void lcdLine(uint8_t row, const String& s) {
  char b[21];
  snprintf(b, sizeof(b), "%-20.20s", s.c_str());
  lcd.setCursor(0, row);
  lcd.print(b);
}

void showMessage(const String& text, unsigned long ms) {
  msgText = text;
  msgUntil = millis() + ms;
}

void saveFloatToEEPROM(int address, float value) {
  float current;
  EEPROM.get(address, current);
  if (current != value) {
    EEPROM.put(address, value);
    EEPROM.commit();
  }
}

String urlEncode(const String& s) {
  String out = "";
  char buf[4];
  for (size_t i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out += c;
    } else {
      snprintf(buf, sizeof(buf), "%%%02X", (uint8_t)c);
      out += buf;
    }
  }
  return out;
}

bool appendLog(const String& line) {
  if (!sdStatus) return false;
  if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(2000)) != pdTRUE) return false;
  bool ok = false;
  File f = SD.open(logPathStr.c_str(), FILE_APPEND);
  if (f) {
    ok = f.println(line) > 0;
    f.close();
  }
  xSemaphoreGive(sdMutex);
  return ok;
}

uint32_t countLines(const char* path) {
  uint32_t lines = 0;
  File f = SD.open(path, FILE_READ);
  if (!f) return 0;
  uint8_t buf[256];
  while (f.available()) {
    int n = f.read(buf, sizeof(buf));
    for (int i = 0; i < n; i++) {
      if (buf[i] == '\n') lines++;
    }
  }
  f.close();
  return lines;
}

bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  vTaskDelay(pdMS_TO_TICKS(100));

  int rssi[WIFI_COUNT];
  int order[WIFI_COUNT];
  for (int i = 0; i < WIFI_COUNT; i++) {
    rssi[i] = -999;
    order[i] = i;
  }

  int n = WiFi.scanNetworks();
  for (int j = 0; j < n; j++) {
    String found = WiFi.SSID(j);
    for (int i = 0; i < WIFI_COUNT; i++) {
      if (found == wifiList[i].ssid && WiFi.RSSI(j) > rssi[i]) {
        rssi[i] = WiFi.RSSI(j);
      }
    }
  }
  WiFi.scanDelete();

  for (int a = 0; a < WIFI_COUNT - 1; a++) {
    for (int b = 0; b < WIFI_COUNT - 1 - a; b++) {
      if (rssi[order[b]] < rssi[order[b + 1]]) {
        int t = order[b];
        order[b] = order[b + 1];
        order[b + 1] = t;
      }
    }
  }

  for (int k = 0; k < WIFI_COUNT; k++) {
    int idx = order[k];
    if (n > 0 && rssi[idx] == -999) continue;

    WiFi.config(localIP, gateway, subnet, dns1);
    WiFi.begin(wifiList[idx].ssid, wifiList[idx].pass);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      vTaskDelay(pdMS_TO_TICKS(200));
    }
    if (WiFi.status() == WL_CONNECTED) {
      strlcpy(currentSsid, wifiList[idx].ssid, sizeof(currentSsid));
      return true;
    }
    WiFi.disconnect();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  return false;
}

bool postRealtime(const Record& r) {
  HTTPClient http;
  WiFiClient client;
  String url = "http://" + String(SERVER_HOST) + ":" + String(DATA_PORT) + String(DATA_PATH);
  if (!http.begin(client, url)) return false;
  http.setTimeout(5000);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String body = "id=" + urlEncode(r.id);
  body += "&device_id=" + urlEncode(deviceID);
  body += "&device_name=" + urlEncode(ESPName);
  body += "&product=" + urlEncode(r.product);
  body += "&weight=" + urlEncode(r.weight);
  body += "&ip_address=" + urlEncode(ipString);
  body += "&wifi=" + urlEncode(currentSsid);
  body += "&timestamp=" + urlEncode(r.datetime);

  int code = http.POST(body);
  http.end();
  return code == 200;
}

void sendLogFile() {
  if (xSemaphoreTake(sdMutex, pdMS_TO_TICKS(3000)) != pdTRUE) {
    logResult = 2;
    return;
  }

  File f = SD.open(logPathStr.c_str(), FILE_READ);
  if (!f || f.size() == 0) {
    if (f) f.close();
    xSemaphoreGive(sdMutex);
    logResult = 3;
    return;
  }

  size_t fileSize = f.size();
  uint32_t lines = 0;
  uint8_t buf[512];
  while (f.available()) {
    int n = f.read(buf, sizeof(buf));
    for (int i = 0; i < n; i++) {
      if (buf[i] == '\n') lines++;
    }
  }
  f.seek(0);

  WiFiClient client;
  if (!client.connect(SERVER_HOST, LOG_PORT, 5000)) {
    f.close();
    xSemaphoreGive(sdMutex);
    logResult = 2;
    return;
  }

  String boundary = "----weigher" + String(millis());
  String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" + logFileName + "\"\r\nContent-Type: text/plain\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";
  size_t contentLength = head.length() + fileSize + tail.length();

  client.print("POST " + String(LOG_PATH) + " HTTP/1.1\r\n");
  client.print("Host: " + String(SERVER_HOST) + "\r\n");
  client.print("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
  client.print("Content-Length: " + String(contentLength) + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(head);
  while (f.available()) {
    int n = f.read(buf, sizeof(buf));
    if (n > 0) client.write(buf, n);
  }
  client.print(tail);
  f.close();

  int status = 0;
  bool headersDone = false;
  String body = "";
  unsigned long t0 = millis();
  while ((client.connected() || client.available()) && millis() - t0 < 30000) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      if (!headersDone) {
        if (line.startsWith("HTTP/1.")) status = line.substring(9, 12).toInt();
        if (line.length() <= 1) headersDone = true;
      } else {
        body += line;
      }
    } else {
      vTaskDelay(pdMS_TO_TICKS(5));
    }
  }
  client.stop();

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, body);
  const char* st = doc["status"] | "";
  uint32_t jumlah = doc["jumlah_data"] | 0;

  bool ok = status == 200 && !err && strcmp(st, "success") == 0 && jumlah == lines;
  if (ok) {
    SD.remove(logPathStr.c_str());
    saveCount = 0;
    lastSent = jumlah;
    prefs.putUInt("lastSent", jumlah);
    logResult = 1;
  } else {
    logResult = 2;
  }
  xSemaphoreGive(sdMutex);
}

void netTask(void* parameter) {
  bool ntpConfigured = false;
  unsigned long lastNtp = 0;
  bool ntpEverOk = false;

  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      wifiUp = false;
      if (!connectWiFi()) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        continue;
      }
    }
    wifiUp = true;

    if (!ntpConfigured) {
      configTime(GMT_OFFSET, 0, NTP_SERVER_1, NTP_SERVER_2);
      ntpConfigured = true;
    }
    if (!ntpEverOk || millis() - lastNtp > 3600000UL) {
      struct tm tmNow;
      if (getLocalTime(&tmNow, 2000) && tmNow.tm_year >= 124) {
        ntpY = tmNow.tm_year + 1900;
        ntpM = tmNow.tm_mon + 1;
        ntpD = tmNow.tm_mday;
        ntpH = tmNow.tm_hour;
        ntpMin = tmNow.tm_min;
        ntpS = tmNow.tm_sec;
        ntpPending = true;
        ntpEverOk = true;
        lastNtp = millis();
      }
    }

    Record r;
    while (wifiUp && xQueueReceive(sendQueue, &r, 0) == pdTRUE) {
      if (postRealtime(r)) {
        okSent++;
      } else {
        failSent++;
      }
    }

    if (logRequested) {
      logRequested = false;
      uploading = true;
      sendLogFile();
      uploading = false;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void applyNtp() {
  if (ntpPending) {
    rtc.adjust(DateTime(ntpY, ntpM, ntpD, ntpH, ntpMin, ntpS));
    ntpPending = false;
  }
}

bool calibrationRun() {
  scale.set_scale(1.0f);
  int vi = 0;
  int di = 0;

  lcd.clear();
  lcdLine(0, "KALIBRASI");
  lcdLine(1, "PILIH BERAT ACUAN");
  while (true) {
    lcdLine(2, "BERAT: " + String(values[vi]) + " KG");
    lcdLine(3, "1:ULANG 2:NAIK 3:OK");
    if (pressedOnce(1)) {
      waitRelease(1);
      return false;
    }
    if (pressedOnce(2)) {
      waitRelease(2);
      vi = (vi + 1) % ARRAY_LENGTH(values);
    }
    if (pressedOnce(3)) {
      waitRelease(3);
      break;
    }
    delay(20);
  }

  lcd.clear();
  lcdLine(0, "KOSONGKAN TIMBANGAN");
  for (int i = 3; i >= 1; i--) {
    lcdLine(1, "TARE " + String(i));
    scale.tare();
    delay(600);
  }
  scale.tare();
  lcdLine(1, "SIAP");
  lcdLine(3, "1:ULANG    3:LANJUT");
  while (true) {
    if (pressedOnce(1)) {
      waitRelease(1);
      return false;
    }
    if (pressedOnce(3)) {
      waitRelease(3);
      break;
    }
    delay(20);
  }

  lcd.clear();
  lcdLine(0, "LETAKKAN BEBAN");
  lcdLine(1, String(values[vi]) + " KG");
  while (true) {
    double reading = scale.get_units(10);
    calibrationFactor = (float)reading / (values[vi] * 1000.0f);
    lcdLine(2, "FAKTOR: " + String(calibrationFactor, 4));
    lcdLine(3, "1:ULANG    3:LANJUT");
    if (pressedOnce(1)) {
      waitRelease(1);
      return false;
    }
    if (pressedOnce(3)) {
      waitRelease(3);
      break;
    }
  }

  lcd.clear();
  lcdLine(0, "SET DIGIT DESIMAL");
  while (true) {
    lcdLine(2, "DIGIT: " + String(digits[di]));
    lcdLine(3, "1:ULANG 2:NAIK 3:OK");
    if (pressedOnce(1)) {
      waitRelease(1);
      return false;
    }
    if (pressedOnce(2)) {
      waitRelease(2);
      di = (di + 1) % ARRAY_LENGTH(digits);
    }
    if (pressedOnce(3)) {
      waitRelease(3);
      digitScale = digits[di];
      break;
    }
    delay(20);
  }

  lcd.clear();
  lcdLine(0, "KALIBRASI SELESAI");
  lcdLine(1, "FAKTOR " + String(calibrationFactor, 4));
  lcdLine(2, "DIGIT " + String(digitScale));
  lcdLine(3, "1:ULANG   4:SIMPAN");
  while (true) {
    if (pressedOnce(1)) {
      waitRelease(1);
      return false;
    }
    if (pressedOnce(4)) {
      waitRelease(4);
      saveFloatToEEPROM(EEPROM_ADDRESS, calibrationFactor);
      saveFloatToEEPROM(EEPROM_ADDRESS + 4, (float)digitScale);
      lcd.clear();
      return true;
    }
    delay(20);
  }
}

void calibrationProcess() {
  while (!calibrationRun()) {
  }
}

void chooseProduct() {
  int idx = 0;
  lcd.clear();
  lcdLine(0, "PILIH PRODUK");
  lcdLine(2, "1:BAWAH   2:ATAS");
  lcdLine(3, "3:PILIH");
  while (true) {
    lcdLine(1, product[idx]);
    if (pressedOnce(1)) {
      waitRelease(1);
      idx = (idx + 1) % ARRAY_LENGTH(product);
    }
    if (pressedOnce(2)) {
      waitRelease(2);
      idx = (idx + ARRAY_LENGTH(product) - 1) % ARRAY_LENGTH(product);
    }
    if (pressedOnce(3)) {
      waitRelease(3);
      productSelected = product[idx];
      break;
    }
    delay(20);
  }
  lcd.clear();
}

void weighAndSave() {
  if (uploading) {
    showMessage("SEDANG KIRIM LOG", 1500);
    return;
  }

  float kg = scale.get_units(5) / 1000.0f;
  if (kg < MIN_WEIGHT_KG) return;

  char w[12];
  dtostrf(kg, 1, digitScale, w);

  DateTime t = rtc.now();
  char dt[20];
  snprintf(dt, sizeof(dt), "%04d-%02d-%02d %02d:%02d:%02d", t.year(), t.month(), t.day(), t.hour(), t.minute(), t.second());

  char id[48];
  snprintf(id, sizeof(id), "%s-%lu-%lu", deviceID.c_str(), (unsigned long)t.unixtime(), (unsigned long)(++seqNumber));

  String ssid = wifiUp ? String(currentSsid) : String("-");
  String line = String(id) + ',' + deviceID + ',' + ESPName + ',' + productSelected + ',' + w + ',' + dt + ',' + ipString + ',' + ssid;

  bool sdOk = true;
  if (sdStatus) {
    if (appendLog(line)) {
      saveCount++;
    } else {
      sdOk = false;
      saveFailed++;
    }
  }

  bool queued = false;
  if (wifiUp) {
    Record r;
    memset(&r, 0, sizeof(r));
    strlcpy(r.id, id, sizeof(r.id));
    strlcpy(r.product, productSelected.c_str(), sizeof(r.product));
    strlcpy(r.weight, w, sizeof(r.weight));
    strlcpy(r.datetime, dt, sizeof(r.datetime));
    queued = xQueueSend(sendQueue, &r, 0) == pdTRUE;
  }
  if (!queued) failSent++;

  if (sdStatus && !sdOk) {
    showMessage("DATA GGL DISAVE", 1500);
  } else if (!queued) {
    showMessage("GAGAL KIRIM", 1500);
  } else {
    showMessage(String(w) + " KG TERSIMPAN", 1000);
  }
}

void requestSendLog() {
  if (!wifiUp) {
    showMessage("WIFI PUTUS", 1500);
  } else if (!sdStatus) {
    showMessage("SD TIDAK ADA", 1500);
  } else if (uploading) {
    showMessage("SEDANG KIRIM LOG", 1500);
  } else {
    logRequested = true;
    showMessage("KIRIM LOG...", 1000);
  }
}

void drawMain(const char* kgStr) {
  DateTime t = rtc.now();

  lcdLine(0, productSelected);
  lcdLine(1, String(kgStr) + " KG");

  char row2[32];
  snprintf(row2, sizeof(row2), "%c %-8.8s .%d %02d:%02d", wifiUp ? 'W' : 'X', wifiUp ? currentSsid : "putus", ip, t.hour(), t.minute());
  lcdLine(2, String(row2));

  if (millis() < msgUntil) {
    lcdLine(3, msgText);
  } else if (uploading) {
    lcdLine(3, "KIRIM LOG...");
  } else {
    char row3[40];
    snprintf(row3, sizeof(row3), "B%lu G%lu S%lu L%lu", (unsigned long)okSent, (unsigned long)failSent, (unsigned long)saveCount, (unsigned long)lastSent);
    lcdLine(3, String(row3));
  }
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) pinMode(BTN[i], INPUT_PULLUP);

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  EEPROM.begin(512);
  prefs.begin("weigher", false);
  lastSent = prefs.getUInt("lastSent", 0);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  rtc.begin();

  logPathStr = "/weigherLog" + String(ip) + ".txt";
  logFileName = "weigherLog" + String(ip) + ".txt";

  sdMutex = xSemaphoreCreateMutex();
  sendQueue = xQueueCreate(20, sizeof(Record));

  if (btn(1) && btn(2)) {
    lcdLine(0, "KALIBRASI");
    while (btn(1) || btn(2)) delay(10);
    delay(50);
    calibrationProcess();
  }

  EEPROM.get(EEPROM_ADDRESS, calibrationFactor);
  float digitF;
  EEPROM.get(EEPROM_ADDRESS + 4, digitF);
  if (isnan(calibrationFactor) || calibrationFactor == 0) calibrationFactor = 1.0f;
  digitScale = (isnan(digitF) || digitF < 0 || digitF > 4) ? 2 : (int)digitF;
  scale.set_scale(calibrationFactor);
  scale.tare();

  lcdLine(0, "SET SD CARD");
  if (!SD.begin(SD_CS)) {
    sdStatus = false;
    lcd.clear();
    lcdLine(0, "SD CARD ERROR");
    lcdLine(1, "X, HUBUNGI IT");
    lcdLine(2, "DATA TDK DISIMPAN SD");
    delay(3000);
  } else {
    sdStatus = true;
    saveCount = countLines(logPathStr.c_str());
  }

  lcd.clear();
  lcdLine(0, "SET WIFI");
  xTaskCreatePinnedToCore(netTask, "netTask", 12288, NULL, 1, NULL, 0);

  unsigned long start = millis();
  int dots = 0;
  while (!wifiUp && millis() - start < 20000) {
    lcdLine(1, "MENCARI" + String("...").substring(0, (dots++ % 3) + 1));
    delay(500);
  }
  if (wifiUp) {
    lcdLine(1, String(currentSsid));
    lcdLine(2, ipString);
    unsigned long t0 = millis();
    while (!ntpPending && millis() - t0 < 5000) delay(100);
  } else {
    lcdLine(1, "WIFI TIDAK ADA");
  }
  applyNtp();
  delay(1500);

  chooseProduct();
}

void loop() {
  applyNtp();

  float kg = scale.get_units(5) / 1000.0f;
  char kgStr[12];
  dtostrf(kg, 7, digitScale, kgStr);

  if (logResult != 0) {
    int res = logResult;
    logResult = 0;
    if (res == 1) {
      showMessage("LOG TERKIRIM " + String(lastSent), 3000);
    } else if (res == 3) {
      showMessage("LOG KOSONG", 2000);
    } else {
      showMessage("GAGAL KIRIM LOG", 3000);
    }
  }

  drawMain(kgStr);

  if (btn(4)) {
    unsigned long s = millis();
    while (btn(4) && millis() - s < WEIGH_HOLD_MS) delay(5);
    if (btn(4)) {
      weighAndSave();
      waitRelease(4);
    }
  }

  if (pressedOnce(3)) {
    scale.tare();
    showMessage("TARE OK", 1000);
    waitRelease(3);
  }

  if (pressedOnce(2)) {
    waitRelease(2);
    chooseProduct();
  }

  if (btn(1)) {
    unsigned long s = millis();
    int lastShown = -1;
    bool triggered = false;
    while (btn(1)) {
      unsigned long held = millis() - s;
      if (held >= SEND_LOG_HOLD_MS) {
        triggered = true;
        break;
      }
      int remain = (SEND_LOG_HOLD_MS - held + 999) / 1000;
      if (remain != lastShown) {
        lastShown = remain;
        lcdLine(3, "TAHAN KIRIM LOG: " + String(remain));
      }
      delay(20);
    }
    if (triggered) {
      requestSendLog();
      waitRelease(1);
    } else {
      lcdLine(3, "");
    }
  }
}
