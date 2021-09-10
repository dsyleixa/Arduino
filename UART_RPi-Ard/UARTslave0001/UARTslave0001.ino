/*     Rx slave     
 *     Arduino DUE
 *     ===========
 *     ver 0006
 *     IDE 1.6.5
 */
 
#include <SPI.h>
#include <SD.h>
#include <UTFTQD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>


#define  clock()  millis()


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

//UTFT   qdUTFT(Model, SDA=MOSI,  SCL, CS,       RESET,  RS)    // Due: 3 exposed SS pins: 4,10,52
  UTFT   qdUTFT(QD220A,   A2,     A1,  A5,       A4,     A3);   // adjust model parameter and pins!
//UTFT   qdUTFT(QD220A,   50,     49,  UTFT_cs,  0,     51);   // A0->Vc (LED), A4->BoardReset
 extern  uint8_t SmallFont[];
//--------------------------------------------------------------------------------------------------
#define    tft_cs     48
#define    tft_dc     49
#define    tft_rst    47    
#define    tft_led    46   // altern.: 3.3V, 5V
#define    tft_3v3    45   // altern.: 3.3V, 5V 

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

const    uint8_t  MSGSIZE=64;
uint8_t  bsync=255; 
uint8_t  sendbuf[MSGSIZE];
uint8_t  recvbuf[MSGSIZE];


//=====================================================================================
const uint32_t UARTclock = 115200;

void setup() {
   char sbuf[128], bbuf;   
   int32_t  i=0;

   pinMode(tft_led, OUTPUT);
   digitalWrite(tft_led, HIGH);  // 3.3V
   pinMode(tft_3v3, OUTPUT);
   digitalWrite(tft_3v3, HIGH);  // 3.3V
         
   // Serial
   Serial.begin(115200);         // USB terminal
 
   Serial1.begin(UARTclock);     // RX-TX UART
   while(Serial1.available() ) bbuf=Serial1.read(); // clear Serial input buf
   sprintf(sbuf, "Rx slave, BAUD= %ld", UARTclock );
   Serial.println(sbuf);
  
      
   // TFT LCD
   Serial.println();
   /*
    * 
   */

}



//=====================================================================================
//=====================================================================================

void displayvalues(char * caption, uint8_t array[], int limit) {
  int cnt=0;
  char sbuf[128];
  
  sprintf(sbuf, "%s ", caption);
  Serial.println(sbuf);
  for(cnt=0; cnt<limit; ++cnt) {
    if(cnt%8==0) {Serial.println(); } // 8 numbers, then LF
    Serial.print(sbuf);                      // Print sendbufue to the Serial Monitor
  }    
  Serial.println();  Serial.println();  
  
}  

// ================================================================

inline uint8_t calcchecksum(uint8_t array[]) {
  int32_t  sum=0;
  for(int i=2; i<MSGSIZE; ++i) sum+=(array[i]);
  return (sum & 0x00ff); 
} 

#define checksumOK(array)  (calcchecksum(array)==array[1])


// ================================================================





//=====================================================================================
//=====================================================================================

void loop()
{  
  char     sbuf[128],  resOK, bbuf;    
  uint8_t  cbuf[MSGSIZE], chk;
  int      i;

  
  //----------------------------------------------------------
  //     Receive from  master 
  //----------------------------------------------------------
  memset(cbuf, 0, sizeof(cbuf));  

  while(!Serial1.available() );
  Serial.println("receiving....");
  
  if (Serial1.available()) {       
    i=0;
    while(Serial1.available() && i<MSGSIZE) {    
       cbuf[i] = Serial1.read();
       //if(i==0 && cbuf[i]!=bsync) continue;
       Serial.print(cbuf[i]); Serial.print(" ");
       i++;
    }
    //while(Serial1.available() ) bbuf=Serial1.read(); // clear Serial input buf
  } 
  // TCP check
  resOK=checksumOK(cbuf) ;  // && cbuf[0]==bsync
  
  // debug
  Serial.print("nrecv: ");  Serial.print(i); Serial.print("  resOK="); Serial.println(resOK);

  if( resOK ) {                                      // byte 0 == syncbyte ?
     displayvalues("Received...:", cbuf, 8);
     chk=(byte)calcchecksum(cbuf);      
     memcpy(recvbuf, cbuf, sizeof(cbuf));
  }
  else {
     sprintf(sbuf, "Rec. bsync?:%3d  resOK:%d \n", cbuf[0], resOK);
     Serial.println(sbuf);
  }


  //----------------------------------------------------------
  //    send to  master 
  //----------------------------------------------------------
  
  // prepare send buffer for testing
  // if receive succeeded: change received values to send back!
  if( resOK ) {     
        
        memcpy(sendbuf, recvbuf, sizeof(sendbuf));   // copy inbuf to outbuf
        sendbuf[4]+=10;                              // change [4] to send back  
        sendbuf[6]+=20;                              // change [6] to send back 
  }      
  
  Serial.println("sending....");  
  sendbuf[0]=bsync;
  sendbuf[1]=calcchecksum(sendbuf);
  for(uint8_t i=0; i<MSGSIZE; i++) {        
     Serial1.write(sendbuf[i]);                      // Send value to the Rx Arduino        
  }        
  
  displayvalues("Transmitted...: ", sendbuf, 8);



 
}


//=====================================================================================
//=====================================================================================
