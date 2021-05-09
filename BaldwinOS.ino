 /* BaldwinOS by John Seong
   An Open-Source Project
   Version 1.0
*/

#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h> // Include the TinyGPS++ library
#include <Wire.h>
#include <SerLCD.h>

TinyGPSPlus tinyGPS; // Create a TinyGPSPlus object

SerLCD lcd;

#define ARDUINO_USD_CS 10 // uSD card CS pin (pin 10 on SparkFun GPS Logger Shield)

/////////////////////////
// Log File Defintions //
/////////////////////////
// Keep in mind, the SD library has max file name lengths of 8.3 - 8 char prefix,
// and a 3 char suffix.
// Our log files are called "gpslogXX.csv, so "gpslog99.csv" is our max file.
#define LOG_FILE_PREFIX "gpslog" // Name of the log file.
#define MAX_LOG_FILES 100 // Number of log files that can be made
#define LOG_FILE_SUFFIX "csv" // Suffix of the log file
char logFileName[13]; // Char string to store the log file name
// Data to be logged:
#define LOG_COLUMN_COUNT 8
char * log_col_names[LOG_COLUMN_COUNT] = {
  "longitude", "latitude", "altitude", "speed", "course", "date", "time", "satellites"
}; // log_col_names is printed at the top of the file.

//////////////////////
// Log Rate Control //
//////////////////////
#define LOG_RATE 5000 // Log every 5 seconds
unsigned long lastLog = 0; // Global var to keep of last time we logged

#define GPS_BAUD 9600 // GPS module baud rate. GP3906 defaults to 9600.

#define SerLCD_Address 0x72

#include <SoftwareSerial.h>
#define ARDUINO_GPS_RX 9 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX 8 // GPS RX, Arduino TX pin
SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX); // Create a SoftwareSerial

#define gpsPort ssGPS

#define SerialMonitor Serial

const int modeButton = 12;

const int buzzer = 6;

int buttonCount = 1;

int clearArg = 0;

void setup() {

  SerialMonitor.begin(9600);
  gpsPort.begin(GPS_BAUD);

  pinMode(modeButton, INPUT_PULLUP);

  SerialMonitor.println("Setting up SD card.");
  
  // see if the card is present and can be initialized:
  if (!SD.begin(ARDUINO_USD_CS)) {
    SerialMonitor.println("Error initializing SD card.");
  }
  
  updateFileName(); // Each time we start, create a new file, increment the number
  printHeader(); // Print a header at the top of the new file

  Wire.begin();
  lcd.begin(Wire);
  
  Wire.setClock(400000);

  lcd.setBacklight(0, 255, 0);
  
  lcd.clear();
}

void loop() {

  int buttonState = digitalRead(modeButton);

  if ((lastLog + LOG_RATE) <= millis()) { // If it's been LOG_RATE milliseconds since the last log:
    if (tinyGPS.location.isUpdated()) {
      if (logGPSData()) {
        SerialMonitor.println("GPS logged."); // Print a debug message
        lastLog = millis(); // Update the lastLog variable
      } 
      
      else { 
        // Print an error, don't update lastLog
        SerialMonitor.println("Failed to log new GPS data.");
      }
    }
    
    else { // if the GPS data is invalid
      // Print a debug message. Maybe we don't have enough satellites yet.
      SerialMonitor.print("No GPS data. Sats: ");
      SerialMonitor.println(tinyGPS.satellites.value());
    }
  }

  if (buttonState == 0) {
    buttonCount++;
  }

  if (buttonCount == 1) {
    clearArg = 1;
    printStats1();
    smartDelay(1000);
    
  } else if (buttonCount == 2) {
    clearArg = 1;
    printStats2();
    smartDelay(1000);
    
  } else if (buttonCount == 3) {
    // printStats3();
    smartDelay(1000);
    
  } else if (buttonCount == 4) {
    // printStats4();
    buttonCount = 0;
    smartDelay(1000);
  }
}

byte logGPSData() {
  File logFile = SD.open(logFileName, FILE_WRITE); // Open the log file

  if (logFile) { // Print longitude, latitude, altitude (in feet), speed (in mph), course
    // in (degrees), date, time, and number of satellites.
    logFile.print(tinyGPS.location.lng(), 6);
    logFile.print(',');
    logFile.print(tinyGPS.location.lat(), 6);
    logFile.print(',');
    logFile.print(tinyGPS.altitude.feet(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.speed.mph(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.course.deg(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.date.value());
    logFile.print(',');
    logFile.print(tinyGPS.time.value());
    logFile.print(',');
    logFile.print(tinyGPS.satellites.value());
    logFile.println();
    logFile.close();

    return 1; // Return success
  }

  return 0; // If we failed to open the file, return fail
}

void printHeader() {
  
  File logFile = SD.open(logFileName, FILE_WRITE); // Open the log file

  if (logFile) {
    int i = 0;
    for (; i < LOG_COLUMN_COUNT; i++) {
      logFile.print(log_col_names[i]);
      if (i < LOG_COLUMN_COUNT - 1) // If it's anything but the last column
        logFile.print(','); // print a comma
      else // If it's the last column
        logFile.println(); // print a new line
    }
    
    logFile.close(); // close the file
  }
}

// updateFileName() - Looks through the log files already present on a card,
// and creates a new file with an incremented file index.
void updateFileName() {
  
  int i = 0;
  for (; i < MAX_LOG_FILES; i++) {
    memset(logFileName, 0, strlen(logFileName)); // Clear logFileName string
    // Set logFileName to "gpslogXX.csv":
    sprintf(logFileName, "%s%d.%s", LOG_FILE_PREFIX, i, LOG_FILE_SUFFIX);
    
    if (!SD.exists(logFileName)) {
      break; // Break out of this loop. We found our index
    }
    
    else {
      SerialMonitor.print(logFileName);
      SerialMonitor.println(" exists"); // Print a debug statement
    }
  }
  
  SerialMonitor.print("File name: ");
  SerialMonitor.println(logFileName); // Debug print the file name
}

void printStats1() {

  if (clearArg == 1) {
    lcd.clear();
    clearArg = 0;
  }
  
  lcd.setCursor(0, 0);
  lcd.print("Course: ");
  lcd.print(tinyGPS.course.deg());
  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print(tinyGPS.speed.mph());
}

void printStats2() {

  if (clearArg == 1) {
    lcd.clear();
    clearArg = 0;
  }

  lcd.setCursor(0, 0);
  lcd.print("Altitude: ");
  lcd.print(tinyGPS.altitude.feet());
  lcd.setCursor(0, 1);
  lcd.print("Satellites: ");
  lcd.print(tinyGPS.satellites.value());
}

static void smartDelay(unsigned long ms) {
  
  unsigned long start = millis();
  
  do {
    // If data has come in from the GPS module
    while (gpsPort.available())
      tinyGPS.encode(gpsPort.read()); // Send it to the encode function
    // tinyGPS.encode(char) continues to "load" the tinGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
    
  } while (millis() - start < ms);
}
