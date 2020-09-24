//  Arduino code to interface an Arduino to an EV3
//  Arduino as an I2C slave
//  compiles for MEGA and DUE, IDE 1.6.5
//  ver. 0.008-patch

#include  <SPI.h>
#include  <Wire.h>

//#include <ardustdio.h>                                                                       

//=====================================================================================
// misc. 
//=====================================================================================

#define  clock()      millis() 


//=====================================================================================
// I2C EV3 Communication
//=====================================================================================

#define  I2CSLVADDR     0x04
#define  I2CMASTADDR    0x00
#define  MSGSIZE        31
byte     recvmsg[MSGSIZE];
byte     sendmsg[MSGSIZE];

int32_t  flag=0;

//=====================================================================================
// GPS
//=====================================================================================
#include  <TinyGPS++.h>

static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;

//=====================================================================================
// TFT LCD
//=====================================================================================

#define   UTFT_SmallFont     8 // UTFT 8x10
#define   UTFT_MediumFont   12 // UTFT ++ 
#define   UTFT_BigFont      18 // UTFT +++ 
#define   _SmallFont_        1 // 9341 6x9
#define   _MediumFont_       2 // 9341 12x16
#define   _BigFont_          3 // 9341 18x23

int16_t  LCDmaxX , LCDmaxY ;                // display size
int16_t  _curx_, _cury_,                    // last x,y cursor pos on TFT screen
         _maxx_, _maxy_;                    // max. x,y cursor pos on TFT screen       



// set LCD TFT type
int16_t  LCDTYPE   =   -1;

#define  _LCD1602_    1  // LCD1602  Hitachi HD44780 driver <LiquidCrystal.h> 
                           // http://www.arduino.cc/en/Tutorial/LiquidCrystal   //
#define  _SERLCD_     2  // Sparkfun serLCD 16x2  
                           // http://playground.arduino.cc/Code/SerLCD   //
#define  _UTFT_       4  // Henning Karlsen UTFT 2.2-2.4" 220x176 - 320x240 lib
                           // http://henningkarlsen.com/electronics/library.php?id=51   //
#define _ILI9341_     8  // https://github.com/adafruit/Adafruit_ILI9340
                           // https://github.com/adafruit/Adafruit-GFX-Library //
#define _ILI9341due_  9  // ILI9341_due NEW lib by Marek Buriak
                           // http://marekburiak.github.io/ILI9341_due/ //
                           
//--------------------------------------------------------------------------------------------------
#define    tft_cs     49
#define    tft_dc     48
#define    tft_rst     0                        
                          
//=====================================================================================
// UTFT Henning Karlsen
//=====================================================================================
#include <UTFTQD.h>  // patch for QD220A

//UTFT   qdUTFT(Model,  SDA=MOSI,   SCL,      CS,     RESET,   RS)    // Due: 3 exposed SS pins: 4,10,52
  UTFT   qdUTFT(QD220A,   A2,       A1,       A5,     A4,      A3);   // adjust model parameter and pins!
//UTFT   qdUTFT(QD220A, _MEGAMOSI_,_MEGASCK_,tft_cs, tft_rst,  49);   // A0->Vc (LED), A4->BoardReset
 extern  uint8_t SmallFont[];
 
//=====================================================================================
// TFT Adafruit LIL9340/ILI9341
//=====================================================================================
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

Adafruit_ILI9341  tft = Adafruit_ILI9341(tft_cs, tft_dc, tft_rst);


//=====================================================================================
// TFT ILI9341_due // http://marekburiak.github.io/ILI9341_due/ //
//=====================================================================================
#include <ILI9341_due_config.h>
#include <ILI9341_due.h>
#include <SystemFont5x7.h>

ILI9341_due      dtft = ILI9341_due(tft_cs, tft_dc);

// Color set
#define  BLACK          0x0000
#define RED             0xF800
#define GREEN           0x07E0
//#define BLUE            0x001F
#define BLUE            0x102E
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0 
#define ORANGE          0xFD20
#define GREENYELLOW     0xAFE5 
#define DARKGREEN       0x03E0
#define WHITE           0xFFFF

uint16_t  color;

//--------------------------------------------------------------------------------------------------

#define  lcdWhiteBlack()  {                                                                 \
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,255,255); qdUTFT.setBackColor(  0,  0,  0);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(WHITE, BLACK) ;} \
}

#define  lcdNormal()      {                                                                 \
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,255,255); qdUTFT.setBackColor(  0,  0,  0);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(WHITE, BLACK) ;} \
}

#define  lcdInvers()      {                                                                 \
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(  0,  0,  0); qdUTFT.setBackColor(255,255,255);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(BLACK, WHITE) ;} \
}

#define  lcdWhiteRed()    {                                                                 \
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,255,255); qdUTFT.setBackColor(255,  0,  0);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_WHITE, ILI9341_RED) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(WHITE, RED) ;} \
}

#define  lcdRedBlack()    {                                                                 \   
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,  0,  0); qdUTFT.setBackColor(  0,  0,  0);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_RED, ILI9341_BLACK) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(RED, BLACK) ;} \
}

#define  lcdYellowBlue()  {                                                                 \     
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,255,  0); qdUTFT.setBackColor( 64, 64, 64);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_YELLOW, ILI9341_BLUE);} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(YELLOW, BLUE);} \
}



int16_t  fontwi= 8;  // default
int16_t  fonthi=10;  // default


void putfonttype(uint8_t fsize) {
  if(LCDTYPE==_UTFT_)  { 
    fontwi= qdUTFT.getFontXsize(); 
    fonthi= qdUTFT.getFontYsize(); 
  }
  else
  if(LCDTYPE==_ILI9341_) {
     if(fsize==_SmallFont_)     { fontwi= 6; fonthi=9; }  // 5x7 + overhead 
     else
     if(fsize==_MediumFont_)    { fontwi=12; fonthi=16; } // ?
     else
     if(fsize==_BigFont_)       { fontwi=18; fonthi=23; } // ?
  }
  else
  if(LCDTYPE==_ILI9341due_) {
     if(fsize==_SmallFont_)     { fontwi= 6; fonthi=9; }  // 5x7 + overhead 
  }
  _maxx_ = LCDmaxX / fontwi;    // max number of letters x>>
  _maxy_ = LCDmaxY / fonthi;    // max number of letters y^^ 
}


void setlcdorient(int16_t orient) {
  
  if(LCDTYPE==_ILI9341_) {
      tft.setRotation(orient);
      LCDmaxX=tft.width();
      LCDmaxY=tft.height();        
   }
   else
   if(LCDTYPE==_ILI9341due_) {
      dtft.setRotation( (iliRotation)orient); 
      LCDmaxX=dtft.width();
      LCDmaxY=dtft.height();   
   }
      
}

void lcdcls()  {                                                         
   if(LCDTYPE==_UTFT_)       { qdUTFT.clrScr();                }  
   else
   if(LCDTYPE==_ILI9341_)    { tft.fillScreen(ILI9341_BLACK);  }
   else
   if(LCDTYPE==_ILI9341due_) { dtft.fillScreen(BLACK);  }
   _curx_ =0;  _cury_ =0;
}

void curlf()   {                                                        
   _curx_=0; 
   if( _cury_ <=(LCDmaxY-10) ) _cury_+=fonthi; 
   else _cury_=0; 
     
   if(LCDTYPE==_ILI9341_)    { tft.setCursor(0, _cury_); }  
   else
   if(LCDTYPE==_ILI9341due_) { dtft.cursorToXY(0, _cury_); }  
}



void curxy(int16_t  x,  int16_t  y) {
   _curx_ = x;
   _cury_ = y; 
   if(LCDTYPE==_ILI9341_)      {tft.setCursor(x, y); }
   else
   if(LCDTYPE==_ILI9341due_)   {dtft.cursorToXY(x, y); }
}



void lcdprintxy(int16_t x, int16_t y, char * str) {
   if(LCDTYPE==_UTFT_)          { 
     qdUTFT.print(str,x,y); 
     _curx_=x+strlen(str)*fontwi; 
     _cury_=y; 
   }
   else if(LCDTYPE==_ILI9341_)  { 
      tft.setCursor(x,y);     
      tft.print(str); 
      _curx_=tft.getCursorX(); 
      _cury_=tft.getCursorY(); 
   }
   else if(LCDTYPE==_ILI9341due_)  { 
      dtft.cursorToXY(x,y);     
      dtft.printAt(str, x, y); 
      _curx_=x+strlen(str)*fontwi; 
      _cury_=y; 
   }
}


void lcdprint(char * str) {
    if(LCDTYPE==_UTFT_)     { 
      qdUTFT.print(str, _curx_, _cury_); 
      _curx_=_curx_+strlen(str)*fontwi; 
    }
    else if(LCDTYPE==_ILI9341_)  { 
       tft.setCursor(_curx_, _cury_); 
       tft.print(str); 
       _curx_=tft.getCursorX(); 
       _cury_=tft.getCursorY(); 
    }
    else if(LCDTYPE==_ILI9341due_)  { 
       dtft.cursorToXY(_curx_, _cury_); 
       dtft.printAt(str, _curx_, _cury_ ); 
      _curx_=_curx_+strlen(str)*fontwi;
    }
}


void initLCD(uint8_t orientation) { // 0,2==Portrait  1,3==Landscape
   if(LCDTYPE==_UTFT_) {
      qdUTFT.InitLCD(orientation%2);
      LCDmaxX=qdUTFT.getDisplayXSize();
      LCDmaxY=qdUTFT.getDisplayYSize();
      qdUTFT.setFont(SmallFont);
      putfonttype(UTFT_SmallFont);
   }
   else
   if(LCDTYPE==_ILI9341_) {
      tft.begin();
      setlcdorient(orientation);       
      tft.setTextSize(_SmallFont_);
      putfonttype(_SmallFont_);
   } 
   else
   if(LCDTYPE==_ILI9341due_) {
      dtft.begin();
      setlcdorient(orientation);       
      dtft.setFont(SystemFont5x7); 
      putfonttype(_SmallFont_);
   }  
}  


//=====================================================================================
// maths
//=====================================================================================

double frac(double param) {
   double fractpart, intpart;
   fractpart = modf (param , &intpart);
   return fractpart;
}

double intp(double param) {
   double fractpart, intpart;
   fractpart = modf (param , &intpart);
   return intpart;
}


//=====================================================================================


uint8_t lowbyte(uint32_t longval) {
   return(longval & 0x000000ff);   
}


uint8_t highbyte(uint32_t longval) {
   return((longval>>8) & 0x000000ff);   
}


uint8_t low3byte(uint32_t longval) {
   return((longval>>16) & 0x000000ff);   
}


uint8_t high4byte(uint32_t longval) {
   return((longval>>24) & 0x000000ff);   
}


//=====================================================================================

#define d2r (M_PI / 180.0)


//=====================================================================================

//calculate haversine distance for linear distance

double dist( double lat1, double lat2, double long1, double long2 )
{
    double dlong = (long2 - long1) * d2r;
    double dlat = (lat2 - lat1) * d2r;
    double a = pow(sin(dlat/2.0), 2) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = 6367 * c;

    return d;
}


//=====================================================================================

uint8_t  calcchecksum(byte array[]) {
  uint32_t  sum=0;
  for(short i=4; i<MSGSIZE; ++i) sum+=(array[i]);
  return lowbyte(sum);
}



//=====================================================================================
// GPS functions
//=====================================================================================
byte databuf[MSGSIZE];

void initGPS() {
  char sbuf[128];
  
  Serial1.begin(GPSBaud);

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
}

//=====================================================================================

void GPSdisplayInfo()
{ 
  char sbuf[128];   
  static   double   fLatt=0, fLong=0, fdeg, fmin, fsec, fsecfrac, 
                    fLattold=0, fLongold=0, fLattmean, fLongmean, fdist=0, ETA=0.7;
  int16_t  decdeg; 
  int16_t  decmin, 
           dday, dmonth, dyear, 
           dhour, dmin, dsec, dcsec, nsat;
  int16_t  Iintpart;
  uint32_t Ifracpart;  // frac*10000
  
  
 
  if (gps.location.isValid())
  {
    fLattold = fLatt ;
    fLongold = fLong ;
        
    fLatt = (double)gps.location.lat(); ///////////////////////////////////////////////
    fLong = (double)gps.location.lng(); 
    
    fLattmean = ETA*fLatt + (1-ETA)*fLattold;  // Lowpass-Filter
    fLongmean = ETA*fLong + (1-ETA)*fLongold; 

    fLattmean -= 0;
    fLongmean = (fLongmean-0);
    
    fdist = dist ( fLattmean, fLattold, fLongmean, fLongold );
    
    //.......... Lattitude ....................
    
    sprintf(sbuf, "Latt: %+012.7f " , fLattmean );     
    Serial.print(sbuf);   
    lcdprintxy(0, 40, sbuf);

    fmin   = abs( 60*modf(fLattmean , &fdeg) );
    decdeg = (int16_t)(fdeg);    

    fsec   = 60*modf(fmin , &fsecfrac);  
    decmin = (int16_t)(fmin); 

    Iintpart  = (int16_t)fsec;  
    Ifracpart = (int16_t)( fsecfrac*10000 );     
     
    databuf[4]=lowbyte(decdeg);                         // Latt
    databuf[5]=highbyte(decdeg);  
    databuf[6]=(byte)decmin;
    databuf[7]=lowbyte(Iintpart);
    databuf[8]=lowbyte(Ifracpart); 
    databuf[9]=highbyte(Ifracpart);
    databuf[10]=low3byte(Ifracpart);
    databuf[11]=high4byte(Ifracpart);
    
    sprintf(sbuf, "Latt: %+04d:%0d'%7.4f ", decdeg, decmin, fsec);
    Serial.print(sbuf); 
    lcdprintxy(0, 50, sbuf);

    //........... Longitude ...................   
    
    sprintf(sbuf, "Long: %+012.7f ", fLongmean );
    Serial.print(sbuf);    
    lcdprintxy(0, 60, sbuf);

    fmin   = abs( 60*modf(fLongmean , &fdeg) );
    decdeg = (int16_t)(fdeg);    

    fsec   = 60*modf(fmin , &fsecfrac);  
    decmin = (int16_t)(fmin); 

    Iintpart  = (int16_t)fsec;  
    Ifracpart = (int16_t)( fsecfrac*10000 );     
    
    databuf[12]=lowbyte(decdeg);                         // Latt
    databuf[13]=highbyte(decdeg);  
    databuf[14]=(byte)decmin;
    databuf[15]=lowbyte(Iintpart);
    databuf[16]=lowbyte(Ifracpart); 
    databuf[17]=highbyte(Ifracpart);
    databuf[18]=low3byte(Ifracpart);
    databuf[19]=high4byte(Ifracpart);
    
    sprintf(sbuf, "Long: %+04d:%02d'%7.4f ", decdeg, decmin, fsec);
    Serial.print(sbuf); 
    lcdprintxy(0, 70, sbuf);
  }
  else
  {
    sprintf(sbuf, "Location:  INVALID  ");
    Serial.print(sbuf);
    
  }

  //........... Date ...................

  if (gps.date.isValid())
  {     
    dday=gps.date.day();
    dmonth=gps.date.month();   
    dyear=gps.date.year();
    sprintf(sbuf, "Date: %02d/%02d/%04d ", dday, dmonth, dyear);
    databuf[20]=(byte)dday; 
    databuf[21]=(byte)dmonth;
    databuf[22]=(byte)(dyear-2000);
    Serial.print(sbuf);
    lcdprintxy(0, 80, sbuf);
  }
  else
  { 
    sprintf(sbuf, "  Date:  INVALID  ");
    //Serial.print(sbuf);    
  }

  //........... Time ...................

  if (gps.time.isValid())
  {
    dhour=gps.time.hour();
    dmin= gps.time.minute();
    dsec= gps.time.second();
    //dcsec=gps.time.centisecond();
    databuf[23]=(byte)dhour; 
    databuf[24]=(byte)dmin;
    databuf[25]=(byte)dsec;
    
    sprintf(sbuf, "UTC :  %02d:%02d:%02d  ", dhour, dmin, dsec);
    Serial.print(sbuf);
    lcdprintxy(0, 90, sbuf);

    //........... n Sat ...................

    nsat =gps.satellites.value();
    databuf[24]=(byte)nsat;
    sprintf(sbuf, "nSat: %02d  ", nsat);
    Serial.print(sbuf);
    lcdprintxy(0,100, sbuf);
  }
  else
  {
    sprintf(sbuf, "  Time:  INVALID  ");
    Serial.print(sbuf);
  }

   if(fdist>=1.0) sprintf(sbuf, "noise~km=%-9.4f ", fdist);
   else sprintf(sbuf, "noise~m = %-7.2f ", fdist*1000);
   Serial.print(sbuf);
  lcdprintxy(0,110, sbuf);

  Serial.println();
}

//=====================================================================================
// setup
//=====================================================================================
void setup() {
   char sbuf[128];   
   int32_t  i=0;

   // Serial USB terminal 
   i=115200;
   Serial.begin(i); 
   Serial.print("Serial started, baud="); Serial.println(i);

   // TFT LCD
   Serial.println();
   LCDTYPE = _UTFT_;
   Serial.print("LCD init ..."); 
   initLCD(1);   
   Serial.println(" done.");   
   lcdcls();
   sprintf(sbuf, "LCD=%d wi%dxhi%d Font %dx%d",LCDTYPE,LCDmaxX,LCDmaxY,fontwi,fonthi);
   Serial.println(sbuf); 
   Serial.println();
   curlf(); lcdprint(sbuf);

   // setup Serial1 for GPS
   initGPS();
   sprintf(sbuf, "GPS init: done.");
   Serial.println(sbuf);
   curlf(); lcdprint(sbuf);
 
   // Wire1 (i2c)
   Wire1.begin(I2CSLVADDR  );  //  start Arduino as a I2C slave, addr=0x04 (7-bit coded)
   Wire1.onReceive(receiveData );
   Wire1.onRequest(sendData );
   memset(sendmsg, 0, sizeof(sendmsg) );
   memset(recvmsg, 0, sizeof(recvmsg) );   
   sprintf(sbuf, "I2C init: done.");
   Serial.println(sbuf);
   curlf(); lcdprint(sbuf);

   sprintf(sbuf, "setup(): done.");
   Serial.println(); Serial.println(sbuf);   
   
   lcdcls();
}




//=====================================================================================
// loop
//=====================================================================================

void loop()
{ 
   char sbuf[256];
   uint8_t cks;

   Serial.println(); Serial.println();   

   //.................... GPS .................... 

   // This sketch displays information every time a new sentence is correctly encoded.
   while ( Serial1.available() > 0) 
      if ( gps.encode(Serial1.read() ) )
        GPSdisplayInfo();

   if (millis() > 5000 && gps.charsProcessed() < 10)
   {
      Serial.println(F("No GPS detected: check wiring."));
      while(true);
   }



   //.................... i2c ....................

   if(flag==1)
   { 
     Serial.println(); Serial.println();

     // do something with the received data
     // and then build the sendmsg [4]...[31]
     memcpy(sendmsg, databuf, MSGSIZE);
     
     sendmsg[0] = 0xff;
     sendmsg[2] = I2CMASTADDR   ;           // to check by master: master address ID 
     sendmsg[3] = flag;                     // flag to master: 1= new data; 127=send again request
     sendmsg[MSGSIZE-1] = I2CSLVADDR  ;     // to check by master: corrupted data ?
     cks = calcchecksum(sendmsg);
     sendmsg[1] = cks;   
     
     flag=0;     
   }
   else
   if(flag==127) {  // builds array with error flag 127
     Serial.println("ERROR! flag==127");
     memset(sendmsg, 0, MSGSIZE);
     sendmsg[0] = 0xff;
     sendmsg[2] = I2CMASTADDR   ;           // to check by master: master address ID 
     sendmsg[3] = flag;                     // flag to master: 1= new data; 127=send again request
     sendmsg[MSGSIZE-1] = I2CSLVADDR  ;     // to check by master: corrupted data ?
     cks = calcchecksum(sendmsg);
     sendmsg[1] = cks;
    
   }

   sprintf(sbuf, " Recvarr[4]=%4d, [5]=%4d, Sendarr[4]=%4d, [5]=%4d, [6]=%4d, [7]=%4d",
                  recvmsg[4], recvmsg[5], sendmsg[4], sendmsg[5], sendmsg[6], sendmsg[7]) ;
   Serial.println(sbuf);
   delay(2);  // short break for the cpu and the bus
   
}


//=====================================================================================

void receiveData(int byteCount) {
    int32_t i;
    byte val;

    while(Wire1.available()<MSGSIZE) ; // wait for 30  bytes to complete
    i=0;  // init counter var
    while(Wire1.available()&& (i<MSGSIZE) )         // read all bytes
    {
      val=Wire1.read();
      recvmsg[i++]=val;
    }
    // check for first 3 bytes and via calcchecksum() function
    if( (recvmsg[0]==0xff) && (recvmsg[1]==calcchecksum(recvmsg)) && (I2CSLVADDR  ==recvmsg[2]) )   
    // if ok:
    flag=1;        // data ok
    else flag=127; // else handle receive error => flag =127  to send array back, request to send again
}

//=====================================================================================

void sendData(){
  // Wire1.write writes data from a slave device in response to a request from a master
  Wire1.write(sendmsg, MSGSIZE); // send own 30 bytes back to master..
}


//=====================================================================================

