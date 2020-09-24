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


//=====================================================================================
double frac(double value) {
   return (value - (double)trunc(value) );
}
//=====================================================================================

#define d2r (M_PI / 180.0)

//calculate haversine distance for linear distance
double dist( double lat1, double lat2, double long1, double long2 )
{
    double dlong = (long2 - long1) * d2r;
    double dlat = (lat2 - lat1) * d2r;
    double a = pow(sin(dlat/2.0), 2) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = 6371 * c;

    return d;
}

//=====================================================================================

void GPSdisplayInfo()
{  
  char sbuf[128]; 
  static   double   fLatt=0, fLong=0, fmin, fdecsec, 
                    fLattold=0, fLongold=0, fLattmean, fLongmean, fdist=0, ETA=0.7;
  uint16_t decdeg, decmin, 
           dday, dmonth, dyear, 
           dhour, dmin, dsec, dcsec, nsat;
  
  if (gps.location.isValid())
  { 
    fLattold = fLatt ;
    fLongold = fLong ;
        
    fLatt = (double)gps.location.lat(); 
    fLong = (double)gps.location.lng(); 
    
    fLattmean = ETA*fLatt + (1-ETA)*fLattold;  // Lowpass-Filter
    fLongmean = ETA*fLong + (1-ETA)*fLongold; 
    
    fdist = dist ( fLattmean, fLattold, fLongmean, fLongold );
    
    sprintf(sbuf, "Lat:%+012.7f " , fLattmean );     
    Serial.print(sbuf);   lcd.setCursor(0, 0);  lcd.print(sbuf);
    
    decdeg = (int)fLattmean;
    fmin   = ( fLattmean - (double)decdeg ) * 60;
    decmin = (int)(fmin);
    fdecsec= ( fmin - (double)decmin ) * 60 ;
   
    sprintf(sbuf, "B%+04d:%02d'%07.4f ", decdeg, decmin, fdecsec);
    Serial.print(sbuf); lcd.setCursor(0, 0);  lcd.print(sbuf);
    
    sprintf(sbuf, " Lng:%+012.7f ", fLongmean ); 
    Serial.print(sbuf);     
    
    decdeg = (int)fLongmean;
    fmin   = ( fLongmean - (double)decdeg ) * 60;
    decmin = (int)(fmin);
    fdecsec= ( fmin - (double)decmin ) * 60 ;
   
    sprintf(sbuf, "L%+04d:%02d'%07.4f ", decdeg, decmin, fdecsec);
    Serial.print(sbuf); lcd.setCursor(0, 1);  //lcd.print(sbuf);       
        
  }
  else  {
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
  else  Serial.print(F("  Date:  INVALID  "));

  if (gps.time.isValid())
  {
    dhour=gps.time.hour();
    dmin= gps.time.minute();
    dsec= gps.time.second();
    dcsec=gps.time.centisecond();
    nsat =gps.satellites.value();
    sprintf(sbuf, "  Time: %02d:%02d:%02d,%03d Sat.=%02d ", dhour, dmin, dsec, dcsec, nsat); 
    Serial.print(sbuf);
  }
  else Serial.print(F("  Time:  INVALID  "));
  
  if(fdist>=1.0) sprintf(sbuf, "~km=%9.4f:%2d ", fdist, nsat);
  else sprintf(sbuf, "~m = %7.2f:%2d ", fdist*1000, nsat);
  if ((fdist!=0.0)&&(fLattold!=0.0)) { Serial.print(sbuf); lcd.setCursor(0, 1);  lcd.print(sbuf); }

  Serial.println();
}

//=====================================================================================
//=====================================================================================

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
  
  Serial.println("setup: GPS test...."); 
  delay(1000);

  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read())) {       
      GPSdisplayInfo();
      Serial.println("setup: GPS OK! ");
    }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
  Serial.println("setup: done."); 
  Serial.println();
}

//=====================================================================================

void loop()
{
  char sbuf[128];
  
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))  GPSdisplayInfo();
  
  delay(10);
}

//=====================================================================================
//=====================================================================================

