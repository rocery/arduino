#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
int value;
String deviceID = "IoT-251-TK0532";

WiFiMulti wifiMulti;
const char* ssid_a = "STTB1";
const char* password_a = "Si4nt4r321";
const char* ssid_b = "Tester_ITB";
const char* password_b = "Si4nt4r321";
const char* ssid_c = "STTB11";
const char* password_c = "Si4nt4r321";
const char* ssid_d = "STTB7";
const char* password_d = "Si4nt4r321";

// Set IP to Static
IPAddress staticIP(192, 168, 7, 12);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

int getLogData() {
  /* Untung mendapatkan data terakhir dari DB, 
  saat ini tidak digunakan karena sudah menggunakan SD Card
  Kode dibawah mohon untuk tidak dihapus.
  */
  HTTPClient http;
  String getData = "http://192.168.7.223/barometer_api/get_tekanan.php?device_id=" + deviceID;
  http.begin(getData);
  int httpCode = http.GET();

  int psiValueDB = 0;

  if (httpCode > 0) {
    String payload = http.getString();
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      if (doc.containsKey("psi_calibration_value")) {
        psiValueDB = doc["psi_calibration_value"].as<int>();
      } else {
        Serial.println("Key 'psi_calibration_value' not found in JSON");
      }
    } else {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("Error get log: ");
    Serial.println(httpCode);
  }

  http.end();
  return psiValueDB;
}


void setup() {
  Serial.begin(115200);

  wifiMulti.addAP(ssid_a, password_a);
  wifiMulti.addAP(ssid_b, password_b);
  wifiMulti.addAP(ssid_c, password_c);
  wifiMulti.addAP(ssid_d, password_d);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  } else {
    Serial.println("STA Success");
  }

  wifiMulti.run();
}

void loop() {
  value = getLogData();
  Serial.println(value);

  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi Error");
    wifiMulti.run();
  }

  delay(2000);
}
