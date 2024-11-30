#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <esp_system.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

const int LOADCELL_DOUT_PIN = 26;
const int LOADCELL_SCK_PIN = 27;
HX711 scale;

float calibrationFactor;
int digitScale;
String productSelected;

// ========= INISIALISASI AWAL =========
/**/ const int ip = 31;
/**/ const String loc = "Giling Gula";
/**/ const String prod = "Biskuit";
// =====================================

const String api = "http://192.168.7.223/iot/api/weigher/save_weigher.php";
String ESPName = "Weigher | " + loc;
String deviceID = "IoT-" + String(ip);

int values[] = { 1, 2, 5, 10, 20, 40 };
int digit[] = { 0, 1, 2, 3, 4 };
String product[] = { "Product 1", "Product 2" };
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

WiFiMulti wifiMulti;
const char* ssid_a_biskuit_mie = "STTB8";
const char* password_a_biskuit_mie = "siantar321";
const char* ssid_b_biskuit_mie = "STTB1";
const char* password_b_biskuit_mie = "Si4nt4r321";
const char* ssid_c_biskuit_mie = "MT3";
const char* password_c_biskuit_mie = "siantar321";
const char* ssid_a_kerupuk = "STTB4";
const char* password_a_kerupuk = "siantar123";
const char* ssid_b_kerupuk = "Amano2";
const char* password_b_kerupuk = "Si4nt4r321";
const char* ssid_c_kerupuk = "MT1";
const char* password_c_kerupuk = "siantar321";
const char* ssid_it = "STTB11";
const char* password_it = "Si4nt4r321";

// Set IP to Static
IPAddress staticIP(192, 168, 7, ip);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
String ip_Address, postData;

// ===== NTP =====
const char* ntpServer = "192.168.7.223";
const long gmtOffsetSec = 7 * 3600;
const int daylightOffsetSec = 0;
String dateTime, dateFormat, timeFormat, lcdFormat;
int year, month, day, hour, minute, second;
bool ntpStatus, getStatus;

LiquidCrystal_I2C lcd(0x27, 16, 4);

const int EEPROM_ADDRESS = 0;

#define buttonUp 34
#define buttonDown 35
#define buttonSelect 32
int buttonUpState = 0;
int buttonDownState = 0;
int buttonSelectState = 0;

float readFloatFromEEPROM(int address) {
  float value = EEPROM.get(address, value);
  return value;
}

// updateFloatInEEPROM(EEPROM_ADDRESS, newValue);
void updateFloatInEEPROM(int address, float newValue) {
  float currentValue = readFloatFromEEPROM(address);

  if (currentValue != newValue) {
    EEPROM.put(address, newValue);
    EEPROM.commit();
    Serial.println("Value updated in EEPROM.");
  } else {
    Serial.println("Value is already up-to-date, no write needed.");
  }
}

void setup() {
  Serial.begin(9600);

  // Setup button pins
  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);
  pinMode(buttonSelect, INPUT);

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // EEPROM
  EEPROM.begin(512);

  // HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Calibration
  if (isButtonPressed(buttonUp) && isButtonPressed(buttonDown)) {
    while (isButtonPressed(buttonUp) && isButtonPressed(buttonDown)) {
      lcd.setCursor(0, 0);
      lcd.print("   KALIBRASI   ");
    }
    calibrationProcess();
  }

  // WiFi
  lcd.setCursor(0, 0);
  lcd.print("Connecting..");
  if (prod == "Biskuit" || prod == "Mie") {
    wifiMulti.addAP(ssid_a_biskuit_mie, password_a_biskuit_mie);
    wifiMulti.addAP(ssid_b_biskuit_mie, password_b_biskuit_mie);
    wifiMulti.addAP(ssid_c_biskuit_mie, password_c_biskuit_mie);
  } else if (prod == "Kerupuk") {
    wifiMulti.addAP(ssid_a_kerupuk, password_a_kerupuk);
    wifiMulti.addAP(ssid_b_kerupuk, password_b_kerupuk);
    wifiMulti.addAP(ssid_c_kerupuk, password_c_kerupuk);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Error, Call IT");
  }
  wifiMulti.addAP(ssid_it, password_it);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    lcd.setCursor(0, 0);
    lcd.print("WiFi STA Fail");
  }
  wifiMulti.run();

  // NTP
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  if (wifiMulti.run() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Fail");
    lcd.setCursor(0, 1);
    lcd.print("Date/Time Error");
    delay(2000);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");

    int tryNTP = 0;
    while (ntpStatus == false && tryNTP <= 2) {
      lcd.setCursor(0, 1);
      lcd.print("Getting Time");
      getLocalTime();
      tryNTP++;
      delay(50);
    }
  }

  // Load calibration factor from EEPROM then tare
  calibrationFactor = readFloatFromEEPROM(EEPROM_ADDRESS);
  digitScale = readFloatFromEEPROM(EEPROM_ADDRESS + 4);
  scale.set_scale(calibrationFactor);
  scale.tare();

  // Choose Product
  chooseProduct();  


  lcd.clear();
}

void loop() {
  double rawLoadCell = scale.get_units(10);
  float kgLoadCell = rawLoadCell / 1000;
  float absValuekgLoadCell = fabs(kgLoadCell);

  char kgLoadCellPrint[10];
  dtostrf(absValuekgLoadCell, 5, digitScale, kgLoadCellPrint);

  lcd.setCursor(0, 0);
  lcd.print(productSelected);
  
  lcd.setCursor(0, 1);
  // lcd.print("Hasil : ");
  lcd.print(kgLoadCellPrint);
  lcd.print(" KG");
  // Serial.println(kgLoadCell, 2);

  if (isButtonPressed(buttonDown)) {
    while (isButtonPressed(buttonDown)) {
      lcd.setCursor(0, 0);
      lcd.print("      TARE      ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
    tareScale();
  }

  if (isButtonPressed(buttonSelect)) {
    while (isButtonPressed(buttonSelect)) {
      // dataSave();
    }
  }

  if (isButtonPressed(buttonUp)) {
    while (isButtonPressed(buttonUp)) {
      lcd.setCursor(0, 0);
      lcd.print("      INFO      ");
    }
  }
}

bool isButtonPressed(int buttonPin) {
  return digitalRead(buttonPin);
}

void tareScale() {
  scale.set_scale(calibrationFactor);
  scale.tare();
  lcd.clear();
}

void calibrationProcess() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  BERAT BARANG  ");
  
  int currentValueIndex = 0;
  int currentDigitIndex = 0;

  // Step 1: Select weight
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print("BERAT: ");
    lcd.print(values[currentValueIndex]);
    lcd.print(" KG");

    int buttonUpState = digitalRead(buttonUp);
    int buttonDownState = digitalRead(buttonDown);

    // If buttonUp is pressed
    if (buttonUpState == HIGH) {
      while (digitalRead(buttonUp) == HIGH)
        ;
      currentValueIndex = (currentValueIndex + 1) % ARRAY_LENGTH(values);
      delay(200);
    }

    // If buttonDown is pressed, move to the next step
    if (buttonDownState == HIGH) {
      while (digitalRead(buttonDown) == HIGH)
        ;
      break;
    }
  }

  // Step 2: Empty the scale
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("KOSONGKAN");
  lcd.setCursor(0, 1);
  lcd.print("TIMBANGAN");
  int timerStep2 = 3;
  for (int i = 0; i <= timerStep2; i++) {
    lcd.setCursor(12, 0);
    lcd.print(timerStep2 - i);
    // scale.set_scale(0);
    scale.tare();
    delay(600);
  }
  lcd.setCursor(11, 1);
  lcd.print("OKE");
  // Wait for user to empty the scale
  while (true) {
    int buttonDownState = digitalRead(buttonDown);

    // If buttonDown is pressed, move to the next step
    if (buttonDownState == HIGH) {
      while (digitalRead(buttonDown) == HIGH)
        ;
      break;
    }
  }

  // Step 3: Weigh the item
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" TIMBANG BARANG");
  lcd.setCursor(1, 1);
  lcd.print(values[currentValueIndex]);
  lcd.print(" KG");
  delay(2000);

  while (true) {
    // Tare the scale and get the reading
    long reading = scale.get_units(10);
    calibrationFactor = (float)reading / (values[currentValueIndex] * 1000);
    lcd.setCursor(10, 1);
    lcd.print(calibrationFactor);
    scale.power_down();
    delay(1000);
    scale.power_up();

    int buttonDownState = digitalRead(buttonDown);

    // If buttonDown is pressed, move to the next step
    if (buttonDownState == HIGH) {
      lcd.clear();
      while (digitalRead(buttonDown) == HIGH) {
        lcd.setCursor(0, 0);
        lcd.print(" DATA DISIMPAN ");
      }
      lcd.clear();
      break;
    }
  }

  // Step 4: Set Digit
  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("    SET DIGIT   ");
    lcd.setCursor(0, 1);
    lcd.print("DIGIT: ");
    lcd.print(digit[currentDigitIndex]);

    int buttonUpState = digitalRead(buttonUp);
    int buttonDownState = digitalRead(buttonDown);

    // If buttonUp is pressed
    if (buttonUpState == HIGH) {
      while (digitalRead(buttonUp) == HIGH)
        ;
      currentDigitIndex = (currentDigitIndex + 1) % ARRAY_LENGTH(digit);
      delay(200);
    }

    // If buttonDown is pressed, move to the next step
    if (buttonDownState == HIGH) {
      while (digitalRead(buttonDown) == HIGH)
        ;
      digitScale = digit[currentDigitIndex];
      break;
    }
  }

  // Step 5: Confirm calibration
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("KALIBRASI");
  lcd.print("  H <==");
  lcd.setCursor(0, 1);
  lcd.print("SELESAI");
  lcd.print("    M ==>");

  while (true) {
    int buttonSelectState = digitalRead(buttonSelect);
    int buttonDownState = digitalRead(buttonDown);

    // If buttonSelect is pressed, exit the calibration process
    if (buttonSelectState == HIGH) {
      while (digitalRead(buttonSelect) == HIGH)
        ;
      updateFloatInEEPROM(EEPROM_ADDRESS, calibrationFactor);
      updateFloatInEEPROM(EEPROM_ADDRESS + 4, digitScale);
      lcd.clear();
      return;
    }

    // If buttonDown is pressed, restart the calibration
    if (buttonDownState == HIGH) {
      while (digitalRead(buttonDown) == HIGH)
        ;
      calibrationProcess();
      break;
    }
  }
}

void chooseProduct() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  PILIH PRODUK ");

  int currentProductIndex = 0;

  // Step 1: Select weight
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print(product[currentProductIndex]);

    int buttonUpState = digitalRead(buttonUp);
    int buttonDownState = digitalRead(buttonDown);

    // If buttonUp is pressed
    if (buttonUpState == HIGH) {
      while (digitalRead(buttonUp) == HIGH)
        ;
      currentProductIndex = (currentProductIndex + 1) % ARRAY_LENGTH(product);
      delay(200);
    }

    // If buttonDown is pressed, move to the next step
    if (buttonDownState == HIGH) {
      while (digitalRead(buttonDown) == HIGH)
        ;
      productSelected = product[currentProductIndex];
      break;
    }
  }
}

void getLocalTime() {
  /* Fungsi bertujuan menerima update waktu
  lokal dari ntpServer */
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    ntpStatus = false;
  } else {
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
    // hh:mm
    lcdFormat = String(hour) + ':' + String(minute);

    ntpStatus = true;
  }
}