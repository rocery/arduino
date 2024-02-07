#define button1 18
#define button2 19
#define button3 23
#define button4 5


void setup() {
  Serial.begin(9600);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  Serial.println("Reseted");
}

void loop() {
  digitalRead(button1);
  digitalRead(button2);
  digitalRead(button3);
  digitalRead(button4);
  
  if (digitalRead(button1) == LOW){
    Serial.println("Tombol1 Ditekan");
  }
  if (digitalRead(button2) == LOW){
    Serial.println("Tombol2 Ditekan");
  }
  if (digitalRead(button3) == LOW){
    Serial.println("Tombol3 Ditekan");
  }
  if (digitalRead(button4) == LOW){
    ESP.restart();
  }
}
