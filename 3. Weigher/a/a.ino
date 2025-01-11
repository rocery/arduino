#include <SPI.h>
#include <Ethernet.h>
#include <NTPClient.h>
#include <EthernetUdp.h>

// Ethernet settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Replace with your MAC address
IPAddress ip(192, 168, 7, 100);                     // Replace with your ESP32's IP
IPAddress ntpServer(192, 168, 7, 223);              // NTP Server IP
unsigned int localPort = 2390;                      // Local UDP port for NTP

// EthernetUDP and NTPClient objects
EthernetUDP udp;
NTPClient ntpClient(udp, ntpServer, 0, 60000); // UTC time, update every 60 seconds

void setup() {
  Serial.begin(115200);
  
  // Start Ethernet
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip); // Use static IP
  }
  delay(1000);

  // Print Ethernet details
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
  
  // Begin UDP for NTP
  udp.begin(localPort);
  ntpClient.begin();
}

void loop() {
  ntpClient.update(); // Update time from NTP server

  // Get epoch time
  unsigned long epochTime = ntpClient.getEpochTime();

  // Convert to readable format
  String formattedTime = getFormattedDateTime(epochTime);

  // Print formatted time
  Serial.println(formattedTime);

  delay(1000); // Wait 1 second before the next update
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

  return String(buffer);
}
