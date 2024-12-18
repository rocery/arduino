#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <esp_system.h>
#include <Ethernet.h>
#include <SPI.h>
#include "FS.h"
#include "SD.h"

// ========= INISIALISASI AWAL =========
/**/ const int ip = 31;
/**/ const String loc = "Kerupuk 31";
/**/ const String prod = "Kerupuk";
// =====================================

// DEVICE INFO
String ESPName = "Weigher | " + loc;
String deviceID = "IoT-" + String(ip);
int sendDataCounter;
int sendDataCounterFailed;

// SERVER API
const char* serverAddress = "192.168.7.223";
const char* serverPath = "POST /iot/api/weigher/save_weigher.php HTTP/1.1";
const int serverPort = 80;
String ip_Address = "192.168.7." + String(ip);
String postData, lanStatus;

// LOAD CELL
const int LOADCELL_DOUT_PIN = 26;
const int LOADCELL_SCK_PIN = 27;
HX711 scale;
float calibrationFactor;
int digitScale;
String productSelected;
/* Deklarasi array
  @param values = Array Nilai
  @param digit = Array Digit
  @param product = Array Produk
*/
int values[] = { 1, 2, 5, 10, 20, 40 };
int digit[] = { 0, 1, 2, 3, 4 };
String product[] = {
  "TIC TIC CBP",
  "LEANET BBQ",
  "V-TOZ",
  "LEANET TIC2 BWG",
  "TIC TIC SAMCOL",
  "FUJI CHIPS",
  "TIC TIC BALADO"
};
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

// NETWORK STATIC CONFIGURATION
#define W5500_CS 5
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ipaddress(192, 168, 7, ip);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 4);

// EEPROM
const int EEPROM_ADDRESS = 0;

// BUTTON
#define buttonUp 34
#define buttonDown 35
#define buttonSelect 32
int buttonUpState = 0;
int buttonDownState = 0;
int buttonSelectState = 0;

// SD CARD
#define SD_CS 4
bool sdStatus = false;

float readFloatFromEEPROM(int address) {
  float value = EEPROM.get(address, value);
  return value;
}

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

bool isButtonPressed(int buttonPin) {
  return digitalRead(buttonPin);
}

void tareScale() {
  // scale.set_scale(calibrationFactor);
  scale.tare();
  lcd.clear();
}

bool sendData() {
  EthernetClient client;

  // Coba koneksi
  if (client.connect(serverAddress, serverPort)) {
    // Kirim HTTP POST Request
    client.println(serverPath);
    client.println("Host: 192.168.7.223");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);

    // Tunggu respon
    while(client.connected()) {
      if(client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }

    client.stop();
    return true;
  } else {
    Serial.println("Koneksi gagal");
    return false;
  }
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
  delay(1000);

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
      lcd.setCursor(0, 1);
      lcd.print("                ");
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

bool checkConnectionLan() {
  if (Ethernet.linkStatus() == LinkON) {
    return true;
  } else {
    return false;
  }
}

// void handleClientTask(void *parameter) {
//   while (true) {
//     EthernetClient client = selfServer.available();  // Periksa apakah ada klien
//     if (client) {
//       Serial.println("Klien terhubung");

//       // Kirim pesan ke klien
//       client.println("ESP32 dengan ENC28J60 berhasil terkoneksi!");
//       client.stop();  // Tutup koneksi
//     }
//     delay(10);  // Hindari penggunaan CPU berlebih
//   }
// }

void setup() {
  Serial.begin(9600);

  // Setup button pins
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonSelect, INPUT_PULLUP);

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

  // Load calibration factor from EEPROM then tare
  calibrationFactor = readFloatFromEEPROM(EEPROM_ADDRESS);
  digitScale = readFloatFromEEPROM(EEPROM_ADDRESS + 4);
  scale.set_scale(calibrationFactor);
  scale.tare();

  // Inisialisasi Ethernet dengan IP statis
  Ethernet.init(W5500_CS);
  Ethernet.begin(mac, ipaddress, gateway, gateway, subnet);

    Serial.println("Inisialisasi SD Card...");
    if (!SD.begin(SD_CS)) {   // Pin CS untuk SD Card
        Serial.println("SD Card gagal diinisialisasi.");
        sdStatus = false;
    } else {
        Serial.println("SD Card berhasil diinisialisasi!");
        sdStatus = true;
    }

  // Choose Product
  chooseProduct();

  lcd.clear();
}

void loop() {
  double rawLoadCell = scale.get_units(5);
  float kgLoadCell = rawLoadCell / 1000;
  float absValuekgLoadCell = fabs(kgLoadCell);

  char kgLoadCellPrint[6];
  dtostrf(absValuekgLoadCell, 5, digitScale, kgLoadCellPrint);

  lcd.setCursor(0, 0);
  lcd.print(productSelected);

  Ethernet.maintain();
  if (checkConnectionLan()) {
    lanStatus = "C";
  } else {
    lanStatus = "D";
  }

  lcd.setCursor(15, 1);
  lcd.print(lanStatus);
  lcd.setCursor(10, 1);
  lcd.print(sendDataCounter);
  lcd.setCursor(0, 1);
  lcd.print(kgLoadCellPrint);
  lcd.print(" KG");

  if (isButtonPressed(buttonDown)) {
    while (isButtonPressed(buttonDown)) {
      lcd.setCursor(0, 0);
      lcd.print("      TARE      ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
    tareScale();
    lcd.clear();
  }

  if (isButtonPressed(buttonSelect)) {
    lcd.clear();
    while (isButtonPressed(buttonSelect)) {
      postData = "device_id=" + deviceID + "&device_name=" + ESPName + "&product=" + productSelected + "&weight=" + String(kgLoadCellPrint) + "&ip_address=" + ip_Address + "&wifi=" + "LAN";
      lcd.setCursor(0, 0);
      lcd.print("  SENDING DATA  ");
    }

    if (lanStatus == " DC") {
      lcd.setCursor(0, 1);
      lcd.print("X, LAN ERROR");
      sendDataCounterFailed++;
      delay(1000);
    } else {
      if (!sendData()) {
        lcd.setCursor(0, 1);
        lcd.print("X, HUBUNGI IT");
        sendDataCounterFailed++;
        delay(1000);
      } else {
        lcd.setCursor(0, 1);
        lcd.print(" BERHASIL : ");
        sendDataCounter++;
        lcd.print(sendDataCounter);
        delay(100);
      }
    }
    lcd.clear();
  }

  if (isButtonPressed(buttonUp)) {
    lcd.clear();
    while (isButtonPressed(buttonUp)) {
      lcd.setCursor(0, 0);
      lcd.print("B : ");
      lcd.print(sendDataCounter);
      lcd.setCursor(0, 1);
      lcd.print("G : ");
      lcd.print(sendDataCounterFailed);
      lcd.setCursor(13, 1);
      lcd.print(lanStatus);
      lcd.print(sdStatus);
      lcd.setCursor(11, 0);
      lcd.print(deviceID);
    }

    lcd.clear();
  }
}
