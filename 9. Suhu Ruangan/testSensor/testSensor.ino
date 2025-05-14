#include <DHT.h>

#define DHTPIN 33
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(2500);
}

void loop() {
  // Read the temperature from the DHT sensor
  float temperature = dht.readTemperature();

  // Read the humidity from the DHT sensor
  float humidity = dht.readHumidity();

  Serial.println(temperature);
  Serial.println(humidity);

  delay(3000);
}