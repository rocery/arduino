/*
  =========IoT-216-KR0366=========
  V 1.0.0
  Update Terakhir : 25-03-2024
  Catatan {
    1. Pzem mempunyai address default, jadi hanya bisa menggunakan 1 sensor.
  }

  Komponen:
  1. NodeMCU ESP8266 V.3
  2. PZEM-004T V 3.0

  Program ini berfungsi menghitung instrumen listrik pada kabel.
  Projek ini mengunakan ESP8266 sebagai micro controller karena memiliki fitur SerialSoftware,
  sehingga komunikasi UART bisa digunakan sebanyak mungkin selama address-nya berbeda dan daya
  dari ESP8266 kuat.

  Proses koneksi WiFi tidak disarakn menggunakan Library MultiWiFiESP8266.h, kecuali Micon yang digunakan ESP32
*/

#include <PZEM004Tv30.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Inisialiasi objek untuk PZEM (sensor, arus)
PZEM004Tv30 pzem(12, 13);  // 12=D6 (Rx), 13=D7 (Tx)

// Define variable
float Power, Energy, Voltase, Current;
const char* ssid = "Tester_ITB";
const char* password = "Si4nt4r321";

const char* ssid2 = "STTB4";
const char* password2 = "siantar123";

const char* ssid3 = "MT1";
const char* password3 = "siantar321";

// Set your Static IP address
IPAddress local_IP(192, 168, 7, 216);
// Set your Gateway IP address
IPAddress gateway(192, 168, 15, 250);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

String postData, ip_address;
String deviceName = "Kerupuk - Gudang Jadi";

void setup() {
  Serial.begin(9600);  // 9600
  connectToWiFi();
}

void connectToWiFi() {
  int flag = 1;

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  int network = WiFi.scanNetworks();
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    for (int i = 0; i < network; i++) {
      switch (flag) {
        case 1:
          flag = 2;
          if (WiFi.SSID(i) == ssid) {
            WiFi.begin(ssid, password);
            Serial.println("Connecting with STTB1 Please Wait...");
            delay(8000);
            break;
          }
        case 2:
          flag = 3;
          if (WiFi.SSID(i) == ssid2) {
            WiFi.begin(ssid2, password2);
            Serial.println("Connecting with STTB4 Please Wait...");
            delay(8000);
            break;
          }
        case 3:
          flag = 1;
          if (WiFi.SSID(i) == ssid3) {
            WiFi.begin(ssid3, password3);
            Serial.println("Connecting with MT1 Please Wait...");
            delay(8000);
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

void loop() {
  // Baca nilai Power (kW)
  Power = pzem.power();
  // Baca nilai Energy (kWh)
  Energy = pzem.energy();
  // Baca nilai Voltase (V)
  Voltase = pzem.voltage();
  // Baca nilai Current (A)
  Current = pzem.current();

  readElectricalCurrent(Power, Energy, Voltase, Current);

  ip_address = WiFi.localIP().toString();

  sendLogData(Power, Energy, Voltase, Current, ip_address);

  sendData(Voltase, deviceName, ip_address);

  delay(15000);
}
