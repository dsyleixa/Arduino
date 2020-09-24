/*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   9600-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
   Alternatively: Serial2 (RX2+TX2, pin 20+21 on Mega/Due)
*/

#include <TinyGPS++.h>

const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

static uint32_t millisav;



void setup()
{
   millisav=millis();
 
   pinMode(44, OUTPUT);
   digitalWrite(44, HIGH); // 3.3V
  
   Serial.begin(115200);

   Serial2.begin(GPSBaud);

   Serial.println("DeviceExample.ino");
   Serial.println("A simple demonstration of TinyGPS++ with an attached GPS module");
   Serial.print("Testing TinyGPS++ library v. "); 
   Serial.println(TinyGPSPlus::libraryVersion());
   Serial.println("by Mikal Hart");
   Serial.println();
}



void loop()  {
  char sbuf[128];
  
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read()))
      displayInfo();
      millisav=millis();
  }
  
  if (millis()-millisav > 5000 && gps.charsProcessed() < 10)  {
    Serial.println("No GPS detected: check wiring.");
    millisav=millis()-3000;
  }
  
}



void displayInfo()
{  
  char sbuf[128]; 
  double    fLatt, fLong, fspeed, fcourse, ftest=12.345678;
  uint16_t  dday, dmonth, dyear, dhour, dmin, dsec, dcsec, nsat;

  //----------------------------
  if (gps.location.isValid())
  { 
    fLatt= (double)gps.location.lat();
    fLong= (double)gps.location.lng(); 
    sprintf(sbuf, "Latt: %+013.8f , Long: %+013.8f ", fLatt, fLong ); 
    Serial.print(sbuf);
  }
  else { Serial.print("Location:  INVALID      ");}

  //----------------------------
  if (gps.speed.isValid())
  { 
    fspeed= (double)gps.speed.kmph();
    sprintf(sbuf, "speed: %5.1f ", fspeed ); 
    Serial.print(sbuf);
  }
  else  { Serial.print("Speed:INVALID"); }


  //----------------------------
  if (gps.course.isValid())
  { 
    fcourse= (double)gps.course.deg();
    sprintf(sbuf, "course: %5.1f ", fcourse ); 
    Serial.print(sbuf);
  }
  else { Serial.print("course:INVALID"); }


  //----------------------------
  if (gps.date.isValid())
  {     
    dday=gps.date.day();
    dmonth=gps.date.month();    
    dyear=gps.date.year();
    sprintf(sbuf, " Date: %02d/%02d/%04d", dday, dmonth, dyear); 
    Serial.print(sbuf);
  }
  else { Serial.print(" Date:  INVALID     "); }

  //----------------------------
  if (gps.time.isValid())
  {
    dhour=gps.time.hour();
    dmin= gps.time.minute();
    dsec= gps.time.second();
    dcsec=gps.time.centisecond();
    nsat =gps.satellites.value();
    sprintf(sbuf, " Time: %02d:%02d:%02d,%03d Satellites=%02d", dhour, dmin, dsec, dcsec, nsat); 
    Serial.print(sbuf);
  }
  else { Serial.print(" Time: INVALID     "); }

  Serial.println();
}
