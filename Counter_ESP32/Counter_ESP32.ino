#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>

// Set the LCD address to 0x3F/0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Sensor IR
const int IRpin = 2;
int IRValue;
const int threshold = 1000;
int count = 0;
bool cond = false;
unsigned long previousMillis = 0;
const long interval = 1000;  // Example: 1 second interval

// RTC
RTC_DS3231 rtc;
DateTime now;
int dataSeconds = 0;
int dataMinutes = 0;
int dataHours = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String formattedTime;

// Setup Wifi
const char* ssid = "STTB1";
const char* password = "Si4nt4r321";
const char* ssid2 = "MT1";
const char* password2 = "siantar321";
const char* ssid3 = "MT3";
const char* password3 = "siantar321";

unsigned long lastConnectionAttempt = 0;  // Waktu terakhir koneksi dicoba
const unsigned long connectionAttemptInterval = 600000;  // Interval antara percobaan koneksi (10 menit)

// Konfigurasi IP statis
// Set your Static IP address
IPAddress local_IP(192, 168, 1, 218);
// Set your Gateway IP address
IPAddress gateway(192, 168, 15, 250);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

// Get Data
String serverNameGet = "http://192.168.15.221/counter_hit_api/getDataLastCounter.php?kode_product=";
String serverNameSend = "http://192.168.15.221/counter_hit_api/saveCounter.php";
String getData;
const char* counterFromDB;
int counterValueDB;
bool sendStatus;

String dateTime;
String dateTimeLCD;
String postData;
String ip_address;
String deviceName = "Counter";
String dataSD;

// Kode Produk
String produk1 = "P-0722-00239";
String produk2 = "produk2";
String produk3 = "produk3";
String produk4 = "produk4";

String nama1 = "Tic Tic Bwg 2000";

TaskHandle_t Core1Task;

void counterHit(void *parameter) {
  while (1) {
    // Your Core 1 specific code here
    IRValue = analogRead(IRpin);
    if (IRValue < threshold && !cond) {
      count++;
      cond = true;
    } else if (IRValue >= threshold) {
      cond = false;
    }
  }
}

void connectToWiFi()  {
  int flag = 1; 

  // Configures static IP address
  if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  int network = WiFi.scanNetworks();
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    for (int i = 0 ; i < network; i++) {
      switch (flag) {
        case 1: 
          flag = 2;
          if (WiFi.SSID(i) == ssid) {
            WiFi.begin(ssid, password);
            lcd.setCursor(1, 0);
            lcd.print("Connecting to WiFi");
            delay(80);
            break;
          }
          case 2: 
            flag = 3;
            if (WiFi.SSID(i) == ssid2) {
            WiFi.begin(ssid2, password2);
            lcd.setCursor(1, 0);
            lcd.print("Connecting to WiFi");
            delay(80);
            break;
          }
          case 3:
            flag = 1;
            if (WiFi.SSID(i) == ssid3) {
            WiFi.begin(ssid3, password3);
            lcd.setCursor(1, 0);
            lcd.print("Connecting to WiFi");
            delay(80);
            break;
          }
      }
    }

    retries++;
    delay(500);
    Serial.print(".");
  }
  if (retries > 14) {
      Serial.println(F("WiFi connection FAILED"));
  }
  if (WiFi.status() == WL_CONNECTED) {
      Serial.println(F("WiFi connected!"));
      Serial.println("IP address : ");
      Serial.println(WiFi.localIP());
  }else if(WiFi.status() != WL_CONNECTED){
      Serial.println(F("WiFi disconnected!"));
  }
}

void sendLogData() {
  now = rtc.now();
  dateTime = String(now.year()) + '-' + String(now.month()) + '-' + String(now.day()) + ' ' + String(timeClient.getFormattedTime());
 
  HTTPClient http;    // http object of clas HTTPClient
  http.begin(serverNameSend);
  postData = "kode_product=" + String(produk1) + "&counter=" + String(count) + "&date=" + dateTime + "&ip_address=" + ip_address;
//  http.begin(wclient, "http://192.168.15.221/counter_hit_api/saveCounter.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

  int httpCode = http.POST(postData);
  // If connection eatablished then do this
  if (httpCode == 200) {
    String webpage = http.getString();
    Serial.println(webpage + "\n");
    return;
  } else { 
    Serial.println(httpCode); 
    Serial.println("Failed to upload values. \n"); 
  }
  http.end();
}

void getLogData() {
  HTTPClient http;    // http object of clas HTTPClient
//  WiFiClient wclient; // wclient object of clas HTTPClient
  getData = "/getDataLastCounter.php?kode_product=" + String(produk1);
  http.begin("http://192.168.15.221/counter_hit_api", 80, getData);

  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String payload = http.getString();
    JSONVar myArray = JSON.parse(payload);
    counterFromDB = myArray["counter"];
    counterValueDB = atoi(counterFromDB);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  sendStatus = false;

  // Create task for Core 1
  xTaskCreatePinnedToCore(
    counterHit,       // Function to run on Core 1
    "Core1Task",      // Task name
    10000,            // Stack size
    NULL,             // Parameters to pass to the task
    1,                // Task priority
    &Core1Task,       // Task handle
    1                 // Core number (1 for Core 1)
  );
  
  // LCD
  lcd.init();
  lcd.clear();         
  lcd.backlight();

  // Get NTP
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  rtc.begin();

  connectToWiFi();
}

void loop() {
  ip_address = WiFi.localIP().toString();
  now = rtc.now();
  timeClient.update();
  dateTime = String(now.year()) + '-' + String(now.month()) + '-' + String(now.day()) + ' ' + String(timeClient.getFormattedTime());
  dateTimeLCD = String(timeClient.getFormattedTime());
  
  lcd.setCursor(1, 0);
  lcd.print(produk1);
  lcd.setCursor(1, 1);
  lcd.print(nama1);
  lcd.setCursor(11, 3);
  lcd.print(dateTimeLCD);
  lcd.setCursor(1, 2);
  lcd.print("Total : ");
  lcd.setCursor(9, 2);
  lcd.print(count);
      
  if(WiFi.status() == WL_CONNECTED) {
    now = rtc.now();
    timeClient.update();
    
    dataSeconds = timeClient.getSeconds();
    if(count == 0) {
      getLogData();
      if(counterValueDB != 0){
        count = counterValueDB;
      }
    }
    
    dataMinutes = timeClient.getMinutes();
    dataHours = timeClient.getHours();
    if(dataHours == 7 && dataMinutes == 50 && dataSeconds == 0) {
      count = 0;
      delay(1000);
      sendLogData();
      delay(1000);
      lcd.clear();
      delay(100);
    }
    
    if(dataHours == 19 && dataMinutes == 50 && dataSeconds == 0) {
      count = 0;
      delay(1000);
      sendLogData();
      delay(1000);
      lcd.clear();
      delay(100);
    }

    if(dataSeconds == 10 || dataSeconds == 25 || dataSeconds == 40 || dataSeconds == 55) {
      if(sendStatus == true) {
        sendStatus = false;
      }
    }
    
    if(dataSeconds == 00 || dataSeconds == 15 || dataSeconds == 30 || dataSeconds == 45) {
      if(sendStatus == false) {
        sendLogData();
        sendStatus = true;
      }
    }
    
    lcd.setCursor(1, 3);
    lcd.print("WiFi OK ");
    
  } else {
    lcd.setCursor(1, 3);
    lcd.print("WiFi Dis");
  }
}
