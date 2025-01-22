#include <WiFi.h>
#include <WiFiMulti.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFiMulti object
WiFiMulti wifiMulti;

// Button pins
const int upButton = 35;
const int downButton = 34;
const int selectButton = 26;

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust I2C address if needed

// Device Configuration
const int deviceID = 7;

// Network Configuration
IPAddress staticIP(192, 168, 7, deviceID);
IPAddress gateway(192, 168, 7, 250);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// WiFi Credentials
const int WIFI_NETWORKS = 6;
const char* wifiSSIDs[WIFI_NETWORKS] = {
  "STTB11",
  "WiFi2",
  "WiFi3",
  "WiFi4",
  "WiFi5",
  "WiFi6"
};

const char* wifiPasswords[WIFI_NETWORKS] = {
  "Si4nt4r321",
  "Pass2",
  "Pass3",
  "Pass4",
  "Pass5",
  "Pass6"
};

// Menu variables
const int MENU_ITEMS = 4;
const String menuItems[MENU_ITEMS] = {
  "Produk 1",
  "Produk 2",
  "Produk 3",
  "Produk 4"
};

const String productCodes[MENU_ITEMS] = {
  "P001",  // Code for Produk 1
  "P002",  // Code for Produk 2
  "P003",  // Code for Produk 3
  "P004"   // Code for Produk 4
};

int currentMenuItem = 0;
String selectedProductCode = "";
String selectedProductName = "";
bool productSelected = false;
bool wifiConnected = false;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Set button pins as INPUT_PULLUP
  pinMode(upButton, INPUT);
  pinMode(downButton, INPUT);
  pinMode(selectButton, INPUT);

  // Configure static IP
  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Static IP configuration failed");
  }

  // Add WiFi networks to WiFiMulti
  for (int i = 0; i < WIFI_NETWORKS; i++) {
    wifiMulti.addAP(wifiSSIDs[i], wifiPasswords[i]);
  }

  // Product Selection Process
  currentMenuItem = 0;
  selectProductBeforeLoop();

  // WiFi Connection Process
  connectToWiFi();
}

void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi...");
  Serial.println("Attempting to connect to WiFi...");

  // Attempt to connect to the best available network
  int attempts = 0;
  while (wifiMulti.run() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;

    // Update LCD with connection progress
    lcd.setCursor(0, 1);
    lcd.print("Connecting");
  }

  // Check connection status
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());

    Serial.println("\nWiFi connected");
    Serial.print("Connected to: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    delay(2000);
  } else {
    // Connection failed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("No connection");

    Serial.println("\nWiFi connection failed");
    delay(2000);
  }
}

void loop() {
  // Check if product and WiFi are selected
  if (!productSelected || !wifiConnected) {
    if (!productSelected) selectProductBeforeLoop();
    if (!wifiConnected) connectToWiFi();
    return;
  }

  // Simply display selected product info
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Produk: " + selectedProductName);
  lcd.setCursor(0, 1);
  lcd.print("IP: " + WiFi.localIP().toString());

  // WiFi connection is maintained
  // You can add your main application logic here

  // Periodically check WiFi connection
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    wifiConnected = false;
    connectToWiFi();
  }

  // Add a small delay to keep the display readable
  delay(1000);
}

void selectProductBeforeLoop() {
  // Ensure menu is displayed immediately
  displayMenu();
  
  bool inSelection = true;
  int lastMenuItem = currentMenuItem;

  while (inSelection) {
    // Up button
    if (digitalRead(upButton) == HIGH) {
      delay(50);  // Debounce
      if (digitalRead(upButton) == HIGH) {
        // Move up, wrapping around to the bottom if at the top
        currentMenuItem = (currentMenuItem - 1 + MENU_ITEMS) % MENU_ITEMS;
        
        // Update display
        displayMenu();
        
        while (digitalRead(upButton) == HIGH);  // Wait for button release
      }
    }

    // Down button
    if (digitalRead(downButton) == HIGH) {
      delay(50);  // Debounce
      if (digitalRead(downButton) == HIGH) {
        // Move down, wrapping around to the top if at the bottom
        currentMenuItem = (currentMenuItem + 1) % MENU_ITEMS;
        
        // Update display
        displayMenu();
        
        while (digitalRead(downButton) == HIGH);  // Wait for button release
      }
    }

    // Select button
    if (digitalRead(selectButton) == HIGH) {
      delay(50);  // Debounce
      if (digitalRead(selectButton) == HIGH) {
        // Store selected product code and name
        selectedProductCode = productCodes[currentMenuItem];
        selectedProductName = menuItems[currentMenuItem];
        productSelected = true;

        // Show selection confirmation
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selected:");
        lcd.setCursor(0, 1);
        lcd.print(selectedProductName);

        // Debug print
        Serial.print("Selected Product: ");
        Serial.print(selectedProductName);
        Serial.print(" (Code: ");
        Serial.print(selectedProductCode);
        Serial.println(")");

        // Wait a moment to show selection
        delay(1500);

        // Exit selection loop
        inSelection = false;
      }
    }
  }
}

void displayMenu() {
  // Ensure the display is clear before showing menu
  lcd.clear();

  // First line with arrow
  lcd.setCursor(0, 0);
  lcd.print("> " + menuItems[currentMenuItem]);

  // Second line with next item
  if (currentMenuItem + 1 < MENU_ITEMS) {
    lcd.setCursor(0, 1);
    lcd.print("  " + menuItems[currentMenuItem + 1]);
  } else {
    // If at the last item, show the first item
    lcd.setCursor(0, 1);
    lcd.print("  " + menuItems[0]);
  }
}
