const int SensorProximty = D5;

void setup()
{
  Serial.begin(9600);
  pinMode(SensorProximty, INPUT);
}

void loop()
{
  int hasil = digitalRead(SensorProximty);
  if(hasil == LOW)
  {
    Serial.println("Ada Halangan");
    Serial.println(SensorProximty);
  }
  if(hasil == HIGH)
  {
    Serial.println("Aman, Tidak Ada Halangan");
    Serial.println(SensorProximty);
  }
  delay(250);
}
