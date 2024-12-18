/*
ESP32 Pin	| W5500 Pin	| SD Card Pin
GPIO23	    MOSI	      MOSI
GPIO19	    MISO	      MISO
GPIO18	    SCK	        SCK
GPIO5	      CS	
GPIO4		                CS
3.3V	      VCC	        VCC
GND	GND	GND
*/

#include <Ethernet.h>
#include <SD.h>

// Pin CS untuk masing-masing perangkat
#define SD_CS 4       // SD Card
#define W5500_CS 5    // W5500

void setup() {
  Serial.begin(115200);

  // Inisialisasi W5500
  Serial.println("Inisialisasi Ethernet...");
  Ethernet.init(W5500_CS);  // Pin CS untuk W5500
  if (Ethernet.begin() == 0) {
    Serial.println("Ethernet gagal diinisialisasi.");
  } else {
    Serial.print("Ethernet berhasil diinisialisasi. IP: ");
    Serial.println(Ethernet.localIP());
  }

  // Inisialisasi SD Card
  Serial.println("Inisialisasi SD Card...");
  if (!SD.begin(SD_CS)) {   // Pin CS untuk SD Card
    Serial.println("SD Card gagal diinisialisasi.");
  } else {
    Serial.println("SD Card berhasil diinisialisasi!");
  }

  delay(5000);
}

void loop() {
  // Mengakses SD Card
  File file = SD.open("/example.txt", FILE_WRITE);
  if (file) {
    file.println("Menulis data ke SD Card...");
    file.close();
    Serial.println("Data berhasil ditulis ke SD Card.");
  } else {
    Serial.println("Gagal membuka file di SD Card.");
  }

  delay(2000);

  // Mengakses W5500
  Serial.println("Mengirim data ke server melalui W5500...");
  EthernetClient client;
  if (client.connect("example.com", 80)) {
    client.println("GET / HTTP/1.1");
    client.println("Host: example.com");
    client.println("Connection: close");
    client.println();
    Serial.println("Permintaan HTTP berhasil dikirim.");
  } else {
    Serial.println("Gagal terhubung ke server.");
  }

  Serial.println(Ethernet.localIP());

  delay(5000);
}
