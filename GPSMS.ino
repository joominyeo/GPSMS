/*
* Carduino GPSMS Code
*/

// library links
// GPS: https://github.com/adafruit/Adafruit_GPS
// GSM: https://github.com/adafruit/Adafruit_FONA
// Accelerometer: https://github.com/sparkfun/SparkFun_MMA8452Q_Arduino_Library
// LCD: https://github.com/adafruit/Adafruit-RGB-LCD-Shield-Library

// libraries
#include <iostream>
#include <string>

#include <LiquidCrystal.h>
#include <Wire.h>

#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

#define REDLITE 3
#define GREENLITE 4
#define BLUELITE 5

#define FONA_RX 0
#define FONA_TX 1
#define FONA_RST 2

// initialise the library with the numbers of the interface pins
LiquidCrystal lcd(6, 7, 8, 9, 10, 11);
int LCD_brightness = 255;

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
char replybuffer[255];
char imei[16] = {0};
char fonaNotificationBuffer[64]; //for notifications from the FONA
char smsBuffer[250];
float latitude, longitude, speed_kph, heading, speed_mph, altitude;

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

void setup() {

  while (!Serial);
  Serial.begin(115200);
  fonaSerial->begin(4800);

  // welcoming screen/message & boot up (start up GPS and GSM connection)
  lcd.begin(16, 2);
  lcd.print("  Initialising  ");
  lcd.setCursor(0,1);
  lcd.print(" Carduino GPSMS ");

  // get the GPS and GSM module up and running
  fona.enableGPS(true);
  uint8_t imeiLen = fona.getIMEI(imei);
  delay(3000); // three second delay for readability
  lcd.clear();

  // print SIM IMEI & Phone Number
  lcd.setCursor(0,0);
  lcd.print("      IMEI      ");
  lcd.setCursor(0,1);
  lcd.print(imei);
  delay(3000);
  lcd.setCursor(0,0);
  lcd.print("  Phone Number  ");
  lcd.setCursor(0,1);
  lcd.print("+1 858 228 7118 ");
  delay(3000);
  fonaSerial->print("AT+CNMI=2,1\r\n");

  // Display initial GPS Coordinate
  boolean gps_success = fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude);
  if (gps_success) {
    lcd.setCursor(0,0);
    lcd.print("   GPS online   ");
    lcd.setCursor(0,1);
    lcd.print("lat,long,mph,alt");
    delay(3000);
    lcd.setCursor(0,0);
    lcd.print("   GPS online   ");
    lcd.setCursor(0,1);
    lcd.print("lat,long,mph,alt");
    delay(3000);
    lcd.print(dtostrf(latitude).substr(0,8) + "," + dtostrf(longitude).substr(0,7));
    lcd.setCursor(0,1);
    lcd.print(dtostrf(0.621371192 * speed_kph).substr(0,4) + "mph," + dtostrf(altitude).substr(0,8));
    delay(3000);
  }

} // end of setup

void loop() {
  // put your main code here, to run repeatedly:

  /* While (not receiving text msg)
  *     display accelerometer information
  * else respond to the incoming command accordingly
  */
}
