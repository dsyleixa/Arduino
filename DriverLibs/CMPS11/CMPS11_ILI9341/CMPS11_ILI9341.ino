/******************************************************************
*                   Arduino CMPS11 example code                   *
*                     CMPS11 running I2C mode                     *
*                     by James Henderson, 2012                    *
*******************************************************************/
// verändert & angepasst für UTFT-Displays von Helmut Wunder, 2015
// (C) CMPS10/CMPS11 code by James Henderson
// (C) UTFT Display Driver Lib by Henning Karlsen
// keine freie Verwendung für kommerzielle Zwecke, 
// Code frei zur privaten Nutzung gemäss
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
// Urheberrechte von  J. Henderson und H. Karlsen bleiben unberührt


#include <SPI.h>

#include <Wire.h>
#define  ADDR_CMPS11  0x60                                // Defines address of CMPS11



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

#define    tft_cs     52
#define    tft_dc     51
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

void setup() {
   char sbuf[128];
   Serial.begin(115200); 
   Wire.begin();                                      // Conects I2C   
   
   ///------------------------------------------------------------------------------------
   // TFT LCD
   
      Serial.println();
      LCDTYPE = _ILI9341due_;
      Serial.print("init LCD... \n");     
      initLCD(1);   
      lcdcls();
      sprintf(sbuf, "LCD=%d wi%dxhi%d Font %dx%d \n",LCDTYPE,LCDmaxX,LCDmaxY,fontwi,fonthi);
      Serial.println(sbuf);
      Serial.println();
      lcdcls(); lcdprint(sbuf);
      Serial.println("[done.] \n");   

   lcdcls();
      
   sprintf(sbuf, "CMPS11 Example V: %d", soft_ver()); // Display software version of the CMPS11
   lcdprintxy( 0, 0, sbuf);   

}


//=====================================================================================

int soft_ver(){
   int data;                                      // Software version of  CMPS11 is read into data and then returned
 
   Wire.beginTransmission(ADDR_CMPS11);
   // Values of 0 being sent with write masked as a byte to be not misinterpreted as NULL 
   // (bug in arduino 1.0)
   Wire.write((byte)0);                           // Sends the register we wish to start reading from
   Wire.endTransmission();

   Wire.requestFrom(ADDR_CMPS11, 1);              // Request byte from CMPS11
   while(Wire.available() < 1);
   data = Wire.read();         
 
   return(data);
}


//=====================================================================================
void display_data(float b,  int p, int r){    // pitch and roll (p, r) are recieved as ints (signed values)
  char sbuf[128];          
 
  sprintf(sbuf, "heading =  %7.1f", b);
  lcdprintxy( 0,20, sbuf);

  sprintf(sbuf, "Pitch =  %7d", p);
  lcdprintxy( 0,30, sbuf);

  sprintf(sbuf, "Roll  =  %7d", r);
  lcdprintxy( 0,40, sbuf);
 
}


//=====================================================================================

void loop(){
   uint8_t  HdgHibyte, HdgLobyte;           // HdgHibyte and HdgLobyte store high and low bytes of the heading 
   int8_t   pitch, roll;                    // Stores pitch and roll values of CMPS11 (signed char value)
 
   float    fheading;                       // Stores full heading (float)
   char     sbuf[128];
 
   Wire.beginTransmission(ADDR_CMPS11);     //starts communication with CMPS11
   Wire.write(2);                           //Sends the register we wish to start reading from
   Wire.endTransmission();

   Wire.requestFrom(ADDR_CMPS11, 4);        // Request 4 bytes from CMPS11
   while(Wire.available() < 4);             // Wait for bytes to become available
   HdgHibyte = Wire.read();         
   HdgLobyte = Wire.read();           
   pitch = Wire.read();             
   roll = Wire.read();             

   fheading = ( (float)(HdgHibyte<<8) + (float)HdgLobyte )/10.0;  // heading (float)
 
   display_data(fheading, pitch, roll);     // Display data to the LCD
 
   delay(10);   

}
//=====================================================================================
//=====================================================================================
