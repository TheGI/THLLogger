// Just to test Git2
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include "DHT.h"
#include <DeepSleepScheduler.h>

#define redLEDpin 13
#define photocellPin A0
#define DHTTYPE DHT22
#define DHTPIN  7
#define aref_voltage 5.0

RTC_DS1307 RTC;
DHT dht(DHTPIN, DHTTYPE);
const int chipSelect = 4;
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  while (1) {
    digitalWrite(redLEDpin, HIGH);
    delay(300);
    digitalWrite(redLEDpin, LOW);
    delay(300);
  }
}

void logData() {
  if (logfile) {
    digitalWrite(redLEDpin, HIGH);
    DateTime now = RTC.now();
    double light = analogRead(photocellPin) * 5.0 * 200.0 / 1024.0;
    double temperature = dht.readTemperature();
    double humidity = dht.readHumidity();
    String dataToWrite = String(now.year(), DEC) + "/" +
                         String(now.month(), DEC) + "/" +
                         String(now.day(), DEC) + "," +
                         String(now.hour(), DEC) + ":" +
                         String(now.minute(), DEC) + ":" +
                         String(now.second(), DEC) + "," +
                         String(light) + "," +
                         String(temperature) + "," +
                         String(humidity);
    logfile.println(dataToWrite);
    logfile.flush();
    digitalWrite(redLEDpin, LOW);
    scheduler.scheduleDelayed(logData, 5000);
  }
  else {
    error("Error Writing To DataFile");
  }
}


void setup(void)
{
  DateTime now;

  // STEP 1: Set Up Serial and Pins
  Serial.begin(57600);
  while (!Serial) {

  }
  pinMode(redLEDpin, OUTPUT);

  // STEP 2: Initialize RTC
  Wire.begin();
  if (!RTC.begin()) {
    error("RTC Failed");
  }

  // STEP 3: Initialize SD Card
  Serial.println("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");

  // STEP 4: Create a new log file
  char filename[] = "THL00000.CSV";
  for (uint8_t i = 0; i < 100000; i++) {
    filename[3] = i / 10000 + '0';
    filename[4] = i / 1000 + '0';
    filename[5] = i / 100 + '0';
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    // If the file name is a new name
    if (! SD.exists(filename)) {
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
    // If the file name exists but is less than size 100
    else {
      File temp = SD.open(filename);
      if (temp.size() < 200) {
        temp.close();
        logfile = SD.open(filename, FILE_WRITE);
        break;
      }
    }
  }

  if (!logfile)
  {
    error("Cannot Create File");
  }
  logfile.println("date,time,light,temperature,humidity");
  Serial.print("Logging to: ");
  Serial.println(filename);

  // STEP 5: Begin DHT 22 Sensor
  dht.begin();

  // STEP 6: Set up low power scheduler
  scheduler.schedule(logData);
}

void loop(void)
{
  scheduler.execute();
}
