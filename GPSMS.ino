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

#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// initialise the library with the numbers of the interface pins
const int rs = 7, en = 8, db4 = 9, db5 = 10, db6 = 11, db7 = 12;
LiquidCrystal lcd(rs, en, db4, db5, db6, db7);

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial* fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
char replybuffer[255];
char imei[16] = { 0 };
char fonaNotificationBuffer[64]; //for notifications from the FONA
char smsBuffer[250];
float latitude, longitude, speed_kph, heading, speed_mph, altitude;
char lat[8], lon[8], spd[6], alt[6], locinfo[40], url[50];
boolean gps_success;
uint8_t type;

void setup()
{
    while (!Serial);

    Serial.begin(115200);
    fonaSerial->begin(4800);
    if (!fona.begin(*fonaSerial)) {
        //Serial.println(F("Couldn't find FONA"));
        //while (1);
    }
    type = fona.type();

    Serial.println(F("Starting Setup"));
    // welcoming screen/message & boot up (start up GPS and GSM connection)
    lcd.begin(16, 2);
    lcd.print("Initialising");
    Serial.println(F("  Initialising  "));
    delay(1000);
    lcd.setCursor(0, 1); // (col, row)
    lcd.print("Carduino GPSMS");
    Serial.println(F(" Carduino GPSMS "));
    delay(1000);

    // get the GPS and GSM module up and running
    fona.enableGPS(true);
    uint8_t imeiLen = fona.getIMEI(imei);
    delay(1000);
    lcd.clear();

    // print SIM IMEI & Phone Number
    lcd.setCursor(0, 0);
    lcd.print("      IMEI      ");
    Serial.println("      IMEI      ");
    lcd.setCursor(0, 1);
    lcd.print(imei);
    Serial.println(imei);
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("  Phone Number  ");
    Serial.println(F("  Phone Number  "));
    lcd.setCursor(0, 1);
    lcd.print("+1 858 228 7118 ");
    Serial.println(F("+1 858 228 7118 "));
    delay(3000);// increase to 3000 for LCD
    fonaSerial->print("AT+CNMI=2,1\r\n");

    Serial.println("SETUP COMPLETE");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" SETUP COMPLETE ");
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print(" SYSTEMS ONLINE ");
} // end of setup

void loop()
{
    autoReply();
} // end of loop

void autoReply()
{
    char* bufPtr = fonaNotificationBuffer; //handy buffer pointer

    if (fona.available()) //any data available from the FONA?
    {
        int slot = 0; //this will be the slot number of the SMS
        int charCount = 0;
        //Read the notification into fonaInBuffer
        do {
            *bufPtr = fona.read();
            Serial.write(*bufPtr);
            delay(1);
        } while ((*bufPtr++ != '\n') && (fona.available()) && (++charCount < (sizeof(fonaNotificationBuffer) - 1)));

        //Add a terminal NULL to the notification string
        *bufPtr = 0;

        //Scan the notification string for an SMS received notification.
        //  If it's an SMS message, we'll get the slot number in 'slot'
        if (1 == sscanf(fonaNotificationBuffer, "+CMTI: " FONA_PREF_SMS_STORAGE ",%d", &slot)) {
            Serial.print("slot: ");
            Serial.println(slot);

            char callerIDbuffer[32]; //we'll store the SMS sender number in here

            // Retrieve SMS sender address/phone number.
            if (!fona.getSMSSender(slot, callerIDbuffer, 31)) {
                Serial.println("Didn't find SMS message in slot!");
            }
            Serial.print(F("FROM: "));
            Serial.println(callerIDbuffer);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("COMMAND FROM");
            lcd.setCursor(0, 1);
            lcd.print(callerIDbuffer);

            // Retrieve SMS value.
            uint16_t smslen;
            if (fona.readSMS(slot, smsBuffer, 250, &smslen)) { // pass in buffer and max len!
                Serial.println(smsBuffer);
            }

            if (strcmp(smsBuffer, "Locc") == 0 || strcmp(smsBuffer, "Locl") == 0 || strcmp(smsBuffer, "locc") == 0 || strcmp(smsBuffer, "locl") == 0) { // ONLY RESPONDS TO COMMAND "Loc" and "loc" CASE-SENSITIVE

                // Send back an automatic response
                gps_success = fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude);
                while(!gps_success) {
                    Serial.println("> Fixing GPS");
                    delay(2000);
                }
                if (strcmp(smsBuffer, "Locc") == 0 || strcmp(smsBuffer, "locc") == 0) {
                    grabGPScoor();
                    fona.sendSMS(callerIDbuffer, locinfo);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("COORDINATES");
                    lcd.setCursor(0, 1);
                    lcd.print("SENT!");
                    delay(3000);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print(" SYSTEMS ONLINE ");
                    delay(1500);
                } else { // Generate and replay back with a Google Maps link
                    grabMapLink();
                    fona.sendSMS(callerIDbuffer, url);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("MAP LINK");
                    lcd.setCursor(0, 1);
                    lcd.print("SENT!");
                    delay(3000);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print(" SYSTEMS ONLINE ");
                    delay(1500);
                }
            }

            // clear mem
            if (fona.deleteSMS(slot)) {
                Serial.println(F("OK!"));
            }
            else {
                Serial.print(F("Couldn't delete SMS in slot "));
                Serial.println(slot);
                fona.print(F("AT+CMGD=?\r\n"));
            }
        }
    }
} // end of autoreply

void grabGPScoor()
{
    dtostrf(latitude, 6,4, lat);
    strcat(locinfo,lat);
    strcat(locinfo, "N, ");
    dtostrf(-1 * longitude, 6,4, lon);
    strcat(locinfo,lon);
    strcat(locinfo,"W, "); //California is in the western hemisphere
    dtostrf(speed_kph * 0.621371, 4,1, spd);
    strcat(locinfo,spd);
    strcat(locinfo,"mph, ");
    dtostrf(altitude, 4,1, alt);
    strcat(locinfo,alt);
    strcat(locinfo,"m");
    Serial.println(locinfo);
} // end of grabGPScoor

void grabMapLink()
{
    strcat(url,"https://www.google.com/maps/place/");
    dtostrf(latitude, 6,4, lat);
    strcat(url,lat);
    strcat(url,"N+");
    dtostrf(-1 * longitude, 6,4, lon);
    strcat(url,lon);
    strcat(url,"W");
    Serial.println(url);
} // end of grabMapLink
