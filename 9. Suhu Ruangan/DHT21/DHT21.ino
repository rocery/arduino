#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT21

float temperature, humidity;

DHT dht(DHTPIN, DHTTYPE);

void readDHT() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  readDHT();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");
  delay(2000);
}