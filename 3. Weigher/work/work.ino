/**
 *
 * HX711 library for Arduino - example file
 * https://github.com/bogde/HX711
 *
 * MIT License
 * (c) 2018 Bogdan Necula
 *
**/
#include "HX711.h"


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 26;
const int LOADCELL_SCK_PIN = 27;


HX711 scale;

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 Demo");

  Serial.println("Initializing the scale");

  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());			// print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  	// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided
						// by the SCALE parameter (not set yet)

  scale.set_scale(46.19f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();				        // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
						// by the SCALE parameter set with set_scale

  Serial.println("Readings:");
}

void loop() {
  delay(100);
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  float a = scale.get_units(10);
  Serial.print(a, 1);
  float kg = a/1000;
  Serial.print("\tkg:\t");
  Serial.println(kg, 2);

  scale.power_down();
  delay(1000);
  scale.power_up();
}



void sendLog(void* parameter) {
  while (true) {
    if (isButtonPressed(buttonSelect)) {
      lcd.clear();
      while (isButtonPressed(buttonSelect)) {
          lcd.setCursor(0, 0);
          lcd.print("  SIMPAN DATA  ");
          vTaskDelay(pdMS_TO_TICKS(100));  // Prevent blocking in FreeRTOS
      }

      if (!sdStatus) {
          if (lanStatus == "D") {
              lcd.setCursor(0, 1);
              lcd.print("X, LAN ERROR");
              sendDataCounterFailed++;
              vTaskDelay(pdMS_TO_TICKS(1000));  // Replaced delay with FreeRTOS delay
          } else {
              postData = "device_id=" + deviceID + "&device_name=" + ESPName + "&product=" + productSelected + "&weight=" + String(kgLoadCellPrint) + "&ip_address=" + ip_Address + "&wifi=" + "LAN";
              if (!sendData()) {
                  lcd.setCursor(0, 1);
                  lcd.print("X, HUBUNGI IT");
                  sendDataCounterFailed++;
                  vTaskDelay(pdMS_TO_TICKS(1000));  // Replaced delay with FreeRTOS delay
              } else {
                  lcd.setCursor(0, 1);
                  lcd.print(" BERHASIL : ");
                  sendDataCounter++;
                  lcd.print(sendDataCounter);
              }
          }
      } else {
          postData = deviceID + ',' + ESPName + ',' + productSelected + ',' + String(kgLoadCellPrint) + ',' + ip_Address + ',' + "LAN";

          if (!appendLog(logName, postData.c_str())) {
              lcd.setCursor(0, 1);
              lcd.print("DATA GGL DISAVE");
              saveDataConterFailed++;
              vTaskDelay(pdMS_TO_TICKS(1000));  // Replaced delay with FreeRTOS delay
          } else {
              saveDataConter++;
          }
      }
    }

    File logFile = SD.open(logName, FILE_READ);
    if (!logFile) {
      continue;
    }

    long fileSize = logFile.size();
    Serial.print("File size: ");
    Serial.println(fileSize);

    if (client.connect(serverAddress, serverPort) && checkLog(logName) && fileSize > 0) {
      Serial.println("Connected to server");

      String boundary = "------------------------abcdef123456";

      // Improved multipart form data headers
      client.println("POST /weigher/upload_log_weigher HTTP/1.1");
      client.println("Host: " + String(serverAddress));
      client.println("Content-Type: multipart/form-data; boundary=" + boundary);

      // Calculate content length more precisely
      long contentLength =
        String("--" + boundary + "\r\n").length() + String("Content-Disposition: form-data; name=\"file\"; filename=\"weigherLog31.txt\"\r\n").length() + String("Content-Type: text/plain\r\n\r\n").length() + fileSize + String("\r\n--" + boundary + "--\r\n").length();

      client.println("Content-Length: " + String(contentLength));
      client.println("Connection: close");
      client.println();

      // Write multipart form data
      client.println("--" + boundary);
      client.println("Content-Disposition: form-data; name=\"file\"; filename=\"weigherLog31.txt\"");
      client.println("Content-Type: text/plain");
      client.println();

      // Send file in chunks
      while (logFile.available()) {
        int bytesRead = logFile.read(buffer, CHUNK_SIZE);
        if (bytesRead > 0) {
          client.write(buffer, bytesRead);
        }
      }

      // Properly close multipart form
      client.println();
      client.println("--" + boundary + "--");

      // Enhanced response handling
      unsigned long timeout = millis();
      String response = "";
      int statusCode = 0;
      bool headersComplete = false;
      String responseBody = "";

      while (client.connected() && millis() - timeout < 10000) {
        if (client.available()) {
          String line = client.readStringUntil('\n');

          // Parse HTTP status code
          if (line.startsWith("HTTP/1.1")) {
            statusCode = line.substring(9, 12).toInt();
            Serial.print("Server Response Status Code: ");
            Serial.println(statusCode);
          }

          // Collect headers
          if (!headersComplete) {
            response += line;

            // Check for end of headers
            if (line.length() <= 2) {
              headersComplete = true;
              Serial.println("Headers complete");
            }
          }
          // Collect response body
          else {
            responseBody += line;
          }
        }

        // Break if no more data and headers are complete
        if (!client.available() && headersComplete) {
          break;
        }
      }

      // Verify upload success
      if (statusCode == 200) {
        Serial.println("File upload successful");
        Serial.println("Server Response Headers:");
        Serial.println("==================");
        Serial.println(response);
        Serial.println("==================");
        Serial.println(responseBody);
    
      } else {
        Serial.print("File upload failed. Status code: ");
        Serial.println(statusCode);
        Serial.println("Response:");
        Serial.println(response);
      }
      Serial.println("File sent attempt completed");
      
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, responseBody);
      if (error) {
        Serial.println("JSON parsing failed");
      }
      const char* status = doc["status"];
      int jumlah_data = doc["jumlah_data"];
      Serial.print("Status: ");
      Serial.println(status);
      Serial.print("Jumlah data: ");
      Serial.println(jumlah_data);
      

      if (String(status) == "success") {
        sendLogCounter++;
        totalLineCount = totalLineCount + jumlah_data;
        deleteLog(logName);
      }
    
    } else {
      Serial.println("Connection failed");
    }

    client.stop();
    logFile.close();
    // Wait before next attempt
    vTaskDelay(30000 / portTICK_PERIOD_MS);  // 30 seconds delay
  } 
}




void sendLog(void* parameter) {
  unsigned long lastServerCheckTime = 0;
  const unsigned long SERVER_CHECK_INTERVAL = 30000; // 30 seconds
  while (true) {
    unsigned long currentTime = millis();
    // Check button press without blocking
    if (isButtonPressed(buttonSelect)) {
      lcd.clear();
      unsigned long buttonPressStartTime = millis();
      
      // Wait for button release with non-blocking approach
      while (isButtonPressed(buttonSelect)) {
        lcd.setCursor(0, 0);
        lcd.print("  SIMPAN DATA  ");
        vTaskDelay(pdMS_TO_TICKS(100));  // Prevent blocking in FreeRTOS
      }
      // Rest of your button press handling logic remains the same
      if (!sdStatus) {
        if (lanStatus == "D") {
          lcd.setCursor(0, 1);
          lcd.print("X, LAN ERROR");
          sendDataCounterFailed++;
          vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
          postData = "device_id=" + deviceID + "&device_name=" + ESPName + "&product=" + productSelected + "&weight=" + String(kgLoadCellPrint) + "&ip_address=" + ip_Address + "&wifi=" + "LAN";
          if (!sendData()) {
            lcd.setCursor(0, 1);
            lcd.print("X, HUBUNGI IT");
            sendDataCounterFailed++;
            vTaskDelay(pdMS_TO_TICKS(1000));
          } else {
            lcd.setCursor(0, 1);
            lcd.print(" BERHASIL : ");
            sendDataCounter++;
            lcd.print(sendDataCounter);
          }
        }
      } else {
        postData = deviceID + ',' + ESPName + ',' + productSelected + ',' + String(kgLoadCellPrint) + ',' + ip_Address + ',' + "LAN";
        if (!appendLog(logName, postData.c_str())) {
          lcd.setCursor(0, 1);
          lcd.print("DATA GGL DISAVE");
          saveDataConterFailed++;
          vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
          saveDataConter++;
        }
      }
    }
    // Periodic server check with non-blocking approach
    if (currentTime - lastServerCheckTime >= SERVER_CHECK_INTERVAL) {
      File logFile = SD.open(logName, FILE_READ);
      if (!logFile) {
        lastServerCheckTime = currentTime;
        continue;
      }
      long fileSize = logFile.size();
      Serial.print("File size: ");
      Serial.println(fileSize);
      if (client.connect(serverAddress, serverPort) && checkLog(logName) && fileSize > 0) {
        // Your existing file upload logic remains the same
        Serial.println("Connected to server");

        String boundary = "------------------------abcdef123456";

        // Improved multipart form data headers
        client.println("POST /weigher/upload_log_weigher HTTP/1.1");
        client.println("Host: " + String(serverAddress));
        client.println("Content-Type: multipart/form-data; boundary=" + boundary);

        // Calculate content length more precisely
        long contentLength =
          String("--" + boundary + "\r\n").length() + String("Content-Disposition: form-data; name=\"file\"; filename=\"weigherLog31.txt\"\r\n").length() + String("Content-Type: text/plain\r\n\r\n").length() + fileSize + String("\r\n--" + boundary + "--\r\n").length();

        client.println("Content-Length: " + String(contentLength));
        client.println("Connection: close");
        client.println();

        // Write multipart form data
        client.println("--" + boundary);
        client.println("Content-Disposition: form-data; name=\"file\"; filename=\"weigherLog31.txt\"");
        client.println("Content-Type: text/plain");
        client.println();

        // Send file in chunks
        while (logFile.available()) {
          int bytesRead = logFile.read(buffer, CHUNK_SIZE);
          if (bytesRead > 0) {
            client.write(buffer, bytesRead);
          }
        }

        // Properly close multipart form
        client.println();
        client.println("--" + boundary + "--");

        // Enhanced response handling
        unsigned long timeout = millis();
        String response = "";
        int statusCode = 0;
        bool headersComplete = false;
        String responseBody = "";

        while (client.connected() && millis() - timeout < 10000) {
          if (client.available()) {
            String line = client.readStringUntil('\n');

            // Parse HTTP status code
            if (line.startsWith("HTTP/1.1")) {
              statusCode = line.substring(9, 12).toInt();
              Serial.print("Server Response Status Code: ");
              Serial.println(statusCode);
            }

            // Collect headers
            if (!headersComplete) {
              response += line;

              // Check for end of headers
              if (line.length() <= 2) {
                headersComplete = true;
                Serial.println("Headers complete");
              }
            }
            // Collect response body
            else {
              responseBody += line;
            }
          }

          // Break if no more data and headers are complete
          if (!client.available() && headersComplete) {
            break;
          }
        }

        // Verify upload success
        if (statusCode == 200) {
          Serial.println("File upload successful");
          Serial.println("Server Response Headers:");
          Serial.println("==================");
          Serial.println(response);
          Serial.println("==================");
          Serial.println(responseBody);
      
        } else {
          Serial.print("File upload failed. Status code: ");
          Serial.println(statusCode);
          Serial.println("Response:");
          Serial.println(response);
        }
        Serial.println("File sent attempt completed");
        
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, responseBody);
        if (error) {
          Serial.println("JSON parsing failed");
        }
        const char* status = doc["status"];
        int jumlah_data = doc["jumlah_data"];
        Serial.print("Status: ");
        Serial.println(status);
        Serial.print("Jumlah data: ");
        Serial.println(jumlah_data);
        

        if (String(status) == "success") {
          sendLogCounter++;
          totalLineCount = totalLineCount + jumlah_data;
          deleteLog(logName);
        }

        lastServerCheckTime = currentTime;
      } else {
        Serial.println("Connection failed");
        lastServerCheckTime = currentTime;
      }
      client.stop();
      logFile.close();
    }
    // Small delay to prevent tight looping
    vTaskDelay(pdMS_TO_TICKS(100));
  } 
}