/*
  V. 1.0.5
  Update Terakhir : 15-04-2024

  PENTING = Harus menggunakan Dual Core Micro Controller
  Komponen:
  1. Micro Controller : ESP32
  2. LCD I2C 20x4                               (3.3v, GND, I2C (22 = SCL, 21 = SDA))
  3. DS3231                                     (3.3v, GND, I2C (22 = SCL, 21 = SDA))
  4. IR Sensor E18-D80NK                        (5v/Vin, GND, 26) -> 5V Brown/Red, GND Blue, Out Black
  5. Module SD Card + SD Card (FAT32 1-16 GB)   (3.3v, GND, SPI(Mosi 23, Miso 19, CLK 18, CS 5))
  6. Tacticle Button 1x1 cm @3                  (3.3v, GND, 34, 35, 25)

  Program ini berfungsi untuk melakukan penghitungan barang pada conveyor.
  Penjelasan program terdapat pada comment baris pada program.

  Semua fungsi Serial.print() pada program ini sebenarnya bisa dihapus/di-comment,
  tapi bisa digunakan untuk debuging. Akan di-comment/dihapus pada saat final
  program sudah tercapai demi menghemat rosource pada ESP32.

  Module RTC pada program ini beleum digunakan, program ini masih memanfaatkan waktu dari
  server NTP, direkomendasikan menggunakan RTC bilamana terjadi gangguan WiFi.
*/

// == Deklarasi semua Library yang digunakan ==
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

String ESPName = "Ctr-Biskuit";

/* Mendeklarasikan LCD dengan alamat I2C 0x27
   Total kolom 20
   Total baris 4 */
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Terdapat 3 tombol pada project ini, up, down, select
// Tidak disarankan menggunakan pin 32 dan 33 (Terdapat Bug), bisa digunakan pin 27
#define upButton 35
#define downButton 34
#define selectButton 32

#define irPin 25

// == WiFi Config ==
/* Deklarasikan semua WiFi yang bisa diakses oleh ESP32
ESP32 akan memilih WiFi dengan sinyal paling kuat secara otomatis
*/
WiFiMulti wifiMulti;
const char* ssid_d = "STTB1";
const char* password_d = "Si4nt4r321";
const char* ssid_b = "MT4";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
const char* ssid_a = "STTB8";
const char* password_a = "siantar123";
const char* ssid_it = "STTB11";
const char* password_it = "Si4nt4r321";

// Set IP to Static
IPAddress staticIP(192, 168, 7, 218);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional
String ip_Address;

// == Get NTP/RTC ==
const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;  // Karena Bekasi ada di GMT+7, maka Offset ditambah 7 jam
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

// == Counter ==
TaskHandle_t Task1;
int counter, newCounter, oldCounter;

// == Data Send/Get ==
bool sendStatus, getStatus;
const char* counterFromDB;
int counterValueDB;
String postData;

// == Product/Menu Related Section ==
// Add Menu Here
bool menuSelect;
int menu = 1;
String productSelected, nameProductSelected;
String productCodeOne = "P-0722-00250", nameProductOne = "IN GORIORIO VAN 2";
String productCodeTwo = "P-0722-00251", nameProductTwo = "IN GORIORIO COK 2";
String productCodeThree = "P-1023-00282", nameProductThree = "NO GORIORIO VAN 2";
String productCodeFour = "P-1023-00283", nameProductFour = "NO GORIORIO COK 2";
String productCodeFive = "P-1122-00260", nameProductFive = "NO GORIORIO VAN 4";
String productCodeSix = "P-1222-00263", nameProductSix = "NO GORIORIO COK 4";
String productCodeSeven = "Test_Mode_218", nameProductSeven = "Test_Mode_218";

// == SD Card ==
String line, logName, logData;
int lineAsInt;
String dateTimeSD, productSelectedSD, counterSD, ipAddressSD;
bool statusSD, readStatusSD, insertLastLineSDCardStatus;

void counterHit(void* parameter) {
  for (;;) {
    // Deklarasi mode pin sensor
    pinMode(irPin, INPUT_PULLUP);
    static int lastIRState = HIGH;
    // Membaca output Sensor
    int irState = digitalRead(irPin);
    if (irState == LOW && lastIRState == HIGH) {
      counter++;
    }
    lastIRState = irState;
    delay(50);
  }
}

void getLocalTime() {
  /* Fungsi bertujuan menerima update waktu
     lokal dari ntp.pool.org */
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
  /* Baris program ini digunakan untuk melakukan update waktu pada RTC,
    akan digunakan pada final produk untuk menanggulangi masalah pada
    koneksi NTP jika WiFi tidak terkoneksi internet.
    Mohon tidak dihapus/dirubah.
  */
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.adjust(DateTime(year, month, day, hour, minute, second));

  if (now.year() != 1990) {
    statusUpdateRTC = true;
  } else if (now.year() == 1990) {
    statusUpdateRTC = false;
  }
}

void sendLogData() {
  /* Mengirim data ke local server
     Ganti isi variabel api sesuai dengan form php
  */
  String api = "http://192.168.7.223/counter_hit_api/saveCounter.php";
  HTTPClient http;
  http.begin(api);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST");
  }
  http.end();
}

int getLogData(String product) {
  /* Untung mendapatkan data terakhir dari DB, 
  saat ini tidak digunakan karena sudah menggunakan SD Card
  Kode dibawah mohon untuk tidak dihapus.
  */
  HTTPClient http;
  String getData = "http://192.168.7.223/counter_hit_api/getDataLastCounter.php?kode_product=" + product;
  http.begin(getData);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    JSONVar myArray = JSON.parse(payload);
    counterFromDB = myArray["counter"];
    counterValueDB = atoi(counterFromDB);
    return counterValueDB;
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
    if (!statusSD) {
      // Jika gagal membaca SD Card, maka data yang dikirim adalah data real time
      // Jika alat mati, maka data tidak disimpan
      postData = "kode_product=" + productSelected + "&counter=" + String(counter) + "&date=" + dateTime + "&ip_address=" + ip_Address;
    } else {
      readLastLineSDCard(logName);
      postData = "kode_product=" + productSelectedSD + "&counter=" + counterSD + "&date=" + dateTimeSD + "&ip_address=" + ipAddressSD;
    }
    sendLogData();
    lcd.setCursor(1, 1);
    lcd.print("Send Update to DB");

    // Send Reset Status to DB
    String data = ESPName + "-" + productSelected;
    postData = "kode_product=" + data + "&counter=" + String(counter) + "&date=" + dateTime + "&ip_address=" + ip_Address;
    sendLogData();
    delay(1000);

    postData = "kode_product=" + ESPName + "&counter=" + String(0) + "&date=" + dateTime + "&ip_address=" + ip_Address;
    sendLogData();

    // Delete Data from SD
    lcd.setCursor(1, 2);
    lcd.print("Deleting Log in SD");
    deleteLog(logName);

    lcd.setCursor(1, 3);
    lcd.print("Resetting...");

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
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("==PILIH PRODUK==");
      lcd.setCursor(0, 1);
      lcd.print(">" + nameProductFour);
      lcd.setCursor(1, 2);
      lcd.print(nameProductFive);
      lcd.setCursor(1, 3);
      lcd.print(nameProductSix);
      break;
    case 5:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("==PILIH PRODUK==");
      lcd.setCursor(1, 1);
      lcd.print(nameProductFour);
      lcd.setCursor(0, 2);
      lcd.print(">" + nameProductFive);
      lcd.setCursor(1, 3);
      lcd.print(nameProductSix);
      break;
    case 6:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("==PILIH PRODUK==");
      lcd.setCursor(1, 1);
      lcd.print(nameProductFour);
      lcd.setCursor(1, 2);
      lcd.print(nameProductFive);
      lcd.setCursor(0, 3);
      lcd.print(">" + nameProductSix);
      break;
    case 7:
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("==PILIH PRODUK==");
      lcd.setCursor(1, 1);
      lcd.print(nameProductFive);
      lcd.setCursor(1, 2);
      lcd.print(nameProductSix);
      lcd.setCursor(0, 3);
      lcd.print(">" + nameProductSeven);
      break;
    case 8:
      menu = 7;
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
    case 4:
      productSelected = productCodeFour;
      nameProductSelected = nameProductFour;
      delay(1000);
      lcd.clear();
      break;
    case 5:
      productSelected = productCodeFive;
      nameProductSelected = nameProductFive;
      delay(1000);
      lcd.clear();
      break;
    case 6:
      productSelected = productCodeSix;
      nameProductSelected = nameProductSix;
      delay(1000);
      lcd.clear();
      break;
    case 7:
      productSelected = productCodeSeven;
      nameProductSelected = nameProductSeven;
      delay(1000);
      lcd.clear();
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
  statusUpdateRTC = false;
  statusSD = false;

  pinMode(upButton, INPUT);
  pinMode(downButton, INPUT);
  pinMode(selectButton, INPUT);

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // SD Card
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    lcd.setCursor(0, 0);
    lcd.print("Card Mount Failed");
  } else if (SD.begin()) {
    Serial.println("Card Mounted");
    lcd.setCursor(0, 0);
    lcd.print("SD Card Mounted");
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

  // NTP-RTC
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  rtc.begin();

  if (wifiMulti.run() == WL_CONNECTED) {
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected");

    int tryNTP = 0;
    while (ntpStatus == false && tryNTP <= 5) {
      getLocalTime();
      tryNTP++;
      delay(50);
      lcd.setCursor(0, 2);
      lcd.print("Getting Date/Time");
    }

  } else {
    lcd.setCursor(0, 1);
    lcd.print("WiFi Disconnected");
    lcd.setCursor(0, 2);
    lcd.print("Date/Time Error");
    delay(5000);
  }
  now = rtc.now();

  // Adjust RTC berdasarkan NTP
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

  Serial.println(WiFi.localIP().toString());
  lcd.clear();   // Clear LCD sebelum memilih menu
  selectMenu();  // Tampilkan pilihan product yang bisa dipilih
  logName = "/logCounter_" + productSelected + ".txt";

  if (SD.begin()) {
    // Cek Last Counter
    // Saat pertama kali alat dihidupkan, cek counter terakhir pada File Log, gunakan log itu
    int tryUpdateStatusSD = 0;
    while (readStatusSD == false && tryUpdateStatusSD <= 5) {
      readLastLineSDCard(logName);
      counter = counterSD.toInt();
      tryUpdateStatusSD++;
    }

    // Jika Cek Last Counter, Gagal Buat File Baru
    if (readStatusSD == false) {
      File myFile = SD.open(logName, FILE_WRITE);
      myFile.println("0,0,0,0");
      myFile.close();
    }

    statusSD = true;

  } else {
    lcd.setCursor(0, 3);
    lcd.print("SD Card Failed");
    delay(1000);
    statusSD = false;
  }

  xTaskCreatePinnedToCore(
    counterHit, /* Fungsi untuk mengimplementasikan tugas */
    "Task1",    /* Nama tugas */
    10000,      /* Ukuran stack dalam kata */
    NULL,       /* Parameter input tugas */
    0,          /* Prioritas tugas */
    &Task1,     /* Handle tugas. */
    0           /* Core tempat tugas harus dijalankan */
  );

  lcd.clear();
}

void loop() {
  // Print Counter Hit
  lcd.setCursor(1, 2);
  lcd.print("TOTAL : ");
  lcd.setCursor(9, 2);
  lcd.print(counter);

  // Print Produk
  lcd.setCursor(1, 0);
  lcd.print(productSelected);
  lcd.setCursor(1, 1);
  lcd.print(nameProductSelected);

  /* Jika WiFi status WiFi tidak terkoneksi,
     coba ulang koneksi
  */
  if (wifiMulti.run() == WL_CONNECTED) {
    lcd.setCursor(1, 3);
    lcd.print(WiFi.SSID());
    getLocalTime();
  } else if (wifiMulti.run() != WL_CONNECTED) {
    lcd.setCursor(1, 3);
    lcd.print("WiFi DC");
    wifiMulti.run();
  }

  // Set RTC berdasarkan NTP
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
    // Jika NTP gagal (Tidak ada koneksi WiFi)
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

  // Print Waktu hh:mm:ss
  lcd.setCursor(12, 3);
  lcd.print(timeFormat);

  ip_Address = WiFi.localIP().toString();

  /* Untung mendapatkan data terakhir dari DB, 
    saat ini tidak digunakan karena sudah menggunakan SD Card
    Kode dibawah mohon untuk tidak dihapus.
    */
  // Get Data Here
  if (counter == 0 && statusSD == false) {
    int a = getLogData(productSelected);
    String check = ESPName + "-" + productSelected;
    int b = getLogData(check);
    if (a != 0 && a == b) {
      counter = 0;
    }
    else {
      counter = a;
    }
  }

  /* Jika dibutuhkan, baris program dibawah memungkinkan otomatis reset nilai counter
    Jika ingin mereset alat, panggil fungsi ResetESP().
    */
  // if ((hour == 7 && minute == 50 && second == 0) || (hour == 19 && minute == 50 && second == 0)) {
  //   counter = 0;
  //   delay(1000);
  //   sendLogData();
  //   delay(1000);
  //   lcd.clear();
  //   delay(100);
  // }

  /* Simpan data SD
      Data akan disimpan setiap kali nilai counter bertambah
    */
  logData = productSelected + ',' + String(counter) + ',' + dateTime + ',' + ip_Address;
  newCounter = counter;
  if (oldCounter != newCounter) {
    insertLastLineSDCard(logName, logData);
    oldCounter = newCounter;
  }

  /* Pengiriman data ke DB dilakukan setiap 15 detik
      Jika dirasa terlalu sering, ganti value second sesuai keperluan
      Misal, if ((second == 0 || second == 30) && !sendStatus) --> Akan mengirim data setiap detik 0 dan 30
    */
  if ((second == 30 || second == 0) && !sendStatus) {
    lcd.setCursor(7, 3);
    lcd.print("SDB");
    if (!SD.begin()) {
      // Jika gagal membaca SD Card, maka data yang dikirim adalah data real time
      // Jika alat mati, maka data tidak disimpan
      postData = "kode_product=" + productSelected + "&counter=" + String(counter) + "&date=" + dateTime + "&ip_address=" + ip_Address;
    } else if (SD.begin()) {
      readLastLineSDCard(logName);
      postData = "kode_product=" + productSelectedSD + "&counter=" + counterSD + "&date=" + dateTimeSD + "&ip_address=" + ipAddressSD;
    }
    sendLogData();
    sendStatus = true;
    lcd.clear();
  } else if ((second == 20 || second == 48) && sendStatus) {
    sendStatus = false;
  }

  resetESP();
}
