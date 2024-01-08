#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
#define upButton 26
#define downButton 32
#define selectButton 35
#define resetButton 34

bool menuSelect;
int menu = 1;

String productSelected;
String nameProductSelected;
String productCodeOne = "P-0722-00239";
String productCodeTwo = "2";
String productCodeThree = "3";
String productCodeFour = "4";
String nameProductOne = "Tic Tic Bwg 2000";
String nameProductTwo = "b";
String nameProductThree = "c";
String nameProductFour = "d";


void setup() {
  Serial.begin(115200);

  pinMode(upButton, INPUT);
  pinMode(downButton, INPUT);
  pinMode(selectButton, INPUT);
  pinMode(resetButton, INPUT);

  // LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  updateMenu();

}

void loop() {
  while (menuSelect == false) {
    if (digitalRead(downButton)) {
      menu++;
      updateMenu();
      delay(100);
      while (digitalRead(downButton));
    }
    if (digitalRead(upButton)) {
      menu--;
      updateMenu();
      delay(100);
      while (digitalRead(upButton));
    }
    if (digitalRead(selectButton)) {
      delay(100);
      while (digitalRead(selectButton));

      menuSelect = true;
      menuSelected();
    }
  }

  if (menuSelect == true) {
    lcd.setCursor(1, 0);
    lcd.print(productSelected);
    lcd.setCursor(1, 1);
    lcd.print(nameProductSelected);
  }

  if (digitalRead(resetButton)) {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Resetting..");
    delay(1000);
    ESP.restart();
  }
  // Wifi
}

void menuSelected() {
  switch (menu) {
    case 1:
      productSelected = productCodeOne;
      nameProductSelected = nameProductOne;
      delay(1000);
      lcd.clear();
      break;
    case 2:
      productSelected = productCodeTwo;
      nameProductSelected = nameProductTwo;
      delay(1000);
      lcd.clear();
      break;
    case 3:
      productSelected = productCodeThree;
      nameProductSelected = nameProductThree;
      delay(1000);
      lcd.clear();
      break;
    case 4:
      productSelected = productCodeFour;
      nameProductSelected = nameProductFour;
      delay(1000);
      lcd.clear();
      break;
  }
}

void updateMenu() {
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Pilih Menu :");
      lcd.setCursor(1, 1);
      lcd.print("> " + nameProductOne);
      lcd.setCursor(1, 2);
      lcd.print("Produk 2");
      lcd.setCursor(1, 3);
      lcd.print("Produk 3");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Pilih Menu :");
      lcd.setCursor(1, 1);
      lcd.print("Produk 1");
      lcd.setCursor(1, 2);
      lcd.print("> Produk 2");
      lcd.setCursor(1, 3);
      lcd.print("Produk 3");
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Pilih Menu :");
      lcd.setCursor(1, 1);
      lcd.print("Produk 1");
      lcd.setCursor(1, 2);
      lcd.print("Produk 2");
      lcd.setCursor(1, 3);
      lcd.print("> Produk 3");
      break;
    case 4:
      menu = 3;
      break;
  }
}
