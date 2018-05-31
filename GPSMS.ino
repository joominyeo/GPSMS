/*
* Carduino GPSMS Code
*/

// library links
// GPS: https://github.com/adafruit/Adafruit_GPS
// GSM: https://github.com/adafruit/Adafruit_FONA
// Accelerometer: https://github.com/sparkfun/SparkFun_MMA8452Q_Arduino_Library
// LCD: https://github.com/adafruit/Adafruit-RGB-LCD-Shield-Library

// libraries
#include <LiquidCrystal.h>
#include <Wire.h>

#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

#define REDLITE 5 //CHANGE
#define GREENLITE 5 //CHANGE
#define BLUELITE 5

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

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
char lat[8], lon[7], spd[4], alt[7], latlon[16], spdalt[16];

uint8_t type;

void setup() {
  
  while (!Serial);
  
  Serial.begin(115200);
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    //Serial.println(F("Couldn't find FONA"));
    //while (1);
    }
  type = fona.type();
  Serial.println(F("FONA is OK"));

  Serial.println(F("Starting Setup"));
  // welcoming screen/message & boot up (start up GPS and GSM connection)
  
  lcd.begin(16, 2);
  lcd.print("  Initialising  ");
  Serial.println(F("  Initialising  "));
  lcd.setCursor(0,1);
  lcd.print(" Carduino GPSMS ");
  Serial.println(F(" Carduino GPSMS "));

  // get the GPS and GSM module up and running
  fona.enableGPS(true);
  uint8_t imeiLen = fona.getIMEI(imei);
  delay(3000); // three second delay for readability
  lcd.clear();

  // print SIM IMEI & Phone Number
  lcd.setCursor(0,0);
  lcd.print("      IMEI      ");
  Serial.println("      IMEI      ");
  lcd.setCursor(0,1);
  lcd.print(imei);
  Serial.println(imei);
  delay(3000);
  lcd.setCursor(0,0);
  lcd.print("  Phone Number  ");
  Serial.println(F("  Phone Number  "));
  lcd.setCursor(0,1);
  lcd.print("+1 858 228 7118 ");
  Serial.println(F("+1 858 228 7118 "));
  delay(3000);
  fonaSerial->print("AT+CNMI=2,1\r\n");

  // Display initial GPS Coordinate
  boolean gps_success = fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude);
  if (gps_success) {
    lcd.setCursor(0,0);
    lcd.print("   GPS online   ");
    Serial.println("   GPS online   ");
    lcd.setCursor(0,1);
    lcd.print("lat,long,mph,alt");
    Serial.println("lat,long,mph,alt");
    delay(3000);

    dtostrf(latitude,8,3, lat);
    dtostrf(longitude,7,3, lon);
    strcat(latlon, lat);
    strcat(latlon, ',');
    strcat(latlon, lon);
    dtostrf(0.621371192 * speed_kph, 4, 2, spd);
    dtostrf(altitude,7,3, alt);
    strcat(spdalt, spd);
    strcat(spdalt, "mph,");
    strcat(spdalt, alt);
    lcd.print(latlon);
    lcd.setCursor(0,1);
    lcd.print(spdalt);
    delay(3000);
  }
  Serial.println("ALL SET UP");
} // end of setup

int counter = 1;

void loop() {
  autoReply();
}

void autoReply() {
    char* bufPtr = fonaNotificationBuffer;    //handy buffer pointer
  
  if (fona.available())      //any data available from the FONA?
  {
    int slot = 0;            //this will be the slot number of the SMS
    int charCount = 0;
    //Read the notification into fonaInBuffer
    do  {
      *bufPtr = fona.read();
      Serial.write(*bufPtr);
      delay(1);
    } while ((*bufPtr++ != '\n') && (fona.available()) && (++charCount < (sizeof(fonaNotificationBuffer)-1)));
    
    //Add a terminal NULL to the notification string
    *bufPtr = 0;
 
    //Scan the notification string for an SMS received notification.
    //  If it's an SMS message, we'll get the slot number in 'slot'
    if (1 == sscanf(fonaNotificationBuffer, "+CMTI: " FONA_PREF_SMS_STORAGE ",%d", &slot)) {
      Serial.print("slot: "); Serial.println(slot);
      
      char callerIDbuffer[32];  //we'll store the SMS sender number in here
      
      // Retrieve SMS sender address/phone number.
      if (! fona.getSMSSender(slot, callerIDbuffer, 31)) {
        Serial.println("Didn't find SMS message in slot!");
      }
      Serial.print(F("FROM: ")); Serial.println(callerIDbuffer);
 
        // Retrieve SMS value.
        uint16_t smslen;
        if (fona.readSMS(slot, smsBuffer, 250, &smslen)) { // pass in buffer and max len!
          Serial.println(smsBuffer);
        }

        if (strcmp(smsBuffer, "Loc")==0 || strcmp(smsBuffer, "loc")==0) { // ONLY RESPONDS TO COMMAND "Loc" and "loc" CASE-SENSITIVE
 
            // Send back an automatic response
            Serial.println("Sending reponse...");
            if (!fona.sendSMS(callerIDbuffer, "This is an automatic reply! IT WORKS!")) {
              Serial.println(F("Failed"));
              //PRINT TO LCD
            } else {
              Serial.println(F("Sent!"));
              //PRINT TO LCD
            }
        }
          
        // clear mem
        if (fona.deleteSMS(slot)) {
          Serial.println(F("OK!"));
        } else {
          Serial.print(F("Couldn't delete SMS in slot ")); Serial.println(slot);
          fona.print(F("AT+CMGD=?\r\n"));
        }
    }
  }
}
