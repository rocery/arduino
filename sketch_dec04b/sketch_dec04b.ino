#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Arduino.h>

// Set the LCD address to 0x3F/0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Setup Wifi
const char* ssid_a = "STTB1";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "MT1";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
unsigned long lastConnectionAttempt = 0;  // Waktu terakhir koneksi dicoba
const unsigned long connectionAttemptInterval = 5000;  // Interval antara percobaan koneksi (10 menit)
// Konfigurasi IP statis
IPAddress staticIP(192, 168, 15, 218);  // Sesuaikan dengan IP yang diinginkan
IPAddress gateway(192, 168, 15, 250);      // Sesuaikan dengan gateway Anda
IPAddress subnet(255, 255, 255, 0);     // Sesuaikan dengan subnet Anda
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
// Get Data
String serverNameGet = "http://192.168.15.221/counter_hit_api/getDataLastCounter.php?kode_product=";
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

// RTC
RTC_DS3231 rtc;
DateTime now;
int dataSeconds = 0;
int dataMinutes = 0;
int dataHours = 0;
// Setting tanggal menjadi nama hari
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String formattedTime;

// Sensor IR
const int IRpin = 34;
int IRValue;
const int threshold = 3500;
int count = 0;
bool cond = false;

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
    Serial.println(IRValue);
//    int c = xPortGetCoreID();
//    Serial.println(c);
  }
}

void attemptConnection() {
  // Cek apakah sudah waktunya mencoba koneksi kembali
  if (millis() - lastConnectionAttempt >= connectionAttemptInterval) {
    // Cek apakah koneksi WiFi sudah terputus
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Mencoba koneksi...");
      
      // Coba terhubung ke WiFi A
      if (connectToWiFi(ssid_a, password_a)) {
        Serial.println("Terhubung ke WiFi A");
      } 
      // Jika tidak berhasil, coba WiFi B
      else if (connectToWiFi(ssid_b, password_b)) {
        Serial.println("Terhubung ke WiFi B");
      } 
      // Jika kedua-duanya tidak berhasil, coba WiFi C
      else if (connectToWiFi(ssid_c, password_c)) {
        Serial.println("Terhubung ke WiFi C");
      } 
      // Jika semuanya gagal
      else {
        Serial.println("Gagal terhubung ke semua WiFi");
      }

      // Set waktu terakhir percobaan koneksi
      lastConnectionAttempt = millis();
    } 
//    else {
//      Serial.println("WiFi sudah terhubung. Tidak perlu mencoba koneksi.");
//    }
  }
}

bool connectToWiFi(const char* ssid, const char* password) {
  Serial.print("Mencoba terhubung ke ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Mengatur konfigurasi IP statis dan DNS
  WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {  // Coba selama 30 detik
      // Tunggu sebentar sebelum memeriksa kembali
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nKoneksi berhasil!");
    return true;
  } else {
    Serial.println("\nGagal terhubung");
    return false;
  }
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
    0                 // Core number (1 for Core 1)
  );
  
  // LCD
  lcd.init();
  lcd.clear();         
  lcd.backlight();

  // Get NTP
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  rtc.begin();
  
  // Connect to WiFi
  attemptConnection();
}

void loop() {
  lcd.setCursor(1, 0);
  lcd.print(produk1);
  lcd.setCursor(1, 1);
  lcd.print(nama1);
  lcd.setCursor(11, 3);
//  lcd.print(dateTimeLCD);
  lcd.setCursor(1, 2);
  lcd.print("Total : ");
  lcd.setCursor(9, 2);
  lcd.print(count);

  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(1, 3);
    lcd.print("WiFi OK ");
      
  } else {
    lcd.setCursor(1, 3);
    lcd.print("WiFi Dis");
  }
  
  attemptConnection();
}
