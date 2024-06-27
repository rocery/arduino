/*
  V. 0.0.1 Alpa
  Update Terakhir : 27-06-2024

  Komponen:
  1. ESP32 Doit V1
  2. SIM 800L
  3. GPS Neo-8M

  Algoritma:
  Setup:
    Setup SIM -> Setup GPS
  Main:
    Pembacaan GPS -> Waktu kirim ? pembacaan GPS, kirim data

  
  Perjenalan singkat tentang pin UART:
  ESP32 memiliki 3 Serial UART, Serial 0, 1, 2
  Serial 0 tidak bisa dipakai karena sudah terhubung ke loader program
  Serial 1 : Bebas, contoh 13, 14
  Serial 2 : Bebas, biasanya menggunakan pin 16, 17
  Penjelasan lengkap: https://mikroavr.com/pin-out-esp32/ 
*/

// ============ INISIALISASI LIBRARY ============
#include <HardwareSerial.h>
#include <TinyGPS++.h>

// ============ GPS ============
TinyGPSPlus gps;

// ============ Hardware Serial ============
// Gunakan Hardware Serial nomor 1 pada ESP32 yaitu pin 13, 14
HardwareSerial gpsSerial(1);
// Gunakan Hardware Serial nomor 2 pada ESP32 yaitu pin 16, 17
HardwareSerial simSerial(2);

/**
 * Initializes the SIM800L module.
 *
 * This function sends a series of AT commands to the SIM800L module to initialize it.
 * The commands check the SIM card status, registration status, attach status,
 * shut down the IP connection, set the multiplexer mode to 0,
 * set the APN, bring up the IP connection, and get the local IP address.
 */
void sim800lInit() {
  // Check SIM card status
  sendCommand("AT", 1000, "OK");
  // Check SIM card readiness
  sendCommand("AT+CPIN?", 1000, "READY");
  // Check network registration status
  sendCommand("AT+CREG?", 1000, "OK");
  // Check attach status
  sendCommand("AT+CGATT?", 1000, "OK");
  // Shut down the IP connection
  sendCommand("AT+CIPSHUT", 1000, "SHUT OK");
  // Set the multiplexer mode to 0
  sendCommand("AT+CIPMUX=0", 1000, "OK");
  // Set the access point name (APN), username, and password
  sendCommand("AT+CSTT=\"your_apn\",\"your_username\",\"your_password\"", 1000, "OK");
  // Bring up the IP connection
  sendCommand("AT+CIICR", 1000, "OK");
  // Get the local IP address
  sendCommand("AT+CIFSR", 1000, ".");
}

/**
 * Sends data to a server using the SIM800L module.
 *
 * @param data The data to send.
 */
void sendDataToServer(String data) {
  // Start TCP connection to server
  sendCommand("AT+CIPSTART=\"TCP\",\"your_server_address\",\"your_server_port\"", 3000, "OK");
  delay(2000); // Wait for connection to establish

  // Set the length of the data to send
  String cmd = "AT+CIPSEND=" + String(data.length() + 4);

  // Send the command to send data
  sendCommand(cmd.c_str(), 1000, ">");

  // Send the data
  sim800l.print(data);

  // Send the end of data signal
  sim800l.write(26); // Send Ctrl+Z

  delay(5000); // Wait for data to send

  // Close the TCP connection
  sendCommand("AT+CIPCLOSE", 1000, "OK");}

/**
 * Sends a command to the SIM800L module and waits for an expected response.
 *
 * @param cmd The command to send.
 * @param timeout The maximum time to wait for a response.
 * @param expected The response to wait for.
 */
void sendCommand(const char* cmd, int timeout, const char* expected) {
  // Send the command to the SIM800L module
  sim800l.println(cmd);

  // Record the start time
  long int startTime = millis();

  // Wait for the expected response or until the timeout is reached
  while ((startTime + timeout) > millis()) {
    // Check if there is data available from the SIM800L module
    while (sim800l.available()) {
      // Read the response as a string
      String response = sim800l.readString();

      // Check if the expected response is in the response
      if (response.indexOf(expected) != -1) {
        // If the expected response is found, return
        return;
      }
    }
  }

  // If the command times out, print an error message
  Serial.print("Command timed out: ");
  Serial.println(cmd);
}

/**
 * Sets up the hardware and initializes the SIM800L module.
 *
 * This function initializes the serial communication with the ESP32 and
 * configures the SIM800L module. It sets the data rate to 9600 baud and
 * uses the SERIAL_8N1 mode. The RX pin is connected to pin 17 and the TX pin
 * is connected to pin 16.
 */
void setup() {
  // Initialize serial communication with the ESP32
  Serial.begin(115200);

  // Initialize serial communication with the SIM800L module
  // Set the data rate to 9600 baud and use the SERIAL_8N1 mode
  // Pin RX SIM800L is connected to pin 17
  // Pin TX SIM800L is connected to pin 16
  simSerial.begin(9600, SERIAL_8N1, 16, 17);

  // Initialize serial communication with the GPS module
  // Set the data rate to 9600 baud and use the SERIAL_8N1 mode
  // Pin RX GPS is connected to pin 14
  // Pin TX GPS is connected to pin 13
  gpsSerial.begin(9600, SERIAL_8N1, 13, 14);

  // Initialize the SIM800L module
  sim800Linit();
  
}

void loop() {
  // put your main code here, to run repeatedly:

}