#include <Ethernet.h>
#include <SD.h>

// Pin CS untuk masing-masing perangkat
#define SD_CS 4       // SD Card
#define W5500_CS 5    // W5500

// Tentukan MAC address untuk W5500
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 

void setup() {
  Serial.begin(115200);

  // Inisialisasi W5500
  Serial.println("Inisialisasi Ethernet...");
  Ethernet.init(W5500_CS);  // Pin CS untuk W5500
  if (Ethernet.begin(mac) == 0) { // Gunakan MAC address
    Serial.println("Ethernet gagal diinisialisasi. Periksa kabel atau router.");
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
}

void loop() {
  // Tambahkan logika loop di sini jika diperlukan
}
