#include <UIPEthernet.h>
#include <SPI.h>
#include <HTTPClient.h>

// Konfigurasi jaringan statis
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Alamat MAC perangkat
IPAddress ip(192, 168, 7, 31);  // IP statis
IPAddress gateway(192, 168, 15, 250);  // Gateway
IPAddress subnet(255, 255, 0, 0);  // Subnet mask

const char* serverName = "http://192.168.7.223/iot/api/weigher/save_weigher_test.php";  // URL tujuan untuk menerima data

String deviceID = "ESP32_001";
String ESPName = "ESP32 Weigher";
String productSelected = "Product1";
// String dateTime = "2024-12-05 12:00:00";
String ip_Address = "192.168.7.31";
String postData;

const char* serverAddress = "192.168.7.223";
const int serverPort = 80;

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi Ethernet dengan IP statis
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  
  // Tampilkan informasi jaringan
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(Ethernet.gatewayIP());

  // Mengirim data
  // sendData();
}

void loop() {
  
  // Ethernet.maintain();
  float weigher = random(1, 11);
  String kgLoadCellPrint = String(weigher, 2);

  // Menyiapkan data untuk dikirimkan dalam format POST
  postData = "device_id=" + deviceID + "&device_name=" + ESPName + "&product=" + productSelected + 
                    "&weight=" + kgLoadCellPrint + "&ip_address=" + ip_Address;
  
  Serial.println(postData);
  kirimDataKeServer();
  delay(60000);
}

void kirimDataKeServer() {
  EthernetClient client;
  
  // // Buat payload
  // String postData = "weight=" + String(dataBobot, 2); // 2 decimal places
  
  // Coba koneksi
  if (client.connect(serverAddress, serverPort)) {
    Serial.println("Terhubung ke server");
    
    // Kirim HTTP POST Request
    client.println("POST /iot/api/weigher/save_weigher_test.php HTTP/1.1");
    client.println("Host: 192.168.7.223");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);
    
    Serial.println("Data terkirim:");
    Serial.println(postData);
    
    // Tunggu respon
    while(client.connected()) {
      if(client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }
    
    client.stop();
  } else {
    Serial.println("Koneksi gagal");
  }
}

