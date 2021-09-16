#include <SPI.h>

#include <SdFat.h>
SdFat SD;

//#include <SD.h>




#define  clock()      millis()  
#define  LRAND_MAX    32767
#define  srand(seed)  randomSeed(seed)
#define  rand()       random(LRAND_MAX)
#define  rando()      ((float)rand()/(LRAND_MAX+1))



//=====================================================================================
// TFT LCD
//=====================================================================================

#define   _SmallFont_        1 // 9341 6x9
#define   _MediumFont_       2 // 9341 12x16
#define   _BigFont_          3 // 9341 18x23

int16_t  LCDmaxX , LCDmaxY ;                // display size
int16_t  _curx_, _cury_,                    // last x,y cursor pos on TFT screen
         _maxx_, _maxy_;                    // max. x,y cursor pos on TFT screen       



// set LCD TFT type
int16_t  LCDTYPE   =   -1;
#define _ILI9341_     8  // https://github.com/adafruit/Adafruit_ILI9340
                           // https://github.com/adafruit/Adafruit-GFX-Library //
#define _ILI9341due_  9  // ILI9341_due NEW lib by Marek Buriak
                           // http://marekburiak.github.io/ILI9341_due/ //
                           
//--------------------------------------------------------------------------------------------------
#define    tft_cs     51
#define    tft_dc     52
#define    tft_rst     0                        

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
   if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(WHITE, BLACK) ;} \
}

#define  lcdNormal()      {                                                                 \
   if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(WHITE, BLACK) ;} \
}

#define  lcdInvers()      {                                                                 \
   if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(BLACK, WHITE) ;} \
}

#define  lcdWhiteRed()    {                                                                 \
   if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_WHITE, ILI9341_RED) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(WHITE, RED) ;} \
}

#define  lcdRedBlack()    {                                                                 \   
   if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_RED, ILI9341_BLACK) ;} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(RED, BLACK) ;} \
}

#define  lcdYellowBlue()  {                                                                 \     
   if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9341_YELLOW, ILI9341_BLUE);} \
   else if(LCDTYPE==_ILI9341due_)   { dtft.setTextColor(YELLOW, BLUE);} \
}



int16_t  fontwi= 8;  // default
int16_t  fonthi=10;  // default


void putfonttype(uint8_t fsize) {

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
  if(LCDTYPE==_ILI9341_)  { 
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
    if(LCDTYPE==_ILI9341_)  { 
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
// SD Card
//=====================================================================================
#define  sd_cs        53                      // <<<<<<<< adjust!
File     myFile; 
char     _fname_[64];


#define fileIO_OK            +1
#define fileIO_NO_ERR         0
#define fileIO_ERR_CREATE    -1
#define fileIO_ERR_OPEN      -2
#define fileIO_ERR_REMOVE    -3
#define fileIO_ERR_WRITE     -4
#define fileIO_ERR_READ      -5
#define fileIO_ERR_IMPLAUS   -6
#define fileIO_ERR_NAME      -8
#define fileIO_ERR_SDCARD   -16


//=====================================================================================
// SD init
//=====================================================================================


int16_t initSD() {
   char sbuf[128];
   uint32_t  tstamp;
   int16_t   ior=0;
  
   tstamp = clock();
   ior=SD.begin(sd_cs);  // 1==true on success; else 0==false
   while( !ior) {      
      sprintf(sbuf, "#: ...SD not found... ");  
      curlf(); lcdprint(sbuf); 
      Serial.println(sbuf);
      delay(1000);   
      ior=SD.begin(sd_cs);
      if (clock()-tstamp>20000) {Serial.println("#: ...break!"); break; }
   } 
  if(!ior) return fileIO_ERR_SDCARD;     // SD ioresult==0 => error = -16 
  return fileIO_OK ;                     // SD ioresult==1 => ok = 1
} 


//=====================================================================================
// user interface:  button pad control pins
//=====================================================================================

#define  PIN_ESC    13
#define  PIN_UP     12
#define  PIN_OK     11
#define  PIN_DN      4 // instead opt.: 6
#define  PIN_LE      3 // instead opt.: 5
#define  PIN_RI      2


//=====================================================================================
// Digital Pins
//=====================================================================================


#define  SensorPTouch(pin) (!digitalRead(pin))    // btn press for _PULLUP Touch Pin (intern. pullup resistor)
#define  SensorTouch(pin)  ( digitalRead(pin))    // btn press for _PULLDOWN Touch Pin (ext. pulldown resistor)
#define  pbtn(pin)         (!digitalRead(pin))    // alias (_PULLUP Touch Pin)
 


//=====================================================================================

int16_t  btnpressed() {
   return ( pbtn(PIN_ESC)||pbtn(PIN_UP)||pbtn(PIN_OK)||pbtn(PIN_DN)||pbtn(PIN_LE)||pbtn(PIN_RI) );
}


//=====================================================================================

int16_t   getbtn() {
   int16_t  choice= -1;
   
   while (!  btnpressed() );  // wait until button pad pressed
   if( pbtn(PIN_ESC) ) choice = PIN_ESC;
   if( pbtn(PIN_UP) )  choice = PIN_UP;
   if( pbtn(PIN_OK) )  choice = PIN_OK;
   if( pbtn(PIN_DN) )  choice = PIN_DN;
   if( pbtn(PIN_LE) )  choice = PIN_LE;
   if( pbtn(PIN_RI) )  choice = PIN_RI;     
   while (  btnpressed() );   // wait until button pad released
   
   return choice;   
}



//=====================================================================================
void setup() {
   char sbuf[128];   
   int32_t  i=0;
         
   // Serial terminal window
   i=115200;
   Serial.begin(i);  
   Serial.print("Serial started, baud="); Serial.println(i);
   
   
   // TFT LCD
   Serial.println();
   LCDTYPE = _ILI9341due_;
   Serial.print("init LCD..."); 

   initLCD(1);   

   Serial.println(" done.");   lcdcls();
   sprintf(sbuf, "LCD=%d wi%dxhi%d Font %dx%d",LCDTYPE,LCDmaxX,LCDmaxY,fontwi,fonthi);
   Serial.println(sbuf); 
   Serial.println();
   lcdcls(); lcdprint(sbuf);
   
   // SD card
   sprintf(sbuf, "SD init... ");   Serial.println(sbuf);
   i = initSD();
   if(i==fileIO_ERR_SDCARD) sprintf(sbuf, "SD failed! ERROR ! ");  
   else sprintf(sbuf, "SD OK ! ");   
   Serial.println(sbuf);   
   curlf();  lcdprint(sbuf);
   
   sprintf(sbuf, "setup(): done.");
   Serial.println(); Serial.println(sbuf);   
   curlf(); curlf(); lcdprint(sbuf);
}



//=====================================================================================
void loop(){
   char     sbuf[128];
  
   

}

//=====================================================================================
