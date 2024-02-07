#include <WiFi.h>
#include <WiFiMulti.h>
#include "time.h"
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// == WiFi Config ==
WiFiMulti wifiMulti;
const char* ssid_a = "STTB1";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "MT1";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
const char* ssid_d = "Djoksen";
const char* password_d = "Welive99";
const char* ssid_e = "A52s";
const char* password_e = "28092005";

// Set IP to Static
IPAddress staticIP(192, 168, 15, 218);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

// == Get NTP ==
const char* ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 7 * 3600;  // GMT +7
const int daylightOffsetSec = 0;
String dateTime;
int year;
int month;
int day;
int hour;
int minute;
int second;

// == Counter ==
int counter;
TaskHandle_t Task1;

// == Data Send/Get ==
bool sendStatus;
const char* counterFromDB;
int counterValueDB;

// == Product Related Section ==
String produk1 = "P-0722-00239";
String produk2 = "produk2";
String produk3 = "produk3";
String produk4 = "produk4";

String nama1 = "Tic Tic Bwg 2000";

// == Try Reset Button ==


void Task1code(void* parameter) {
  pinMode(21, INPUT);
  counter = 0;
  int lastButtonState = LOW;
  for (;;) {
    // Membaca status tombol
    int buttonState = digitalRead(21);
    if (buttonState == HIGH && lastButtonState == LOW) {
      counter++;
    }
    lastButtonState = buttonState;
  }
}

void getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  char timeStringBuff[50];  //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  dateTime = String(timeStringBuff);

  // Save time data to variabel
  year = timeinfo.tm_year + 1900;
  month = timeinfo.tm_mon + 1;
  day = timeinfo.tm_mday;
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
}

// void sendLogData() {
//   HTTPClient http;

//   http.begin("http://192.168.15.221/counter_hit_api/saveCounter.php");
//   http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//   postData = "kode_product=" + String(produk1) + "&counter=" + String(counter) + "&date=" + dateTime + "&ip_address=" + String(WiFi.localIP());
//   int httpResponseCode = http.POST(postData);

//   if (httpResponseCode > 0) {
//     String response = http.getString();
//     Serial.println(httpResponseCode);
//     Serial.println(response);
//   } else {
//     Serial.print("Error on sending POST: ");
//     Serial.println(httpResponseCode);
//   }
//   http.end();
// }

 void getLogData() {
   HTTPClient http;
   String getData = "http://192.168.15.221/counter_hit_api/getDataLastCounter.php?kode_product=" + String(produk1);

   http.begin(getData);
   int httpCode = http.GET();

   if (httpCode > 0) {
     String payload = http.getString();
     JSONVar myArray = JSON.parse(payload);
     counterFromDB = myArray["counter"];
     counterValueDB = atoi(counterFromDB);
   } else {
     Serial.print("Error code: ");
     Serial.println(httpCode);
   }
   http.end();
 }

void resetESP() {
  int buttonReset = digitalRead(23);
  if (buttonReset == LOW) {
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  sendStatus = false;

  pinMode(23, INPUT);

  xTaskCreatePinnedToCore(
    Task1code, /* Fungsi untuk mengimplementasikan tugas */
    "Task1",   /* Nama tugas */
    10000,     /* Ukuran stack dalam kata */
    NULL,      /* Parameter input tugas */
    0,         /* Prioritas tugas */
    &Task1,    /* Handle tugas. */
    0          /* Core tempat tugas harus dijalankan */
  );

  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);
  wifiMulti.addAP(ssid_d, password_d);
  wifiMulti.addAP(ssid_e, password_e);
  // if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
  //   Serial.println("STA Failed to configure");
  // }

  // Init and get the time
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
}

void loop() {
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    delay(1000);
    wifiMulti.run();
  } else {
    getLocalTime();

    // Get Data Here1
    // if (counter == 0) {
    //   getLogData();
    //   if (counterValueDB != 0) {
    //     counter = counterValueDB;
    //   }
    // }

    // Reset Counter Here
    // if ((hour == 7 && minute == 50 && second == 0) || (hour == 19 && minute == 50 && second == 0)) {
    //   counter = 0;
    //   delay(1000);
    //   sendLogData();
    //   delay(1000);
    //   lcd.clear();
    //   delay(100);
    // }

    // Send Data Here
    if ((second == 15 || second == 30 || second == 45 || second == 0) && !sendStatus) {
      Serial.println("WiFi connected!");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("Connected to SSID: " + WiFi.SSID());
      Serial.println(dateTime);
      Serial.println("Detik: " + String(second));
      Serial.println("Counter: " + String(counter));
      sendStatus = true;
      // getLogData();
      Serial.println("Counter DB: " + String(counterValueDB));
    } else if ((second == 10 || second == 25 || second == 40 || second == 55) && sendStatus) {
      sendStatus = false;
    }
  }
  // resetESP();
}