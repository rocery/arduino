

void setup() {
  Serial.begin(115200);
  pinMode(34, INPUT);
  pinMode(35, INPUT);
  pinMode(32, INPUT);
  pinMode(26, INPUT);
  pinMode(25, INPUT);
}

void loop() {
  Serial.println("Tombol 1 = " + String(digitalRead(34)));
  Serial.println("Tombol 2 = " + String(digitalRead(35)));
  Serial.println("Tombol 3 = " + String(digitalRead(32)));
  Serial.println("Tombol 4 = " + String(digitalRead(26))); 
  Serial.println("IR Value = " + String(digitalRead(25)));
  delay(1000);
}
