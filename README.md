# BaroThermLogger with MPL3115A2

Tags: TSL2561, FEATHER 32U4 ADALOGGER, Arduino, I2C, SPI, Barometric Pressure, Altitude, Temperature Sensor, Data logger

Simple data logger based on ADAFRUIT FEATHER 32U4 ADALOGGER with Barometric Pressure/Altitude/Temperature sensor MPL3115A2 on an ADAFRUIT breakout board. Programming language is C++ (Arduino sketch). Four different functions are available by selecting them with the help of a 4 Bit DIP switch. 100 text files with acquired data can be stored on the SD card.

## Functionality:
* Read temperature (Degree Celsius / Fahrenheit) and barometric pressure data (Pa) from sensor and store it as string values on SD card
* Read all data from all text files from SD card and send it via USB serial to terminal program
* Read directory from SD card and send it via USB serial to terminal program
* Toggle logging/no logging with DIP switch
* Delete all stored text files on SD card
* Blink some error codes with the internal LED if necessary

## Used this hardware:
* https://www.adafruit.com/product/2795
* https://www.adafruit.com/product/1893

## Used this software:
* Arduino IDE v1.8.13 https://www.arduino.cc
* https://github.com/sparkfun/SparkFun_MPL3115A2_Breakout_Arduino_Library

## Thanks
I like to thank all the people who contributed most of the code in this project for their cleverness and effort to make things work.
