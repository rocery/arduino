int t = 1;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  Serial.print("test");
  Serial.println(t);
  t++;
  delay(100);
}
