#include <PZEM004Tv30.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

//#define RXD2 16
//#define TXD2 17
//#define RXD1 26
//#define TXD1 27
#define PZEM_SERIAL Serial2

// == Initialization PZEM ==
float pzem12Power, pzem12Energy, pzem12Voltage, pzem12Current;
float pzem13Power, pzem13Energy, pzem13Voltage, pzem13Current;

PZEM004Tv30 pzem12(PZEM_SERIAL, 16, 17);
PZEM004Tv30 pzem13(PZEM_SERIAL, 16, 17);

// == WiFi Config ==
WiFiMulti wifiMulti;
const char* ssid_a = "STTB1";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "MT1";
const char* password_b = "siantar321";
const char* ssid_c = "MT3";
const char* password_c = "siantar321";
const char* ssid_d = "STTB4";
const char* password_d = "siantar123";
const char* ssid_it = "STTB5";
const char* password_it = "siantar123";

// Set your Static IP address
IPAddress staticIP(192, 168, 15, 219);
// Set your Gateway IP address
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

String postData;
String ip_address;

String pzem12Chanel = "Chanel 12";
String pzem13Chanel = "Chanel 13";

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

void sendLogData(float power_, float energy_, float voltage_, float current_, String ip) {
  HTTPClient http;
  postData = "power=" + String(power_) + "&energy=" + String(energy_) + "&voltage=" + String(voltage_) + "&current=" + String(current_) + "&ip_address=" + String(ip);
  http.begin("http://192.168.15.221/arduino_api/createFile.php");
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

void sendData(float voltage_, String deviceName, String ip) {
  HTTPClient http;
  postData = postData = "voltage=" + String(voltage_) + "&device_name=" + deviceName + "&ip_address=" + String(ip);
  http.begin("http://192.168.15.221/arduino_api/saveStatus.php");
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

void setup() {
  Serial.begin(115200);
//  Serial1.begin(9600, SERIAL_8N1, PZEM1_RX, PZEM1_TX);
//  Serial2.begin(9600, SERIAL_8N1, PZEM2_RX, PZEM2_TX);

  wifiMulti.addAP("ssid_a", "password_a");
  wifiMulti.addAP("ssid_b", "password_b");
  wifiMulti.addAP("ssid_c", "password_c");
  wifiMulti.addAP("ssid_d", "password_d");
  wifiMulti.addAP("ssid_it", "password_it");

//  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
//    Serial.println("STA Failed to configure");
//  }

  wifiMulti.run();
  delay(1000);
  Serial.println("Connected to SSID: " + WiFi.SSID());
}

void loop() {
  pzem12Power = pzem12.power();
  pzem12Energy = pzem12.energy();
  pzem12Voltage = pzem12.voltage();
  pzem12Current = pzem12.current();

  pzem13Power = pzem13.power();
  pzem13Energy = pzem13.energy();
  pzem13Voltage = pzem13.voltage();
  pzem13Current = pzem13.current();

  ip_address = WiFi.localIP().toString();

  readElectricalCurrent(pzem12Power, pzem12Energy, pzem12Voltage, pzem12Current);

  //  // Data Pzem 12
  //  sendLogData(pzem12Power, pzem12Energy, pzem12Voltage, pzem12Current, ip_address);
  //  sendData(pzem12Voltage, pzem12Chanel, ip_address);
  //  delay(1000);
  //
  //  // Data Pzem 13
  //  sendLogData(pzem13Power, pzem13Energy, pzem13Voltage, pzem13Current, ip_address);
  //  sendData(pzem13Voltage, pzem13Chanel, ip_address);
  //  delay(14000);

  delay(2000);
}
