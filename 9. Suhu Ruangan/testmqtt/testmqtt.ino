#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "STTB7";
const char* password = "Si4nt4r321";

// MQTT broker information
const char* mqtt_server = "192.168.7.10";
const int mqtt_port = 1883;
const char* mqtt_topic = "suhuruangcalibrator/";

// Create WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

IPAddress staticIP(192, 168, 7, 7);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional


// Variable to store received data
String receivedData;

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';                 // Null-terminate the payload
  receivedData = String((char*)payload);  // Convert payload to String
  Serial.println("Data received: " + receivedData);
}

void setup() {
  Serial.begin(115200);


  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  } else {
    Serial.println("STA Success");
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect("ESP32Subscriber")) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed to connect. State: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  client.loop();  // Keep the client connected and process incoming messages

  // Do something with receivedData if needed
  // For example, print the data to Serial Monitor
  if (receivedData.length() > 0) {
    Serial.println("Processing received data: " + receivedData);
    // Clear receivedData after processing
    // receivedData = "";
  }
}
