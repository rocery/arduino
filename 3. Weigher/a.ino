#include <Ethernet.h>
#include <EthernetUdp.h>
#include <NTPClient.h>
#include <time.h>

// Ethernet and MAC configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetUDP Udp;

// Custom NTP server IP and port
IPAddress ntpServerIP(192, 168, 7, 223);
const int NTP_PORT = 123;

NTPClient timeClient(Udp, ntpServerIP, 0, 60000);

void setup() {
  Serial.begin(115200);
  
  // Start Ethernet connection
  Ethernet.begin(mac);
  
  // Wait for Ethernet to configure
  delay(1000);
  
  // Initialize NTP
  timeClient.begin();
  timeClient.update();
}

void loop() {
  timeClient.update();
  
  String formattedDateTime = getFormattedDateTime();
  Serial.println(formattedDateTime);
  
  delay(1000);
}

String getFormattedDateTime() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm* timeinfo = localtime(&rawTime);
  
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  
  return String(buffer);
}
