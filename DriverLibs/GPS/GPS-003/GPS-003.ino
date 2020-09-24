/*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   9600-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
   Alternatively: Serial1 (RX1+TX1, pin 18+19 on Mega/Due)
*/

#include <TinyGPS++.h>
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(22, 23, 24, 25, 26, 27);

static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;



double frac(double value) {
   return (value - (double)trunc(value) );
}



void setup()
{
  // setup Serial for USB-Monitor
  Serial.begin(115200);
  
  // setup Serial1 for GPS
  Serial1.begin(GPSBaud);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);  // init LCD 1602

  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  Serial.print(F("Testing TinyGPS++ library v. ")); 
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
}

void loop()
{
  char sbuf[128];
  
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}



void displayInfo()
{  
  char sbuf[128]; 
  double   fLatt, fLong, fmin, fdecsec;
  uint16_t decdeg, decmin, 
           dday, dmonth, dyear, 
           dhour, dmin, dsec, dcsec, nsat;
  
  if (gps.location.isValid())
  { 
    fLatt= (double)gps.location.lat();
    fLong= (double)gps.location.lng(); 
    
    sprintf(sbuf, "Lat:%+012.7f " , fLatt );     
    Serial.print(sbuf);   lcd.setCursor(0, 0);  lcd.print(sbuf);
    
    decdeg = (int)fLatt;
    fmin   = ( fLatt - (float)decdeg) * 60;
    decmin = (int)(fmin);
    fdecsec= (fmin - (float)decmin) * 60 ;
   
    sprintf(sbuf, "B%+04d:%02d'%7.4f ", decdeg, decmin, fdecsec);
    Serial.print(sbuf); lcd.setCursor(0, 0);  lcd.print(sbuf);
    
    sprintf(sbuf, " Lng:%+012.7f ", fLong ); 
    Serial.print(sbuf);     
    
    decdeg = (int)fLong;
    fmin   = ( fLong - (float)decdeg) * 60;
    decmin = (int)(fmin);
    fdecsec= (fmin - (float)decmin) * 60 ;
   
    sprintf(sbuf, "L%+04d:%02d'%7.4f ", decdeg, decmin, fdecsec);
    Serial.print(sbuf); lcd.setCursor(0, 1);  lcd.print(sbuf);
  }
  else
  {
    Serial.print(F("Location:  INVALID  "));
    lcd.setCursor(0, 1);  lcd.print("Loc.: INVALID");
  }

  if (gps.date.isValid())
  {     
    dday=gps.date.day();
    dmonth=gps.date.month();    
    dyear=gps.date.year();
    sprintf(sbuf, "  Date: %02d/%02d/%04d", dday, dmonth, dyear); 
    Serial.print(sbuf);
  }
  else
  {
    Serial.print(F("  Date:  INVALID  "));
  }

  if (gps.time.isValid())
  {
    dhour=gps.time.hour();
    dmin= gps.time.minute();
    dsec= gps.time.second();
    dcsec=gps.time.centisecond();
    nsat =gps.satellites.value();
    sprintf(sbuf, "  Time: %02d:%02d:%02d,%03d Sat.=%02d", dhour, dmin, dsec, dcsec, nsat); 
    Serial.print(sbuf);
  }
  else
  {
    Serial.print(F("  Time:  INVALID  "));
  }

  Serial.println();
}
