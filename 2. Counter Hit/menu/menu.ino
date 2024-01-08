#include <LiquidCrystal_I2C.h>
#include <Wire.h>


LiquidCrystal_I2C lcd(0x27, 20, 4);

String productCode, productName;
String productCodeOne = "P-0722-00239";
String productCodeTwo = "P-0xxx-xxxxx";
String productCodeThree = "P-0xxx-xxxxx";
String productCodeFour = "P-0xxx-xxxxx";
String productCodeFive = "P-0xxx-xxxxx";
String productCodeSix = "P-0xxx-xxxxx";

String productNameOne = "Tic Tic Bwg 2000";
String productNameTwo = "Tic Tic Bwg 2000";
String productNameThree = "Tic Tic Bwg 2000";
String productNameFour = "Tic Tic Bwg 2000";
String productNameFive = "Tic Tic Bwg 2000";
String productNameSix = "Tic Tic Bwg 2000";

int upButton = 10;
int downButton = 11;
int selectButton = 12;
int menu = 0;
bool menuStatus;

void setup() {
  lcd.init();
  lcd.clear();
  lcd.backlight();

  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);

  menuStatus = false;
  updateMenu();
}

void loop() {
  while (!menuStatus) {
    if (!digitalRead(downButton)) {
      menu++;
      updateMenu();
      delay(100);
      while (!digitalRead(downButton));
    }
    if (!digitalRead(upButton)) {
      menu--;
      updateMenu();
      delay(100);
      while (!digitalRead(upButton));
    }
    if (!digitalRead(selectButton)) {
      executeAction();
      // updateMenu();
      delay(100);
      menuStatus = true;
      while (!digitalRead(selectButton));
    }
  }
}

void updateMenu() {
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pilih Produk : ");
      lcd.setCursor(0, 1);
      lcd.print(">" + productNameOne);
      lcd.setCursor(0, 2);
      lcd.print(productNameTwo);
      lcd.setCursor(0, 3);
      lcd.print(productNameThree);
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pilih Produk : ");
      lcd.setCursor(0, 1);
      lcd.print(productNameOne);
      lcd.setCursor(0, 2);
      lcd.print(">" + productNameTwo);
      lcd.setCursor(0, 3);
      lcd.print(productNameThree);
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pilih Produk : ");
      lcd.setCursor(0, 1);
      lcd.print(productNameOne);
      lcd.setCursor(0, 2);
      lcd.print(productNameTwo);
      lcd.setCursor(0, 3);
      lcd.print(">" + productNameThree);
      break;
    case 4:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pilih Produk : ");
      lcd.setCursor(0, 1);
      lcd.print(productNameTwo);
      lcd.setCursor(0, 2);
      lcd.print(productNameThree);
      lcd.setCursor(0, 3);
      lcd.print(">" + productNameFour);
      break;
    case 5:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pilih Produk : ");
      lcd.setCursor(0, 1);
      lcd.print(productNameThree);
      lcd.setCursor(0, 2);
      lcd.print(productNameFour);
      lcd.setCursor(0, 3);
      lcd.print(">" + productNameFive);
      break;
    case 6:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pilih Produk : ");
      lcd.setCursor(0, 1);
      lcd.print(productNameFour);
      lcd.setCursor(0, 2);
      lcd.print(productNameFive);
      lcd.setCursor(0, 3);
      lcd.print(">" + productNameSix);
      break;
    case 7:
      menu = 6;
      break;
  }
}

void executeAction() {
  switch (menu) {
    case 1:
      productCode = productCodeOne;
      productName = productNameOne;
      break;
    case 2:
      productCode = productCodeTwo;
      productName = productNameTwo;
      break;
    case 3:
      productCode = productCodeThree;
      productName = productNameThree;
      break;
    case 4:
      productCode = productCodeFour;
      productName = productNameFour;
      break;
    case 5:
      productCode = productCodeFive;
      productName = productNameFive;
      break;
    case 6:
      productCode = productCodeSix;
      productName = productNameSix;
      break;
  }
}
