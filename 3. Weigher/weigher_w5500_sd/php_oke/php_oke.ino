#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// W5500 Ethernet settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
const char* serverIP = "192.168.15.223";
const int serverPort = 80;
const int chipSelect = 4;
const int CHUNK_SIZE = 512;
uint8_t buffer[CHUNK_SIZE];

EthernetClient client;

void sendLogFile() {
  File logFile = SD.open("/log31.txt", FILE_READ);
  if (!logFile) {
    Serial.println("Error opening log31.txt");
    return;
  }

  long fileSize = logFile.size();
  Serial.print("File size: ");
  Serial.println(fileSize);

  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server");

    String boundary = "------------------------abcdef123456";
    
    // Start the multipart form
    client.println("POST /iot/api/weigher/upload.php HTTP/1.1");  // Changed to .php
    client.println("Host: " + String(serverIP));
    client.println("Content-Type: multipart/form-data; boundary=" + boundary);
    
    // Calculate content length
    long contentLength = 0;
    contentLength += String("--" + boundary + "\r\n").length();
    contentLength += String("Content-Disposition: form-data; name=\"file\"; filename=\"log31.txt\"\r\n").length();
    contentLength += String("Content-Type: text/plain\r\n\r\n").length();
    contentLength += fileSize;
    contentLength += String("\r\n--" + boundary + "--\r\n").length();
    
    client.println("Content-Length: " + String(contentLength));
    client.println();
    
    // Send multipart form header
    client.print("--" + boundary + "\r\n");
    client.print("Content-Disposition: form-data; name=\"file\"; filename=\"log31.txt\"\r\n");
    client.print("Content-Type: text/plain\r\n\r\n");
    
    // Send file in chunks
    long totalSent = 0;
    while (logFile.available()) {
      int bytesRead = logFile.read(buffer, CHUNK_SIZE);
      if (bytesRead > 0) {
        client.write(buffer, bytesRead);
        totalSent += bytesRead;
        Serial.print("Sent bytes: ");
        Serial.println(totalSent);
      }
    }
    
    // Send multipart form footer
    client.print("\r\n--" + boundary + "--\r\n");
    
    // Wait for response
    Serial.println("Waiting for response...");
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 10000) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    
    Serial.println("File sent successfully!");
  } else {
    Serial.println("Connection failed");
  }

  client.stop();
  logFile.close();
}

void setup() {
  Serial.begin(115200);
  
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
  Serial.println("SD card initialized.");

  Ethernet.init(5);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    while (1);
  }
  Serial.println("Ethernet initialized.");
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
  
  delay(1000);
  
  // Send file once
  sendLogFile();
  
  Serial.println("Task completed. Device will now halt.");
  while(1); // Stop execution after sending
}

void loop() {
  // Empty loop - all work done in setup
}