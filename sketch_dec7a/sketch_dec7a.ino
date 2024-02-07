#define counter 21

void setup() {
  Serial.begin(115200);
  pinMode(21, INPUT);
  
}

void loop() {
  if(digitalRead(21) == LOW) {
    Serial.println("LOW");
  } else {
    Serial.println("HIGH");
  }
}
