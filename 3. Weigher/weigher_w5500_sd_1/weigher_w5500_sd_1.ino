/*
 * V 3.0.0
 * Update Terakhir : 11-01-2025
 * 
 * PENTING = Harus menggunakan Dual Core Micro Controller/Microprocessor
 * - API yang digunakan adalah FLask karena terjadi kendala pada peroses pengiriman file dari esp ke php
 *   github.com/rocery/iot.git
 * 
 * Hal-hal yang perlu disesuaikan:
 * 1. int ip
 * 2. String loc
 * 3. String prod
 * 4. Pada function sendLog(), terdapat 2 baris form-data; name=\"file\"; filename=\"weigherLog31.txt\"
 *    Ganti filename sesuai dengan nama file log yang akan diupload
 * 5. const char* logName
 * 
 * T:
 * 1. Save variabel totalLineCount to sdCard totalLineCount.txt
 * 2. Save variabel saveDataConter to sdCard saveDataConter.txt
 * 
 * 06:00:00 - 13:59:59
 * 14:00:00 - 21:59:59
 * 22:00:00 - 05:59:59
 * 
 * Jika kondisi kiri terpenuhi -> reset -> txt 0
 * Jika kondisi kanan terpenuhi -> reset 0 -> reset 1 
 * 
 * default 1
 * if jam >= 6 dan menit >= 0 dan detik >= 1 < 6+1 dan txt 1 maka reset kemuadian txt 0
 * jam 6 + 1 -> txt 1 
*/

#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <esp_system.h>
#include <Ethernet.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <EthernetUdp.h>

// ========= INISIALISASI AWAL =========
/**/ const int ip = 31;
/**/ const String loc = "Kerupuk 31";
/**/ const String prod = "Kerupuk";
// =====================================

// DEVICE INFO
String ESPName = "Weigher | " + loc;
String deviceID = "IoT-" + String(ip);
int sendDataCounter, sendDataCounterFailed, sendLogCounter, totalLineCount, saveDataConter, saveDataConterFailed;

// SERVER API
const char* serverAddress = "192.168.7.223";
const char* serverPathData = "POST /iot/api/weigher/save_weigher.php HTTP/1.1";
const char* serverPathLog = "POST /iot/api/weigher/save_weigher_log.php HTTP/1.1";
const char* serverPathLog_Flask = "POST /weigher/upload_log_weigher HTTP/1.1";
const int serverPort = 5001;
const int CHUNK_SIZE = 512;
uint8_t buffer[CHUNK_SIZE];
EthernetClient client;

// NETWORK STATUS
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
  "TIC TIC BALADO",
  "TEST_31"
};
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))
char kgLoadCellPrint[6];

// NETWORK STATIC CONFIGURATION
#define W5500_CS 5
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ipaddress(192, 168, 7, ip);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);

// NTP CONFIGURATION
IPAddress ntpServer(192, 168, 7, 223);
unsigned int localNtpPort = 2390;
EthernetUDP udp;
NTPClient ntpClient(udp, ntpServer, 0, 60000); // UTC time, update every 60 seconds
String formattedTime;
unsigned long lastNTPUpdateTime = 0;
const unsigned long NTP_UPDATE_INTERVAL = 1000;
int hourNTP, minuteNTP, secondNTP;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lastLcdClearTime = 0;
const unsigned long LCD_CLEAR_INTERVAL = 30000;

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
const char* logName = "/weigherLog31.txt";
const char* logSave = "/weigherLog31.txt";
const char* logSend = "/weigherLog31.txt";
bool hasReset = false;

// TASK HANDLER CORE 0 FOR SEND DATA
TaskHandle_t SendLogTaskHandle;
TaskHandle_t ProcessDataTaskHandle;

/**
 * @brief Read a float value from EEPROM at given address.
 *
 * @param address Address in EEPROM to read from.
 * @return The float value read from EEPROM.
 */
float readFloatFromEEPROM(int address) {
  float value = EEPROM.get(address, value);
  return value;
}

/**
 * @brief Read a float value from EEPROM at given address and update it if different.
 *
 * @param address Address in EEPROM to read from and write to.
 * @param newValue The new value to write to EEPROM if it differs from the
 *                 current value.
 */
void updateFloatInEEPROM(int address, float newValue) {
  float currentValue = readFloatFromEEPROM(address);

  if (currentValue != newValue) {
    // If the new value is different from the current value in EEPROM, update it.
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

/**
 * Tares the scale to reset its weight reading to zero and clears the LCD display.
 * This function should be used when you want to ignore the weight currently on the scale,
 * allowing you to measure only the additional weight added afterward.
 */
void tareScale() {
  scale.tare();
  lcd.clear();
}

bool sendData() {
  // EthernetClient client;

  // Coba koneksi
  if (client.connect(serverAddress, serverPort)) {
    // Kirim HTTP POST Request
    client.println(serverPathData);
    client.println("Host: 192.168.7.223");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);

    // Tunggu respon
    while (client.connected()) {
      if (client.available()) {
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

/**
 * Runs the calibration process, which guides the user through the steps to
 * calibrate the scale and set the digit.
 *
 * The process consists of the following steps:
 *
 * 1. Select the weight of the item to be measured
 * 2. Empty the scale
 * 3. Weigh the item
 * 4. Set the digit
 * 5. Confirm calibration
 *
 * The user can exit the calibration process by pressing the select button.
 * The user can restart the calibration process by pressing the down button.
 *
 * The calibration factor and digit scale are stored in EEPROM.
 */
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

/**
 * @brief Prompts the user to select a product from the list.
 *
 * This function displays the list of products and allows the user to select one
 * by pressing the UP and DOWN buttons. When the user presses the DOWN button,
 * the selected product is stored in the `productSelected` variable and the
 * function returns.
 */
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

bool checkLog(const char* path) {
  if (SD.exists(path)) {
    return true;
  } else {
    return false;
  }
}

bool createLog(const char* path) {
  File file = SD.open(path, FILE_WRITE);
  if (file) {
    file.close();
    return true;
  } else {
    return false;
  }
}

bool appendLog(const char* path, const char* log) {
  File file = SD.open(path, FILE_APPEND);
  if (file) {
    file.println(log);
    file.close();
    Serial.println(log);
    return true;
  } else {
    return false;
  }
}

bool deleteLog(const char* path) {
  if (SD.remove(path)) {
    Serial.println("File deleted");
    return true;
  } else {
    return false;
  }
}

bool deleteAllLog() {
  File root = SD.open("/");
  if (root) {
    File file;
    while ((file = root.openNextFile())) {
      String fileName = file.name();
      if (fileName.endsWith(".txt")) {
        SD.remove(fileName.c_str());
      }
      file.close();
    }
    root.close();
    return true;
  } else {
    return false;
  }
}

bool isTimeToResetCounterSaveSend(int hour_, int minute_, int second_) {
  int resetTimes[3][3] = {
    {6, 0, 0},  // 06:00:00
    {14, 0, 0}, // 14:00:00
    {22, 0, 0}  // 22:00:00
  };

  for (int i = 0; i < 3; i++) {
    if (hour == resetTimes[i][0] && minute == resetTimes[i][1] && second == resetTimes[i][2]) {
      return true;
    }
  }
  return false;
}

/*
 * Fungsi sendLog() mengirimkan log ke server menggunakan HTTP POST.
 * Fungsi ini dijalankan dalam task dengan prioritas tertinggi.
 * Fungsi ini akan menunggu sampai buttonSelect ditekan untuk menghentikan pengiriman log.
 * Pada saat proses mengirim data ke server, semua proses akan di'halt' sampai proses pengiriman selesai.
 * Server yang digunakan adalah Flask, bisa dilihat di github.com/rocery/iot.git
 * Untuk mengganti interval otomatis kirim log ke server, ganti SERVER_CHECK_INTERVAL
*/
void sendLog(void* parameter) {
  bool buttonProcessed = false;
  unsigned long lastServerCheckTime = 0;
  const unsigned long SERVER_CHECK_INTERVAL = 30000;  // 30 seconds
  while (true) {
    unsigned long currentTime = millis();

    if (isButtonPressed(buttonSelect)) {
      unsigned long buttonPressStartTime = millis();
      buttonProcessed = false;

      // Wait for button release
      while (isButtonPressed(buttonSelect)) {
        unsigned long currentPressTime = millis();

        // Check if button is pressed for more than 200 ms
        if (currentPressTime - buttonPressStartTime >= 100 && !buttonProcessed) {
          // Execute code when button is pressed longer than 0.5 seconds
          if (!sdStatus) {
            if (lanStatus == "D") {
              lcd.setCursor(0, 1);
              lcd.print("X, LAN ERROR");
              sendDataCounterFailed++;
              vTaskDelay(pdMS_TO_TICKS(1000));
            } else {
              postData = "device_id=" + deviceID + "&device_name=" + ESPName + "&product=" + productSelected + "&weight=" + String(kgLoadCellPrint) + "&ip_address=" + ip_Address + "&wifi=" + "LAN";
              if (!sendData()) {
                lcd.setCursor(0, 1);
                lcd.print("X, HUBUNGI IT");
                sendDataCounterFailed++;
                vTaskDelay(pdMS_TO_TICKS(1000));
              } else {
                sendDataCounter++;
                lcd.print(sendDataCounter);
              }
            }
          } else {
            postData = deviceID + ',' + ESPName + ',' + productSelected + ',' + String(kgLoadCellPrint) + ',' + formattedTime + ',' + ip_Address + ',' + "LAN";

            if (!checkLog(logName)) {
              createLog(logName);
            }

            if (!appendLog(logName, postData.c_str())) {
              lcd.setCursor(0, 1);
              lcd.print("DATA GGL DISAVE");
              // Jumlah data yang gagal disimpan pada sdCard
              saveDataConterFailed++;
              vTaskDelay(pdMS_TO_TICKS(1000));
            } else {
              // Jumlah data yang behasil disimpan pada sdCard
              saveDataConter++;
            }
          }

          // Mark as processed to prevent repeated execution
          buttonProcessed = true;
        }

        vTaskDelay(pdMS_TO_TICKS(10));  // Small delay to prevent tight looping
      }
    }

    // Periodic server check with non-blocking approach
    if (currentTime - lastServerCheckTime >= SERVER_CHECK_INTERVAL) {
      File logFile = SD.open(logName, FILE_READ);
      if (!logFile) {
        lastServerCheckTime = currentTime;
        continue;
      }
      long fileSize = logFile.size();
      Serial.print("File size: ");
      Serial.println(fileSize);
      if (client.connect(serverAddress, serverPort) && checkLog(logName) && fileSize > 0) {
        // Your existing file upload logic remains the same
        Serial.println("Connected to server");

        String boundary = "------------------------abcdef123456";

        // Improved multipart form data headers
        client.println("POST /weigher/upload_log_weigher HTTP/1.1");
        client.println("Host: " + String(serverAddress));
        client.println("Content-Type: multipart/form-data; boundary=" + boundary);

        // Calculate content length more precisely
        long contentLength =
          String("--" + boundary + "\r\n").length() + String("Content-Disposition: form-data; name=\"file\"; filename=\"weigherLog31.txt\"\r\n").length() + String("Content-Type: text/plain\r\n\r\n").length() + fileSize + String("\r\n--" + boundary + "--\r\n").length();

        client.println("Content-Length: " + String(contentLength));
        client.println("Connection: close");
        client.println();

        // Write multipart form data
        client.println("--" + boundary);
        client.println("Content-Disposition: form-data; name=\"file\"; filename=\"weigherLog31.txt\"");
        client.println("Content-Type: text/plain");
        client.println();

        // Send file in chunks
        while (logFile.available()) {
          int bytesRead = logFile.read(buffer, CHUNK_SIZE);
          if (bytesRead > 0) {
            client.write(buffer, bytesRead);
          }
        }

        // Properly close multipart form
        client.println();
        client.println("--" + boundary + "--");

        // Enhanced response handling
        unsigned long timeout = millis();
        String response = "";
        int statusCode = 0;
        bool headersComplete = false;
        String responseBody = "";

        while (client.connected() && millis() - timeout < 10000) {
          if (client.available()) {
            String line = client.readStringUntil('\n');

            // Parse HTTP status code
            if (line.startsWith("HTTP/1.1")) {
              statusCode = line.substring(9, 12).toInt();
              Serial.print("Server Response Status Code: ");
              Serial.println(statusCode);
            }

            // Collect headers
            if (!headersComplete) {
              response += line;

              // Check for end of headers
              if (line.length() <= 2) {
                headersComplete = true;
                Serial.println("Headers complete");
              }
            }
            // Collect response body
            else {
              responseBody += line;
            }
          }

          // Break if no more data and headers are complete
          if (!client.available() && headersComplete) {
            break;
          }
        }

        // Verify upload success
        if (statusCode == 200) {
          Serial.println("File upload successful");
          Serial.println("Server Response Headers:");
          Serial.println("==================");
          Serial.println(response);
          Serial.println("==================");
          Serial.println(responseBody);

        } else {
          Serial.print("File upload failed. Status code: ");
          Serial.println(statusCode);
          Serial.println("Response:");
          Serial.println(response);
        }
        Serial.println("File sent attempt completed");

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, responseBody);
        if (error) {
          Serial.println("JSON parsing failed");
        }
        const char* status = doc["status"];
        int jumlah_data = doc["jumlah_data"];
        Serial.print("Status: ");
        Serial.println(status);
        Serial.print("Jumlah data: ");
        Serial.println(jumlah_data);


        if (String(status) == "success") {
          // Total file yang dikirim
          // sendLogCounter++;
          
          // Total data yang berhasil dikirim
          totalLineCount = totalLineCount + jumlah_data;
          deleteLog(logName);
        }

        lastServerCheckTime = currentTime;
      } else {
        Serial.println("Connection failed");
        lastServerCheckTime = currentTime;
      }
      client.stop();
      logFile.close();
    }
    // Small delay to prevent tight looping
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Function to convert epoch time to "YYYY-MM-DD hh:mm:ss"
String getFormattedDateTime(unsigned long epochTime) {
  const int SECS_IN_DAY = 86400;
  const int SECS_IN_HOUR = 3600;
  const int SECS_IN_MIN = 60;

  // Calculate date and time
  epochTime += 3600 * 7; // Adjust for timezone (e.g., UTC+7)
  unsigned long days = epochTime / SECS_IN_DAY;
  int year = 1970;

  // Calculate year
  while (days >= 365) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) { // Leap year check
      if (days >= 366) {
        days -= 366;
      } else {
        break;
      }
    } else {
      days -= 365;
    }
    year++;
  }

  // Calculate month
  int month;
  int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
    monthDays[1] = 29; // Adjust February for leap year
  }
  for (month = 0; month < 12; month++) {
    if (days < monthDays[month]) {
      break;
    }
    days -= monthDays[month];
  }
  month++;

  // Remaining days are the day of the month
  int day = days + 1;

  // Calculate time
  int remainingSecs = epochTime % SECS_IN_DAY;
  int hour = remainingSecs / SECS_IN_HOUR;
  remainingSecs %= SECS_IN_HOUR;
  int minute = remainingSecs / SECS_IN_MIN;
  int second = remainingSecs % SECS_IN_MIN;

  // Format as "YYYY-MM-DD hh:mm:ss"
  char buffer[20];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);

  hourNTP = hour;
  minuteNTP = minute;
  secondNTP = second;

  return String(buffer);
}

void updateTime() {
  ntpClient.update();
  unsigned long epochTime = ntpClient.getEpochTime();
  formattedTime = getFormattedDateTime(epochTime);
}

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

  // Inisialisasi Ethernet dan IP statis
  Ethernet.init(W5500_CS);
  Ethernet.begin(mac, ipaddress, gateway, gateway, subnet);

  /*
  Inisialisasi SD Card.
  Jika gagal maka data tidak disimpan di sd card,
  data akan langsung dikirim ke server.
  Weigher tetap bisa digunakan tanpa ada kendala.
  */
  if (!SD.begin(SD_CS)) {
    lcd.setCursor(0, 0);
    lcd.print("SD CARD ERROR");
    lcd.setCursor(0, 1);
    lcd.print("X, HUBUNGI IT");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WEIGHER OK, DATA");
    lcd.setCursor(0, 1);
    lcd.print("TDK DISIMPAN SD");
    delay(3000);
    lcd.clear();
    sdStatus = false;
  } else {
    sdStatus = true;
  }

  // Begin UDP for NTP
  udp.begin(localNtpPort);
  ntpClient.begin();

  // Choose Product
  chooseProduct();

  // Buat task untuk mengirim data
  xTaskCreate(
    sendLog,            // Fungsi task
    "sendLog",          // Nama task
    4096,               // Ukuran stack
    NULL,               // Parameter task
    1,                  // Prioritas task
    &SendLogTaskHandle  // Task handle
    // 0
  );

  lcd.clear();
}

void loop() {
  unsigned long currentTime = millis();
  double rawLoadCell = scale.get_units(5);
  float kgLoadCell = rawLoadCell / 1000;
  // float absValuekgLoadCell = fabs(kgLoadCell);

  if (currentTime - lastNTPUpdateTime >= NTP_UPDATE_INTERVAL) {
    lastNTPUpdateTime = currentTime;
    updateTime();
  }

  dtostrf(kgLoadCell, 5, digitScale, kgLoadCellPrint);

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
  if (!sdStatus) {
    lcd.setCursor(9, 1);
    lcd.print(sendDataCounter);
  } else {
    lcd.setCursor(9, 1);
    lcd.print(saveDataConter);
  }

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

  if (isButtonPressed(buttonUp)) {
    lcd.clear();
    if (!sdStatus) {
      while (isButtonPressed(buttonUp)) {
        lcd.setCursor(0, 0);
        lcd.print("B : ");
        lcd.print(sendDataCounter);
        lcd.setCursor(0, 1);
        lcd.print("G : ");
        lcd.print(sendDataCounterFailed);
        lcd.setCursor(7, 1);
        lcd.print("SD : ");
        lcd.print(sdStatus);
        lcd.setCursor(15, 1);
        lcd.print(lanStatus);
        lcd.setCursor(11, 0);
        lcd.print(deviceID);
      }
    } else {
      while (isButtonPressed(buttonUp)) {
        lcd.setCursor(0, 0);
        lcd.print("B:");
        lcd.print(saveDataConter);
        lcd.setCursor(6, 0);
        lcd.print("-SD:");
        lcd.print(sdStatus);
        lcd.setCursor(13, 0);
        lcd.print(".");
        lcd.print(ip);

        lcd.setCursor(0, 1);
        lcd.print("G:");
        lcd.print(saveDataConterFailed);
        lcd.setCursor(6, 1);
        lcd.print("-S:");
        lcd.print(totalLineCount);
        lcd.setCursor(15, 1);
        lcd.print(lanStatus);
      }
    }
    lcd.clear();
  }

  // Non-blocking LCD clear
  if (currentTime - lastLcdClearTime >= LCD_CLEAR_INTERVAL) {
    lcd.clear();
    lastLcdClearTime = currentTime;
  }

  if (hourNTP )

  // Reset Counter Save dan Send
  if (isButtonPressed(buttonUp) && isButtonPressed(buttonDown)) {
    while (isButtonPressed(buttonUp) && isButtonPressed(buttonDown)) {
      lcd.clear();
      // reset counter
  }
}