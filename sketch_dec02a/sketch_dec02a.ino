const int interruptPin = D5;  // Example: Using digital pin D2 on NodeMCU
volatile bool interruptOccurred = false;
int counter;

int IRValue;
const int threshold = 200;
int count = 0;
bool cond = false;
unsigned long previousMillis = 0;
const long interval = 1000;  // Example: 1 second interval


void ICACHE_RAM_ATTR myInterruptFunction() {
  counter++;
}

void setup() {
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(interruptPin), myInterruptFunction, FALLING);
}

void loop() {
  unsigned long currentMillis = millis();
  IRValue = analogRead(A0);

  if (IRValue < threshold && !cond) {
    count++;
    cond = true;
  } else if (IRValue >= threshold) {
    cond = false;
  }
  // Timing using millis()
  if (currentMillis - previousMillis >= interval) {
    // Perform actions every interval (e.g., update database, print count)
    Serial.print("Count: ");
    Serial.println(count);

    // Reset the timer
    previousMillis = currentMillis;
  }
}
