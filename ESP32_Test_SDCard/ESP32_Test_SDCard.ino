#include "FS.h"
#include "SD.h"
#include "SPI.h"


int counter;
String line;
int lineAsInt;

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void readLastLineSDCard(const char *path) {
  File file = SD.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("Failed to open file for reading");
    return;
  }

  while (file.available()) {
    line = file.readStringUntil('\n');
  }
  file.close();

  lineAsInt = line.toInt();
  Serial.print("Last line: ");
  Serial.println(lineAsInt);
}

void insertLastLineSDCard(const char *path, String line) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  if (file.print(line)) {
    Serial.println("Line written");
  } else {
    Serial.println("Write failed");
  }

  file.close();
}

void deleteLog(const char *path) {
  if (SD.exists(path)) {
    SD.remove(path);
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed, file does not exist");
  }
}

void setup() {
  Serial.begin(115200);
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
}

void loop() {
//  counter++;
//  insertLastLineSDCard("log.txt", String(counter));
//  readLastLineSDCard("log.txt");
//  delay(100);
//  if (lineAsInt >= 100) {
//    deleteLog("/log.txt");
//    Serial.println("Log Deleted");
//    counter = 0;
//  }
}
