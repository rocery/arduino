#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// W5500 Ethernet settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
const char* serverIP = "192.168.15.10";
const int serverPort = 5000;
const int chipSelect = 4;
const int CHUNK_SIZE = 512;
uint8_t buffer[CHUNK_SIZE];

EthernetClient client;

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
}

void loop() {
  File logFile = SD.open("/log31.txt", FILE_READ);
  if (!logFile) {
    Serial.println("Error opening log31.txt");
    delay(5000);
    return;
  }

  long fileSize = logFile.size();
  Serial.print("File size: ");
  Serial.println(fileSize);

  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server");

    String boundary = "boundary=------------------------abcdef123456";
    String delimiter = "--------------------------abcdef123456";
    
    // Start the multipart form
    client.println("POST /upload HTTP/1.1");
    client.println("Host: " + String(serverIP));
    client.println("Content-Type: multipart/form-data; " + boundary);
    
    // Calculate content length
    long contentLength = String(delimiter + "\r\n").length();
    contentLength += String("Content-Disposition: form-data; name=\"file\"; filename=\"log31.txt\"\r\n").length();
    contentLength += String("Content-Type: text/plain\r\n\r\n").length();
    contentLength += fileSize;
    contentLength += String("\r\n" + delimiter + "--\r\n").length();
    
    client.println("Content-Length: " + String(contentLength));
    client.println("Connection: close");
    client.println();
    
    // Send form data
    client.println(delimiter);
    client.println("Content-Disposition: form-data; name=\"file\"; filename=\"log31.txt\"");
    client.println("Content-Type: text/plain");
    client.println();
    
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
    
    // End the multipart form
    client.println();
    client.println(delimiter + "--");
    
    // Read response
    Serial.println("Waiting for response...");
    unsigned long timeout = millis();
    String response = "";
    while (client.connected() && millis() - timeout < 10000) {
      if (client.available()) {
        char c = client.read();
        response += c;
        if (response.indexOf("\r\n\r\n") != -1) {
          Serial.println("Response headers:");
          Serial.println(response);
          break;
        }
      }
    }
    
    Serial.println("File sent successfully!");
  } else {
    Serial.println("Connection failed");
  }

  client.stop();
  logFile.close();
  
  delay(60000);
}