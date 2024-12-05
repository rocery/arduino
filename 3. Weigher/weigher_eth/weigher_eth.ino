#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <esp_system.h>
#include <UIPEthernet.h>
#include <SPI.h>


// ========= INISIALISASI AWAL =========
/**/ const int ip = 31;
/**/ const String loc = "Kerupuk 31";
/**/ const String prod = "Kerupuk";
// =====================================

String ESPName = "Weigher | " + loc;
String deviceID = "IoT-" + String(ip);
int sendDataCounter;

const char* serverName = "http://192.168.7.223/iot/api/weigher/save_weigher_test.php";
const char* serverAddress = "192.168.7.223";
const int serverPort = 80;
String ip_Address = "192.168.7." + ip;

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


// Konfigurasi jaringan statis
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Alamat MAC perangkat
IPAddress ip(192, 168, 7, ip);  // IP statis
IPAddress gateway(192, 168, 15, 250);  // Gateway
IPAddress subnet(255, 255, 0, 0);  // Subnet mask


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

    // Load calibration factor from EEPROM then tare
  calibrationFactor = readFloatFromEEPROM(EEPROM_ADDRESS);
  digitScale = readFloatFromEEPROM(EEPROM_ADDRESS + 4);
  scale.set_scale(calibrationFactor);
  scale.tare();

  // Inisialisasi Ethernet dengan IP statis
  Ethernet.begin(mac, ip, gateway, gateway, subnet);

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
    lcd.clear();
  }

  if (isButtonPressed(buttonSelect)) {
    lcd.clear();
    while (isButtonPressed(buttonSelect)) {
      ip_Address = WiFi.localIP().toString();
      postData = "device_id=" + deviceID + "&device_name=" + ESPName + "&product=" + productSelected + "&weight=" + kgLoadCellPrint + "&date=" + dateTime + "&ip_address=" + ip_Address + "&wifi=" + WiFi.SSID();
      lcd.setCursor(0, 0);
      lcd.print("  SENDING DATA  ");
    }

    if (!wiFiStatus) {
      lcd.setCursor(0, 1);
      lcd.print("X, WiFi ERROR");
      delay(5000);
    } else {
      if (!sendData()) {
        lcd.setCursor(0, 1);
        lcd.print("X, HUBUNGI IT");
        delay(5000);
      } else {
        lcd.setCursor(0, 1);
        lcd.print(" BERHASIL : ");
        sendDataCounter++;
        lcd.print(sendDataCounter);
        delay(1000);
      }
    }
    lcd.clear();
  }

  if (isButtonPressed(buttonUp)) {
    lcd.clear();
    while (isButtonPressed(buttonUp)) {
      lcd.setCursor(0, 0);
      lcd.print(WiFi.SSID());
      lcd.print("  ");
      lcd.print(ip);
      lcd.setCursor(0, 1);
      lcd.print(lcdFormat);
      lcd.print("  ");
      lcd.print(sendDataCounter);
    }
    lcd.clear();
  }
}