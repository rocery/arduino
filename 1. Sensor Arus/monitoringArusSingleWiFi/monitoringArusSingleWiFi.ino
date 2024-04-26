#include <PZEM004Tv30.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 13
#define ledPzem12 2
#define ledPzem13 0

// == Initialization PZEM ==
float pzem12Power, pzem12Energy, pzem12Voltage, pzem12Current;
float pzem13Power, pzem13Energy, pzem13Voltage, pzem13Current;

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem12(pzemSWSerial, 0x12);
PZEM004Tv30 pzem13(pzemSWSerial, 0x13);

const char* ssid = "A52s";
const char* password = "28092005";

// Set your Static IP address
IPAddress staticIP(192, 168, 15, 215);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

String postData;
String ip_address;

String pzem12Chanel = "Chanel 12";
String pzem13Chanel = "Chanel 13";

void sendData(float Voltage, String deviceName, String IP) {
  HTTPClient http;     // http object of clas HTTPClient
  WiFiClient wclient;  // wclient object of clas HTTPClient

  postData = "voltage=" + String(Voltage) + "&device_name=" + deviceName + "&ip_address=" + String(IP);

  http.begin(wclient, "http://192.168.15.221/arduino_api/saveStatus.php");  // Connect to host where MySQL databse is hosted
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

  http.begin(wclient, "http://192.168.15.221/arduino_api/createFile.php");  // Connect to host where MySQL databse is hosted
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

void setup() {
  Serial.begin(115200);
  pzemSWSerial.begin(9600);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);

  //ketika WiFI.status nilainya TDK sama dg WL_CONNECTED
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  pinMode(ledPzem12, OUTPUT);
  pinMode(ledPzem13, OUTPUT);
}

void loop() {
  // Try Wifi if Disconnected
  int tryWifi = 0;
  while (WiFi.status() != WL_CONNECTED && tryWifi <= 20) {
    WiFi.begin(ssid, password);
    tryWifi++;
    delay(500);
  }

  pzem12Power = pzem12.power();
  pzem12Energy = pzem12.energy();
  pzem12Voltage = pzem12.voltage();
  pzem12Current = pzem12.current();
  delay(1000);

  pzem13Power = pzem13.power();
  pzem13Energy = pzem13.energy();
  pzem13Voltage = pzem13.voltage();
  pzem13Current = pzem13.current();
  delay(1000);

  if (isnan(pzem12Power)) {
    digitalWrite(ledPzem12, LOW);
  } else {
    digitalWrite(ledPzem12, HIGH);
  }

  if (isnan(pzem13Power)) {
    digitalWrite(ledPzem13, LOW);
  } else {
    digitalWrite(ledPzem13, HIGH);
  }

  ip_address = WiFi.localIP().toString();

  readElectricalCurrent(pzem12Power, pzem12Energy, pzem12Voltage, pzem12Current);
  delay(1000);
  readElectricalCurrent(pzem13Power, pzem13Energy, pzem13Voltage, pzem13Current);
  delay(1000);
  Serial.println(ip_address);

   // Data Pzem 12
   sendLogData(pzem12Power, pzem12Energy, pzem12Voltage, pzem12Current, ip_address);
   sendData(pzem12Voltage, pzem12Chanel, ip_address);
   delay(1000);
  
   // Data Pzem 13
   sendLogData(pzem13Power, pzem13Energy, pzem13Voltage, pzem13Current, ip_address);
   sendData(pzem13Voltage, pzem13Chanel, ip_address);
   delay(1000);
}
