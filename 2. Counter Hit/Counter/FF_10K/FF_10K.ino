/*
  Update Terakhir : 22-09-2025
  Lihat 0_counter_hit_template untuk penjelasan kode
  */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <LiquidCrystal_I2C.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <RTClib.h>
#include "time.h"

String ESPName = "FF 10000";

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define upButton 34
#define downButton 35
#define selectButton 32

#define sensorPin 14
// #define sensorReject 26

WiFiMulti wifiMulti;
const char* ssid_a = "STTB4";
const char* password_a = "siantar123";
const char* ssid_d = "STTB1";
const char* password_d = "Si4nt4r321";
const char* ssid_b = "MT1";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
const char* ssid_it = "STTB11";
const char* password_it = "Si4nt4r321";

IPAddress staticIP(192, 168, 7, 222);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
String ip_Address;

const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat;
int year, rtcYear;
int month, rtcMonth;
int day, rtcDay;
int hour, rtcHour;
int minute, rtcMinute;
int second, rtcSecond;
RTC_DS3231 rtc;
DateTime now;
bool ntpStatus, statusUpdateRTC;

TaskHandle_t Task1;
int counter, newCounter, oldCounter, counterReject;

bool sendStatus, getStatus;
const char* counterFromDB;
int counterValueDB;
String postData;

bool menuSelect;
int menu = 1;
String productSelected, nameProductSelected;
String productCodeOne = "P-0722-00243";
String nameProductOne = "FF 10000";
String productCodeTwo = "P-0722-00239";
String nameProductTwo = "FF 10000 (M-LOW)";
String productCodeFour = "Test_Mode_222";
String nameProductFour = "Test_Mode_222";

int lineAsInt;
String dateTimeSD, productSelectedSD, counterSD, counterRejectSD, ipAddressSD, line, logName, logData;
bool statusSD, readStatusSD, insertLastLineSDCardStatus;

// Jika NPN
void counterHit(void* parameter) {
  for (;;) {
    // IR Counter
    pinMode(sensorPin, INPUT_PULLUP);
    static int lastIRState = HIGH;
    int irState = digitalRead(sensorPin);
    if (irState == LOW && lastIRState == HIGH) {
      counter++;
    }
    lastIRState = irState;
    delay(50);

    // IR Reject
    // pinMode(sensorReject, INPUT_PULLUP);
    // static int lastIRStateReject = LOW;
    // int irStateReject = digitalRead(sensorReject);
    // if (irStateReject == HIGH && lastIRStateReject == LOW) {
    //   counterReject++;
    // }
    // lastIRStateReject = irStateReject;
    // delay(50);

    Serial.print("Counter 1:" );
    Serial.println(counter);
    // Serial.print("Counter 2: ");
    // Serial.println(counterReject);
  }
}

void getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    ntpStatus = false;
  } else {
    ntpStatus = true;

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
}

void updateRTC() {
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.adjust(DateTime(year, month, day, hour, minute, second));

  if (now.year() != 1990) {
    statusUpdateRTC = true;
  } else if (now.year() == 1990) {
    statusUpdateRTC = false;
  }
}

void sendLogData() {
  String api = "http://192.168.7.223/counter_hit_api/saveCounter.php";
  HTTPClient http;
  http.begin(api);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
  } else {
    String response = http.getString();
    Serial.println(response);
    Serial.print("Error on sending POST");
  }
  http.end();
}

void getLogData() {
  HTTPClient http;
  String getData = "http://192.168.7.223/counter_hit_api/getDataLastCounter.php?kode_product=" + productSelected;
  http.begin(getData);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    JSONVar myArray = JSON.parse(payload);
    counterFromDB = myArray["counter"];
    counterValueDB = atoi(counterFromDB);
  } else {
    Serial.print("Error get log");
  }
  http.end();
}

void resetESP() {
  if (digitalRead(selectButton) && digitalRead(upButton) && digitalRead(downButton)) {
    lcd.clear();
    // Send Data to SD
    insertLastLineSDCard(logName, logData);
    lcd.setCursor(1, 0);
    lcd.print("Loading...");

    // Get Data from SD
    readLastLineSDCard(logName);
    postData = "kode_product=" + productSelectedSD + "&counter=" + counterSD + "&rejector=" + counterRejectSD + "&date=" + dateTimeSD + "&ip_address=" + ipAddressSD;

    // Send Data to DB
    sendLogData();
    lcd.setCursor(1, 1);
    lcd.print("Send Update to DB");
    delay(100);

    // Send Reset Status to DB
    postData = "kode_product=" + ESPName + "&counter=" + String(0) + "&rejector=" + String(0) + "&date=" + String(0) + "&ip_address=" + ip_Address;
    sendLogData();

    // Delete Data from SD
    lcd.setCursor(1, 2);
    lcd.print("Deleting Log in SD");
    deleteLog(logName);

    lcd.setCursor(1, 3);
    lcd.print("Resetting...");
    delay(1000);

    ESP.restart();
  }
}

void selectMenu() {
  updateMenu();
  while (menuSelect == false) {
    resetESP();
    if (digitalRead(downButton)) {
      menu++;
      updateMenu();
      delay(100);
      while (digitalRead(downButton))
        ;
    }
    if (digitalRead(upButton)) {
      menu--;
      updateMenu();
      delay(100);
      while (digitalRead(upButton))
        ;
    }
    if (digitalRead(selectButton)) {
      delay(100);
      while (digitalRead(selectButton))
        ;
      menuSelect = true;
      menuSelected();
    }
  }
}

void updateMenu() {
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("==PILIH PRODUK==");
      lcd.setCursor(0, 1);
      lcd.print(">" + nameProductOne);
      lcd.setCursor(1, 2);
      lcd.print(nameProductTwo);
      lcd.setCursor(1, 3);
      lcd.print("WiFi : " + WiFi.SSID());
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("==PILIH PRODUK==");
      lcd.setCursor(1, 1);
      lcd.print(nameProductOne);
      lcd.setCursor(0, 2);
      lcd.print(">" + nameProductTwo);
      lcd.setCursor(1, 3);
      lcd.print("WiFi : " + WiFi.SSID());
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("==PILIH PRODUK==");
      lcd.setCursor(1, 1);
      lcd.print(nameProductOne);
      lcd.setCursor(1, 2);
      lcd.print(nameProductTwo);
      lcd.setCursor(0, 3);
      lcd.print(">" + nameProductThree);
      break;
    case 4:
      menu = 3;
      break;
  }
}

void menuSelected() {
  switch (menu) {
    case 1:
      productSelected = productCodeOne;
      nameProductSelected = nameProductOne;
      delay(1000);
      lcd.clear();
      break;
    case 2:
      productSelected = productCodeTwo;
      nameProductSelected = nameProductTwo;
      delay(1000);
      lcd.clear();
      break;
    case 3:
      productSelected = productCodeThree;
      nameProductSelected = nameProductThree;
      delay(1000);
      lcd.clear();
      break;
  }
}

void readLastLineSDCard(String path) {
  // Open the file for reading
  File file = SD.open(path);

  // Check if file opened successfully
  if (!file || file.isDirectory()) {
    Serial.println("Failed to open file for reading");
    readStatusSD = false;
    return;
  } else {
    readStatusSD = true;
  }

  // Read the last line of the file
  while (file.available()) {
    line = file.readStringUntil('\n');
  }
  file.close();

  // Print the data read from the file
  Serial.print("Data dari SD : ");
  Serial.println(line);

  // Extract product selected from the line
  int firstCommaIndex = line.indexOf(',');
  productSelectedSD = line.substring(0, firstCommaIndex);

  // Extract counter value from the line
  line = line.substring(firstCommaIndex + 1);
  int secondCommaIndex = line.indexOf(',');
  counterSD = line.substring(0, secondCommaIndex);

  // Extract reject from the line
  line = line.substring(secondCommaIndex + 1);
  int thirdCommaIndex = line.indexOf(',');
  counterRejectSD = line.substring(0, thirdCommaIndex);

  // Extract date and time from the line
  line = line.substring(thirdCommaIndex + 1);
  int fourthCommaIndex = line.indexOf(',');
  dateTimeSD = line.substring(0, fourthCommaIndex);

  // Extract IP address from the line
  ipAddressSD = line.substring(fourthCommaIndex + 1);
}

void insertLastLineSDCard(String path, String line) {
  // Open the file for writing
  File file = SD.open(path, FILE_WRITE);
  
  // Check if file opened successfully
  if (!file) {
    // Print error message if failed to open
    Serial.println("Failed to open file for writing");
    insertLastLineSDCardStatus = false;
    return;
  } else {
    // Set insertLastLineSDCardStatus to true if file opened successfully
    insertLastLineSDCardStatus = true;
  }

  // Write the line of text to the file
  if (file.println(line)) {
    // Print success message if line written successfully
    Serial.println("Line written");
  } else {
    // Print error message if write failed
    Serial.println("Write failed");
  }
  
  // Close the file
  file.close();
  
  // Print the data written to the SD card
  Serial.print("Data ke SD : ");
  Serial.println(line);
}

void deleteLog(String path) {
  if (SD.exists(path)) {
    SD.remove(path);
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed, file does not exist");
  }
}

void setup() {
  Serial.begin(115200);
  sendStatus = false;
  getStatus = false;
  menuSelect = false;
  readStatusSD = false;
  statusUpdateRTC = false;

  pinMode(upButton, INPUT);
  pinMode(downButton, INPUT);
  pinMode(selectButton, INPUT);

  lcd.init();
  lcd.clear();
  lcd.backlight();

  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    lcd.setCursor(0, 0);
    lcd.print("Card Mount Failed");
  } else if (SD.begin()) {
    Serial.println("Card Mounted");
    lcd.setCursor(0, 0);
    lcd.print("SD Card Mounted");
  }

  Serial.println("Try Connect to WiFi");

  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);
  wifiMulti.addAP(ssid_d, password_d);
  wifiMulti.addAP(ssid_it, password_it);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  } else {
    Serial.println("STA OKE");
  }

  wifiMulti.run();

  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  rtc.begin();

  if (wifiMulti.run() == WL_CONNECTED) {
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 2);
    lcd.print("Loading Date/Time");
    getLocalTime();
  } else {
    lcd.setCursor(0, 1);
    lcd.print("WiFi Disconnected");
    lcd.setCursor(0, 2);
    lcd.print("Date/Time Error");
  }
  now = rtc.now();

  if (ntpStatus == true) {
    updateRTC();
    if (statusUpdateRTC == true) {
      rtcYear = now.year();
      rtcMonth = now.month();
      rtcDay = now.day();
      rtcHour = now.hour();
      rtcMinute = now.minute();
      rtcSecond = now.second();
    }
  }

  lcd.clear();
  Serial.println("Select Menu");
  Serial.println(WiFi.localIP());

  selectMenu();

  productSelected = productCodeOne;
  nameProductSelected = nameProductOne;
  
  logName = "/logCounter_" + productSelected + ".txt";

  if (SD.begin()) {
    int tryUpdateStatusSD = 0;
    while (readStatusSD == false && tryUpdateStatusSD <= 5) {
      readLastLineSDCard(logName);
      counter = counterSD.toInt();
      counterReject = counterRejectSD.toInt();
      tryUpdateStatusSD++;
    }

    if (readStatusSD == false) {
      File myFile = SD.open(logName, FILE_WRITE);
      myFile.println("0,0,0,0");
      myFile.close();
    }

  } else {
    lcd.setCursor(0, 3);
    lcd.print("SD Card Failed");
    delay(5000);
  }

  xTaskCreatePinnedToCore(
    counterHit,
    "Task1",
    10000,
    NULL,
    0,
    &Task1,
    0
  );
}

void loop() {
  lcd.setCursor(1, 2);
  lcd.print("Total : ");
  lcd.setCursor(9, 2);
  lcd.print(counter);
  // lcd.setCursor(11, 2);
  // lcd.print(" | ");
  // lcd.setCursor(14, 2);
  // lcd.print(counterReject);

  lcd.setCursor(1, 0);
  lcd.print(productSelected);
  lcd.setCursor(1, 1);
  lcd.print(nameProductSelected);

  if (wifiMulti.run() == WL_CONNECTED) {
    lcd.setCursor(1, 3);
    lcd.print(WiFi.SSID());
    getLocalTime();
  } else if (wifiMulti.run() != WL_CONNECTED) {
    lcd.setCursor(1, 3);
    lcd.print("WiFi DC");
    wifiMulti.run();
  }

  now = rtc.now();
  if (ntpStatus == true) {
    updateRTC();
    if (statusUpdateRTC == true) {
      rtcYear = now.year();
      rtcMonth = now.month();
      rtcDay = now.day();
      rtcHour = now.hour();
      rtcMinute = now.minute();
      rtcSecond = now.second();
    }
  } else if (ntpStatus == false) {
    rtcYear = now.year();
    rtcMonth = now.month();
    rtcDay = now.day();
    rtcHour = now.hour();
    rtcMinute = now.minute();
    rtcSecond = now.second();
    dateFormat = String(rtcYear) + '-' + String(rtcMonth) + '-' + String(rtcDay);
    timeFormat = String(rtcHour) + ':' + String(rtcMinute) + ':' + String(rtcSecond);
    dateTime = dateFormat + ' ' + timeFormat;
  }

  lcd.setCursor(12, 3);
  lcd.print(timeFormat);

  ip_Address = WiFi.localIP().toString();

  logData = productSelected + ',' + String(counter) + ',' + String(counterReject) + ',' + dateTime + ',' + ip_Address;
  newCounter = counter;
  if (oldCounter != newCounter) {
    insertLastLineSDCard(logName, logData);
    oldCounter = newCounter;
  }

  if ((second == 30 || second == 0) && !sendStatus) {
    lcd.setCursor(7, 3);
    lcd.print("SDB");
    if (!SD.begin()) {
      postData = "kode_product=" + productSelected + "&counter=" + String(counter) + "&rejector=" + String(counterReject) + "&date=" + dateTime + "&ip_address=" + ip_Address;
    } else if (SD.begin()) {
      readLastLineSDCard(logName);
      postData = "kode_product=" + productSelectedSD + "&counter=" + counterSD + "&rejector=" + counterRejectSD +"&date=" + dateTimeSD + "&ip_address=" + ipAddressSD;
    }
    sendLogData();
    sendStatus = true;
    lcd.clear();
  } else if ((second == 20 || second == 48) && sendStatus) {
    sendStatus = false;
  }

  resetESP();
}