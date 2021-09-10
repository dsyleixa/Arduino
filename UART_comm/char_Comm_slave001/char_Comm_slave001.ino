/*     Rx slave       Receiver
 For more details see: http://projectsfromtech.blogspot.com/
 
 The Tx sketch makes use of Serial1. This functionality is only found on the Arduino Mega and other 3rd party boards.
 Connect the Tx->Rx1  and Rx ->Tx1 pins with the board running Add_Five_Tx.ino
 Connect the Grounds of the two boards
 
 Receive an integer value over the serial, adds 5 to it,
 and then returns the value to the Tx board
 
 Note: The Tx and Rx boards are just names I chose. The communication is 2-way. 
 Master-slave would probably be a better option.
 */
 
#include <SPI.h>
#include <SD.h>
#include <UTFTQD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>
#include <ardustdio.h>



//=====================================================================================
// misc.  
//=====================================================================================

#define  _DUEMISO_    74  // Arduino Due SPI Header
#define  _DUEMOSI_    75
#define  _DUESCK_     76

#define  clock()      millis()  
#define  LRAND_MAX    32767
#define  srand(seed)  randomSeed(seed)
#define  rand()       random(LRAND_MAX)
#define  rando()      ((float)rand()/(LRAND_MAX+1))



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
char     wspace[128];                       // line of white space


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
                           
                           
//--------------------------------------------------------------------------------------------------
#define  UTFT_cs      52    // <<<<<<<< adjust!

//UTFT   qdUTFT(Model, SDA=MOSI,  SCL, CS,         RESET,  RS)    // Due: 3 exposed SS pins: 4,10,52
 UTFT   qdUTFT(QD220A,   A2,     A1,  A5,         A4,     A3);   // adjust model parameter and pins!
//UTFT   qdUTFT(QD220A,   50,     49,  UTFT_cs,  0,     51);   // A0->Vc (LED), A4->BoardReset
 extern  uint8_t SmallFont[];
//--------------------------------------------------------------------------------------------------
#define    tft_cs     50
#define    tft_dc     49
#define    tft_rst     0
Adafruit_ILI9340   tft = Adafruit_ILI9340(tft_cs, tft_dc, tft_rst);

//--------------------------------------------------------------------------------------------------

#define  lcdWhiteBlack()  {                                                                 \
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,255,255); qdUTFT.setBackColor(  0,  0,  0);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9340_WHITE, ILI9340_BLACK) ;} \
}

#define  lcdNormal()      {                                                                 \
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,255,255); qdUTFT.setBackColor(  0,  0,  0);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9340_WHITE, ILI9340_BLACK) ;} \
}

#define  lcdInvers()      {                                                                 \
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(  0,  0,  0); qdUTFT.setBackColor(255,255,255);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9340_BLACK, ILI9340_WHITE) ;} \
}

#define  lcdWhiteRed()    {                                                                 \
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,255,255); qdUTFT.setBackColor(255,  0,  0);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9340_WHITE, ILI9340_RED) ;} \
}

#define  lcdRedBlack()    {                                                                 \   
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,  0,  0); qdUTFT.setBackColor(  0,  0,  0);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9340_RED, ILI9340_BLACK) ;} \
}

#define  lcdYellowBlue()  {                                                                 \     
   if(LCDTYPE==_UTFT_)      { qdUTFT.setColor(255,255,  0); qdUTFT.setBackColor( 64, 64, 64);} \
   else if(LCDTYPE==_ILI9341_)   { tft.setTextColor(ILI9340_YELLOW, ILI9340_BLUE);} \
}

int16_t  fontwi= 8;  // default
int16_t  fonthi=10;  // default


void putfonttype(uint8_t fsize) {
  if(LCDTYPE==_UTFT_)  { fontwi= qdUTFT.getFontXsize(); fonthi=qdUTFT.getFontYsize(); }
  else
  if(fsize==_SmallFont_)     { fontwi= 6; fonthi=9; }  // 5x7 + overhead ?
  else
  if(fsize==_MediumFont_)    { fontwi=12; fonthi=16; } // ?
  else
  if(fsize==_BigFont_)       { fontwi=18; fonthi=23; } // ?
  
  _maxx_ = LCDmaxX / fontwi;    // max number of letters x>>
  _maxy_ = LCDmaxY / fonthi;    // max number of letters y^^ 
  memset(wspace, ' ', _maxx_);  // line of white space
  wspace[_maxx_]='\0';
}



void setlcdorient(int8_t orient) {
  if(LCDTYPE==_ILI9341_) {
      tft.setRotation(orient);
      LCDmaxX=tft.width();
      LCDmaxY=tft.height();        
   }
}

void lcdcls()  {                                                         
   if(LCDTYPE==_UTFT_)      { qdUTFT.clrScr();                }  
   if(LCDTYPE==_ILI9341_)   { tft.fillScreen(ILI9340_BLACK);  }
   _curx_ =0;  _cury_ =0;
}

void curlf()   {                                                        
   _curx_=0; 
   if( _cury_ <=(LCDmaxY-10) ) _cury_+=fonthi; 
   else _cury_=0;   
   if(LCDTYPE==_ILI9341_)   {tft.setCursor(0, _cury_); }  
}



void curxy(int16_t  x,  int16_t  y) {
   _curx_ = x;
   _cury_ = y; 
   if(LCDTYPE==_ILI9341_)   {tft.setCursor(x, y); }
}


void lcdprintxy(int16_t x, int16_t y, char * str) {
   if(LCDTYPE==_UTFT_)     { qdUTFT.print(str,x,y); _curx_=x+strlen(str)*fontwi; _cury_=y; }
   else if(LCDTYPE==_ILI9341_)  { 
      tft.setCursor(x,y);  tft.print(str); 
      _curx_=tft.getCursorX(); _cury_=tft.getCursorY(); 
   }
}


void lcdprint(char * str) {
    if(LCDTYPE==_UTFT_)     { qdUTFT.print(str, _curx_, _cury_); _curx_=_curx_+strlen(str)*fontwi; }
    else if(LCDTYPE==_ILI9341_)  { 
       tft.setCursor(_curx_, _cury_); tft.print(str); 
       _curx_=tft.getCursorX(); _cury_=tft.getCursorY(); 
    }
}


void initlcd(uint8_t orient) { // 0,2==Portrait  1,3==Landscape
   if(LCDTYPE==_UTFT_) {
      qdUTFT.InitLCD();
      LCDmaxX=qdUTFT.getDisplayXSize();
      LCDmaxY=qdUTFT.getDisplayYSize();
      qdUTFT.setFont(SmallFont);
      putfonttype(UTFT_SmallFont);
      fontwi=qdUTFT.getFontXsize();
      fonthi=qdUTFT.getFontYsize();
   }
   else
   if(LCDTYPE==_ILI9341_) {
      tft.begin();
      setlcdorient(orient);       
      tft.setTextSize(_SmallFont_);
      putfonttype(_SmallFont_);
   }   
}  




//=====================================================================================
// SD Card
//=====================================================================================
#define  sd_cs        38     // <<<<<<<< adjust!
File     myFile; 
char     _fname_[64];


#define fileIO_OK            +1
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

int16_t SDinit() {
   char sbuf[50];
   uint32_t  tstamp;
   int16_t   ior=0;

   pinMode(13, OUTPUT);
   digitalWrite(13, LOW);
   tstamp = clock();
   ior=SD.begin(sd_cs);  // true on sucess; else false
   while( !ior) {      
      sprintf(sbuf, "#: ...SD not found... ");  Serial.println(sbuf);
      delay(1000);   
      ior=SD.begin(sd_cs);
      if (clock()-tstamp>20000) {Serial.println("#: ...break!"); break; }
   } 
  if(!ior) return fileIO_ERR_SDCARD;     // SD ioresult == 0
  digitalWrite(13, HIGH);
  return fileIO_OK ;            // SD init OK  == 1
} 


//=====================================================================================
// SD file remove
//=====================================================================================

int16_t  SDfremove(char * fname_) {
   char sbuf[50];
   
   if (SD.exists(fname_) ) {
       sprintf(sbuf,"# %s found          ", fname_);  Serial.println(sbuf);   
       sprintf(sbuf,"# Removing %s       ", fname_);  Serial.println(sbuf);          
       SD.remove(fname_);
                                    // removed: success ?
       if (SD.exists(fname_) ) {
          sprintf(sbuf,"# %s remove error", fname_); Serial.println(sbuf);
          return fileIO_ERR_REMOVE;                 // SD file remove failed
       }
       else {
          sprintf(sbuf,"# %s  N/A        ", fname_);    Serial.println(sbuf);
          return fileIO_OK;                         // SD file remove OK
       }
    }
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
//=====================================================================================

const    uint8_t  bwidth=8;
uint8_t  chksum=127, bsync=255; 
uint8_t  val[bwidth];
uint8_t  inval[bwidth];




void setup() {
   char sbuf[128];   
   int32_t  i=0;
         
   // Serial
   Serial.begin(115200);   // USB terminal
   Serial1.begin(256000);  // RX-TX UART
   Serial.parseInt();      // clear any garbage in the buffer.
   
   // GPIO pins default = INPUT_PULLUP, 13 for LED13
   Serial.println();
   Serial.println("GPIO pin mode default: INPUT_PULLUP");
   for ( i= 2; (i<=13); ++i)  pinMode(i, INPUT_PULLUP); 
   pinMode(13, OUTPUT);
   for ( i=22; (i<=53); ++i)  pinMode(i, INPUT_PULLUP); 
   
   
   // TFT LCD
   Serial.println();
   LCDTYPE = _UTFT_;
   Serial.print("init LCD..."); 
   initlcd(1);   
   Serial.println(" done.");   lcdcls();
   sprintf(sbuf, "LCD=%d wi%d x hi%d",LCDTYPE,LCDmaxX,LCDmaxY);
   Serial.println(sbuf); 
   Serial.println();
   lcdcls(); lcdprint(sbuf);
   /*
   // SD card
   sprintf(sbuf, "SD init... ");   Serial.println(sbuf);
   i = SDinit();
   if(i==fileIO_ERR_SDCARD) sprintf(sbuf, "SD failed! ERROR ! ");  
   else sprintf(sbuf, "SD OK ! ");   
   Serial.println(sbuf);   
   curlf(); lcdprint(sbuf);
   */
   sprintf(sbuf, "setup(): done.");
   Serial.println(); Serial.println(sbuf);   
   curlf(); curlf(); lcdprint(sbuf);
   
   lcdcls();


}



//=====================================================================================
//=====================================================================================
void loop(){
  char     sbuf[128];     
  static   int cnt=0; 
  uint8_t  buf;
  int      incoming1  = 0;
   

  sprintf(sbuf, "Rx slave");
  lcdprintxy(0, 0, sbuf);
  
  cnt=0; buf=0;
  incoming1 = Serial1.available();
  while( (incoming1 != 0)  && (buf!=bsync) )  // While there is something to be read {
  {                                           // sync = 127
    buf = Serial1.read();                     // Reads incoming bytes
    incoming1 = Serial1.available();
  }  
  inval[0]=buf;
  cnt=1;  
  while( (incoming1 != 0)  && (cnt<bwidth) )   // While there is something to be read {
  {  
    inval[cnt] = Serial1.read();               // Reads incoming bytes
    cnt++;
    incoming1 = Serial1.available();
  }    
    
  // display values
  lcdprintxy(0,10, "input:");
  
  if(inval[0]==bsync) 
  {
     for (cnt=0; cnt<bwidth; ++cnt) {
        sprintf(sbuf, "%3d  ", inval[cnt]);
        lcdprintxy(cnt*3*8, 20, sbuf);
     }
  }
  
  delay(10);
  
  // build array to send back to master:
  
  memcpy(val, inval, sizeof(val));   
  
  lcdprintxy(0,30, "output..."); 
  val[0]=bsync; val[1]=chksum; val[2]+=1; val[3]+=1; val[4]+=1; val[5]+=1; val[6]+=1;
  
  for (cnt=0; cnt<bwidth; ++cnt) {
     Serial1.write(val[cnt]);                  //Send the new val back to the Tx master
  }
  
  // display values
  for (cnt=0; cnt<bwidth; ++cnt) {
     sprintf(sbuf, "%3d  ", val[cnt]);
     lcdprintxy(cnt*3*8,40, sbuf);
  } 

  
      
  
}


//=====================================================================================
//=====================================================================================
