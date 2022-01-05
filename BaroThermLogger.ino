
/* 
 *  Version: 0-2-0
 *  Arduino IDE V1.8.13
 *  Board: Adafruit Feather M0 Adalogger https://learn.adafruit.com/adafruit-feather-m0-adalogger
 *  Sensor: MPL3115A2 - I2C Barometric Pressure/Altitude/Temperature Sensor: https://www.adafruit.com/product/1893
 *  Created:  23.01.2021
 *  Modified: 28.12.2021
 *  Tested:   31.01.2021
 *  Resources used: 20620 Bytes (71%) of Program Memory. Maximum are 28672 Bytes.
 *  Courtesy: Adafruit https://learn.adafruit.com/adafruit-feather-m0-adalogger
 *  and https://learn.adafruit.com/adafruit-feather-m0-adalogger/using-the-sd-card
 *  
 *  Our weather page presents pressure in Inches (Hg) 
 *  Use http://www.onlineconversion.com/pressure.htm for other units
 *  Other resources: SparkFun Electronics MPL3115A2 Barometric Pressure Sensor Library Example Code Lib V1.2.4
 *  https://github.com/sparkfun/SparkFun_MPL3115A2_Breakout_Arduino_Library
 *  By: Nathan Seidle
 *  Date: September 24th, 2013
 *  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 *  Uses the MPL3115A2 library to display the current pressure and temperature
 *  Hardware Connections (Breakoutboard to Arduino):
 *  -VCC = 3.3V
 *  -SDA = A4 (use inline 10k resistor if your board is 5V)
 *  -SCL = A5 (use inline 10k resistor if your board is 5V)
 *  -INT pins can be left unconnected for this demo
 *  Available functions:
 *  .begin() Gets sensor on the I2C bus.
 *  .readAltitude() Returns float with meters above sealevel. Ex: 1638.94
 *  .readAltitudeFt() Returns float with feet above sealevel. Ex: 5376.68
 *  .readPressure() Returns float with barometric pressure in Pa. Ex: 83351.25
 *  .readTemp() Returns float with current temperature in Celsius. Ex: 23.37
 *  .readTempF() Returns float with current temperature in Fahrenheit. Ex: 73.96
 *  .setModeBarometer() Puts the sensor into Pascal measurement mode.
 *  .setModeAltimeter() Puts the sensor into altimetery mode.
 *  .setModeStandy() Puts the sensor into Standby mode. Required when changing CTRL1 register.
 *  .setModeActive() Start taking measurements!
 *  .setOversampleRate(byte) Sets the # of samples from 1 to 128. See datasheet.
 *  .enableEventFlags() Sets the fundamental event flags. Required during setup.
 *  
 *  Connections
 *  ===========
 *  Connect SCL to I2C SCL Clock
 *  Connect SDA to I2C SDA Data
 *  Connect VCC/VDD to 3.3V or 5V (depends on sensor's logic level, check the datasheet)
 *  Connect GROUND to common ground
 *  Power by connecting Vin to 3-5V, GND to GND
 *  Uses I2C - connect SCL to the SCL pin, SDA to SDA pin
 *  See the Wire tutorial for pinouts for each Arduino
 *  http://arduino.cc/en/reference/wire   
 *  
 *  History
 *  =======
 *  2013/SEP/13 First version (@author   Nathan Seidle
 *  2024/JAN/31 Something new from OTR@backhaus-net.de
 *  
 *  Sensor Test
 *  =======
 *  31.01.2021
 *  MPL3115A2 vs. two Amarell.de lab precision Hg-thermometers, range 0.0 - 50.0 °C, 2 centigrade division, error +/- 0.2 °C (Deviation between these thermometers: 0.1°C)
 *  MPL3115A2: 20.4 °C | Reference: 22.2 °C | Deviation: - 1.8 °C (too much!)
 *  MPL3115A2: 20.8 °C | Reference: 22.6 °C | Deviation: - 1.8 °C (too much!)
 *  Conclusion: Electronic sensor needs calibration!
*/
#include <SPI.h>                  // SPI bus for SD card comms
#include <SD.h>                   // SD card lib
#include <Wire.h>                 // I2C bus for sensor comms
#include "SparkFunMPL3115A2.h"    // Lib for the Pressure-Temp-Sensor

// Set the pins used
#define cardSelect 4
#define sdLED 8                   // Internal LED #8 (
#define intLED 13                 // Internal LED #13 (red)
// Each port for DIP switch connected to GND via 10k pull-down resistor
#define intDIP_SW4 12             // DIP-Switch #4 Function: Switch measurement on/off
#define intDIP_SW3 11             // DIP-Switch #3 Function: Function: Switch to read all content of existing files LUXDAT(xy).TXT (storing via terminal program to an ASCII file)
#define intDIP_SW2 10             // DIP-Switch #2 Function: Switch to read SD-Card directory; only possible when connected to a terminal
#define intDIP_SW1 9              // DIP-Switch #1 Function: Switch to destroy all existing files LUXDAT(xy).TXT (stored on SD-Card); only possible when connected to a terminal

MPL3115A2 myPressure;             //Create an instance of the object

File logfile;
   
/**************************************************************************/
/*
    blink out an error code for ever
    error(n) means blink n times
    error(2): "Card init. failed!"
    error(3): "Couldnt create " (logfile)
    error(4): "Error opening: " (logfile)
*/
/**************************************************************************/
void error(uint8_t errno) {
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++){ // blink requested code
      digitalWrite(intLED, HIGH);
      delay(100);
      digitalWrite(intLED, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++){ // wait to complete cycle of two seconds
      delay(200);
    }
  }
}

/**************************************************************************/
/*
    printDirectory function
*/
/**************************************************************************/
void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry){
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)

CHECK CODE for using "Serial.begin(9600);"


    
*/
/**************************************************************************/
void setup() {
  pinMode(intDIP_SW1, INPUT); // Setup ports for DIP-Switch first
  pinMode(intDIP_SW2, INPUT);
  pinMode(intDIP_SW3, INPUT);
  pinMode(intDIP_SW4, INPUT);
  pinMode(sdLED, OUTPUT);
  pinMode(intLED, OUTPUT);
  uint8_t val;
  
  Serial.println();
  Serial.println("Card initialization started...");
  if (!SD.begin(cardSelect)) {
    Serial.println("Card initialization failed!");
    error(2);
  }
  Serial.println("Card initialization done!");
  Serial.println();


  myPressure.begin(); // Get sensor online

  //Configure the sensor
  //myPressure.setModeAltimeter(); // Measure altitude above sea level in meters
  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa

  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags 

// Destroy all existing files BT_DAT(xy).TXT (stored on SD-Card); only possible when connected to a terminal
  val = digitalRead(intDIP_SW1);
  if (val == HIGH) {
    Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    }
    //sd_init();
    // create filename, supposing file exist on SD-Card
    char filename[15];
    strcpy(filename, "/BT_DAT00.TXT");
    for (uint8_t i = 0; i < 100; i++) { 
      filename[7] = '0' + i/10;
      filename[8] = '0' + i%10;
      if (SD.exists(filename)) {
        Serial.print("Deleting file: "); Serial.println(filename);
        SD.remove(filename);
      }
      if (SD.exists(filename)) {
        Serial.print(filename); Serial.println(" exists.");
      }
      else {
        Serial.print(filename); Serial.println(" doesn't exist.");
      }
    }
  }

  // Reading SD-Card directory
  val = digitalRead(intDIP_SW2);
  if (val == HIGH) {
    Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("------------------------------------");
    Serial.println("Listing SD card directory...");
    //sd_init();
    File root;
    root = SD.open("/");
    printDirectory(root, 0);
    Serial.println("Listing directory done!");
    Serial.println("------------------------------------");
    Serial.println();
  }

// Reading all content of existing files BT_DAT(xy).TXT (storing via terminal program to an ASCII file)
  val = digitalRead(intDIP_SW3);
  if (val == HIGH) {
    Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    }
    //sd_init();
    // create filename, supposing file exist on SD-Card
    char filename[15];
    strcpy(filename, "/BT_DAT00.TXT");
    for (uint8_t i = 0; i < 100; i++) { 
      filename[7] = '0' + i/10;
      filename[8] = '0' + i%10;
      if (SD.exists(filename)){
        File dataFile = SD.open(filename);
        // if the file is available, open and write to serial
        if (dataFile) {
          Serial.print("Reading data from: "); Serial.println(filename); // Write name of file
          while (dataFile.available()) {
            Serial.write(dataFile.read());
          }
          dataFile.close();
        }
        // if the file isn't open, pop up an error:
        else {
        Serial.print("Error opening: "); Serial.println(filename);
        error(4);
        }
      }
    }
  }
  
// Create a new logfile (default)  
  Serial.begin(9600);
  Serial.println("Baro Temp Sensor Test"); Serial.println();
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2);
  }
  char filename[15];
  strcpy(filename, "/BT_DAT00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename[7] = '0' + i/10;
    filename[8] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (!SD.exists(filename)) {
      break;
    }
  }

  logfile = SD.open(filename, FILE_WRITE);
  if(!logfile) {
    Serial.print("Couldnt create "); Serial.println(filename);
    error(3);
  }
  Serial.print("Writing to "); Serial.println(filename);
  Serial.println("Ready!");
  Serial.println();

  /* We're ready to go! */
  Serial.println("Starting measuring!");
  Serial.println();
  logfile.println("Data;Pa;Data2;*C"); // Write header for data listing
}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete
*/
/**************************************************************************/
void loop() {
  uint8_t val;
  val = digitalRead(intDIP_SW4); // to log or not to log...
  if (val == HIGH) {

  //float altitude = myPressure.readAltitude();
  //Serial.print("Altitude(m):");
  //Serial.print(altitude, 2);

  //float altitude = myPressure.readAltitudeFt();
  //Serial.print(" Altitude(ft):");
  //Serial.print(altitude, 2);

  //float pressure = myPressure.readPressure();
  //Serial.print("Pressure(Pa):");
  //Serial.print(pressure, 2);

  //float temperature = myPressure.readTemp();
  //Serial.print(" Temp(c):");
  //Serial.println(temperature, 2);

  //float temperature = myPressure.readTempF();
  //Serial.print(" Temp(f):");
  //Serial.print(temperature, 2);

    float pressure = myPressure.readPressure();
    float temperature = myPressure.readTemp();
    
    digitalWrite(sdLED, HIGH);
    logfile.print(pressure); logfile.print(";Pa;"); logfile.print(temperature); logfile.println(";*C");
    logfile.flush(); // storing value and unit divided by delimiter
    digitalWrite(sdLED, LOW);
    
    Serial.print((pressure/100), 1); Serial.println(" hPa");
    Serial.print(temperature, 1); Serial.println(" *C"); 
  }
  else {
    Serial.println("Logger not enabled, measuring and waiting...");
    digitalWrite(intLED, HIGH);
    delay(200);
    digitalWrite(intLED, LOW);
    delay(200);
    float pressure = myPressure.readPressure();
    Serial.print((pressure/100), 1); Serial.println(" hPa");
    float temperature = myPressure.readTemp();
    Serial.print(temperature, 1); Serial.println(" *C");
  }
  delay(60000); // wait for 1.0 min
}
