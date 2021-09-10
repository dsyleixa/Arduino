/*    Tx master        Transmitter
 For more details see: http://projectsfromtech.blogspot.com/
 
 This sketch makes use of Serial1. This functionality is only found on the Arduino Mega and other 3rd party boards.
 Tx1: pin 18   Rx1: pin 19
 Connect the Tx1->Rx  and Rx1 ->Tx pins with the board running Add_Five_Rx.ino
 Connect the Grounds of the two boards
 
 Read integer value from Serial Monitor
 Transmit that value to the Rx board
 Receive integer from Rx and print to Serial Monitor
 
 Note: The Tx and Rx boards are just names I chose. The communication is 2-way. 
 Master-slave would probably be a better option.
 */

#include <SPI.h>
#include <SD.h>
#include <UTFTQD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>




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
//=====================================================================================

const  uint8_t  bwidth=8;
uint8_t  bsync=255; 
uint8_t  val[]={bsync,1,2,3,0,5,0,7};
uint8_t  inval[bwidth];

//=====================================================================================
#define UARTclock  256000

void setup() {
   char sbuf[128];   
   int32_t  i=0;
         
   // Serial
   Serial.begin(115200);      // USB terminal
   Serial1.begin(UARTclock);  // RX-TX UART
   Serial1.parseInt();        //clear any garbage in the buffer. 
   
   
   // TFT LCD
   LCDTYPE = _UTFT_;
   initlcd(1);   
   sprintf(sbuf, "LCD=%d wi%d x hi%d",LCDTYPE,LCDmaxX,LCDmaxY);
   lcdcls(); lcdprint(sbuf);
   
   
   sprintf(sbuf, "setup(): done.");
   curlf(); curlf(); lcdprint(sbuf);
   
   lcdcls();
   
   sprintf(sbuf, "Tx master, clock= %ld", UARTclock );
   lcdprintxy(0, 0, sbuf);

}

//=====================================================================================

uint8_t checksum(uint8_t array[]) {
  uint32_t  sum=0;
  for(int i=2; i<bwidth; ++i) sum+=(array[i]);
  return (byte)(sum & 0x00ff);  
}  


//=====================================================================================

void displayvalues(int line, char * caption, uint8_t array[]) {
  int cnt;
  char sbuf[128];
  
  sprintf(sbuf, "%s cks=%4d", caption, array[1]);
  lcdprintxy(0, line, sbuf);  
  //Serial.println(sbuf);
  for(cnt=0; cnt<bwidth; ++cnt) {
    sprintf(sbuf, "%3d ", array[cnt]);      // print on TFT
    lcdprintxy(cnt*3*8, line+10, sbuf);
    //Serial.print(sbuf);                      // Print value to the Serial Monitor
  }    
  //Serial.println(); 
  
}  


//=====================================================================================
//=====================================================================================

void loop()
{
  char     sbuf[128], chk;     
  static   int cnt=0; 
  uint8_t  ibuf[bwidth];
  
 
  //   send to Rx slave Arduino
  
  //Serial.println();  
  chk=checksum(val);
  val[1]=chk;  
  for(cnt=0; cnt<bwidth; ++cnt) {        
     Serial1.write(val[cnt]);        // Send value to the Rx Arduino        
  }         
  displayvalues(20, "Transmitted...: ", val);
  

  
  //     Receive from Rx slave Arduino
  
  cnt=0; 
  memset(ibuf, 0, sizeof(ibuf));  
  
  if (Serial1.available()) {
    delay(10); //allows all serial sent to be received together
    while(Serial1.available() && cnt<bwidth) {
      ibuf[cnt++] = Serial1.read();
    }
  }   
  
    
  if( ibuf[0]==bsync ) {                       // byte 0 == syncbyte ?
     displayvalues(60, "Received...:", ibuf);
     chk=checksum(ibuf);                      
     //if(chk==ibuf[1]) {                        // chksum ok? <<<<<<<<<<< outcomment ?
        memcpy(inval, ibuf, sizeof(ibuf));
        displayvalues(100, "checked...:", inval);
     
        // change invalues to send back!
        memcpy(val, inval, sizeof(val));       // copy inbuf to outbuf
        val[0]=bsync;  
        val[4]+=1;                             // change [4] to send back 
     //}
  }   
 
}

//=====================================================================================
//=====================================================================================
