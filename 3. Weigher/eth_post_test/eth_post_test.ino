#include <UIPEthernet.h>

// Konfigurasi jaringan
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Alamat MAC perangkat
IPAddress ip(192, 168, 7, 31);                      // IP statis
IPAddress gateway(192, 168, 15, 250);                // Gateway
IPAddress subnet(255, 255, 0, 0);                 // Subnet mask

EthernetServer server(80); // Membuat server di port 80

void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi ENC28J60 dengan IP statis...");

  // Memulai Ethernet dengan IP statis
  Ethernet.begin(mac, ip, gateway, gateway, subnet);

  // Tampilkan alamat IP
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());

  server.begin();
}

void loop() {
  // Tunggu koneksi klien
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Klien terhubung");
    client.println("ESP32 dengan ENC28J60 berhasil terkoneksi!");
    client.stop();
  }
}
