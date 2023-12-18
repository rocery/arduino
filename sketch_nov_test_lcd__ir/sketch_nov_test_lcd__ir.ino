#include <LiquidCrystal_I2C.h>

//set the LCD address to 0x3F/0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);  
int totalBarang;
boolean barang;
int IRValue;
int count = 0;
int cond = 0;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  lcd.setCursor(2, 1);
  lcd.print("Total Barang :");
  lcd.setCursor(2, 2);
  lcd.print(0);
}
 
void loop() {
  hitungBarang();
}

void hitungBarang() {
  IRValue = analogRead(A0);
//  Serial.println(IRValue);
  if (IRValue > 100) {
//    Serial.println(IRValue);
    count = count;
    cond = 0;
  } else if(IRValue < 100 && cond == 0) {
    count += 1;
    cond = 1;
  } else if(IRValue < 100 && cond == 1) {
    count = count;
    cond = 1;
  }
//  Serial.println(count); 
  lcd.setCursor(2, 1);
  lcd.print("Total Barang: ");
  lcd.setCursor(2, 2);
  lcd.print(count);
}
