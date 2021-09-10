/*  Rx slave     
 *  Arduino Due
 *  ===========
 *  version RCsender0010
 *  IDE 1.6.5
 */
 
// (C) Helmut Wunder (HaWe) 2015
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.6.1 - 1.6.5)
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/

 
#include <SPI.h>
#include <Scheduler.h>


// SPI header            //             |--|
// SPI pins Due          //  -----------|  |------------
  #define DUEMISO     74 //  |  RES   76_SCK   74_MISO |
  #define DUEMOSI     75 //  | -GND   75_MOSI   Vc+5V  |
  #define DUESCK      76 //  --------------------------

// SPI pins Mega
//#define MISO     50
//#define MOSI     51
//#define SCK      52



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
#define  _UTFT_       4  // Henning Karlsen UTFT 2.2-2.4" 220x176 - 320x240 lib
                           // http://henningkarlsen.com/electronics/library.php?id=51   //
#define _ILI9341_     8  // https://github.com/adafruit/Adafruit_ILI9340
                           // https://github.com/adafruit/Adafruit-GFX-Library //
#define _ILI9341due_  9  // ILI9341_due NEW lib by Marek Buriak
                           // http://marekburiak.github.io/ILI9341_due/ //
                           
//--------------------------------------------------------------------------------------------------

#define   tft_rst      0 
#define   tft_cs      48
#define   tft_dc      49
#define   _MEGAMISO_  50   
#define   _MEGAMOSI_  51
#define   _MEGASCK_   52

               
                         
//=====================================================================================
// UTFT Henning Karlsen
//=====================================================================================
#include <UTFTQD.h>  // patch for QD220A

// UTFT  qdUTFT(Model,  SDA=MOSI,   SCL,      CS,     RESET,   RS)    // Due: 3 exposed SS pins: 4,10,52
// UTFT  qdUTFT(QD220A,   A2,       A1,       A5,      A4,     A3);   // adjust model parameter and pins!
   UTFT  qdUTFT(QD220A,   23,       22,       26,      25,     24);   //
// UTFT  qdUTFT(QD220A, _MEGAMOSI_,_MEGASCK_, tft_cs, tft_rst, 47);   // A0->Vc (LED), A4->BoardReset
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
// UART DATA PACKAGES
//=====================================================================================
const    uint8_t  MSGSIZE   = 64;
const    uint32_t UARTclock = 115200;
uint8_t  bsync= 0xff,  // transmission control bytes: begin of msg signal : bsync=0xff (255)
         esync= 0xfe;  // end of msg signal: optional esync=0xfe (254)

#define  MAXDIGPINS   16
#define  MAXANALOG     9
#define  MAXMOTORS    10

uint8_t  sendbuf[MSGSIZE];
uint8_t  recvbuf[MSGSIZE];
uint8_t  sdispbuf[MSGSIZE];
uint8_t  rdispbuf[MSGSIZE];

// message array setup:

#define SYNCSLOT      0  // start sync signal of this Msg: bsync=0xff (255)
#define CKSSLOT       1  // chksum this Msg
#define BYTE0         2  // byte 0     // byte: 8-bit => 8 digital bits 0-7
#define BYTE1         3  // byte 1     // byte: 8-bit => 8 digital bits 8-15
#define ENC0          4  // motorenc 0 // 10 encoders: 32-bit
#define ENC1          8  // motorenc 1
#define ENC2         12  // motorenc 2
#define ENC3         16  // motorenc 3
#define ENC4         20  // motorenc 4
#define ENC5         24  // motorenc 5
#define ENC6         28  // motorenc 6
#define ENC7         32  // motorenc 7
#define ENC8         36  // motorenc 8
#define ENC9         40  // motorenc 9
#define ANA0         44  // analog 0   // 9 analog: 16-bit
#define ANA1         46  // analog 1   // analog 0+1 = joystick for drive
#define ANA2         48  // analog 2
#define ANA3         50  // analog 3
#define ANA4         52  // analog 4
#define ANA5         54  // analog 5
#define ANA6         56  // analog 6
#define ANA7         58  // analog 7
#define ANA8         60  // analog 8
#define BYTE2        62  // byte 2     // byte: 8-bit => 8 digital bits 16-23
#define TERM         63  // end signal of this Msg block: 


// motor runstates:

#define OUT_REGSTATE_NUL            0
#define OUT_REGSTATE_COAST          2
#define OUT_REGSTATE_BRAKE          3
#define OUT_REGSTATE_EMERG_STOP     5
#define OUT_REGSTATE_ON             8

#define OUT_REGSTATE_PIDIDLE       15

#define OUT_REGSTATE_PIDACTIVE     16
#define OUT_REGSTATE_PIDRELATIVE   17
#define OUT_REGSTATE_PIDABSOLUTE   18
#define OUT_REGSTATE_PIDHOLD       19
#define OUT_REGSTATE_PIDHOLDCLOSE  20



int8_t   digval[MAXDIGPINS];
uint16_t digvalraw=0;
int16_t  anaval[MAXANALOG];
int32_t  motenc[MAXMOTORS],  oldenc[MAXMOTORS] ;


// buffer for motor commands
uint8_t   _motorstate_[7];      // 0: MOTORSLOT: MotNr
                                // 1: mot_runstate
                                // 2: pwm
                                // 3+4: mot enc_int16
                                // 5+6: mot enc_int32
                               



//*************************************************************************************
//*************************************************************************************
// ARDUINO GPIO PIN SETUP
//*************************************************************************************
//*************************************************************************************


//=====================================================================================
// MOTORS
//=====================================================================================
// NA

//=====================================================================================
//  analog Pins
//=====================================================================================
// A0, A1: Jstick_0 (remote tank propulsion drive)

//=====================================================================================
//  digital  Pins
//=====================================================================================
// 12 dPins R/W (free) :  D02-D13
// 8 dPins R/W 16-23 :    D22-D29
// 16 dPins R/W 0-15 :    D30-D45
//    7 button pins: D30-D36
//    9 keypad pins: D37-D45
// 2 dPins (free) :       D46-D47
// 2 TFT cs+dc pins:      D48+D49
// SPI (Mega):            D50-D52
// SPI (Due):             D74-D76 
// SD cs:                 D53

#define  joystBtn_0       30
    
void setupdPins() {
   int i;   
   for (i=22; i<29; ++i) {pinMode(i, INPUT_PULLUP);}
   for (i=30; i<45; ++i) {pinMode(i, INPUT_PULLUP);}
 
}




//************************************************************************************
// bit and byte and pin operations
//************************************************************************************
// convert byte arrays to int

inline int16_t  ByteArrayToInt16(uint8_t  array[], uint8_t  slot) {
  return ((array[slot + 1] << 8) + (array[slot]));
}

inline long  ByteArrayToInt32(uint8_t  array[], uint8_t  slot) {
  return ( (array[slot+3]<<24) + (array[slot+2]<<16) + (array[slot+1]<<8) + array[slot] );
}

//------------------------------------------------------------------------------
// convert int to byte arrays

inline void Int16ToByteArray(int16_t vint16,  uint8_t *array,  uint8_t slot) {
  memcpy(array+slot*sizeof(char), &vint16, sizeof(int16_t));    // copy int16 to array
}

inline void Int32ToByteArray(int32_t vint32,  uint8_t *array,  uint8_t slot) {
  memcpy(array+slot*sizeof(char), &vint32, sizeof(int32_t));    // copy int32 to array
}

inline void FloatToByteArray(float vfloat32,  uint8_t *array,  uint8_t slot) {   
  memcpy(array+slot*sizeof(char), &vfloat32, sizeof(float));    // copy float to array
}

//------------------------------------------------------------------------------
// read+write bits in numbers
/*
#define bitRead(source, bit) ( ((source) >> (bit)) & 0x01 )
#define bitSet (source, bit) ( (source) |= (1UL << (bit)) )
#define bitClear(source, bit) ( (source) &= ~(1UL << (bit)) )
#define bitWrite(source, bit, bitvalue) ( bitvalue ? bitSet(source, bit) : bitClear(source, bit) )
*/


//------------------------------------------------------------------------------------

int16_t  toggleup(int16_t  nr,  int16_t  max) {
  if ( nr < (max - 1) ) ++nr;
  else nr = 0;
  return nr;
}


//------------------------------------------------------------------------------------

#define sensortouch(pinHIGH) !digitalRead(pinHIGH)

//------------------------------------------------------------------------------------







//*************************************************************************************
//*************************************************************************************
// UART TRANSMISSION CONTROL
//*************************************************************************************
//*************************************************************************************


//=====================================================================================
// CHECKSUM
//=====================================================================================

uint8_t calcchecksum(uint8_t array[]) {
  int32_t  sum=0;
  for(int i=2; i<MSGSIZE; ++i) sum+=(array[i]);
  return (sum & 0x00ff);
}

bool checksumOK(uint8_t array[]){
return (calcchecksum(array)==array[1]);
}

//=====================================================================================
// UART SEND/RECEIVE BUFFER
//=====================================================================================
// addToBuffer and receive function courtesy of chucktodd

bool addToBuffer( uint8_t buf[], uint8_t *cnt, uint16_t timeout){
bool inSync = *cnt>0;
unsigned long start=millis();
while((*cnt<MSGSIZE)&&(millis()-start<timeout)){
  if(Serial1.available()){ // grab new char, test for sync char, if so start adding to buffer
    buf[*cnt] = (uint8_t)Serial1.read();
    if(inSync) *cnt += 1;  // my origional *cnt++ was updating the pointer address, not
                           // the pointed to sendbuffer
    else{
     if(buf[*cnt]==0xFF){
       inSync = true;
       *cnt +=1;
       }
     }
    }
  }
return (*cnt==MSGSIZE);
}


//=====================================================================================

bool receive(uint8_t * buf, uint16_t timeout, uint8_t *cnt){ // by passing cnt in and out,
// i can timeout and still save a partial buffer, so a resync costs less (less data lost)

bool inSync=false;
unsigned long start=millis();
uint8_t * p;  // pointer into buf for reSync operation
bool done=false;

  do
  {
    done = addToBuffer(buf,cnt,timeout); // if this return false, a timeout has occured, and the while will exit.
    if(done){ // do checksumOK test of buffer;
      done=checksumOK(buf);
      if (!done){// checksumOK failed, scan buffer for next sync char
         p = (uint8_t*)memchr((buf+1),0xff,(MSGSIZE-1)); //forgot to skip the current sync at 0
       
       
         if(p){ // found next sync char, shift buffer content, refill buffer
           *cnt = MSGSIZE -(p-buf); // count of characters to salvage from this failure
           memcpy(buf,p,*cnt); //cnt is now where the next character from Serial is stored!
           }
         else *cnt=0; // whole buffer is garbage
         }
      }
   
  } while(!done&&(millis()-start<timeout));

  return done; // if done then buf[] contains a sendbufid buffer, else a timeout occurred
}







//=====================================================================================
// DISPLAY VALUES
//=====================================================================================

void displayvalues(int line, char * caption, uint8_t array[]) {
  int cnt;
  char sbuf[128];
 
  sprintf(sbuf, "%s cks=%-4d", caption, array[1]);
  lcdprintxy(0, line, sbuf);
  Serial.println(sbuf);
  for(cnt=0; cnt<8; ++cnt) {
    sprintf(sbuf, "%3d ", array[cnt]);      // print on TFT
    lcdprintxy(cnt*3*8, line+10, sbuf);
    Serial.print(sbuf);                      // Print sendbufue to the Serial Monitor
  }   
  Serial.println();
 
}


//=====================================================================================
// acquire remote control values for send buffer
//=====================================================================================
int16_t  analogTmpx=0, analogTmpy=0;
int16_t  throttle, direction = 0; //throttle (Y axis) and direction (X axis)

int16_t  leftMotor,leftMotorScaled = 0;

float    leftMotorScale = 0;

int16_t  rightMotor,rightMotorScaled = 0; //right Motor helper variables
float    rightMotorScale = 0;

float    maxMotorScale = 0; //holds the mixed output scaling factor

const byte joyst_0XA = A0; //Analog Jostick X axis  // Joyst Anschluesse links
const byte joyst_0YA = A1; //Analog Jostick Y axis

//=====================================================================================

void getremotevalues()
{   
   char sbuf [128];
   int16_t anabuf; 
   int8_t  digbuf;
  
  //aquire the analog input for Y  and rescale the 0..1023 range to -255..255 range
  analogTmpx = 1024 - analogRead(joyst_0YA);    
  throttle =  (512-analogTmpx)/2;   
  //...and  the same for X axis
  analogTmpy = 1024- analogRead(joyst_0XA);
  direction  =  (512-analogTmpy)/2;
  sprintf(sbuf, "x:%6d, y:%6d ", analogTmpx, analogTmpy);  Serial.print(sbuf); 

  //mix throttle and direction
  leftMotor  =  -throttle + direction;    
  rightMotor =  -throttle - direction;
  //print the initial mix results
  sprintf(sbuf, "LIN:%6d, RIN:%6d ", leftMotor, rightMotor);  Serial.print(sbuf); 

  //calculate the scale of the results in comparision base 8 bit PWM resolution
  leftMotorScale =  leftMotor/255.0;
  leftMotorScale = abs(leftMotorScale);
  rightMotorScale =  rightMotor/255.0;
  rightMotorScale = abs(rightMotorScale);
  sprintf(sbuf, "| LSCALE:%4.1f, RSCALE:%4.1f ", leftMotorScale, rightMotorScale);  Serial.print(sbuf);

  //choose the max scale value if it is above 1
  maxMotorScale = max(leftMotorScale,rightMotorScale);
  maxMotorScale = max(1,maxMotorScale);

  //and apply it to the mixed values
  leftMotorScaled  = constrain(leftMotor/maxMotorScale,-255,255); 
  rightMotorScaled = constrain(rightMotor/maxMotorScale,-255,255);
  sprintf(sbuf, "| LOUT:%6d,  ROUT:%6d ", leftMotorScaled, rightMotorScaled);  Serial.print(sbuf);
  Serial.println("");
 
  Int16ToByteArray(leftMotorScaled, sendbuf, ANA0);
  Int16ToByteArray(rightMotorScaled, sendbuf, ANA1);

  digval[0] = digbuf = sensortouch(joystBtn_0);
  bitWrite(anabuf, 0, digbuf);
  Int16ToByteArray(anabuf, sendbuf, BYTE0);

  delay(1);


}





//=====================================================================================
// L O O P
//=====================================================================================

void loop()
{
  char     sbuf[128],  resOK;   
  static   uint8_t cnt=0;
  uint8_t  cbuf[MSGSIZE], chk;
  uint32_t xtime;
 

  //     Receive fromTx master

  memset(cbuf, 0, sizeof(cbuf));
   
  // debug
  resOK = receive ( cbuf, 10000,&cnt);
 
  if( resOK ) {                                      // byte 0 == syncbyte ?
    cnt=0;

    //displayvalues(60, "Received...:", cbuf);
     chk=(byte)calcchecksum(cbuf);     
     memcpy(recvbuf, cbuf, sizeof(cbuf));
     memcpy(rdispbuf, cbuf, sizeof(cbuf));
   }

 
  //   send to Tx master
  memset(sendbuf, 0, sizeof(sendbuf));
  getremotevalues();
 
  //TCP sync bytes
  sendbuf[0]=bsync;
  //sendbuf[TERM]=esync;
 
  /*
  // debug: fake dig pins
  digvalraw++;
  Int16ToByteArray(digvalraw, sendbuf, BYTE0); 
  */
 
  // checksum
  sendbuf[1]=calcchecksum(sendbuf);

  // Send value to the Rx Master
  for(uint8_t i=0; i<MSGSIZE; i++) {       
     Serial1.write(sendbuf[i]);                             
  }        

  memcpy(sdispbuf, sendbuf, sizeof(sendbuf));

}

//=====================================================================================
// L O O P 2:  Display
//=====================================================================================

void loop2() {
   char     sbuf[128];  
   
   sprintf(sbuf, "JSt_0 | Le=%4d | Ri=%4d | Btn=%1d |", leftMotorScaled, rightMotorScaled, digval[0] );
   lcdprintxy(0, 20, sbuf);

   yield();
   delay(50);
   yield();
}


//*************************************************************************************
//*************************************************************************************
// setup
//*************************************************************************************
//*************************************************************************************


void setup() {
   char sbuf[128];   
   int32_t  i=0;
         
   // Serial
   Serial.begin(115200);   // USB terminal
 
   Serial1.begin(UARTclock);                    // RX-TX UART
   while(Serial1.available())  Serial1.read();  // clear output buffer

   
   // TFT LCD
   Serial.println();
   LCDTYPE = _ILI9341due_;
   Serial.print("init LCD...");
   initLCD(3);   
   Serial.println(" done.");   lcdcls();
   sprintf(sbuf, "LCD=%d wi%dxhi%d Font %dx%d",LCDTYPE,LCDmaxX,LCDmaxY,fontwi,fonthi);
   Serial.println(sbuf);
   Serial.println();
   lcdcls(); lcdprint(sbuf);

   setupdPins();   
   sprintf(sbuf, "dpins(): done.");
   Serial.println(); Serial.println(sbuf);   
   curlf(); lcdprint(sbuf);   
   
   sprintf(sbuf, "Rx slave, BAUD= %ld", UARTclock );;
   curlf(); lcdprint(sbuf);

   // Add "loopX"  to scheduling.
   Scheduler.startLoop(loop2); 
   sprintf(sbuf, "MT Scheduler started", UARTclock );;
   curlf(); lcdprint(sbuf);

   sprintf(sbuf, "setup(): done.");
   Serial.println(); Serial.println(sbuf);   
   curlf(); curlf(); lcdprint(sbuf);

   sprintf(sbuf, "press JoystickButton 0 to start!");
   Serial.println(); Serial.println(sbuf);   
   curlf(); curlf(); lcdprint(sbuf);

   while( !sensortouch( joystBtn_0 ) );   while(  sensortouch( joystBtn_0 ) );
   lcdcls(); 

}




//=====================================================================================
//=====================================================================================
