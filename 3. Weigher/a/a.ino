#include <Ethernet.h>
#include <EthernetUdp.h>
#include <NTPClient.h>
#include <time.h>

// Ethernet and MAC configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetUDP Udp;

// Network configuration
IPAddress ipaddress(192, 168, 7, 32);
IPAddress ntpServerIP(192, 168, 7, 223);
IPAddress gateway(192, 168, 15, 250);
IPAddress subnet(255, 255, 0, 0);
const int NTP_PORT = 123;

NTPClient timeClient(Udp, ntpServerIP, 0, 60000);

void setup() {
  Serial.begin(115200);
  
  // Ethernet initialization with pin 10 as default SS pin
  Ethernet.init(5);  // Change 10 to your specific SS pin if different
  
  Serial.println("Initializing Ethernet...");

  // Try to configure Ethernet
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    
    // Static IP configuration
    Ethernet.begin(mac, ipaddress, gateway, gateway, subnet);
  }

  // Wait for Ethernet to configure
  delay(1000);

  // Check Ethernet connection
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());

  // Initialize NTP
  timeClient.begin();
  
  // Attempt multiple updates
  int updateAttempts = 0;
  while (!timeClient.update() && updateAttempts < 5) {
    Serial.println("Trying to update time...");
    timeClient.forceUpdate();
    updateAttempts++;
    delay(1000);
  }

  if (updateAttempts >= 5) {
    Serial.println("Failed to update time from NTP server");
  } else {
    Serial.println("NTP time synchronized");
  }
}

void loop() {
  // Attempt to update time
  if (timeClient.update()) {
    String formattedDateTime = getFormattedDateTime();
    Serial.println(formattedDateTime);
  } else {
    Serial.println("Failed to update time");
  }
  
  delay(1000);
}

String getFormattedDateTime() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm* timeinfo = localtime(&rawTime);
  
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  
  return String(buffer);
}