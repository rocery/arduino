// ---------------------------------------------------------------------------
// Example NewPing library sketch that does a ping about 20 times per second.
// ---------------------------------------------------------------------------

#include <NewPing.h>

#define TRIGGER_PIN  2  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     3  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 400 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
int IRValue;
int count;
int cond;

float cons = 30.0;
float conss = 31.0;

 int counter = 0;
 int currentState = 0;
 int previousState = 0;
 
void setup() {
  Serial.begin(9600); // Open serial monitor at 115200 baud to see ping results.
}

void loop() {
  delay(50);                     // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  Serial.print("Ping: ");
  Serial.println(sonar.ping_cm()); 
  hitungBarang();
}

void hitungBarang() {
  IRValue = sonar.ping_cm();
  if (IRValue > 30) {
    Serial.println("30");
  }
//  if (IRValue <= 60){  
//  currentState = 1;
//  }
//  else {
//  currentState = 0;
//  }
//  delay(200);
//  if(currentState != previousState){
//    if(currentState == 1){
//      counter = counter + 1;
//      
//      Serial.println(counter);
//    }
//  }
}
