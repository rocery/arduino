


void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
}

void loop() {
  Serial.println(analogRead(2));
}
