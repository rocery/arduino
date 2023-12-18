#include <WiFi.h>
#include <WiFiMulti.h>
#include "time.h"
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <LiquidCrystal_I2C.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <RTClib.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define upButton 26
#define downButton 32
#define selectButton 35
#define resetButton 34

#define irPin 25

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
const char* ssid_it = "STTB5";
const char* password_it = "siantar123";

// Set IP to Static
IPAddress staticIP(192, 168, 15, 218);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional
String ip_Address;

// == Get NTP/RTC ==
const char* ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 7 * 3600;  // GMT +7
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat;
int year;
int month;
int day;
int hour;
int minute;
int second;
RTC_DS3231 rtc;
DateTime now;
bool ntpStatus;

// == Counter ==
int counter;
TaskHandle_t Task1;
int newCounter;
int oldCounter;

// == Data Send/Get ==
bool sendStatus;
bool getStatus;
const char* counterFromDB;
int counterValueDB;
String postData;

// == Product/Menu Related Section ==
bool menuSelect;
int menu = 1;
String productSelected;
String nameProductSelected;
String productCodeOne = "P-0722-00239";
String productCodeTwo = "2";
String productCodeThree = "3";
String productCodeFour = "4";
String nameProductOne = "Tic Tic Bwg 2000";
String nameProductTwo = "Tic Tic Bwg SMBL";
String nameProductThree = "FF 10000";
String nameProductFour = "d";

// == SD Card ==
String line;
String logName;
String logData;
int lineAsInt;
String dateTimeSD, productSelectedSD, counterSD, ipAddressSD;
bool statusSD, readStatusSD, insertLastLineSDCardStatus;

void counterHit(void* parameter) {
  for (;;) {
    pinMode(irPin, INPUT_PULLUP);
    static int lastIRState = HIGH;
    // Membaca status tombol
    int irState = digitalRead(irPin);
    if (irState == LOW && lastIRState == HIGH) {
      counter++;
    }
    lastIRState = irState;
    //    Serial.print("Counter : ");
    //    Serial.println(counter);
  }
}

void selectMenu() {
  menu = 1;
  updateMenu();
  while (menuSelect == false) {
    if (digitalRead(downButton)) {
      menu++;
      updateMenu();
      delay(100);
      while (digitalRead(downButton));
    }
    if (digitalRead(upButton)) {
      menu--;
      updateMenu();
      delay(100);
      while (digitalRead(upButton));
    }
    if (digitalRead(selectButton)) {
      delay(100);
      while (digitalRead(selectButton));

      menuSelect = true;
      menuSelected();
    }
  }
  lcd.setCursor(1, 0);
  lcd.print(productSelected);
  lcd.setCursor(1, 1);
  lcd.print(nameProductSelected);
  //  if (menuSelect == true && getStatus == false) {
  //    lcd.setCursor(1, 0);
  //    lcd.print(productSelected);
  //    lcd.setCursor(1, 1);
  //    lcd.print(nameProductSelected);
  //
  //    //Get SD Card Data
  //    readLastLineSDCard(logName);
  //    counter = counterSD.toInt();
  //    getStatus = true;
  //  } else if (menuSelect == true && getStatus == true) {
  //    lcd.setCursor(1, 0);
  //    lcd.print(productSelected);
  //    lcd.setCursor(1, 1);
  //    lcd.print(nameProductSelected);
  //    lcd.setCursor(1, 2);
  //    lcd.print("Total : ");
  //    lcd.setCursor(9, 2);
  //    lcd.print(counter);
  //  }
}

void getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    ntpStatus = false;
  } else {
    ntpStatus = true;
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

  dateFormat = String(year) + '-' + String(month) + '-' + String(day);
  timeFormat = String(hour) + ':' + String(minute) + ':' + String(second);
}

void sendLogData() {
  HTTPClient http;
  http.begin("http://192.168.15.221/counter_hit_api/saveCounter.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void getLogData() {
  HTTPClient http;
  String getData = "http://192.168.15.221/counter_hit_api/getDataLastCounter.php?kode_product=" + productSelected;

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
  if (digitalRead(resetButton)) {
    lcd.clear();
    // Send Data to SD
    insertLastLineSDCard(logName, logData);
    lcd.setCursor(1, 0);
    lcd.print("Loading...");
    delay(1000);

    // Get Data from SD
    readLastLineSDCard(logName);
    postData = "kode_product=" + productSelectedSD + "&counter=" + counterSD + "&date=" + dateTimeSD + "&ip_address=" + ipAddressSD;
    delay(1000);
    
    // Send Data to DB
    sendLogData();
    lcd.setCursor(1, 1);
    lcd.print("Send Update to DB");
    delay(1000);
    
    // Delete Data from SD
    lcd.setCursor(1, 2);
    lcd.print("Deleting Log in SD");
    delay(1000);
    deleteLog(logName);
    
    lcd.setCursor(1, 3);
    lcd.print("Resetting...");
    delay(2000);
    
    ESP.restart();
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
    case 4:
      productSelected = productCodeFour;
      nameProductSelected = nameProductFour;
      delay(1000);
      lcd.clear();
      break;
  }
}

void updateMenu() {
  switch (menu) {
    case 0:
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Pilih Menu :");
      lcd.setCursor(1, 1);
      lcd.print("> " + nameProductOne);
      lcd.setCursor(1, 2);
      lcd.print(nameProductTwo);
      lcd.setCursor(1, 3);
      lcd.print(nameProductThree);
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Pilih Menu :");
      lcd.setCursor(1, 1);
      lcd.print("> " + nameProductOne);
      lcd.setCursor(1, 2);
      lcd.print(nameProductTwo);
      lcd.setCursor(1, 3);
      lcd.print(nameProductThree);
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Pilih Menu :");
      lcd.setCursor(1, 1);
      lcd.print(nameProductOne);
      lcd.setCursor(1, 2);
      lcd.print("> " + nameProductTwo);
      lcd.setCursor(1, 3);
      lcd.print(nameProductThree);
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Pilih Menu :");
      lcd.setCursor(1, 1);
      lcd.print(nameProductOne);
      lcd.setCursor(1, 2);
      lcd.print(nameProductTwo);
      lcd.setCursor(1, 3);
      lcd.print("> " + nameProductThree);
      break;
    case 4:
      menu = 3;
      break;
  }
}

void readLastLineSDCard(String path) {
  File file = SD.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("Failed to open file for reading");
    readStatusSD = false;
    return;
  } else if (file || file.isDirectory()) {
    readStatusSD = true;
  }

  while (file.available()) {
    line = file.readStringUntil('\n');
  }
  file.close();
  Serial.print("Data dari SD : ");
  Serial.println(line);
  int firstCommaIndex = line.indexOf(',');
  productSelectedSD = line.substring(0, firstCommaIndex);

  line = line.substring(firstCommaIndex + 1);
  int secondCommaIndex = line.indexOf(',');
  counterSD = line.substring(0, secondCommaIndex);

  line = line.substring(secondCommaIndex + 1);
  int thirdCommaIndex = line.indexOf(',');
  dateTimeSD = line.substring(0, thirdCommaIndex);

  ipAddressSD = line.substring(thirdCommaIndex + 1);
}

void insertLastLineSDCard(String path, String line) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    insertLastLineSDCardStatus = false;
    return;
  } else if (file) {
    insertLastLineSDCardStatus = true;
  }

  if (file.println(line)) {
    Serial.println("Line written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
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

  pinMode(upButton, INPUT);
  pinMode(downButton, INPUT);
  pinMode(selectButton, INPUT);
  pinMode(resetButton, INPUT);

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // SD Card
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  } else if (SD.begin()) {
    Serial.println("Card Mounted");
  }

  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);
  wifiMulti.addAP(ssid_d, password_d);
  wifiMulti.addAP(ssid_it, password_it);
  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  wifiMulti.run();
  delay(1000);
  // NTP-RTC
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  rtc.begin();
  getLocalTime();
  delay(1000);

  selectMenu();

  xTaskCreatePinnedToCore(
    counterHit, /* Fungsi untuk mengimplementasikan tugas */
    "Task1",   /* Nama tugas */
    10000,     /* Ukuran stack dalam kata */
    NULL,      /* Parameter input tugas */
    0,         /* Prioritas tugas */
    &Task1,    /* Handle tugas. */
    0          /* Core tempat tugas harus dijalankan */
  );


}

void loop() {
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    lcd.setCursor(1, 3);
    lcd.print("WiFi DC");
    delay(1000);
    wifiMulti.run();
  } else {
    int tryNTP = 0;
    while (ntpStatus == false && tryNTP <= 20) {
      getLocalTime();
      tryNTP++;
      delay(50);
    }

    getLocalTime();
    ip_Address = WiFi.localIP().toString();
    now = rtc.now();
    logName = "/logCounter_" + productSelected + '_' + dateFormat + ".txt";

    if (insertLastLineSDCardStatus == false) {
      File myFile = SD.open(logName, FILE_WRITE);
      myFile.println("0,0,0,0");
      myFile.close();
    }

    int tryUpdateStatusSD = 0;
    while (readStatusSD == false && tryUpdateStatusSD <= 20) {
      readLastLineSDCard(logName);
      counter = counterSD.toInt();
      tryUpdateStatusSD++;
    }

    lcd.setCursor(1, 2);
    lcd.print("Total : ");
    lcd.setCursor(9, 2);
    lcd.print(counter);
    lcd.setCursor(12, 3);
    lcd.print(timeFormat);

    // Get Data Here
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

    // Save Data Here
    logData = productSelected + ',' + String(counter) + ',' + dateTime + ',' + ip_Address;
    newCounter = counter;
    if (oldCounter != newCounter) {
      insertLastLineSDCard(logName, logData);
      oldCounter = newCounter;
    }

    

    if ((second == 15 || second == 30 || second == 45 || second == 0) && !sendStatus) {
      //      Serial.println("WiFi connected!");
      //      Serial.println("IP address: ");
      //      Serial.println(WiFi.localIP());
      //      Serial.println("Connected to SSID: " + WiFi.SSID());
      //      Serial.println(dateTime);
      //      Serial.println("Detik: " + String(second));
      //      Serial.println("Counter: " + String(counter));
      //      sendStatus = true;
      //      getLogData();
      //      Serial.println("Counter DB: " + String(counterValueDB));

      if (!SD.begin()) {
        postData = "kode_product=" + productSelected + "&counter=" + String(counter) + "&date=" + dateTime + "&ip_address=" + ip_Address;
      } else if (SD.begin()) {
        readLastLineSDCard(logName);
        postData = "kode_product=" + productSelectedSD + "&counter=" + counterSD + "&date=" + dateTimeSD + "&ip_address=" + ipAddressSD;
        lcd.setCursor(9, 3);
        lcd.print("SD");
      }

      sendLogData();
      sendStatus = true;
    } else if ((second == 10 || second == 25 || second == 40 || second == 55) && sendStatus) {
      sendStatus = false;
    }
    lcd.setCursor(1, 3);
    lcd.print("WiFi OK");
  }
//  if ((hour == 7 && minute >= 30) || (hour == 8 && minute <= 30)) {
//    resetESP();
//  }
  resetESP();
}