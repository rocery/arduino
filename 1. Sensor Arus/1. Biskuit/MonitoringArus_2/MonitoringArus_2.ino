/*
  V 1.0.0
  Update Terakhir : 18-12-2023
  Last Change Log {
    1. 
  }

  Komponen:
  1. NodeMCU ESP8266 V.3
  2. PZEM-004T V 3.0
  3. LED @2 (Beda Warna)

  Program ini berfungsi menghitung instrumen listrik pada kabel.
  Projek ini mengunakan ESP8266 sebagai micro controller karena memiliki fitur SerialSoftware,
  sehingga komunikasi UART bisa digunakan sebanyak mungkin selama address-nya berbeda dan daya
  dari ESP8266 kuat.

  Proses koneksi WiFi tidak disarakn menggunakan Library MultiWiFiESP8266.h, kecuali Micon yang digunakan ESP32
*/

// == Deklarasikan semua Library yang digunakan ==
#include <PZEM004Tv30.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

// Digunakan pin 12 dan 13 (6, 7) sebagai pin UART (RX, TX)
// Pin TX dari Pzem dihubungkan ke pin RX (6) ESP8266
// Pin RX dari Pzem dihubungkan ke pin TX (7) ESP8266
#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 13

// Pada ESP8266 pin dibawah tidak bisa digunakan, gunakan pin SLC-SDA
// #define ledPzem12 2
// #define ledPzem13 0

// == Initialization PZEM ==
float pzem12Power, pzem12Energy, pzem12Voltage, pzem12Current;
float pzem13Power, pzem13Energy, pzem13Voltage, pzem13Current;

// Inisialisasi SoftwareSerial untuk komunikasi UART
SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
// 0x12/0x13 adalah alamat dari setiap sensor PZEM yang digunakan
// Untuk merubah alamat ini jalankan program "ChangeAdressPzem004T"
PZEM004Tv30 pzem12(pzemSWSerial, 0x12);
PZEM004Tv30 pzem13(pzemSWSerial, 0x13);

const char* ssid_1 = "STTB5";
const char* password_1 = "siantar123";
const char* ssid_2 = "STTB1";
const char* password_2 = "Si4nt4r321";
const char* ssid_3 = "MT3";
const char* password_3 = "siantar321";

// Set your Static IP address
IPAddress staticIP(192, 168, 7, 215);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

String postData;
String ip_address;
int sendDataDB = 0;

String pzem12Chanel = "Conv. Stuffle Mie";
String pzem13Chanel = "Conv. Stuffle Biskuit";

void sendData(float Voltage, String deviceName, String IP) {
  HTTPClient http;     // http object of clas HTTPClient
  WiFiClient wclient;  // wclient object of clas HTTPClient

  postData = "voltage=" + String(Voltage) + "&device_name=" + deviceName + "&ip_address=" + String(IP);

  http.begin(wclient, "http://192.168.7.223/arduino_api/saveStatus.php");  // Connect to host where MySQL databse is hosted
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");      //Specify content-type header

  int httpCode = http.POST(postData);  // Send POST request to php file and store server response code in variable named httpCode
  Serial.println("Values are, " + postData);

  // if connection eatablished then do this
  if (httpCode == 200) {
    Serial.println("Values uploaded successfully.");
    Serial.println(httpCode);
    String webpage = http.getString();  // Get html webpage output and store it in a string
    Serial.println(webpage + "\n");
  } else {
    Serial.println(httpCode);
    Serial.println("Failed to upload values. \n");
    http.end();
    return;
  }
}

void sendLogData(float Power, float Energy, float Voltase, float Current, String IP) {
  HTTPClient http;     // http object of clas HTTPClient
  WiFiClient wclient;  // wclient object of clas HTTPClient

  postData = "power=" + String(Power) + "&energy=" + String(Energy) + "&voltage=" + String(Voltase) + "&current=" + String(Current) + "&ip_address=" + String(IP);

  http.begin(wclient, "http://192.168.7.223/arduino_api/createFile.php");  // Connect to host where MySQL databse is hosted
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");      //Specify content-type header

  int httpCode = http.POST(postData);  // Send POST request to php file and store server response code in variable named httpCode
  Serial.println("Values are, " + postData);

  // if connection eatablished then do this
  if (httpCode == 200) {
    Serial.println("Values uploaded successfully.");
    Serial.println(httpCode);
    String webpage = http.getString();  // Get html webpage output and store it in a string
    Serial.println(webpage + "\n");
  } else {
    Serial.println(httpCode);
    Serial.println("Failed to upload values. \n");
    http.end();
    return;
  }
}

void readElectricalCurrent(float Power, float Energy, float Voltase, float Current) {
  // Jika gagal membaca Power
  if (isnan(Power)) {
    Serial.println("Gagal membaca Power");
  } else {
    Serial.print("Power : ");
    Serial.print(Power);
    Serial.println(" kW");
  }

  // Jika gagal membaca Energy
  if (isnan(Energy)) {
    Serial.println("Gagal membaca Energy");
  } else {
    Serial.print("Energy : ");
    Serial.print(Energy, 4);
    Serial.println(" kWh");
  }

  // Jika gagal membaca Voltase
  if (isnan(Voltase)) {
    Serial.println("Gagal membaca Voltase");
  } else {
    Serial.print("Voltase : ");
    Serial.print(Voltase);
    Serial.println(" V");
  }

  // Jika gagal membaca Current
  if (isnan(Current)) {
    Serial.println("Gagal membaca Current");
  } else {
    Serial.print("Current : ");
    Serial.print(Current);
    Serial.println(" A");
  }

  Serial.println();
}

void connectToWiFi() {
  int flag = 1;

  // Configures static IP address
  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  int network = WiFi.scanNetworks();
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    for (int i = 0; i < network; i++) {
      switch (flag) {
        case 1:
          flag = 2;
          if (WiFi.SSID(i) == ssid_1) {
            WiFi.begin(ssid_1, password_1);
            Serial.println("Connecting with STTB1 Please Wait...");
            delay(80);
            break;
          }
        case 2:
          flag = 3;
          if (WiFi.SSID(i) == ssid_2) {
            WiFi.begin(ssid_2, password_2);
            Serial.println("Connecting with STTB4 Please Wait...");
            delay(80);
            break;
          }
        case 3:
          flag = 1;
          if (WiFi.SSID(i) == ssid_3) {
            WiFi.begin(ssid_3, password_3);
            Serial.println("Connecting with MT1 Please Wait...");
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
  } else if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi disconnected!"));
  }
}

void setup() {
  Serial.begin(115200);
  pzemSWSerial.begin(9600);

  //  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
  //    Serial.println("STA Failed to configure");
  //  }

  //  WiFi.begin(ssid, password);
  //
  //  //ketika WiFI.status nilainya TDK sama dg WL_CONNECTED
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(500);
  //    Serial.print(".");
  //  }
  connectToWiFi();
  // Un-Comment baris dibawah setelah ganti #define pin
  // pinMode(ledPzem12, OUTPUT);
  // pinMode(ledPzem13, OUTPUT);
}

void loop() {
  pzem12Power = pzem12.power();
  pzem12Energy = pzem12.energy();
  pzem12Voltage = pzem12.voltage();
  pzem12Current = pzem12.current();
  delay(100);

  pzem13Power = pzem13.power();
  pzem13Energy = pzem13.energy();
  pzem13Voltage = pzem13.voltage();
  pzem13Current = pzem13.current();
  delay(100);

  // if (isnan(pzem12Power)) {
  //   digitalWrite(ledPzem12, LOW);
  // } else {
  //   digitalWrite(ledPzem12, HIGH);
  // }

  // if (isnan(pzem13Power)) {
  //   digitalWrite(ledPzem13, LOW);
  // } else {
  //   digitalWrite(ledPzem13, HIGH);
  // }

  ip_address = WiFi.localIP().toString();

  readElectricalCurrent(pzem12Power, pzem12Energy, pzem12Voltage, pzem12Current);
  delay(100);
  readElectricalCurrent(pzem13Power, pzem13Energy, pzem13Voltage, pzem13Current);
  delay(100);
  Serial.println(ip_address);

  // Data Pzem 12
  sendLogData(pzem12Power, pzem12Energy, pzem12Voltage, pzem12Current, ip_address);
  sendData(pzem12Voltage, pzem12Chanel, ip_address);
  delay(1000);

  // Data Pzem 13
  sendLogData(pzem13Power, pzem13Energy, pzem13Voltage, pzem13Current, ip_address);
  sendData(pzem13Voltage, pzem13Chanel, ip_address);
  
  sendDataDB++;
  Serial.println(ip_address);
  Serial.println(sendDataDB++);
  if (sendDataDB >= 1000) {
    ESP.reset();
  }

  delay(14000);
}
