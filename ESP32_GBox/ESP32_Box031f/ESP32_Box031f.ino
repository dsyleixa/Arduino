// ESP32 Gamebox
// Adafruit_HX8357_ini feat.:
//    Adafruit_HX8357 3.5" TFT
//    Adafruit_STMPE610 TS
//    Adafruit_ImageReader
//    SD
//    Fonts
//    Color names
// ads1115
// PCF8574
// MPU6050
// analog joystick
// analog 5x button pad
// analog 4x button pad

// ver 3.1e


// change log:
// 3.1. SDfilelistFix d,e:chessInit+stopThink :chesstrash
// 3.0. c:SDlist2Heap; d:SDVector da:illegal
// 3.0: ChessMoveHistory
// 2.9. d:redrawU e,f:redrawTS
// 2.9: chess GUI ts moves (based 2.6.g) a:rework illegal b:ts:reset c:fixes
// 2.6: chess GUI a:xyoffs b:no ASCII board c:ts-sqr d:ts-from/to
// 2.5: UI, kibitz, GUI; e:LCG_rand f:NTP time g:Quit_CHESS
// 2.4: c+p ESP basic program
// 2.2: a:PAINT  b-f:CHESS g: noGraph
// 2.1: SD Explorer select/unselect;
// 2.0: dir fixes a: Explorer c: SD-readStringUntil(EoL) d:SdExist(SD)-workaround
// 1.9: ImageReader, TxtReader, PONG fix; a: Menu select
// 1.8: racket pos offset a: speed+reflex, btn-dn, b: reflex  c: TS btn
// 1.7: Pong racket moves b: edge reflex fix c: dyn racketspeed
// 1.6: Pong a: field b: reflex c: rackets
// 1.5: variable IPs, read config from SD - a: option: TS E/D; b: new Menu
// 1.4: MT fixes, delay(), SD config.txt
// 1.3: website (Wifiserver)
// 1.2: website (Webserver)
// 1.1: std::thread
// 1.0: console GameBoy-like; a,b: ads1115 buttonpads fixed
// 0.9: WiFi  + Webserver + Time tm * timeinfo
// 0.8: WiFi  + Webserver example HelloServer
// 0.7: TFT debug line, TSbutton1-4, MPU6050
// 0.6: SD ls() + file select
// 0.3: ads1115


static bool DEBUG=true;


//---------------------------------------------------------------------
// brownout detector
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

//---------------------------------------------------------------------
#include <thread>
#include <freertos/task.h>
#include <mutex>

#define msleep(t) std::this_thread::sleep_for(std::chrono::milliseconds(t))

std::thread *thread_1;


std::mutex display_mutex;

volatile static int8_t THREADRUN=1, THREADSUSPENDED=0;



//===========================================================
// TFT
//===========================================================


#include <TFT_HX8357.h>
/*  TFT_HX8357.h featuring:
       #include "Adafruit_GFX.h"
       #include "Adafruit_HX8357.h"
       #include <color16bit.h>  // 16 bit color defs
       #include <Fonts/FreeSans9pt7b.h>
       #include <Fonts/FreeMono12pt7b.h>
       #include <Fonts/FreeMono9pt7b.h>
       Adafruit_HX8357_ini(orientation);
*/
#include <Fonts/Tiny3x3a2pt7b.h>

char   sbuf[200]; // for all print()


//Adafruit_ImageReader reader;            // from  <TFT_HX8357.h>
Adafruit_Image sqrWhite, sqrBlack;        // An image loaded into RAM


//-------------------------------------------
// TFT size + colors

#define MAXDISPX display.width()
#define MAXDISPY display.height()

int    COLOR_BGND = BLACK;
int    TFT_ROTAT  = 3;




//===========================================================
// virtual Screen LED
//===========================================================

void LED_writeScreen(int LED_pin_STATE) {
   //digitalWrite(LED_pin, LED_pin_STATE);
   if(LED_pin_STATE)
      display.fillCircle(display.width()-6, display.height()-6,6, LIME);
   else display.fillCircle(display.width()-6, display.height()-6,6, DARKBLUE);
}





//===========================================================
// SD
//===========================================================
File SD_File;

#include <Ardustdio.h>
#include <stringEx.h>

//=================================================================
static int    cursormarknr=0, selectfilenr=-1;
static String fileMarked="", fileSelected="";

//-------------------------------------------
// list dir
int lswinmin=0, oldwinmin=-2;

//-------------------------------------------
// mark curs pos for file select

void markPos(int cursornr) {
   display.fillRect ( 0,0, 14,MAXDISPY, COLOR_BGND);
   display.setTextColor(LIME);
   display.setCursor( 0, 0+20*(cursornr-lswinmin) );
   display.print(">");
   display.setTextColor(WHITE);
   delay(50);
}

void ls(std::vector<String> list) {
   int cnt=0, cursornr=cursormarknr;

   display.setRotation(TFT_ROTAT);
   lswinmin=0;
   cursornr=max(0,cursormarknr);             // cursormarknr starts from -1
   if( cursornr>=13 && ( cursornr-lswinmin>=13 || cursornr-lswinmin<=2) ) {
      lswinmin=( cursornr-2 - (cursornr%13)  ) ;
   }
   else {
      lswinmin=0;
   }
   Serial.println("print list[]:");
   if(oldwinmin!=lswinmin) {
      display.fillScreen(BLACK);  // <<<<<<< todo
   }

   for (cnt = lswinmin; cnt < filecount; cnt++) {
      if (cnt < 100) Serial.print(" ");
      if (cnt < 10)  Serial.print(" ");
      Serial.print((String)cnt+":");
      Serial.println( list[cnt] );
      int COLOR_TEXT;
      if(cnt==selectfilenr) COLOR_TEXT=LIME;
      else COLOR_TEXT=WHITE;
      display.setTextColor(COLOR_TEXT);
      display.setCursor(20,  20*(cnt-lswinmin) );  // <<<< filepos
      strncpy(sbuf,list[cnt].c_str(),198 );
      if(strlen(sbuf)>36) {
         strpatch(sbuf, "...", 19);
         while(strlen(sbuf)>36) {
            strdelnpos(sbuf, 23, 1);
         }
      }
      display.print(sbuf);
   }
   oldwinmin=lswinmin;

   markPos(cursormarknr);
   Serial.println("print list[] done!\n");
}
//-------------------------------------------
// SD_Reset()

int SD_Reset() {
   display.setTextColor(WHITE);
   Serial.print("try SDPath.close: ");
   SdPath.close();
   Serial.println("closed!");
   delay(100);

   //SD card removed ?
   Serial.print("try SD check: ");
   //SDioerr=SD.begin(SD_CS);
   SDioerr = SdExist(SD);

   // debug
   //while(1) delay(100);

   delay(10);
   if( !SDioerr ) {
      Serial.println("SD access  failed!");
      Serial.println();
      display.println("SD access failed!");
      delay(1000); // delay here
   }
   if( SDioerr )  {
      Serial.println("SD OK.");
      Serial.println();
      display.println("SD OK.");
   }
   SdPath = SD.open("/");
   Serial.println("SD setup done!\n");
   display.setTextColor(WHITE);

   display.println("SD setup done!");
   filecount = 0;
   selectfilenr = -1;
   cursormarknr = 0;
   readDirectory(SdPath, 0);
   return SDioerr;
}

//===========================================================
// simple LCG random tool
//===========================================================
static int rand_last = 0;

static int LCG_rand()  {
   rand_last = (rand_last * 1103515245 + 12345) & 0x7fffffff;
   return rand_last;
}




//===========================================================
// I/O
//===========================================================
#include <Wire.h>
#include <ardustdio.h>  // file I/O, String filelist
#include <arduarray.h>  //
#include <stringEx.h>   // 


static bool     LED_pin_STATE=false;
int LED_pin  =  LED_BUILTIN;
int LED1pin  =  LED_BUILTIN;
uint8_t OUT1 =  LOW;




//===========================================================
// ADC
//===========================================================

#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads0;

static uint16_t adc00, adc01, adc02, adc03, adcraw;
static uint16_t btn00=0, btn01=0;

void read_ADS0() {
   static int valmax=17600;

   adcraw=adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
   adc00 = mapConstrain(adc00, 0, valmax);
   btn00 = btnstate00(adc00);
   delay(2);

   adcraw=adc01 = ads0.readADC_SingleEnded(1);  // ADS1115 port A1
   adc01 = mapConstrain(adc01, 0, valmax);  // 4Btn pad raw
   delay(2);

   adcraw=adc02 = ads0.readADC_SingleEnded(2);  // ADS1115 port A2
   adc02 = mapConstrain(adc02, 0, valmax);  // Poti
   delay(2);

   adcraw=adc03 = ads0.readADC_SingleEnded(3);  // ADS1115 port A3
   adc03 = mapConstrain(adc03, 0, valmax);  // Poti
   delay(2);
}


//===========================================================
// analog 5-Button Pad (10bit):
// 0 NONE=800-1010
//          1 esc=250-400
//          2 up= 110-200
// 3 left=18-40     4 right=50-90
//          5 dn=1020-1024
//


int16_t PAD0_NONE  =  800,   // ADC min values
        PAD0_ESC   =  250,
        PAD0_UP    =  110,
        PAD0_LEFT  =   18,
        PAD0_RIGHT =   50,
        PAD0_DOWN  = 1020;

//===========================================================
int32_t btnstate00(int val) {
   if (isInRange( adc00,  PAD0_NONE, 1010 ) ) return 0; // NONE
   if (isInRange( adc00,  PAD0_ESC,   400 ) ) return 1; // ESC
   if (isInRange( adc00,  PAD0_UP,    200 ) ) return 2; // up
   if (isInRange( adc00,  PAD0_LEFT,   40 ) ) return 3; // left
   if (isInRange( adc00,  PAD0_RIGHT,  90 ) ) return 4; // right
   if (isInRange( adc00,  PAD0_DOWN, 1024 ) ) return 5; // down
   if (isInRange( adc00,  0,   2 ) )         return 5; // down
}


//===========================================================
// MPU6050
//===========================================================

#include "data\MPU6050Kalman.h"
// MPU6050dmp6noIRQ
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"


int8_t   yaw, pitch, roll;
float    fyaw, fpitch, froll;
int16_t  accx, accy, accz;




//===========================================================
// WIFI + WEB + NTP TIME
//===========================================================

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

String SSID         = "WLAN-";
String PASSWORD     = "1865-4657";
String StringGW_IP  = "192.168.2.1";
String StringLOC_IP = "192.168.2.202";
String StringSUB_IP = "255.255.255.0";

IPAddress LOCALIP;
//IPAddress LOCALIP(192, 168,  2, 202);
IPAddress GATEWAY;
//IPAddress GATEWAY (192, 168,  2,  1);
IPAddress SUBNET;
//IPAddress SUBNET  (255, 255, 255, 0);

int WIFIPORT = 8008;
int WEBPORT =  8084;

WiFiServer   wifiserver(WIFIPORT);
WebServer    webserver(WEBPORT);

//------------------------------------
#include <time.h>
time_t now;
tm     tm;      // the structure tm holds time information in a more convient way


char* ntpServer = "at.pool.ntp.org";
// choose your time zone from this list
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
// Europe/Berlin    CET-1CEST,M3.5.0,M10.5.0/3



//------------------------------------
void getLocalTime(char * buffer) {
   time(&now); // read the current time
   localtime_r(&now, &tm); // update the structure tm with the current time
   strftime (buffer,80,"%a %d %b %Y %H:%M:%S ", &tm);
}

//------------------------------------
void displayLocalTime()
{
   char buffer [80];
   Serial.println("Date & Time:  ");

   getLocalTime(buffer);
   Serial.println(buffer);


   display.setTextSize(3);
   display.fillRect(0,130, display.width(),30, COLOR_BGND);  // x1,y1, dx,dy
   display.setTextColor(GREEN);
   display.setCursor(0, 60);
   display.print("Date & Time:  ");
   display.setCursor(0, 130);
   display.print(buffer);
   display.setTextSize(2);
   display.setTextColor(WHITE);

}





//===========================================================
// TOUCHSCREEN
//===========================================================

Adafruit_GFX_Button TSbutton0, TSbutton1, TSbutton2, TSbutton3, TSbutton4, TSbutton5;

int tsx=-1, tsy=-1, tsz=-1;

//---------------------------------------------------------------------
int TSBUTTONSENABLED = 1;

void drawAllTSbuttons() {

   if(TSBUTTONSENABLED) {
      TSbutton0.drawButton();
      TSbutton1.drawButton();
      TSbutton2.drawButton();
      TSbutton3.drawButton();
      TSbutton4.drawButton();
      TSbutton5.drawButton();
   }
}

int transformTSrotat() {
   if (TFT_ROTAT==3 ) {
      tsx=p.y;
      tsy=p.x;
      ///// !!!!! /////
      tsy=tsy-10; // tsy shift corr!
   }
   else {
      tsx=p.x;
      tsy=p.y;
   }
   return TFT_ROTAT;
}


//-------------------------------------------

int GetTSbuttons() {
   //char str1[30];
   int btnUp=-1;

   //-------------------------------------------
   // Touchscreen readings
   // My new changes
   //-------------------------------------------

   btnUp=-1;

   if (ts.touched()) {
      // Empty the touchscreen buffer to ensure that the latest touchscreen interaction is held in the variable
      while ( ! ts.bufferEmpty() ) {
         p = ts.getPoint();
      }
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // clear the 'touched' interrupts
   }
   // end my changes-------------------------------------------------


   // Scale from ~0->4000 to display.width using the calibration #'s
   p.x = map(p.x, TS_MAXX, TS_MINX, 0, display.height());
   p.y = map(p.y, TS_MINY, TS_MAXY, 0, display.width());
   transformTSrotat();
   tsz = p.z;

   if(DEBUG) {
      /*
         Serial.print("X = "); Serial.print(p.x);
         Serial.print("\tY = "); Serial.print(p.y);
         Serial.print("\tPressure = "); Serial.println(p.z);
         Serial.println();
      */
      /*
         sprintf(sbuf, "x%3d y%3d z%3d", p.x, p.y, p.z);
         display.setTextColor(WHITE);
         display.fillRect(display.width()-80,40, 80,40, BLACK);  // x1,y1, dx,dy
         display.setCursor(display.width()-75,40);    display.print(sbuf);
      */
   }
   //-------------------------------------------
   // check if a TSbutton was pressed
   //-------------------------------------------
   TSbutton0.press(TSbutton0.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton1.press(TSbutton1.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton2.press(TSbutton2.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton3.press(TSbutton3.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton4.press(TSbutton4.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton5.press(TSbutton5.contains(p.y, p.x)); // tell the TSbutton it is pressed

   //-------------------------------------------
   // process TSbutton states+changes
   //-------------------------------------------

   if (TSbutton0.justReleased() ) {
      btnUp=0;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton0.justPressed()) {
      if(TSBUTTONSENABLED) TSbutton0.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton1.justReleased() ) {
      btnUp=1;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton1.justPressed()) {
      if(TSBUTTONSENABLED) TSbutton1.drawButton(true); // draw invert!
   }

   //-------------------------------------------


   else if (TSbutton2.justReleased() ) {
      btnUp=2;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton2.justPressed()) {
      if(TSBUTTONSENABLED) TSbutton2.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton3.justReleased() ) {
      btnUp=3;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton3.justPressed()) {
      if(TSBUTTONSENABLED) TSbutton3.drawButton(true); // draw invert!
   }


   else if (TSbutton4.justReleased() ) {
      btnUp=4;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton4.justPressed()) {
      if(TSBUTTONSENABLED) TSbutton4.drawButton(true); // draw invert!
   }


   else if (TSbutton5.justReleased() ) {
      btnUp=5;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton5.justPressed()) {
      if(TSBUTTONSENABLED) TSbutton5.drawButton(true); // draw invert!
   }

   delay(2);  // <<<<<<<<<<<<<<<< new 1.8c

   //-------------------------------------------

   if (ts.touched()) {
      // Empty the touchscreen buffer
      while ( ! ts.bufferEmpty() ) {
         p = ts.getPoint();
      }
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // clear the 'touched' interrupts
      delay(2);
   }
   else tsx = tsy = tsz = -1;
   p.x = p.y =-1;

   return btnUp;
}





//===========================================================
// PONG
//===========================================================

#define CENTDIX MAXDISPX/2.0
#define CENTDIY MAXDISPY/2.0
#define CENTRECT (MAXDISPY-RECHIGH)/2.0
float BALLRAD = 8.0;
float RECHIGH = 100.0;
float RECWIDX = 5.0;
float RECOFFS = 50.0;

float Ballx=CENTDIX, Bally=CENTDIY, lRposy=CENTRECT, rRposy=CENTRECT;
float oldbx=CENTDIX, oldby=CENTDIY, oldlRy=CENTRECT, oldrRy=CENTRECT;

bool  side=1;
int   ScoreL=0, ScoreR=0;

float RvMin=1, RvMax=1, lRvy=1, rRvy=1;
float Bvx=1, Bvy=1, BvMin=1, BvMax=1, BvDynMax=1;   // ball speed params
bool  BallOff = false;


//-------------------------------------------
void PaintBall(float x, float y, bool bIn=true) {
   display.fillCircle(oldbx, oldby, BALLRAD, COLOR_BGND);
   if(bIn) display.fillCircle(x, y, BALLRAD, LIGHTBLUE);
   else display.fillCircle(x, y, BALLRAD, RED);
   oldbx=x; oldby=y;
}
//-------------------------------------------
void PaintLRec(float y)  {
   display.fillRect(RECOFFS, oldlRy, RECWIDX, 100, COLOR_BGND);
   display.fillRect(RECOFFS, y, RECWIDX, RECHIGH, WHITE);
   oldlRy=y;
}
//-------------------------------------------
void PaintRRec(float y)  {
   display.fillRect(MAXDISPX-RECWIDX-RECOFFS, oldrRy, RECWIDX, 100, COLOR_BGND);
   display.fillRect(MAXDISPX-RECWIDX-RECOFFS, y, RECWIDX, RECHIGH, WHITE);
   oldrRy=y;
}

//-------------------------------------------
void ResetToDefaults() {
   side=random(2);
   ScoreL=0; ScoreR=0;

   RECHIGH = 100.0;
   lRposy=CENTRECT;
   rRposy=CENTRECT;
   lRvy=0;
   rRvy=0;
   RECHIGH = 100.0;
   PaintLRec(lRposy);
   PaintRRec(rRposy);
   RvMax=40.0;           // Racket max speed
   RvMin=BALLRAD;        // Racket min speed
   BvMin=10.0;           // ball speed rand min (xy)
   BvMax=1.5*BvMin;      // ball speed rand max (xy)
   BvDynMax=1.5*BvMax;   // ball dynamic speed max (y)
   Bvx=random(BvMin,BvMax);
   Bvy=random(BvMin/2,BvMax);
   if(!side) Bvx=-Bvx;
   Ballx=CENTDIX+111-side*222;
   Bally=CENTDIY;
   BallOff=false;
}

//-------------------------------------------

void Pong() {
   bool PONGACTIVE=true;
   bool StopMode=false;
   float dvtemp;     // racket speed params

   // read all Buttons
   read_ADS0();

   display.setTextColor(YELLOW);
   display.setCursor(  0, 140 );   display.print("Pong started");
   display.setTextColor(YELLOW);
   display.setCursor(  0, 180 );   display.print("Quit: press BtnPad Top");
   display.fillScreen(COLOR_BGND);

   ResetToDefaults();
   PaintBall(Ballx, Bally);
   delay(1000);

   // Scores
   //display.fillRect(150, 0, 200, 10, COLOR_BGND);

   while(PONGACTIVE) {
      //display.fillRect(150, 0, 200, 35, COLOR_BGND);
      display.setTextColor(YELLOW);
      display.setTextSize(4);
      display.setCursor(  150, 5 );   display.print(ScoreL);
      display.setCursor(  300, 5 );   display.print(ScoreR);
      display.setTextSize(2);

      // read all Buttons
      read_ADS0();

      // left Racket move
      if(lRvy==0) dvtemp=RvMin;
      else if(lRvy<=6) dvtemp=2*BALLRAD;
      else if(lRvy<=20) dvtemp=40;
      else dvtemp=80;

      if(adc02>640)      lRvy= lRvy -dvtemp;   // Joystick (left)
      else if(adc02<420) lRvy= lRvy +dvtemp;
      else {
         lRvy=0;   // Poti middle=500-600
      }
      if(lRvy<-RvMax) lRvy=-RvMax;
      else if(lRvy>RvMax) lRvy=RvMax;

      // right Racket move
      if(rRvy==0) dvtemp=RvMin;
      else if(rRvy<=6) dvtemp=2*BALLRAD;
      else if(rRvy<=20) dvtemp=40;
      else dvtemp=80;

      if(btn00==2)      rRvy= rRvy -dvtemp;   // Btn Pad (right)
      else if(btn00==5) rRvy= rRvy +dvtemp;
      else {
         rRvy=0;   // Rack middle
      }
      if(rRvy<-RvMax) rRvy=-RvMax;
      else if(rRvy>RvMax) rRvy=RvMax;

      // Pause: button=4
      if(btn00==4) StopMode=true; // pause
      while(StopMode) {
         while(btn00==4) {
            read_ADS0();
            delay(50);
         }
         while(btn00!=4) {
            read_ADS0();
            delay(50);
         }
         if(btn00==4)  StopMode=false;
         read_ADS0();
         while(btn00==4) {
            read_ADS0();
            delay(50);
         }
      }

      lRposy=lRposy+lRvy; // racket left move vector
      rRposy=rRposy+rRvy; // racket right move vector

      if(lRposy>MAXDISPY-RECHIGH-1) lRposy=MAXDISPY-RECHIGH-1;
      if(lRposy<0) lRposy=0;
      if(rRposy>MAXDISPY-RECHIGH-1) rRposy=MAXDISPY-RECHIGH-1;
      if(rRposy<0) rRposy=0;

      PaintLRec(lRposy);
      PaintRRec(rRposy);
      delay(1);

      Ballx=Ballx+Bvx; // ball x move vector
      Bally=Bally+Bvy; // ball x move vector

      // bottom/top reflexion
      if(Bally>MAXDISPY-BALLRAD-1) {
         Bally=MAXDISPY-BALLRAD-1;
         Bvy=-Bvy;
      }
      else if(Bally<BALLRAD)           {
         Bally=BALLRAD;
         Bvy=-Bvy;
      }

      else
         // racket hit
         // left Racket
         if( Ballx>RECOFFS-2*BALLRAD  && Ballx<=RECOFFS+RECWIDX+BALLRAD
               && Bally>=lRposy && Bally<=lRposy+RECHIGH )
         {
            if(Bally==lRposy || Bally==lRposy+RECHIGH)  Bvy=-Bvy;
            Ballx=RECOFFS+RECWIDX+BALLRAD;
            Bvy=Bvy + (lRvy/3);
            Bvx=-Bvx;
            Bvx=Bvx + random(-1,1);
         }
         else if(Ballx<=BALLRAD+1)  { //   => to off
            BallOff=true;
            ScoreR+=1;
            //Ballx=BALLRAD; Bvx=-Bvx;
         }

         else
            // right Racket
            if( Ballx<MAXDISPX-RECOFFS+2*BALLRAD && Ballx>=MAXDISPX-RECOFFS-RECWIDX-BALLRAD
                  && Bally>=rRposy && Bally<=rRposy+RECHIGH )
            {
               if(Bally==rRposy || Bally==rRposy+RECHIGH)
                  Bvy=-Bvy;
               Ballx=MAXDISPX-RECOFFS-RECWIDX-BALLRAD;
               Bvy=Bvy + (rRvy/3);
               Bvx=-Bvx;
               Bvx=Bvx + random(-1,1);
            }
            else if(Ballx>MAXDISPX-BALLRAD-1) { //  => to off
               BallOff=true;
               ScoreL+=1;
               //Ballx=MAXDISPX-BALLRAD-1; Bvx=-Bvx;
            }

      if(Bvy>BvDynMax)  Bvy=BvDynMax;
      if(Bvy<-BvDynMax) Bvy=-BvDynMax;

      if ( /*adc01>=400 || */ btn00==1) {
         display.setTextColor(RED);
         display.setCursor( 100, 240 ); display.print("terminated");
         delay(500);
         PONGACTIVE=false;
         btn00=1;
         return;
      }

      if(BallOff) {
         if(Ballx<0) Ballx=1;
         if(Ballx>MAXDISPX) Ballx=MAXDISPX;
         PaintBall(Ballx, Bally, false);
         delay(150);
         PaintBall(Ballx, Bally, true);
         delay(150);
         PaintBall(Ballx, Bally, false);
         delay(150);

         side=!side;
         if(!side) {   // Level++ !!    // increase Score level by -- Racket size
            if(RECHIGH>=3*BALLRAD) RECHIGH-=0.7;
         }
         if( (ScoreL+ScoreR)%10==0 ) {  // increase Score level by ++ ball speed
            if(BvMin<15) BvMin+=0.5;
            if(BvMax<20) BvMax+=0.5;
         }

         display.fillRect(150, 0, 200, 35, COLOR_BGND);
         display.setTextColor(YELLOW);
         display.setTextSize(4);
         display.setCursor( 150, 5 );   display.print(ScoreL);
         display.setCursor( 300, 5 );   display.print(ScoreR);
         display.setTextSize(2);

         if(ScoreL==100 || ScoreR==100) {         // ball off, Score=Max
            display.setCursor( 100, 100 );
            display.setTextSize(4);
            if(ScoreL>ScoreR) display.print("Player 1 WON!");
            else display.print("Player 2 WON!");
            display.setTextSize(2);
            display.setCursor( 100, 160 );
            display.print("new Game? press Btn-left! ");
            read_ADS0();
            while(btn00!=3 && btn00!=1) {
               read_ADS0();
               if (btn00==1) {
                  display.setTextColor(RED);
                  display.setCursor( 100, 240 );
                  display.print("terminated");
                  delay(500);
                  PONGACTIVE=false;
                  btn00=1;
                  return;
               }
               delay(50);
            }
            display.fillScreen(COLOR_BGND);
            ResetToDefaults();
            delay(500);
         }
         else {                                    // ball off, Score<100
            Bvx=random(BvMin,BvMax);
            if(!side) Bvx=-Bvx;
            Bvy=random(BvMin/2,BvMax);
            if(random(2)) Bvy=-Bvy;
            BallOff=false;
         }

         Ballx=CENTDIX+111-side*222;
         Bally=CENTDIY;
         PaintBall(Ballx, Bally);
         lRvy=0;
         rRvy=0;
         lRposy=CENTRECT;
         rRposy=CENTRECT;
         PaintLRec(lRposy);
         PaintRRec(rRposy);

         delay(500);
         //display.fillScreen(COLOR_BGND);
         display.fillRect(150, 0, 200, 35, COLOR_BGND);
      }

      else {                                       // not ball off
         display.fillRect(150, 0, 200, 35, COLOR_BGND);
         PaintBall(Ballx, Bally);
      }

      delay(1);
   }
}


//===========================================================
// PONG END
//===========================================================
//

//===========================================================
// PAINT
//===========================================================

void Paint() {
   bool PAINTACTIVE = true;
   static int BOXSIZE = 26;
   static int PENRADIUS = 3;
   static int currentcolor, oldcolor;

   static int x=-1, y=-1, z=-1;
   int tsBtnPressed=-1;

   display.setFont();
   display.setTextSize(2);

anew:
   read_ADS0();

   display.fillScreen(HX8357_BLACK);
   // draw all Buttons
   TSBUTTONSENABLED = 1;
   drawAllTSbuttons();

   currentcolor = HX8357_RED;
   oldcolor = GRAY;

   // make the color selection boxes
   display.fillRect(  0,       0, BOXSIZE, BOXSIZE, HX8357_RED);
   display.fillRect(BOXSIZE,   0, BOXSIZE, BOXSIZE, HX8357_YELLOW);
   display.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, HX8357_GREEN);
   display.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, HX8357_CYAN);
   display.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, HX8357_BLUE);
   display.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, HX8357_MAGENTA);
   display.fillRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, HX8357_BLACK);
   display.fillRect(BOXSIZE*7, 0, BOXSIZE, BOXSIZE, WHITE);
   display.fillRect(BOXSIZE*8, 0, BOXSIZE, BOXSIZE, currentcolor);
   display.drawRect(2+BOXSIZE*8, 2, BOXSIZE-4, BOXSIZE-4, WHITE);
   display.drawRect(4+BOXSIZE*8, 4, BOXSIZE-7, BOXSIZE-7, HX8357_BLACK);


   while (PAINTACTIVE) {
      read_ADS0();
      tsBtnPressed=-1;

      drawAllTSbuttons();
      tsBtnPressed = GetTSbuttons();
      delay(1);

      if(tsBtnPressed==2) { // BtnLeft
         x=y=z=-1;
         goto anew;
      }
      if(tsBtnPressed==1) { // BtnUp
         if(PENRADIUS<=10)PENRADIUS ++;
         tsBtnPressed=-1;
      }
      if(tsBtnPressed==4) { // BtnDown
         if(PENRADIUS>=2) PENRADIUS --;
         tsBtnPressed=-1;
      }

      x=tsx; y=tsy; z=tsz; // transformed ts coords to local var
      sprintf(sbuf, "%3d %3d %3d ", x, y, PENRADIUS);

      display.setTextColor(WHITE);
      display.fillRect(270,0, 150,40, BLACK);  // x1,y1, dx,dy
      display.setCursor(270,0);    display.print(sbuf);

      if (y > 0 && y < BOXSIZE) {
         if (x > 0 && x < BOXSIZE) {
            currentcolor = HX8357_RED;
            oldcolor = GRAY;
            x=y=z=-1;
         }
         if (x > BOXSIZE && x < BOXSIZE*2) {
            currentcolor = HX8357_YELLOW;
            oldcolor = GRAY;
            x=y=z=-1;
         }
         if (x > BOXSIZE*2 && x < BOXSIZE*3) {
            currentcolor = HX8357_GREEN;
            oldcolor = GRAY;
            x=y=z=-1;
         }
         if (x > BOXSIZE*3 && x < BOXSIZE*4) {
            currentcolor = HX8357_CYAN;
            oldcolor = GRAY;
            x=y=z=-1;
         }
         if (x > BOXSIZE*4 && x < BOXSIZE*5) {
            currentcolor = HX8357_BLUE;
            oldcolor = GRAY;  x=y=z=-1;
         }
         else if (x > BOXSIZE*5 && x < BOXSIZE*6) {
            currentcolor = HX8357_MAGENTA;
            oldcolor = GRAY;
            x=y=z=-1;
         }
         if (x > BOXSIZE*6 && x < BOXSIZE*7) {
            currentcolor = HX8357_BLACK;
            oldcolor = GRAY;
            x=y=z=-1;
         }
         if (x > BOXSIZE*7 && x < BOXSIZE*8) {
            currentcolor = WHITE;
            oldcolor = GRAY;
            x=y=z=-1;
         }
         display.fillRect(BOXSIZE*8, 0, BOXSIZE, BOXSIZE, currentcolor);
         display.drawRect(2+BOXSIZE*8, 2, BOXSIZE-4, BOXSIZE-4, WHITE);
         display.drawRect(4+BOXSIZE*8, 4, BOXSIZE-7, BOXSIZE-7, HX8357_BLACK);
      }

      if (((y-PENRADIUS) > 0) && ((y+PENRADIUS) < display.height())
            && x>0 && y>0 ) {
         display.fillCircle(x, y, PENRADIUS, currentcolor);
      }

      if ( btn00==1) {
         display.setTextColor(RED);
         display.setCursor( 100, 300 ); display.print("terminated");
         delay(500);
         PAINTACTIVE=false;
         btn00=1;
         return;
      }
   }
}

//===========================================================
// Ende Paint
//===========================================================


//===========================================================
// CHESS
//===========================================================

int xoffs=160, yoffs=18, frx=15, fry=15; // GUI piece offset, GUI boardframe offset


// 'bishop24', 24x24px
const unsigned char   bishop24 [] PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x36, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00,
   0x7f, 0x00, 0x00, 0xff, 0x80, 0x01, 0xf7, 0xc0, 0x01, 0xe3, 0xe0, 0x03, 0xe3, 0xe0, 0x03, 0xf7,
   0xe0, 0x03, 0xff, 0xe0, 0x01, 0xff, 0xc0, 0x00, 0xc0, 0xc0, 0x00, 0xff, 0x80, 0x00, 0xc0, 0xc0,
   0x00, 0xff, 0xc0, 0x00, 0xff, 0x80, 0x00, 0xff, 0x80, 0x1f, 0xff, 0xfe, 0x3f, 0xff, 0xfe, 0x1f,
   0xf3, 0xfe, 0x1e, 0x00, 0x3c, 0x00, 0x00, 0x00
};
// 'king24', 24x24px
const unsigned char   king24 [] PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x78, 0x00, 0x00, 0x38, 0x00, 0x00, 0x30, 0x00, 0x00,
   0x38, 0x00, 0x00, 0x7c, 0x00, 0x0e, 0x7d, 0xe0, 0x33, 0xff, 0x30, 0x2e, 0xfc, 0xc8, 0x5f, 0x7b,
   0xe8, 0x5f, 0xbb, 0xe8, 0x2f, 0xb7, 0xf8, 0x2f, 0xb7, 0xd0, 0x17, 0x97, 0xb0, 0x18, 0x00, 0x60,
   0x0f, 0xff, 0xe0, 0x0f, 0xff, 0xc0, 0x0f, 0xff, 0xc0, 0x0e, 0x00, 0xc0, 0x07, 0xff, 0xc0, 0x07,
   0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'knight24', 24x24px
const unsigned char   knight24 [] PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x01, 0xfc, 0x00, 0x01,
   0xfe, 0x00, 0x01, 0xff, 0xc0, 0x01, 0xfc, 0xe0, 0x03, 0xbf, 0x70, 0x03, 0x3f, 0xb8, 0x03, 0xff,
   0xd8, 0x07, 0xff, 0xec, 0x07, 0xff, 0xec, 0x0f, 0xff, 0xf4, 0x0f, 0xff, 0xf6, 0x17, 0xe7, 0xf6,
   0x0f, 0xcf, 0xf6, 0x0f, 0x9f, 0xf2, 0x00, 0x3f, 0xf2, 0x00, 0x3f, 0xf2, 0x00, 0x7f, 0xfe, 0x00,
   0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'pawn24', 24x24px
const unsigned char   pawn24 [] PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x1f, 0x00, 0x00,
   0x1f, 0x80, 0x00, 0x1f, 0x80, 0x00, 0x1f, 0x80, 0x00, 0x3f, 0xc0, 0x00, 0x3f, 0xc0, 0x00, 0x7f,
   0xe0, 0x00, 0x3f, 0xc0, 0x00, 0x3f, 0xc0, 0x00, 0x1f, 0x80, 0x00, 0x3f, 0xc0, 0x00, 0x7f, 0xe0,
   0x00, 0xff, 0xf0, 0x01, 0xff, 0xf8, 0x01, 0xff, 0xf8, 0x01, 0xff, 0xf8, 0x01, 0xff, 0xf8, 0x01,
   0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'queen24', 24x24px
const unsigned char   queen24 [] PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x07, 0x28, 0xc0, 0x05, 0x29, 0x20, 0x65, 0x39, 0xec, 0xd3,
   0x19, 0xca, 0xd3, 0x19, 0xda, 0x73, 0x99, 0x8c, 0x33, 0xb9, 0x9c, 0x3b, 0xbb, 0x98, 0x3f, 0xff,
   0xf8, 0x3f, 0xff, 0xf8, 0x1f, 0xff, 0xf8, 0x1f, 0xff, 0xf8, 0x1c, 0x00, 0x70, 0x0f, 0xff, 0xe0,
   0x07, 0xff, 0xe0, 0x07, 0xff, 0xc0, 0x06, 0x00, 0x40, 0x07, 0xff, 0xe0, 0x07, 0xff, 0xe0, 0x07,
   0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'rook24', 24x24px
const unsigned char   rook24 [] PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3c, 0xe0, 0x07, 0xbe, 0xf0, 0x07,
   0xff, 0xf0, 0x07, 0xff, 0xf0, 0x07, 0xff, 0xe0, 0x03, 0xff, 0xc0, 0x01, 0x80, 0x80, 0x01, 0xff,
   0x80, 0x01, 0xff, 0x80, 0x01, 0xff, 0x80, 0x01, 0xff, 0x80, 0x01, 0xff, 0x80, 0x01, 0x80, 0x80,
   0x01, 0xff, 0xc0, 0x03, 0xff, 0xe0, 0x07, 0xff, 0xf0, 0x0f, 0xff, 0xf8, 0x0f, 0xff, 0xf8, 0x0f,
   0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


//------------------------------------------------------------

int  GetSqrColor(int sqr) {     // 16*8 sqrs
   return ( 1+sqr+(sqr/16) )%2; // 0x88 board,
}


//------------------------------------------------------------

const unsigned char*  GetPieceBmp(int ptype) {     // 16*8 sqrs
   // {r,n,b,q,k,+,R,N,B,Q,K,*} => { (+16:) 6,3,5,7,4,2,  (+8:) 6,3,5,7,4,1 }
   if (ptype=='r') return rook24;
   if (ptype=='n') return knight24;
   if (ptype=='b') return bishop24;
   if (ptype=='q') return queen24;
   if (ptype=='k') return king24;
   if (ptype=='p') return pawn24;
   if (ptype=='+') return pawn24;

   if (ptype=='R') return rook24;
   if (ptype=='N') return knight24;
   if (ptype=='B') return bishop24;
   if (ptype=='Q') return queen24;
   if (ptype=='K') return king24;
   if (ptype=='P') return pawn24;
   if (ptype=='*') return pawn24;
}

//------------------------------------------------------------

int BLACKPIECECOL = BLACK;
int WHITEPIECECOL = LIGHTBLUE; // WHITE;  //LIGHTBLUE;

//------------------------------------------------------------
void PaintBlankSqr(int sqr) {    // 16*8 sqrs
   int x, y;
   int col=GetSqrColor(sqr);

   // int xoffs=160, yoffs=18, frx=15, fry=15; // GUI piece offset, GUI boardframe offset

   x=sqr%16;
   y=sqr/16;
   x=(x)*30+xoffs-2;
   y=(y)*30+yoffs-2;

   if(col==0) sqrBlack.draw(display, x,y);
   else if(col==1) sqrWhite.draw(display, x,y);
}


//------------------------------------------------------------

int pieceToVal(char p) {
   // {r,n,b,q,k,+,R,N,B,Q,K,*} => { (+16:) 6,3,5,7,4,2,  (+8:) 6,3,5,7,4,1 }
   // psymbol[]= ".?+nkbrq?*?NKBRQ"
   // psymbol[]= ":?pnkbrq?P?NKBRQ"
   // 16=white, 8=black

   // default=0, for all the rest, e.g.:
   // if(p=='.') return 0;
   // if(p==':') return 0;
   // if(p==' ') return 0;

   if(p=='r') return 6+16;
   if(p=='n') return 3+16;
   if(p=='b') return 5+16;
   if(p=='q') return 7+16;
   if(p=='k') return 4+16;
   if(p=='+'|| p=='p') return 18;  // black P-

   if(p=='R') return 6+8;
   if(p=='N') return 3+8;
   if(p=='B') return 5+8;
   if(p=='Q') return 7+8;
   if(p=='K') return 4+8;
   if(p=='*'|| p=='P') return 9;   // white P+

   //default:
   return 0;
}

//------------------------------------------------------------

void Piece2Sqr(const unsigned char *piecearr, int side, int sqr) { // 16*8 sqrs
   int x, y, piececolor;
   // int xoffs=160, yoffs=18, frx=15, fry=15; // GUI piece offset, GUI boardframe offset

   x=sqr%16;
   y=sqr/16;
   x=x*30+xoffs; // put to sqr center
   y=y*30+yoffs;

   if(side==8) piececolor=BLACKPIECECOL;
   else if(side==16) piececolor=WHITEPIECECOL;

   PaintBlankSqr(sqr);
   display.drawBitmap( x, y, piecearr, 24, 24, piececolor);
}

//------------------------------------------------------------


//===========================================================
// Chess Move Generator
//===========================================================
// ver 48006

int tsBtnPressed=-1;

#define K(A,B) *(int*)(T+A+(B&8)+S*(B&7))
#define J(A) K(y+A,board[y])-K(x+A,u)-K(H+A,t)

#define U (1<<12) // (1<<11) 
struct HT_ {
   int K,V;
   int X,Y,D;
} A[U];           /* hash table, 16M+8 entries*/

int   M=136,      /* M=0x88                */
      S=128,
      I=8000,
      Q,
      O, savEPsq,
      K,
      N,
      R,
      J,
      Z,
      turn=16, oldturn=16, savturn=16;

int32_t MAXDEEP=1e6, CUSTDEEP=1e6;
int     STOPTHINK=0;


signed char L,
       w[]= {0,2,2,7,-1,8,12,23},                     /* relative piece values    */
            o[]= {-16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0, /* step-vector lists */
                  7,-1,11,6,8,3,6
                 },                          /* 1st dir. in o[] per piece*/
                 bsetup[]= {6,3,5,7,4,5,3,6},                   /* initial piece setup      */
                           board[129],                                    /* board: half of 16x8+dummy*/
                           oldboard[129],
                           savOboard[129],
                           T[1035];                                       /* hash translation table   */
signed char  psymbol[]= ".?+nkbrq?*?NKBRQ";
int    mfrom, mto,    // buffer for current ply from - to
       savto;
int    RemP;          // buffer fo remove piece sqr
int    busycount=0;
int    checkscore=0;

int Minimax(int q,int l,int e,int E,int z,int n)          /* recursive minimax search, k=moving side, n=depth*/
//int q,l,e,E,z,n;        /* (q,l)=window, e=current eval-score, E=e.p. sqr.*/
{  /* e=score, z=prev.dest; J,Z=hashkeys; return score*/
   int j,r,m,v,d,h,i,F,G,V,P,f=J,g=Z,C,s;
   char t,p,u,x,y,X,Y,H,B;
   struct HT_*a=A+(J+turn*E&U-1);                     /* lookup pos. in hash table*/

   q--;                                          /* adj. window: delay bonus */
   turn^=24;                                        /* change sides             */

   d=a->D; m=a->V; X=a->X; Y=a->Y;               /* resume at stored depth   */
   if(a->K-Z|z|                                  /* miss: other pos. or empty*/
         !(m<=q|X&8&&m>=l|X&S))                  /*   or window incompatible */
      d=Y=0;                                       /* start iter. from scratch */
   X&=~M;                                       /* start at best-move hint  */


   while(
      d++<n || d<3 ||                           /* iterative deepening loop */
      z&K==I &&
      ( N<MAXDEEP & d<98 ||                    /* root: deepen upto time   */
        (K=X,L=Y&~M,d=3) )
   )                                         /* time's up: go do best    */
   {
      if( STOPTHINK && turn==oldturn ) break;

      x=B=X;                                       /* start scan at prev. best */
      h=Y&S;                                       /* request try noncastl. 1st*/
      P=d<3?I:Minimax(-l,1-l,-e,S,0,d-3);                /* Search null move         */
      m=-P<l|R>35?d>2?-I:e:-P;                     /* Prune or stand-pat       */
      N++;                                         /* node count (for timing)  */
      do {
         u=board[x];                                   /* scan board looking for   */
         if(u&turn)                                     /*  own piece (inefficient!)*/
         {  r=p=u&7;                                   /* p = piece type (set r>0) */
            j=o[p+16];                                 /* first step vector f.piece*/
            while(r=p>2&r<0?-r:-o[++j])                    /* loop over directions o[] */
            {  A:                                        /* resume normal after best */
               y=x; F=G=S;                               /* (x,y)=move, (F,G)=castl.R*/
               do {                                      /* y traverses ray, or:     */
                  H=y=h?Y^h:y+r;                           /* sneak in prev. best move */
                  if(y&M)break;                            /* board edge hit           */
                  m=E-S&board[E]&&y-E<2&E-y<2?I:m;             /* bad castling             */
                  if(p<3&y==E)H^=16;                       /* shift capt.sqr. H if e.p.*/
                  t=board[H]; if(t&turn|p<3&!(y-x&7)-!t)break;    /* capt. own, bad pawn mode */
                  i=37*w[t&7]+(t&192);                     /* value of capt. piece t   */
                  m=i<0?I:m;                               /* K capture                */
                  if(m>=l&d>1)goto C;                      /* abort on fail high       */

                  v=d-1?e:i-p;                             /* MVV/LVA scoring          */
                  if(d-!t>1)                               /* remaining depth          */
                  {  v=p<6?board[x+8]-board[y+8]:0;                  /* center positional pts.   */
                     board[G]=board[H]=board[x]=0; board[y]=u|32;            /* do move, set non-virgin  */
                     if(!(G&M))board[F]=turn+6,v+=50;               /* castling: put R & score  */
                     v-=p-4|R>29?0:20;                       /* penalize mid-game K move */
                     if(p<3)                                 /* pawns:                   */
                     {  v-=9*((x-2&M||board[x-2]-u)+               /* structure, undefended    */
                              (x+2&M||board[x+2]-u)-1              /*        squares plus bias */
                              +(board[x^16]==turn+36))                 /* kling to non-virgin King */
                           -(R>>2);                          /* end-game Pawn-push bonus */
                        V=y+r+1&S?647-p:2*(u&y+16&32);         /* promotion or 6/7th bonus */
                        board[y]+=V; i+=V;                         /* change piece, add score  */
                     }
                     v+=e+i; V=m>q?m:q;                      /* new eval and alpha       */
                     J+=J(0); Z+=J(8)+G-S;                   /* update hash key          */
                     C=d-1-(d>5&p>2&!t&!h);
                     C=R>29|d<3|P-I?C:d;                     /* extend 1 ply if in check */
                     do
                        s=C>2|v>V?-Minimax(-l,-V,-v,                 /* recursive eval. of reply */
                                           F,0,C):v;        /* or fail low if futile    */
                     while(s>q&++C<d); v=s;
                     if(z&&K-I&&v+I&&x==K&y==L)              /* move pending & in root:  */
                     {  Q=-e-i; O=F;                           /*   exit if legal & found  */
                        a->D=99; a->V=0;                       /* lock game in hash as draw*/
                        R+=i>>7; return l;                     /* captured non-P material  */
                     }
                     J=f; Z=g;                               /* restore hash key         */
                     board[G]=turn+6; board[F]=board[y]=0; board[x]=u; board[H]=t;  /* undo move,G can be dummy */
                  }
                  if(v>m)                                  /* new best, update max,best*/
                     m=v,X=x,Y=y|S&F;                        /* mark double move with S  */
                  if(h) {
                     h=0;   /* redo after doing old best*/
                     goto A;
                  }
                  if(x+r-y|u&32|                           /* not 1st step,moved before*/
                        p>2&(p-4|j-7||                        /* no P & no lateral K move,*/
                             board[G=x+3^r>>1&7]-turn-6                   /* no virgin R in corner G, */
                             ||board[G^1]|board[G^2])                      /* no 2 empty sq. next to R */
                    )t+=p<5;                               /* fake capt. for nonsliding*/
                  else F=y;                                /* enable e.p.              */
               } while(!t);                                   /* if not capt. continue ray*/
            }
         }
      } while((x=x+9&~M)-B);                          /* next sqr. of board, wrap */
C:
      if(m>I-M|m<M-I)d=98;                         /* mate holds to any depth  */
      m=m+I|P==I?m:0;                              /* best loses K: (stale)mate*/
      if(a->D<99)                                  /* protect game history     */
         a->K=Z,a->V=m,a->D=d,                       /* always store in hash tab */
            a->X=X|8*(m>q)|S*(m<l),a->Y=Y;              /* move, type (bound/exact),*/
      //if(z)printf("%2d ply, %9d searched, score=%6d by %c%c%c%c\n",d-1,N-S,m,
      //               'a'+(X&7),'8'-(X>>4),'a'+(Y&7),'8'-(Y>>4&7));
      /* uncomment for Kibitz */

      checkscore=I;
      if(z) {
         busycount=0;
         //if(checkscore==I)
         checkscore=m;
         delay(1);
         sprintf(sbuf, "\n%2d ply, %9d searched, score=%6d by %c%c%c%c\n",
                 d-1,   N-S,                 m,
                 'a'+(X&7),'8'-(X>>4),'a'+(Y&7),'8'-(Y>>4&7));
         Serial.print(sbuf);
         sprintf(sbuf, "%2d ply,     %9d search(%c%c%c%c)",
                 d-1,   N-S,  'a'+(X&7),'8'-(X>>4),'a'+(Y&7),'8'-(Y>>4&7));
         display.fillRect(0,280, 420,40, BLACK);
         display.setCursor(0,280);
         display.setTextColor(WHITE);
         display.print(sbuf);
      }
      else if( (N%10000)<1 ) {
         if(busycount>30) {
            busycount=0;
            Serial.print("\n");
         }
         if(busycount==0) {
            display.fillRect(0,300, 420,20, BLACK);
            display.setCursor(0,295);
         }
         Serial.print(".");
         display.print(".");
         busycount++;
      }
      else if( (N%25000)<1 ) {
         tsBtnPressed = GetTSbuttons();
         if(tsBtnPressed==3) { // TsBtnESC=Quit
            tsBtnPressed=-1;
            //debug
            Serial.println("\n tsBtnPressed=AI=3");
            STOPTHINK=1;
         }
         read_ADS0();
         if (btn00== 4) {
            //debug
            do {
               read_ADS0();
            }
            while(btn00== 4);
            Serial.println("\n ADBtnPressed=AI=4");
            STOPTHINK=1;
         }
      }

   }
   /*    encoded in X S,8 bits */

   turn^=24;                                  /* change sides back        */
   mfrom=K; mto=L;
   return m+=m<e;                                /* delayed-loss bonus       */
}




//------------------------------------------------------------

void centerPointsTable() {                            // center-points table
   int col=8, x;                                      // (in unused half board[])
   while(col--) {
      x=8;
      while(x--) {
         board[16*x + col+8]=(col-4)*(col-4)+(x-3.5)*(x-3.5);
      }
   }
}


//------------------------------------------------------------
// Openings Library

const int smnum=6; // max number of openings in StartBib

String StartBib(int num) {
   String startmenu[smnum];

   startmenu[0]="d2d4";    // closed, Gambit,...
   startmenu[1]="e2e4";    // open, half-open, Indian,...
   startmenu[2]="c2c4";    // Englisch
   startmenu[3]="b2b4";    // Sokolski
   startmenu[4]="g1f3";    // Reti
   startmenu[smnum-1]="";  // deep thought

   if(num<0 || num> smnum-1) num=0;
   return (String)startmenu[num];

}

//------------------------------------------------------------

typedef struct {
   float c;
   char piece[3];
   char sfrom[5];
   char sto[5];
   char sb[3];
}  MvHist;

MvHist *mvh;

//------------------------------------------------------------

void listMovHist(int l, int j) {
   char spiece;
   for(int k=l; k<=l+12 && k<=j; k++) {
      if(k<20) display.print(" ");
      display.print( ((float)(k))/2.0, 1);
      spiece=mvh[k].piece[0];
      if(spiece=='*' || spiece=='+' || spiece=='P' || spiece=='p')
         display.print(": ");
      else display.print((String)":"+mvh[k].piece[0]);
      display.print((String)mvh[k].sfrom + mvh[k].sb + mvh[k].sto);
      display.println();
   }
}


//------------------------------------------------------------


void Chess() {
   bool CHESSACTIVE = true;
   TSBUTTONSENABLED = 1;

   int32_t       score=1, i, j;
   char          cstring[20];
   char          pfrom[3], pto[3];
   signed char   spiece, oldpiece, savpiece;
   float         movecount=1.0, oldmovecount=1.0;
   int           TSMOVESTATE;
   int           sx, sy;
   int           CHESSFIGHT;
   int           UNDOcount = 1;
   int           MaxMvh;

   free(mvh);
   MaxMvh = 5;
   mvh = (MvHist*)malloc( MaxMvh * sizeof(MvHist) );
   memset( mvh, 0, MaxMvh * sizeof(MvHist) );

   rand_last = (int)millis();  // srand()
   display.setTextColor(WHITE);

   //  read UI buttons
   read_ADS0();

   //static int currentcolor, oldcolor;
   static int x=-1, y=-1, z=-1;

   //------------------------------------------------------------
   // GUI Board settings

   int sqr, col;

   ImageReturnCode stat; // Status from image-reading functions

   stat = reader.loadBMP("/chess/sqrWhite30.bmp", sqrWhite);
   stat = reader.loadBMP("/chess/sqrBlack30.bmp", sqrBlack);

   //------------------------------------------------------------
   // GUI test
   int Player;

   display.fillScreen(BLACK);

   tsBtnPressed=-1;
   tsBtnPressed = GetTSbuttons();
   delay(1);

   THREADSUSPENDED=1;
   display_mutex.lock();
   display.fillScreen(BLACK);
   ShowImageFileBmp("/chess/chessboard270.bmp", xoffs-frx-2, yoffs-fry-2);

   display_mutex.unlock();
   THREADSUSPENDED=0;
   drawAllTSbuttons();

   //------------------------------------------------------------
   // getting serious....
   //------------------------------------------------------------
   // Serial + GUI Chess Game User Interface
   //------------------------------------------------------------



   //#define LegalMove  (score>4 && score!=15 && score!=24 && score<=I && (score<62 || score>150))
   int LegalMove = 1;

RESTART:
   display.fillRect(0, 0, 140,240, BLACK);
   display.setCursor( 0, 0 );
   oldmovecount=movecount=1;
   free(mvh);
   MaxMvh = 5;
   mvh = (MvHist*)malloc( MaxMvh * sizeof(MvHist) );
   memset( mvh, 0, MaxMvh * sizeof(MvHist) );

   turn=16;
   memset(board, 0, sizeof(board));
   movecount=1.0;

   K=8;
   while(K--)
   {
      board[K]=(board[K+112]=bsetup[K]+8)+8;
      board[K+16]=18;
      board[K+96]=9;                               // initial board setup
      L=8;
      while(L--)board[16*L+K+8]=(K-4)*(K-4)+(L-3.5)*(L-3.5);     // center-pts table
   }                                                             //(in unused half board[])
   memcpy(oldboard, board, sizeof(board));

   N=1035;
   while(N-->M)T[N]=LCG_rand()>>9;                     // Hashtable ranom init

   UNDOcount = 1;

UNDO:
   display.setTextColor(WHITE);
   CHESSFIGHT = 1;

   while(CHESSACTIVE && CHESSFIGHT) {
      read_ADS0();
      if (btn00==1) {
         goto QUIT_CHESS;
      }
      Serial.print("\n");

      display.setCursor( 0, 40 );
      sprintf(sbuf,"  A B C D E F G H \n  --------------- \n");
      Serial.print(sbuf);
      //display.print(sbuf);
      for(int b=0; b<8; b++) {
         display.setCursor(xoffs+8+b*30,0);
         display.print((char)('a'+b));
         display.setCursor(xoffs+8+b*30,255);
         display.print( (char)('a'+b) );
         display.setCursor(xoffs-15,  22+b*30);
         display.print(8-b);
         display.setCursor(xoffs+241, 22+b*30);
         display.print(8-b);
      }
      // print board
      N=-1;
      while(++N<121) {
         if(N&8 && (N+7!=0) ) {           // 1...8 board right
            sprintf(sbuf," %1d \n", 1+((120-N)>>4));
            Serial.print(sbuf);
            //display.print("  \n");      // skip this sbuf
            N+=7;
         }
         else {
            if(N%8==0) {     // 1...8 board left
               sprintf(sbuf, "%1d", 1+((120-N)>>4));
               Serial.print(sbuf);
               //display.print(sbuf);
            }
            sprintf(sbuf," %c", psymbol[board[N]&15]);
            Serial.print(sbuf);
            //display.print(sbuf);

            //psymbol[]= ":?+nkbrq?*?NKBRQ";
            int spiece=psymbol[board[N]&15];
            int side=-1;
            if( (spiece>='A' && spiece<='Z') || spiece=='*') side=16;
            else if( (spiece>='a' && spiece<='z') || spiece=='+') side=8;
            PaintBlankSqr(N);
            if(side>1) Piece2Sqr( GetPieceBmp(spiece), side, N);
         }
      }

      STOPTHINK=0;
      MAXDEEP==CUSTDEEP;

      sprintf(sbuf,"  --------------- \n  A B C D E F G H ");
      Serial.print(sbuf);
      //display.print(sbuf);
      Serial.println();
      if(turn==16) sprintf(sbuf,"%4.1f WHITE> ", movecount);
      else sprintf(sbuf,"%4.1f BLACK> ", movecount);
      Serial.print(sbuf);
      display.fillRect(0, 240, 140,20, BLACK);
      display.setCursor(0,240);
      display.print(sbuf);

      display.fillRect(0, 0, 140,240, BLACK);
      display.setCursor( 0, 0 );
      int m = ( (int)(oldmovecount*2) );
      int l = max(2,m-12);
      listMovHist(l,m);


      strcpy(cstring,"");
      strcpy(pfrom, "");
      strcpy(pto, "");
      sx=sy=0;
      z=0;
      tsBtnPressed = GetTSbuttons();
      x=tsx; y=tsy; z=tsz; // transformed ts coords to local var

      if(tsBtnPressed==0) { // TsBtnESC=Quit
         tsBtnPressed=-1;
         strcpy(cstring,"Q");
         score=I;
         i=1;
         goto EVAL_STRING;
      }
      else if(tsBtnPressed==1) { // TsBtnUp=Restart
         tsBtnPressed=-1;
         strcpy(cstring,"R");
         score=I;
         i=1;
         goto EVAL_STRING;
      }
      else if(tsBtnPressed==2) { // TsBtnDRight=UNDO <<<<<<<<<<<<<<<<UUUUU
         tsBtnPressed=-1;
         strcpy(cstring,"U");
         score=I;
         i=1;
         goto EVAL_STRING;
      }
      else if(tsBtnPressed==4) { // TsBtnDown=auto
         tsBtnPressed=-1;
         strcpy(cstring,"");
         score=I;
         goto EVAL_STRING;
      }

      if (!Serial.available()) {
         while(z<20 ) {
            strcpy(cstring,"");
            strcpy(pfrom, "");
            strcpy(pto, "");

            read_ADS0();
            if (btn00==1) {
               goto QUIT_CHESS;
            }
            if (btn00==5) {
               strcpy(cstring,"");
               goto EVAL_STRING;
            }

            delay(1);
            tsBtnPressed = GetTSbuttons();
            x=tsx; y=tsy; z=tsz; // transformed ts coords to local var
            if (Serial.available() ) break;
            delay(1);
         }
      }

      TSMOVESTATE=0;

      if(tsBtnPressed==0) { // TsBtnESC=Quit
         tsBtnPressed=-1;
         strcpy(cstring,"Q");
         score=I;
         i=1;
         goto EVAL_STRING;
      }
      else if(tsBtnPressed==1) { // TsBtnUp=Restart
         tsBtnPressed=-1;
         strcpy(cstring,"R");
         score=I;
         i=1;
         goto EVAL_STRING;
      }
      else if(tsBtnPressed==2) { // TsBtnDRight=UNDO <<<<<<<<<<<<<<<<UUUUU
         tsBtnPressed=-1;
         strcpy(cstring,"U");
         score=I;
         i=1;
         goto EVAL_STRING;
      }
      else if(tsBtnPressed==4) { // TsBtnDown=auto
         tsBtnPressed=-1;
         strcpy(cstring,"");
         score=I;
         goto EVAL_STRING;
      }
      else if(x>=xoffs-20 && y>=yoffs && x<xoffs+237  && y<MAXDISPY && z>30 ) {
         TSMOVESTATE=1;
         strcpy(pfrom, "");
         strcpy(pto, "");
         strcpy(cstring, "");
         //display.fillRect(0,260, 140,60, BLACK);

         while( z>20 ) {
            sx=(x-xoffs)/30;
            sy=(y-yoffs)/30; // 
            if(sx>=0 && sy>=0) {
               if(TSMOVESTATE==1) {
                  pfrom[0]=(char)('a'+ (sx&7));
                  pfrom[1]=(char)('8'- (sy&7));
                  pfrom[2]=0;
                  TSMOVESTATE=2;
                  delay(1);
               }
               // 'a'+(X&7),'8'-(X>>4),'a'+(Y&7),'8'-(Y>>4&7)
               if(TSMOVESTATE==2) {
                  if(y<=260)
                  {
                     pto[0]=(char)('a'+ (sx&7));
                     pto[1]=(char)('8'- (sy&7)); 
                     pto[2]=0;
                     delay(1);
                  }
                  else  if(y>280) {
                     pto[0]=(char)('t'); // y<1: <<<<<<<<<<<<< trash container
                     pto[1]=(char)('r'); // y<1: <<<<<<<<<<<<< trash container
                     pto[2]=0;
                     delay(1);
                  }
               }
            }
            sprintf(sbuf, "pfromto: %3d %3d %s%s", x, y, pfrom, pto);
            display.fillRect(140,280, 280,40, BLACK);  // x1,y1,...
            Serial.println(sbuf);
            display.setCursor(140,280);
            display.print(sbuf);

            tsBtnPressed = GetTSbuttons();
            x=tsx; y=tsy; z=tsz; // transformed ts coords to local var; tsy shift corr!
            delay(1);
         } // while

         tsBtnPressed = -1;
         strcpy(cstring,"");
         if ( strlen(pfrom)==2 && strlen(pto)==2 ) {
            strcpy(cstring,pfrom);
            strcat(cstring,pto);
            i=4;
            cstring[i]=0;
         }
         sprintf(sbuf, "cstring:%3d %3d %s", x, y, cstring);
         Serial.println(sbuf);
         display.setCursor(140,300);
         display.print(sbuf);
         delay(1);

         goto  EVAL_STRING;

      } // if


      i = 0;
      do   {
         read_ADS0();
         if (btn00==1) {
            goto QUIT_CHESS;
         }

         if(i<10) cstring[i] = Serial.read();
         if(cstring[i]==13) {
            cstring[i]=0;
            break;
         }
         else i++;
      } while(i < 10);


EVAL_STRING:

      if(cstring[0]=='Q' && strlen(cstring)<4 ) {
         goto QUIT_CHESS;
      }
      else if(cstring[0]=='R' && strlen(cstring)<4 ) {
         strcpy(cstring, "");
         score=I;
         display.fillRect(0,280, 280,20, BLACK);
         display.setCursor( 0, 280 );
         Serial.println("\n> RESET/RESTART ");
         display.print ("> RESET/RESTART ");
         goto RESTART;
      }
      else if((cstring[0]=='U' && strlen(cstring)<4 && UNDOcount<1) ) {
         UNDOcount += 1;
         score=I;
         memcpy(board, savOboard, sizeof(board));
         turn=savturn;
         spiece=savpiece;
         movecount=oldmovecount;
         oldmovecount=movecount-0.5;

         display.setCursor( 0, 300 );
         Serial.println("\n> UNDO ");
         display.print("> UNDO ");

         goto UNDO;
      }

      // else/if() debug
      else
      {
         K=I;
         sprintf(sbuf,"%4.1f:", oldmovecount);
         Serial.println(sbuf) ;
         score=I;

         // test start-bibliotheque
         if(movecount<1.5 && cstring[0]==0) {
            strcpy( cstring, StartBib(random(smnum)).c_str() );
         }
         if(strlen(cstring)>0) {
            if(strlen(cstring)<4) {
               strcpy(cstring, "a8a8");                 // dummy: input error
            }
            if(strlen(cstring)>=4) {                    /* parse entered move */
               K= cstring[0]-16*cstring[1]+799;
               L= cstring[2]-16*cstring[3]+799;
            }
         }

         // DEBUG
         Serial.print("\n DEBUG cstring : "); Serial.print(cstring);
         sprintf(sbuf,"\n DEBUG K: %d  \n DEBUG L: %d \n:",  K, L);
         Serial.print(sbuf);

         memcpy(oldboard, board, sizeof(board));
         savto=mto;
         savEPsq=O;

         memcpy(oldboard, board, sizeof(board));
         oldturn=turn;
         oldpiece=spiece;
         if(movecount<2.0) MAXDEEP=min((int32_t)1e5,CUSTDEEP);
         else if(movecount<2.5) MAXDEEP=min((int32_t)3e5,CUSTDEEP);
         else if(movecount<3.0) MAXDEEP=min((int32_t)6e5,CUSTDEEP);
         else MAXDEEP=CUSTDEEP;
         //-------------------------------------------------------------
         score=Minimax(-I, I, Q, O, 1, 3);    /* think or check & do*/
         //-------------------------------------------------------------
         LegalMove=(turn != oldturn);

         display.fillRect(0,280, 140,40, BLACK);
         display.setCursor(0,280);
         Serial.println((String)"scr="+score+"/"+checkscore );
         display.print ((String)"scr="+score+"/"+checkscore );

         if( score==15 || score<0 ) {
            Serial.println(" ILLEGAL!");
            display.print(" ILLEGAL!");
         }

         if( LegalMove )
         {
            UNDOcount =0;
            memcpy(savOboard, oldboard, sizeof(board));
            savturn=oldturn;
            savpiece=oldpiece;

            RemP=S;
            if(oldboard[mto])   RemP=mto;
            if(mto==savEPsq)  RemP=savto;

            spiece=psymbol[board[mto]&15];
            if(spiece=='*' || spiece=='+' || spiece=='P' || spiece=='p') {
               spiece=' ';
            }
            char sb='-';
            sprintf(sbuf,"%4.1f:%c%c%c", movecount, spiece,'a'+(mfrom&7),'8'-(mfrom>>4) );
            if(oldboard[mto]) {
               strcat(sbuf,"X");
               sb='X';
            }
            else strcat(sbuf,"-");
            Serial.print("\n\n"); Serial.print(sbuf);
            sprintf(sbuf,"%c%c ", 'a'+(mto&7),'8'-(mto>>4&7));
            Serial.print(sbuf);

            /*
               typedef struct {
               char piece[3];
               char sfrom[5];
               char sto[5];
               char sb[3];
               float c;
               }  MvHist;
            */
            MaxMvh+=1;
            mvh = (MvHist*)realloc( mvh, MaxMvh * sizeof(MvHist) );

            j=( (int)(movecount*2) );
            if(j>=MaxMvh-1) j=MaxMvh-1;
            mvh[j].c=movecount;
            mvh[j].piece[0]=psymbol[board[mto]&15];
            mvh[j].piece[1]=0;
            mvh[j].sfrom[0]='a'+(mfrom&7);
            mvh[j].sfrom[1]='8'-(mfrom>>4);
            mvh[j].sfrom[2]=0;
            mvh[j].sto[0]='a'+(mto&7);
            mvh[j].sto[1]='8'-(mto>>4&7);
            mvh[j].sto[2]=0;
            mvh[j].sb[0]=sb;
            mvh[j].sb[1]=0;

            display.fillRect(0, 0, 140,240, BLACK);
            display.setCursor( 0, 0 );

            int l=max(2,j-12);
            listMovHist(l,j);

            sprintf(sbuf, " \nDEBUG: %d to %d  \n", mfrom, mto);
            Serial.print(sbuf);
            //display.print(sbuf);
            sprintf(sbuf,"  EPsq: %c%c (%3d)\n",
                    'a'+(O&7), '8'-(O>>4&7), O);
            Serial.print(sbuf);
            //display.print(sbuf);
            sprintf(sbuf,"  RemP: %c%c (%3d)\n\n",
                    'a'+(RemP&7), '8'-(RemP>>4&7), RemP);
            Serial.print(sbuf);
            //display.print(sbuf);

            if( LegalMove )  // redundant
            {
               movecount += 0.5;
            }
            oldmovecount=movecount-0.5;
         }
      }
   }

QUIT_CHESS:
   TSBUTTONSENABLED = 0;
   Serial.print("\n GAME QUIT \n");
   CHESSFIGHT = 0;
   free( mvh );
   display.setTextColor(RED);
   display.fillRect(0,280, 420,40, BLACK); // <<<<<<<
   display.setCursor(0,285);
   display.print("GAME QUIT \n");
   display.print("terminated");
   display.setTextColor(WHITE);
   delay(500);
   CHESSACTIVE=false;
   btn00=1;
   return;
}

//===========================================================
// Ende Chess
//===========================================================


//===========================================================
//  ShowImageFileBmp
//===========================================================

void  ShowImageFileBmp(char *filename, int x, int y) {
   int32_t width, height;
   img_stat = reader.bmpDimensions(filename, &width, &height);
   if(height>display.height() && width<=display.width() ) {
      display.setRotation(2);
   }
   Serial.print("Loading to screen: " + (String)filename);
   img_stat = reader.drawBMP(filename, display, x, y);
   reader.printStatus(img_stat);             // How'd we do?
   display.setRotation(TFT_ROTAT);
}

//===========================================================
// EXPLORER
//===========================================================

void ExplorerExecFunc() {
   char filename[80]="";
   strcpy( filename, filelist[selectfilenr].c_str() ); // for opt modifications
   if(strEndsWith(filename, ".bmp") ) {

      THREADSUSPENDED=1;
      msleep(1);
      display_mutex.lock();
      display.fillScreen(BLACK);

      ShowImageFileBmp(filename, 0, 0) ;

      read_ADS0();
      while (btn00!=1 && btn00!=3 && btn00!=4) {
         delay(50);
         read_ADS0();
      }

      display_mutex.unlock();
      THREADSUSPENDED=0;
      //display.setRotation(TFT_ROTAT); // incl in ShowImageFileBmp()
      display.fillScreen(BLACK);
      ls(filelist);
   }

   else if(!strEndsWith(filename,"..") && !strEndsWith(filename,".")
           && !strEndsWith(filename,"/"))
   {
      THREADSUSPENDED=1;
      msleep(1);
      display_mutex.lock();
      display.setRotation(2);
      display.fillScreen(BLACK);

      File myTxtFile = SD.open(filename);
      if (myTxtFile) {
         //display.setFont(&FreeSans9pt7b);
         display.setTextSize(2);
         display.setTextColor(WHITE);
         display.setCursor(0, 0);
         int c;
         while (myTxtFile.available()) {
            c=myTxtFile.read();
            Serial.write(c);
            display.write(c);
         }
         display.setTextColor(RED);
         display.write(20);  // EOF symbol
         display.setTextColor(WHITE);
         display.setFont();
         //display.setTextSize(1);

         read_ADS0();
         while(btn00!=1 && btn00!=3 && btn00!=4) {
            delay(50);
            read_ADS0();
         }

         // close the file:
         myTxtFile.close();
         delay(2);
         display_mutex.unlock();
         THREADSUSPENDED=0;
         display.setRotation(TFT_ROTAT);
         display.fillScreen(BLACK);
         ls(filelist);
      }
   } // if(!strEndsWith(...);

}


bool Explorer() {

   // default cursormarknr=0;

   // btn 0 ESC
   if (btn00==1
         &&  selectfilenr >=0) {
      selectfilenr=-1;
      ls(filelist);
      btn00=0;
   }

   // btn down 5
   if (btn00==5
         &&  cursormarknr <= filecount) {
      if (cursormarknr < filecount-1) {
         cursormarknr++;
         ls(filelist);
      }
   }

   // btn up 2
   else if (btn00==2
            && cursormarknr >= 0) {
      if (cursormarknr >= 0) cursormarknr--;
      ls(filelist);
   }

   if(cursormarknr>=0) {
      //if (MENULEVEL==1) fileMarked=filelist[cursormarknr];
      //else
      fileMarked="";
   }

   // btn right 4
   if (btn00==4
         && cursormarknr > 0) {
      if(selectfilenr==cursormarknr) {
         // selectfilenr= -1;      fileSelected= "";
         ExplorerExecFunc();
         oldwinmin=-2;
         ls(filelist);
      }
      else if(selectfilenr!=cursormarknr) {
         selectfilenr=cursormarknr;
         fileSelected=filelist[selectfilenr];
         ls(filelist);
      }
   }

   // btn left 3
   else if (btn00==3) {
      if(selectfilenr==cursormarknr) {
         // ExplorerExecFunc();
         selectfilenr= -1;   // unselect
         fileSelected= "";
      } // if(selectfilenr==cursormarknr)
      ls(filelist);

   } // else if (btn00==3)
   return true;
}

//===========================================================
// EXPLORER END
//===========================================================


//===========================================================
// show Apps Menu
//===========================================================

static int  MENULEVEL=1;

void showAppsMenu() {
   static uint32_t currentTime=millis();
   static bool newMenuLevel = true;
   static int  currAppNr=0;
   const  int  MaxAppCnt=9;

   if(MENULEVEL>MaxAppCnt) MENULEVEL=0;

   TSBUTTONSENABLED = 0;


   //----------------------
   // Menu
   // MENULEVEL==0

   if (MENULEVEL==0) {                     // show menu

      String AppName1=" App Menu "; // this
      String mItems[MaxAppCnt];
      mItems[0]=    " App Menu     ";
      mItems[1]=    " Instructions ";
      mItems[2]=    " SD Explorer  ";
      mItems[3]=    " Date + Time  ";
      mItems[4]=    " CHESS        ";
      mItems[5]=    " PAINT        ";
      mItems[6]=    " PONG         ";
      mItems[7]=    " Ascii Table  ";
      mItems[8]=    " SD reboot    ";

      if(newMenuLevel) {
         //drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<

         int COLOR_TEXT = WHITE;
         int i;
         display.setTextColor(COLOR_TEXT); display.setCursor(  20,  20 );
         display.print (AppName1);
         for (i=0; i<MaxAppCnt; i++) {
            if(i==currAppNr) {
               display.setTextColor(LIME);
               display.setCursor( 40, 40+(currAppNr*20) );
               display.print(">");
            }

            display.setTextColor(COLOR_TEXT);
            display.setCursor( 40+10, 40+(i*20) );
            display.print(mItems[i]);
         }
         display.println();
         display.setTextColor(YELLOW);
         display.setCursor( 0, 40+(i+2)*20) ;
         display.println(" move up/dn,  Btn [ > ] to select");

         newMenuLevel=false;
      }

      if(!newMenuLevel) {
         /*
            if (btn00==1) {
            MENULEVEL += 1;
            newMenuLevel = true;
            display.fillScreen(COLOR_BGND);
            //drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
            }
         */
         if (btn00==5  &&  currAppNr < MaxAppCnt-1) {
            display.fillRect(40, 40+(currAppNr*20), 10, 16, COLOR_BGND);
            currAppNr++;
            display.setTextColor(LIME);
            display.setCursor( 40, 40+(currAppNr*20) );
            display.print(">");
            delay(30);
         }
         if (btn00==2  &&  currAppNr > 0) {
            display.fillRect(40, 40+(currAppNr*20), 10, 16, COLOR_BGND);
            currAppNr--;
            display.setTextColor(LIME);
            display.setCursor( 40, 40+(currAppNr*20) );
            display.print(">");
            delay(30);
         }
         if ((btn00==3 || btn00==4) &&  currAppNr >= 0 &&  currAppNr < MaxAppCnt) {
            MENULEVEL=currAppNr;
            currAppNr=0;  // reset to this menu level
            newMenuLevel = true;
            display.fillScreen(COLOR_BGND);
            delay(30);
         }
         display.setCursor( 0, 0 );
         //display.fillRect(0, 0, 10, 16, COLOR_BGND);
         //display.print(currAppNr);
      }
      return;
   }


   //----------------------
   // Instructions
   // MENULEVEL==1

   if(MENULEVEL==1) {
      if(newMenuLevel) {
         //drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         display.setTextColor(WHITE);
         display.setCursor(  0,  20 );
         display.println("Instructions:");
         display.println("");
         display.println(" use 5-switch-button pad to move/select ");
         display.println("");
         display.setTextColor(LIGHTGRAY);
         display.println("      (1: EX)      ");
         display.setTextColor(BLUE);   display.println();
         display.println("      (2: Up)      ");
         display.setTextColor(RED);    display.println();
         display.print  ("  (3: Le)");
         display.setTextColor(LIME);
         display.println(          " (4: Ri)");
         display.setTextColor(YELLOW); display.println();
         display.println("      (5: Dn)      ");
         display.println(); display.setTextColor(WHITE);
         display.println();
         display.println(" press TOP EX Btn to proceed");
         newMenuLevel=false;
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL = 0;
            newMenuLevel = true;
            display.fillScreen(COLOR_BGND);
            //drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         }
      }
      return;
   }



   //----------------------
   // SD file menu + viewer
   // MENULEVEL==2

   if (MENULEVEL==2) {
      DEBUG=false;
      //display.setFont(&FreeMono9pt7b);
      //display.setFont(&Tiny3x3a2pt7b);
      display.setFont();
      //display.setTextSize(1);
      if(newMenuLevel) {
         readDirectory(SdPath, 0);
         ls(filelist);
         newMenuLevel=false;
      }
      else if(!newMenuLevel) {
         // btn 0 ESC
         if (btn00==1 &&  cursormarknr>=0) {
            cursormarknr=-1;
            ls(filelist);
            btn00=0;
         }
         else if (btn00==1 &&  cursormarknr<0) {
            MENULEVEL = 0;
            newMenuLevel=true;
            oldwinmin=-2;
            display.fillScreen(COLOR_BGND);
            display.setFont();
            display.setTextSize(2);
            DEBUG=true;
         }
         else {
            Explorer();
         }
      }
      return;
   }


   //----------------------
   // show date+time
   // MENULEVEL==3

   if (MENULEVEL==3)
   {
      static bool newm3=true;
      DEBUG=false;
      if(newMenuLevel) {
         if(newm3)  {
            display.fillScreen(COLOR_BGND);  // 1st time clear
            newm3=false;
         }
         time_refreshDisplay();
         read_ADS0();
         if (btn00==1) newMenuLevel=0;
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL = 0;
            newMenuLevel=true;
            DEBUG=true;
            newm3=true;
            display.fillScreen(COLOR_BGND);
         }
      }
      return;
   }

   //----------------------
   // Game: CHESS
   if(MENULEVEL==4) {
      if(newMenuLevel) {
         drawAllTSbuttons();
         display.fillScreen(COLOR_BGND);
         display.setTextColor(YELLOW);
         display.setCursor( 100,  60 );
         display.println("CHESS\n\nBtn-Top: ESC  -  Btn-Left: Start");
         display.println();
         display.setTextColor(WHITE);
         newMenuLevel=false;

         while(btn00!=1 && btn00!=3) {
            read_ADS0();
            if(btn00==1) return;
            delay(100);
         }
         Chess();  // <<<<<<<<<<<<<<<<<   CHESS !
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL = 0;
            newMenuLevel=true;
            display.fillScreen(COLOR_BGND);
         }
      }
      return;
   }


   //----------------------
   // Game: PAINT
   if(MENULEVEL==5) {
      if(newMenuLevel) {
         //TSBUTTONSENABLED = 1;
         //drawAllTSbuttons();
         display.fillScreen(COLOR_BGND);
         display.setTextColor(YELLOW);
         display.setCursor( 100,  60 );
         display.println("PAINT\n\nBtn-Top: ESC  -  Btn-Left: Start");
         display.println();
         newMenuLevel=false;

         while(btn00!=1 && btn00!=3) {
            read_ADS0();
            if(btn00==1) return;
            delay(100);
         }
         Paint();  // <<<<<<<<<<<<<<<<<   PAINT !
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL = 0;
            newMenuLevel=true;
            display.fillScreen(COLOR_BGND);
         }
      }
      return;
   }


   //----------------------
   // Game: PONG
   if(MENULEVEL==6) {
      if(newMenuLevel) {
         display.fillScreen(COLOR_BGND);
         display.setTextColor(YELLOW);
         display.setCursor( 100,  60 );
         display.println("PONG\n\nBtn-Top: ESC  -  Btn-Left: Start");
         display.println();
         newMenuLevel=false;

         while(btn00!=1 && btn00!=3) {
            read_ADS0();
            if(btn00==1) return;
            delay(100);
         }
         Pong();
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL = 0;
            newMenuLevel=true;
            display.fillScreen(COLOR_BGND);
         }
      }
      return;
   }



   //----------------------
   // Widget tools
   if(MENULEVEL==7) {
      if(newMenuLevel) {
         //drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         display.setTextColor(WHITE);
         display.setCursor(  0, 0 );
         int l=0;
         char cbuf[30];
         for(int i=0; i<256; i++) {
            if( !(i%20) ) {
               sprintf(cbuf, "\n%3d ", l);
               display.print(cbuf);
               l+=20;
            }
            if(i=='\n') display.write('N');
            else if(i=='\r') display.write('R');
            else display.write( (char)i );
         }
         display.println("\n");
         display.print("page # ");  display.print(MENULEVEL);
         display.print(" press switch-button: top \nto scroll through menu options!");
         newMenuLevel=false;
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL = 0;
            newMenuLevel = true;
            display.fillScreen(COLOR_BGND);
            // drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         }
      }
      return;
   }
   //----------------------
   // SD reboot
   if(MENULEVEL==8) {
      display.fillScreen(COLOR_BGND);
      Serial.println("MENULEVEL==6");
      display.println("MENULEVEL==6");
      delay(1000);
      SD_Reset();
      Serial.println("SD reboot done: " + String(SDioerr));
      display.println("SD reboot done:" + String(SDioerr));
      delay(1000);
      MENULEVEL = 0;
   }
}

//===========================================================
// Apps Menu End
//===========================================================



//=====================================================================
// show date+time
//=====================================================================

void time_refreshDisplay() {
   static uint32_t printtimestamp=millis()+800;
   if(millis()-printtimestamp>1000) {
      displayLocalTime();
      printtimestamp=millis();
   }
}


//=====================================================================
// Thread blinker_loop()
//=====================================================================
void blinker_loop() {
   vTaskPrioritySet( NULL, 0 ); // ESP_TASK_MAIN_PRIO = main prio

   while(THREADRUN != 0) {
      if(!THREADSUSPENDED) {
         LED_writeScreen(LED_pin_STATE);  // virual LED on TFT screen
         LED_pin_STATE=!LED_pin_STATE;
         delay(500);
      }
   }
}



//=====================================================================
// handleWebsite()
//=====================================================================

void handleWebsite() {
   char timestr[80];
   getLocalTime(timestr);

   String script="";
   String website_title = "myESP32WifiServer";

   WiFiClient client = wifiserver.available();   // listen for incoming clients

   //if(!client) return;  // <<<<<<<< needed?

   if (client) {                               // if you get a client,
      Serial.println("New Client.");           // print a message out the serial port
      String currentLine = "";                 // make a String to hold incoming data from the client
      while (client.connected()) {             // loop while the client's connected
         if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();            // read a byte, then
            Serial.write(c);                   // print it out the serial monitor
            if (c == '\n') {                   // if the byte is a newline character

               // if the current line is blank, you got two newline characters in a row.
               // that's the end of the client HTTP request, so send a response:
               if (currentLine.length() == 0) {
                  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                  // and a content-type so the client knows what's coming, then a blank line:

                  script += "HTTP/1.1 200 OK \n";
                  script += "Content-type:text/html \n";
                  script += "\n";

                  script += "<head> \n";
                  // autom. Aktualisierung alle 10 sec.
                  script += "<meta http-equiv=\"refresh\" content=\"10\" > \n" ;

                  // utf-8 für "°" Zeichen
                  script += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> \n" ;
                  script += "<title>";
                  script += website_title;
                  script += "</title> ";

                  script += "</head> \n";


                  script += "<h1>My First WiFiServer with ESP32 - Station Mode &#128522;</h1> \n";

                  script += "<h2>";
                  script += (String)timestr +"<br>\n";
                  script += "</h2> \n";

                  //---------------------------------------
                  // text font Courier, color black
                  script += "<p> <font face=\"courier\">"; // <<<<<<<<<<<<<<<< Courier
                  script += "<font style=\"color:rgb(0,0,0);\" > </p>";
                  script += "<br> \n";

                  script += "<h3> ";

                  script += "Output 1 ist: ";
                  if (OUT1 == 1)
                  {
                     script += ("EIN &nbsp; <wbr> <wbr> <br>\n");
                  }
                  else
                  {
                     script += ("AUS &nbsp; <wbr> <wbr> <br>\n");
                  }

                  script += "<a href=\" /OUT1H\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ";
                  script += "<a href=\" /OUT1L\"\"> <button style=\"height:70px;width:140px\" > AUS </button></a>  ";
                  script += "<br> \n\n";

                  script += "</h3> ";
                  script += "\n";

                  // The HTTP response ends with another blank line:
                  script += "\n";

                  client.print(script);

                  delay(1);

                  // break out of the while loop:
                  break;

               }
               else {    // if you got a newline, then clear currentLine:
                  currentLine = "";
               }
            }
            else if (c != '\r') {  // if you got anything else but a carriage return character,
               currentLine += c;      // add it to the end of the currentLine
            }
            delay(1);

            // Check if the client request was "GET /OUT1H" or "GET /OUT1L":
            if (currentLine.endsWith("GET /OUT1H")) {
               OUT1=HIGH;              // GET /OUT1H turns the LED on
            }
            if (currentLine.endsWith("GET /OUT1L")) {
               OUT1=LOW;               // GET /OUT1L turns the LED off
            }

         }
      }
      // close the connection:
      client.stop();
      delay(1);
      Serial.println("Client Disconnected.");
   }
   delay(1);

}


//------------------------------------
void handleNotFound() {

   delay(1);
}

//------------------------------------
void handleRoot() {
   //handleClients();
}



//=====================================================================
// setup()
//=====================================================================
void setup() {
   int tftline=10;
   String StrBuf="";

   Serial.begin(115200);
   delay(1000);
   Serial.println("Serial started!");

   //LED_pin=LED_BUILTIN;  // default: LED_pin=LED_BUILTIN
   pinMode(LED_pin, OUTPUT);
   pinMode(LED1pin, OUTPUT);

   Wire.begin();
   Wire.setClock(400000);
   delay(10);

   //---------------------------------------------------------------------
   //disable brownout detector
   //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
   // alternatively:

   uint32_t brown_reg_temp = READ_PERI_REG(RTC_CNTL_BROWN_OUT_REG); //save WatchDog register
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
   //WiFi.mode(WIFI_MODE_STA); // turn on WiFi
   //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, brown_reg_temp); //enable brownout detector


   //---------------------------------------------------------
   Adafruit_HX8357_ini(TFT_ROTAT);  // init function in <TFT_HX3857.h>:: TFT, Touch, SD
   delay(10);

   COLOR_BGND = BLACK;
   display.fillScreen(COLOR_BGND);
   display.setTextColor(WHITE);
   display.setTextSize(2);
   //display.setFont(&FreeMono9pt7b);
   Serial.println("display setup done!");
   Serial.println();
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("display setup done!");
   tftline+=15;
   delay(10);

   //---------------------------------------------------------
   // TS buttons
   //
   char cbuf[5]="    ";
   TSbutton0.initButton(&display, display.width()-30, 20,  60,28,  CYAN, BLUE, YELLOW, "ESC", 2);
   cbuf[0]='1'; cbuf[1]=' '; cbuf[3]=24;
   TSbutton1.initButton(&display, display.width()-30, 72,  60,28,  CYAN, BLUE, YELLOW, cbuf,  2);
   cbuf[0]='2'; cbuf[1]=' '; cbuf[3]=27;
   TSbutton2.initButton(&display, display.width()-30,124,  60,28,  CYAN, BLUE, YELLOW, cbuf, 2);
   cbuf[0]='3'; cbuf[1]=' '; cbuf[3]=26;
   TSbutton3.initButton(&display, display.width()-30,176,  60,28,  CYAN, BLUE, YELLOW, cbuf, 2);
   cbuf[0]='4'; cbuf[1]=' '; cbuf[3]=25;
   TSbutton4.initButton(&display, display.width()-30,228,  60,28,  CYAN, BLUE, YELLOW, cbuf, 2);
   TSbutton5.initButton(&display, display.width()-30,280,  60,28,  CYAN, LIGHTBLUE, YELLOW, "SET", 2);


   Serial.println("ts buttons setup done!");
   Serial.println();
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("ts buttons setup done!");
   tftline+=15;
   delay(10);

   //---------------------------------------------------------
   // SD
   // SD.begin(); // by TFT lib
   display.setCursor(0, tftline);
   delay(10);
   /*
      if(!SdExist(SD)) {
      //SD card removed
      SDioerr = 0;
      }
   */
   if( !SDioerr ) {
      Serial.println("SD.begin(SD_CS) failed!");
      Serial.println();
      display.setTextColor(RED);
      display.print("SD.begin(SD_CS) failed!");
      display.setTextColor(WHITE);
      return;  //
   }
   else {
      SdPath = SD.open("/");
      Serial.println("SD setup done!\n");
      display.setTextColor(WHITE);
      display.print("SD setup done!");

      SD_File = SD.open("/config.txt");
      if (!SD_File) {
         Serial.print("The text file cannot be opened");
         display.setTextColor(RED);
         display.print("The text file cannot be opened");
         tftline+=15;
         delay(500);
      }
      else {
         String buf1="", bufold="";
         Serial.println("reading SD config.txt...");
         while (SD_File.available()) {
            buf1 = SD_File.readStringUntil('\n');
            int blen=buf1.length();
            if( blen>=1 && buf1[blen-1]=='\r' ) buf1.remove(blen-1, 1);
            if( bufold.startsWith((String)"SSID") ) { //myString.startsWith(myString2)
               SSID=buf1;
               Serial.println("SD-SSID value="+SSID+(String)"<");
            }
            if( bufold.startsWith((String)"PASSWORD") ) {
               PASSWORD=buf1;
               Serial.println("SD-PASSWORD value="+PASSWORD+(String)"<");
            }
            if( bufold.startsWith((String)"LOCALIP") ) {
               StringLOC_IP=buf1;
               Serial.println("SD-LOCALIP value="+StringLOC_IP+(String)"<");
            }
            if( bufold.startsWith((String)"GATEWAY") ) {
               StringGW_IP=buf1;
               Serial.println("SD-GATEWAY value="+StringGW_IP+(String)"<");
            }
            if( bufold.startsWith((String)"SUBNET") ) {
               StringSUB_IP=buf1;
               Serial.println("SD-SUBNET value="+StringSUB_IP+(String)"<");
            }

            bufold=buf1;
         }
         SD_File.close();
      }
   }
   Serial.println();
   // DEBUG
   //while(1) delay(100);

   tftline+=15;
   delay(10);


   //---------------------------------------------------------
   //i2c devices

   ads0.begin(0x48);
   Serial.println("i2c+ads1115 setup done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("i2c+ads1115 setup done!");
   tftline+=15;
   delay(10);

   init_MPU6050();
   delay(1);
   Serial.println("MPU6050 setup done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("MPU6050 setup done!");
   tftline+=15;
   delay(10);


   //--------------------------------------------------------
   // connecting to router, WiFi, Time, WiFiServer, WebServer
   //--------------------------------------------------------
   GATEWAY.fromString(StringGW_IP.c_str() );
   LOCALIP.fromString(StringLOC_IP.c_str() );
   SUBNET.fromString(StringSUB_IP.c_str() );
   Serial.println(SSID);
   Serial.println(LOCALIP);
   Serial.println(GATEWAY);
   Serial.println(SUBNET);
   Serial.println(WIFIPORT);
   Serial.println(WEBPORT);
   // DEBUG
   // while(1) delay(100);

   delay(500);

   Serial.print("try WiFi mode: ");
   display.setTextColor(YELLOW);
   display.setCursor(0, tftline);
   WiFi.persistent(false);

   display.print("try WiFi mode: ");
   WiFi.mode(WIFI_STA);
   //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, brown_reg_temp); //enable brownout detector
   delay(10);
   Serial.println("boot mode done ");
   display.setTextColor(WHITE);
   display.print("boot mode done ");
   tftline+=15;
   delay(10);

   Serial.print("try WiFi config: ");
   display.setTextColor(YELLOW);
   display.setCursor(0, tftline);
   display.print("try WiFi config: ");
   WiFi.config(LOCALIP, GATEWAY, SUBNET, GATEWAY, GATEWAY);
   Serial.println("config done ");
   display.setTextColor(WHITE);
   display.print("config done ");
   tftline+=15;
   delay(10);

   char ssid[64], pwd[64];
   strcpy(ssid, SSID.c_str() );
   strcpy(pwd, PASSWORD.c_str() );

   delay(10);
   Serial.println("try WiFi begin: ");
   display.setTextColor(YELLOW);
   display.setCursor(0, tftline);
   display.print("try WiFi begin: ");

   WiFi.begin( ssid, pwd );
   delay(10);
   // Wait for connection
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      display.print(".");
   }
   delay(1);
   Serial.println();
   tftline+=15;
   delay(1);

   StrBuf="  IP address: " + WiFi.localIP().toString();
   Serial.println(StrBuf);
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print(StrBuf);
   tftline+=15;

   //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, brown_reg_temp); //enable brownout detector
   delay(1);

   //---------------------------------------------------------
   //init and get the time
   // POSIX timezones strings reading data from /usr/share/zoneinfo

#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03" // Europe/Berlin  CT-1CEST,M3.5.0,M10.5.0/3

   configTime(0, 0, ntpServer); // 0, 0 because we will use TZ in the next line
   setenv("TZ", MY_TZ, 1);      // Set environment variable with your time zone
   tzset();

   delay(1);
   Serial.println();
   Serial.print("get NTP: ");
   display.setTextColor(YELLOW);
   display.setCursor(0, tftline);
   display.print("call NTP: ");
   getLocalTime(sbuf);
   Serial.println(sbuf);
   Serial.println();
   display.setTextColor(WHITE);
   display.print(sbuf);
   tftline+=15;
   delay(1);

   // DEBUG
   // while(1) delay(100);


   //----------------------------------------
   // Start the WiFiServer -> website
   wifiserver.begin();
   StrBuf="WiFiServer started, port " + (String)WIFIPORT;
   Serial.println();
   Serial.println(StrBuf);
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print(StrBuf);
   tftline+=15;
   delay(1);


   //---------------------------------------------------------
   // WebServer
   /*
      webserver.on("/", handleRoot) ;
      webserver.on("/client/client0/", handleClients);
      webserver.on("/client/client1/", handleClients);
      webserver.on("/client/client2/", handleClients);
      webserver.on("/client/client3/", handleClients);
      //webserver.onNotFound(handleNotFound);
   */
   webserver.begin();
   delay(1);
   Serial.println();
   StrBuf="WebServer started, port " + (String)WEBPORT;
   Serial.println(StrBuf);
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print(StrBuf);
   tftline+=15;
   delay(1);

   // DEBUG
   //while(1) delay(100);


   //---------------------------------------------------------
   // RTOS std::thread
   thread_1 = new std::thread(blinker_loop);

   delay(10);
   Serial.println();
   Serial.println("std::thread init done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("std::thread init done!");
   tftline+=15;
   delay(10);

   //---------------------------------------------------------
   randomSeed( millis() + analogRead(A0) );


   //---------------------------------------------------------
   // end of setup
   Serial.println();
   Serial.println("setup(): done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("setup(): done!");
   tftline+=15;
   delay(2000);
   display.fillScreen(COLOR_BGND);
   // drawAllTSbuttons(); // for debug <<<<<<<<<<

   Serial.println("setup() terminated!\n");

   // debug colors
   //int mycol=ColorRGB2Color16bit(192,192,255);
   //Serial.println( "mycol=" + (String)mycol );

}


//=====================================================================
//=====================================================================
void loop() {
   //char str1[128];

   // GPIOs

   if(OUT1)   {
      digitalWrite(LED1pin, HIGH);
   }
   else   {
      digitalWrite(LED1pin, LOW);
   }

   handleWebsite();  // WiFiServer for website

   /*
      // WebServer for clients
      webserver.handleClient();
      delay(1);
   */
   //-----------------------------------------
   // touch screen buttons
   GetTSbuttons();

   //-----------------------------------------

   // IMU readings
   if(DEBUG) {
      //Serial.print("read MPU6050: ");
   }
   read_MPU6050(fyaw, fpitch, froll);
   pitch = (int16_t)fpitch;
   roll = -(int16_t)froll;
   delay(2);    // 3

   // ads1115 readings
   read_ADS0();

   // debug
   if (DEBUG) {
      sprintf(sbuf, "%-4d %-4d y=%-4d x=%-4d p=%-4d r=%-4d",
              adc00, adc01, adc02, adc03, pitch, roll);
      // Serial.print((String)sbuf); Serial.println();

      display.fillRect( 0, display.height()-14, display.width()-13, 14, BLACK);
      display.setTextColor(RED);
      display.setTextSize(2);
      display.setFont();
      display.setCursor(0, display.height()-14);
      display.print((String)sbuf);
   }


   delay(20);
   showAppsMenu();
   delay(30);

   // debug
   uint8_t DEBUGSELECTFILE=0;
   if(DEBUGSELECTFILE) {
      display.fillRect( 0, display.height()-30, display.width(), 16, BLACK);
      display.setCursor(0, display.height()-30);
      display.setTextColor(YELLOW);
      display.print(fileSelected);
   }
}

/*
   references:
   https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/pinouts
   https://werner.rothschopf.net/microcontroller/202103_arduino_esp32_ntp_en.htm
   https://github.com/TKJElectronics/KalmanFilter
   https://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/
   https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/SimpleWiFiServer/SimpleWiFiServer.ino
   https://charbase.com/block/emoticons
   https://github.com/espressif/arduino-esp32/issues/863#issuecomment-575923300
   http://home.hccnet.nl/h.g.muller/umax4_8.c


*/
