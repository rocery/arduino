#include <SPI.h>
#include <Ethernet.h>

// Konfigurasi pin CS W5500
#define W5500_CS_PIN 5  // Pin D5 untuk Chip Select

// Alamat MAC untuk Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi SPI
  SPI.begin();
  
  // Set pin CS sebagai output
  pinMode(W5500_CS_PIN, OUTPUT);
  digitalWrite(W5500_CS_PIN, HIGH);
  
  // Mulai koneksi Ethernet
  Ethernet.begin(mac);
  
  // Cetak alamat IP
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Tambahkan logika program Anda di sini
}