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

// ver 2.0c

// change log:
// 2.3: debug: no threads...  
// 2.2: a:PAINT  b-e:CHESS
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
std::thread *thread_2;
std::thread *thread_3;

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


static int    cursorfilenr=0, selectfilenr=-1;
static String fileMarked="", fileSelected="";

//-------------------------------------------
// list dir

void ls(String *list, int size, int selectnr) {
   Serial.println("print list[]:");
   for (int cnt = 0; cnt < size; cnt++) {
      if (cnt < 100) Serial.print(" ");
      if (cnt < 10)  Serial.print(" ");
      Serial.println((String)cnt+"  "+list[cnt]);

      int COLOR_TEXT;
      if(cnt==selectnr) COLOR_TEXT=LIME;
      else COLOR_TEXT=WHITE;

      display.setTextColor(COLOR_TEXT);
      display.setCursor(20+240*(cnt%2), 0+12*(cnt/2));  // <<<< filepos
      display.print(list[cnt]);
   }
   Serial.println("print list[] done!\n");
}


//-------------------------------------------
// mark curs pos for file select

void markPos(int old, int cnt) {
   display.fillRect ( 0+240*(old%2), 12*(old/2), 14, 14, COLOR_BGND); // <<<< filepos
   display.setTextColor(LIME);
   display.setCursor( 0+240*(cnt%2), 0+12*(cnt/2));
   display.print(">");
   display.setTextColor(WHITE);
   delay(200);
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
   cursorfilenr = 0;
   readDirectory(SdPath, 0);
   return SDioerr;
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
// 0 NONE=800-1022
//          1 esc=310-360
//          2 up= 150-180
// 3 left=20-40     4 right=70-100
//          5 dn=1022-1024 // ESP32 port bug
//


int16_t PAD_NONE  =  800,   // ADC min values
        PAD_ESC   =  310,
        PAD_UP    =  150,
        PAD_LEFT  =   20,
        PAD_RIGHT =   65,
        PAD_DOWN  = 1022;

//===========================================================
int32_t btnstate00(int val) {
   if (isInRange( adc00,  PAD_NONE, 1021 ) ) return 0; // NONE
   if (isInRange( adc00,  PAD_ESC,   400 ) ) return 1; // ESC
   if (isInRange( adc00,  PAD_UP,    200 ) ) return 2; // up
   if (isInRange( adc00,  PAD_LEFT,   60 ) ) return 3; // left
   if (isInRange( adc00,  PAD_RIGHT, 100 ) ) return 4; // right
   if (isInRange( adc00,  PAD_DOWN, 1025 ) ) return 5; // down
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
#include "time.h"
struct tm * timeinfo;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;

time_t currenttime;

/*
   // uses from: <arduArray.h>
   int32_t  mapConstrain
   (int32_t val, int valmin=0, int valmax=17600, int newmin=0, int newmax=1023);
*/


//------------------------------------
void getLocalTime(char * buffer) {
   time_t rawtime;

   time (&rawtime);
   timeinfo = localtime (&rawtime);
   strftime (buffer,80,"%a %d %b %Y %H:%M:%S ",timeinfo);
   Serial.println(buffer);
}

//------------------------------------
void displayLocalTime()
{
   char buffer [80];
   getLocalTime(buffer);

   Serial.print("Date & Time:  ");
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

Adafruit_GFX_Button TSbutton0, TSbutton1, TSbutton2, TSbutton3, TSbutton4;

int tsx=0, tsy=0, tsz=0;

//---------------------------------------------------------------------

void drawAllTSbuttons() {
   TSbutton0.drawButton();
   TSbutton1.drawButton();
   TSbutton2.drawButton();
   TSbutton3.drawButton();
   TSbutton4.drawButton();
}

int transformTSrotat() {
   if (TFT_ROTAT==3 ) {
      tsx=p.y;
      tsy=p.x;
   }
   else {
      tsx=p.x;
      tsy=p.y;
   }
   return TFT_ROTAT;
}

//-------------------------------------------

int GetTSbuttons() {
   char str1[30], str2[30];
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

   delay(1);
    
   // Scale from ~0->4000 to display.width using the calibration #'s
   p.x = map(p.x, TS_MAXX, TS_MINX, 0, display.height());
   p.y = map(p.y, TS_MINY, TS_MAXY, 0, display.width());
   transformTSrotat();
   tsz = p.z;
   delay(1);

   if(DEBUG) {
      Serial.print("X = "); Serial.print(p.x);
      Serial.print("\tY = "); Serial.print(p.y);
      Serial.print("\tPressure = "); Serial.println(p.z);
      Serial.println();
      sprintf(str1, "x%3d y%3d z%3d", p.x, p.y, p.z);

      //display.setTextColor(WHITE);
      //display.fillRect(display.width()-80,40, 80,40, COLOR_BKGR);  // x1,y1, dx,dy
      //display.setCursor(display.width()-75,40);    display.print(str1);

      delay(1);
   }
   //-------------------------------------------
   // check if a TSbutton was pressed
   //-------------------------------------------
   TSbutton0.press(TSbutton0.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton1.press(TSbutton1.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton2.press(TSbutton2.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton3.press(TSbutton3.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton4.press(TSbutton4.contains(p.y, p.x)); // tell the TSbutton it is pressed

   delay(1);

   //-------------------------------------------
   // process TSbutton states+changes
   //-------------------------------------------
   if(DEBUG) {
      Serial.println("buttons");
   }

   if (TSbutton0.justReleased() ) {
      btnUp=0;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton0.justPressed()) {
      TSbutton0.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton1.justReleased() ) {
      btnUp=1;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton1.justPressed()) {
      TSbutton1.drawButton(true); // draw invert!
   }

   //-------------------------------------------


   else if (TSbutton2.justReleased() ) {
      btnUp=2;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton2.justPressed()) {
      TSbutton2.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton3.justReleased() ) {
      btnUp=3;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton3.justPressed()) {
      TSbutton3.drawButton(true); // draw invert!
   }


   else if (TSbutton4.justReleased() ) {
      btnUp=4;
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton4.justPressed()) {
      TSbutton4.drawButton(true); // draw invert!
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

   delay(1);

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


//===========================================================
// PAINT
//===========================================================

void Paint() {
   bool PAINTACTIVE = true;
   static int BOXSIZE = 26;
   static int PENRADIUS = 3;
   static int currentcolor, oldcolor;
   char str1[50];
   static int x=-1, y=-1, z=-1;
   int tsBtnPressed=-1;

   display.setFont();
   display.setTextSize(1);
anew:
   read_ADS0();

   display.fillScreen(HX8357_BLACK);
   // draw all Buttons
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
      sprintf(str1, "%3d %3d %3d ", x, y, PENRADIUS);

      display.setTextColor(WHITE);
      display.fillRect(270,0, 150,40, BLACK);  // x1,y1, dx,dy
      display.setCursor(270,0);    display.print(str1);

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

#define HASHSIZE (1<<8) //  wegen RAM, für PC: (1<<24) // <<<<<<  für Arduino Mega 1<<8 !

struct HTab {
   int  Key,
        Score;
   int  From,
        To,
        Draft;
} HashTab[HASHSIZE];                             // hash table, HTsize entries



#define MAXNODES  160000L   // max deepening: increased values => higher skills

int  K,
     RootEval,
     R,
     HashKeyLo,
     HashKeyHi;

int32_t   N,
          INF=8000;                                     // INF=8000: "infinity eval score"   ;

int   bchk=136,                                            // board check: bchk=0x88   board system
      S=128,                                               // dummy square 0x80, highest valid square =127
      Side=16,                                             // 16=white, 8=black; Side^=24 switches ther and back
      Player=Side;
#define PLAYERWHITE  (Side==16)
#define PLAYERPBLACK (Side!=16)

signed char  L;

//------------------------------------------------------------

// piece list (number, type) is {1,2,3,4,5,6,7} = {P+,P-,N,K,B,R,Q}.
// Type 0 is not used, so it can indicate empty squares on the board.
signed char
pval[]= {0,2,2,7,-1,8,12,23},                       // relative piece values
        // step-vect list:
        // piece type p finds starting dir[j] at [p+16]
        pvect[]= {-16,-15,-17,0,                            // pawns
                  1,16,0,                                    // rook
                  1,16,15,17,0,                              // king, queen and bishop
                  14,18,31,33,0,                             // knight
                  7,-1,11,6,8,3,6
                 },                          // 1st dir. in pvect[] per piece

                 stdBaseLn[]= {6,3,5,7,4,5,3,6},    // initial piece setup

                              board[129],                                         // board: half of 16x8+dummy
                              T[1035];                                            // hash translation table

signed char  psymbol[]= ".?+nkbrq?*?NKBRQ";              // .empty ?undef +downstreamPawn *upstreamPawn
// nKnight kKing bBishop rRook qQueen

int  mfrom, mto;       // current ply from - to
int  Rootep,           // e.PieceType. square
     rmvP=128;         // remove piece


//------------------------------------------------------------
// compute HashTable value
// #define Kb(A,B) *(int*)(T+A+(B&8)+S*(B&7))
static int  Kb( const signed char A, const int B )
{
   void const * const ptr = T+A+(B&8)+S*(B&7);
   int result;
   memcpy(&result, ptr, 4);
   return result;
}


//------------------------------------------------------------
// recursive minimax search, Side=moving side, n=depth
//------------------------------------------------------------

int busycount=0;

int  Minimax (int32_t  Alpha, int32_t  Beta, int32_t  Eval, int  epSqr, int  prev, int32_t   Depth)
// (Alpha,Beta)=window, Eval, EnPass_sqr.
// prev=prev.dest; HashKeyLo,HashKeyHi=hashkeys; return EvalScore

{
   int  j,
        StepVec,
        BestScore,
        v,
        IterDepth,
        h,
        i,
        SkipSqr,
        RookSqr,
        V,
        P,            // Null Move Prawning
        f=HashKeyLo,
        g=HashKeyHi,
        C,
        s;
   signed char  Victim,
          PieceType,
          Piece,
          FromSqr ,
          ToSqr ,
          BestFrom,
          BestTo,
          CaptSqr,
          StartSqr ;

   struct HTab *a = HashTab + ((HashKeyLo + Side * epSqr) & (HASHSIZE-1)); // lookup pos. in hash table, improved

   char sbuf[100];

   Alpha--;                                             // adj. window: delay bonus
   Side^=24;                                            // change sides
   IterDepth  = a->Draft;
   BestScore  = a->Score;
   BestFrom   = a->From;
   BestTo     = a->To;                                  // resume at stored depth

   if(a->Key-HashKeyHi | prev |                         // miss: other pos. or empty
         !(BestScore<=Alpha | BestFrom&8&&BestScore>=Beta | BestFrom&S))            //   or window incompatible
   {
      IterDepth=BestTo=0;   // start iter. from scratch
   }

   BestFrom&=~bchk;                                        // start at best-move hint

   while( IterDepth++ <  Depth || IterDepth<3           // iterative deepening loop
          || prev&K == INF
          && ( (N<MAXNODES  && IterDepth<98)                   // root: deepen upto time; changed from N<60000
               || (K=BestFrom, L=BestTo&~bchk, IterDepth=3)
             )
        )                                                  // time's up: go do best
   {
      FromSqr =StartSqr =BestFrom;                      // start scan at prev. best
      h=BestTo&S;                                       // request try noncastl. 1st

      P=IterDepth<3 ? INF : Minimax(-Beta,1-Beta,-Eval,S,0,IterDepth-3);            // Search null move

      BestScore = (-P<Beta | R>35) ? ( IterDepth>2 ? -INF : Eval ) : -P;            // Prune or stand-pat
      N++;                                              // node count (for timing)
      do
      {
         Piece=board[FromSqr];                         // scan board looking for
         if(Piece & Side)                               //  own piece (inefficient!)
         {
            StepVec = PieceType = Piece&7;              // PieceType = piece type (set StepVec>0)
            j = pvect[PieceType+16];                    // first step pvect f.piece
            while(StepVec = PieceType>2 & StepVec<0 ? -StepVec : -pvect[++j] )       // loop over directions pvect[]
            {
labelA:                                                                 // resume normal after best
               ToSqr =FromSqr ;                                         // (FromSqr ,ToSqr )=move
               SkipSqr= RookSqr =S;                                     // (SkipSqr, RookSqr )=castl.R

               do
               {  // ToSqr  traverses ray, or:
                  CaptSqr=ToSqr =h?BestTo^h:ToSqr +StepVec;             // sneak in prev. best move

                  if(ToSqr &bchk)break;                                 // board edge hit

                  BestScore= epSqr-S&board[epSqr]&&ToSqr -epSqr<2&epSqr-ToSqr <2?INF:BestScore;      // bad castling

                  if( PieceType<3 && ToSqr==epSqr) CaptSqr^=16;         // CaptSqr for E.P. if Eval of PieceType=Pawn.
                  // <3 is a piecetype which represent pawns (1,2)
                  // (officers are >=3: 6,3,5,7,4)
                  Victim =board[CaptSqr];

                  if(Victim & Side|PieceType<3 & !(ToSqr -FromSqr & 7)-!Victim )break;            // capt. own, bad pawn mode
                  i=37*pval[Victim & 7]+(Victim & 192);                 // value of capt. piece Victim
                  BestScore =i<0?INF:BestScore ;                        // K capture

                  if(BestScore >=Beta & IterDepth>1) goto labelC;       // abort on fail high

                  v=IterDepth-1?Eval:i-PieceType;                       // MVV/LVA scoring

                  if(IterDepth-!Victim >1)                              // remaining depth
                  {
                     v=PieceType<6?board[FromSqr+8]-board[ToSqr+8]:0;            // center positional pts.
                     board[RookSqr]=board[CaptSqr]=board[FromSqr]=0; board[ToSqr]=Piece|32;            // do move, set non-virgin
                     if(!( RookSqr & bchk))board[SkipSqr]=Side+6,v+=50;               // castling: put R & Eval
                     v-=PieceType-4|R>29?0:20;                                     // penalize mid-game K move

                     if(PieceType<3)                                               // pawns:
                     {
                        v-=9*((FromSqr -2 & bchk||board[FromSqr -2]-Piece)+           // structure, undefended
                              (FromSqr +2 & bchk||board[FromSqr +2]-Piece)-1         //        squares plus bias
                              +(board[FromSqr ^16]==Side+36))                      // kling to non-virgin King
                           -(R>>2);                                             // end-game Pawn-push bonus
                        V=ToSqr +StepVec+1 & S?647-PieceType:2*(Piece & ToSqr +16 & 32);           // promotion or 6/7th bonus
                        board[ToSqr]+=V;
                        i+=V;                                                     // change piece, add Eval
                     }

                     v+= Eval+i;
                     V=BestScore >Alpha ? BestScore  : Alpha;                      // new eval and alpha
                     HashKeyLo+=Kb(ToSqr +0,board[ToSqr])-Kb(FromSqr +0,Piece)-Kb(CaptSqr +0,Victim );
                     HashKeyHi+=Kb(ToSqr+8,board[ToSqr])-Kb(FromSqr+8,Piece)-Kb(CaptSqr+8,Victim )+ RookSqr -S;  // update hash key
                     C = IterDepth-1-(IterDepth>5 & PieceType>2 & !Victim & !h);
                     C = R>29|IterDepth<3|P-INF?C:IterDepth;                       // extend 1 ply if in check
                     do {
                        s= (C>2)||(v>V)?
                           -Minimax(-Beta,-V,-v, SkipSqr,0,C)              // recursive eval. of reply
                           :v;                                            // or fail low if futile
                     } while( s>Alpha && ++C<IterDepth );

                     v=s;
                     if(prev && K-INF && v+INF && (FromSqr==K) & (ToSqr==L) )      // move pending & in root:
                     {
                        RootEval=-Eval-i; Rootep=SkipSqr;               // exit if legal & found
                        a->Draft=99; a->Score=0;                        // lock game in hash as draw
                        R+=i>>7;
                        return Beta;                                    // captured non-P material
                     }
                     HashKeyLo=f;
                     HashKeyHi=g;                                       // restore hash key
                     board[RookSqr]=Side+6;
                     board[SkipSqr]=board[ToSqr]=0;
                     board[FromSqr]=Piece;
                     board[CaptSqr]=Victim ;                           // undo move, RookSqr can be dummy
                  }
                  if(v>BestScore )                                      // new best, update max,best
                  {
                     BestScore = v, BestFrom=FromSqr, BestTo=(ToSqr | (S & SkipSqr));   // mark double move with S
                  }
                  if(h)
                  {
                     h=0;
                     goto labelA;                                       // redo after doing old best
                  }
                  if (
                     FromSqr + StepVec - ToSqr  || Piece & 32 ||               // not 1st step,moved before
                     PieceType>2 && (PieceType-4 | j-7 ||                      // no P & no lateral K move,
                                     board[RookSqr = FromSqr+3 ^StepVec>>1 & 7] -Side-6      // no virgin R in corner  RookSqr,
                                     || board[RookSqr^1] || board[RookSqr^2] )             // no 2 empty sq. next to R
                  )
                  {
                     Victim += PieceType<5;
                  }                                                     // fake capt. for nonsliding
                  else SkipSqr=ToSqr ;                                  // enable e.PieceType.

               } while(!Victim );                                       // if not capt. continue ray

            }
         }  // (Piece &  Side)

      } while( (FromSqr = (FromSqr +9) & ~bchk)-StartSqr );                // next sqr. of board, wrap

labelC:
      if (BestScore >INF-bchk || BestScore <bchk-INF) IterDepth=98;           // mate holds to any depth
      BestScore = BestScore +INF || P==INF ? BestScore  : 0;            // best loses K: (stale)mate

      if(a->Draft<99) {                                                 // protect game history
         a->Key=HashKeyHi;
         a->Score=BestScore ;
         a->Draft=IterDepth;                                            // always store in hash tab
         a->From=BestFrom|8*(BestScore >Alpha)|S*(BestScore <Beta);
         a->To=BestTo;                                                  // move, type (bound/exact),
      }
      // uncomment for Kibitz
      // if(prev) sprintf(sbuf, "%2d ply, %9d searched, score=%6d by %c%c%c%c\n",
      //     IterDepth-1, N-S, BestScore , 'a'+(BestFrom & 7),'8'-(BestFrom>>4),'a'+(BestTo & 7),'8'-(BestTo>>4 & 7));

      if( (N%2000)<1) {
          delay(1);
      }      
      if(prev  && BestFrom!=BestTo) {
         busycount=0;
         sprintf(sbuf,  "\n%2d ply, searched: %9d ", IterDepth-1, N-S );
         display.fillRect(0,260, 480,60, BLACK);
         Serial.print(sbuf);
         display.setCursor(0,265);
         display.print(sbuf);
      }
      else if( ((N-S)%10000)<1) {
         Serial.print(".");  
         if(busycount>40){
            busycount=0;
            display.fillRect(0,295, 480,25, BLACK);
            display.setCursor(0,295);
         }
         display.print("."); 
         busycount++;
      }

   }  // while (iterative deepening loop)

   Side^=24;                                                            // change sides back
   mfrom=K; mto=L;
   return BestScore += BestScore <Eval;                                 // delayed-loss bonus
}


//------------------------------------------------------------
void standardBoard() {                                // initial board setup
   int col=8;                                         // count by column (=file)
   memset(board, 0, sizeof(board));
   while(col--) {
      board[col]=(board[col+112]=stdBaseLn[col]+8)+8; // 1st+8th line (=rank): pcs by setup series
      board[col+16]=18;                               // 2nd line: black P-
      board[col+96]=9;                                // 7th line: white P+
   }
}


//------------------------------------------------------------
int pieceToVal(char p) {
   // {r,n,b,q,k,+,R,N,B,Q,K,*} => { (+16:) 6,3,5,7,4,2,  (+8:) 6,3,5,7,4,1 }
   // psymbol[]= ".?+nkbrq?*?NKBRQ"
   // 16=white, 8=black

   // default=0, for all the rest, e.g.:
   // if(p=='.') return 0;
   // if(p==' ') return 0;

   if(p=='r') return 6+16;
   if(p=='n') return 3+16;
   if(p=='b') return 5+16;
   if(p=='q') return 7+16;
   if(p=='k') return 4+16;
   if(p=='+') return 18;  // black P-

   if(p=='R') return 6+8;
   if(p=='N') return 3+8;
   if(p=='B') return 5+8;
   if(p=='Q') return 7+8;
   if(p=='K') return 4+8;
   if(p=='*') return 9;   // white P+

   //default:
   return 0;

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
void hashTblInit() {
   int h=1035;
   while(h-- > bchk) T[h]=rand()>>9;              // board check: bchk=136=0x88 board system
}


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

int BLACKSIDECOL = BLACK;
int WHITESIDECOL = LIGHTBLUE;

void Piece2Sqr(const unsigned char *bitarr, int side, int sqr) {
   int x, y, color;
   int xoffs=224, yoffs=12, frx=19, fry=-5; // piece offset, boardframe offset
   int BLACKSIDECOL = BLACK;
   int WHITESIDECOL = LIGHTBLUE;
   x=sqr%8;
   y=sqr/8;
   x=(x)*30+xoffs;
   y=(y)*30+yoffs;

   if(side==0) color=BLACKSIDECOL;
   else if(side==1) color=WHITESIDECOL;

   display.drawBitmap( x, y, bitarr, 24, 24, color);
}



int  GetSqrColor(int sqr) {
   return ( 1+sqr+(sqr/8) )%2;
}



void Blank2Sqr(int sqr, int col) {
   int x, y;
   int xoffs=224, yoffs=12, frx=-4, fry=-3; // piece offset, boardframe offset

   x=sqr%8;
   y=sqr/8;
   x=(x)*30+xoffs+frx;
   y=(y)*30+yoffs+fry;

   if(col==0) sqrBlack.draw(display, x,y);
   else if(col==1) sqrWhite.draw(display, x,y);
}


void Chess() {
   bool CHESSACTIVE = true;


   //  read UI buttons
   read_ADS0();

   //------------------------------------------------------------
   // GUI Board settings

   int sqr, col;
   int xoffs=224, yoffs=12, frx=19, fry=-5; // piece offset, boardframe offset

   ImageReturnCode stat; // Status from image-reading functions

   stat = reader.loadBMP("/chess/sqrWhite30.bmp", sqrWhite);
   stat = reader.loadBMP("/chess/sqrBlack30.bmp", sqrBlack);

   //------------------------------------------------------------
   // GUI test

   display.fillScreen(BLACK);
   // draw all Buttons
   drawAllTSbuttons();

   THREADSUSPENDED=1;
   display_mutex.lock();
   display.fillScreen(BLACK);
   ShowImageFileBmp("/chess/chessboard270.bmp", xoffs-frx, fry);

   display_mutex.unlock();
   THREADSUSPENDED=0;

   Player=0;
   //display.drawBitmap( 0*30+xoffs, 0*30+yoffs,  rook24, 24, 24, BLACKSIDECOL);
   Piece2Sqr( rook24, Player, 0);
   Piece2Sqr( rook24, Player, 7);
   Piece2Sqr( knight24, Player, 1);
   Piece2Sqr( knight24, Player, 6);
   Piece2Sqr( bishop24, Player, 2);
   Piece2Sqr( bishop24, Player, 5);
   Piece2Sqr( queen24, Player, 3);
   Piece2Sqr( king24, Player, 4);
   for(int p=0; p<8; p++) {
      Piece2Sqr( pawn24, Player, 8+p);
   }

   Player=1;
   //display.drawBitmap( 0*30+xoffs, 0*30+yoffs,  rook24, 24, 24, BLACKSIDECOL);
   Piece2Sqr( rook24, Player, 56+0);
   Piece2Sqr( rook24, Player, 56+7);
   Piece2Sqr( knight24, Player, 56+1);
   Piece2Sqr( knight24, Player, 56+6);
   Piece2Sqr( bishop24, Player, 56+2);
   Piece2Sqr( bishop24, Player, 56+5);
   Piece2Sqr( queen24, Player, 56+3);
   Piece2Sqr( king24, Player, 56+4);
   for(int p=0; p<8; p++) {
      Piece2Sqr( pawn24, Player, 48+p);
   }


   //------------------------------------------------------------
   // getting serious....
   //------------------------------------------------------------
   // Serial Chess Game Interface
   //------------------------------------------------------------

   int32_t       score=0, i;
   int16_t       oldto, oldEPSQ;
   char          sbuf[50], sbuf2[50];
   char          cstring[20];
   signed char   oboard[129], spiece;


   K=8;
   while(K--)
   {
      board[K]=(board[K+112]=stdBaseLn[K]+8)+8;
      board[K+16]=18;
      board[K+96]=9;                               /* initial board setup*/
      L=8;
      while(L--)board[16*L+K+8]=(K-4)*(K-4)+(L-3.5)*(L-3.5);     /* center-pts table   */
   }                                                             /*(in unused half board[])*/
   N=1035;
   while(N-->bchk)T[N]=rand()>>9;

   int CHESSFIGHT = 1;

   while(CHESSACTIVE && CHESSFIGHT>0) {
      read_ADS0();
      if (btn00==1) {
         display.setTextColor(RED);
         display.setCursor( 100, 300 ); display.print("terminated");
         delay(500);
         CHESSACTIVE=false;
         btn00=1;
         return;
      }

      N=-1;
      display.setTextColor(WHITE);
      Serial.print("\n");
      display.fillRect(0,40, 200,240, BLACK);

      if(score==15) {
         display.fillRect(0, 0, 200, 40, BLACK);
         display.setCursor( 0, 0 );
         display.print("ILLEGAL MOVE!");
      }

      display.setCursor( 0, 40 );
      sprintf(sbuf,"  A B C D E F G H \n  --------------- \n");
      Serial.print(sbuf);
      display.print(sbuf);
      while(++N<121) {                                            /* print board */
         if(N&8 && (N+7!=0) ) {  // 1...8 board right
            sprintf(sbuf," %1d \n", 1+((120-N)>>4));
            Serial.print(sbuf);
            display.print("\n");  // skip this
            N+=7;
         }
         else {
            if(N%8==0) {     // 1...8 board left
               sprintf(sbuf, "%1d", 1+((120-N)>>4));
               Serial.print(sbuf);
               display.print(sbuf);
            }
            sprintf(sbuf," %c", psymbol[board[N]&15]);
            Serial.print(sbuf);
            display.print(sbuf);
         }
      }

      sprintf(sbuf,"  --------------- \n  A B C D E F G H ");
      Serial.print(sbuf);
      display.print(sbuf);

      if(Side==16) sprintf(sbuf,"\n> WHITE: ");  else sprintf(sbuf,"\n>  BLACK:  ");
      Serial.print(sbuf);
      display.print(sbuf);

      i = 0;
      strcpy(cstring,"");
      do   {
         while (Serial.available()==0);
         cstring[i] = Serial.read();
         if(cstring[i]==13) {
            cstring[i]=0;
            break;
         }
         else i++;
      } while(i < 10);

      if(cstring[0]=='Q' && strlen(cstring)==1 ) {
         Serial.print("\n GAME QUIT \n");
         display.print("\n GAME QUIT \n");
         CHESSFIGHT = 0;
      }
      else {
         K=INF;

         if(cstring[0]!=0) {                                   /* parse entered move */
            K= cstring[0]-16*cstring[1]+799;
            L= cstring[2]-16*cstring[3]+799;
         }
         /*
            Serial.print("\n DEBUG cstring : "); Serial.print(cstring);
            sprintf(sbuf,"\n DEBUG K: %d  \n DEBUG L: %d \n",  K, L);
            Serial.print(sbuf);
         */
         memcpy(oboard, board, sizeof(board));
         oldto=mto;
         oldEPSQ=Rootep;

         display.setCursor(0,280);
         score=Minimax(-INF, INF, RootEval, Rootep, 1, 3);       /* think or check & do*/
         display.print(" ready " );

         if(score!=15) {
            rmvP=S;
            if(oboard[mto])   rmvP=mto;
            if(mto==oldEPSQ)  rmvP=oldto;

            spiece=psymbol[board[mto]&15];
            if(spiece=='*' || spiece=='+') spiece=' ';
            sprintf(sbuf,"moved >> %c %c%c", spiece,'a'+(mfrom&7),'8'-(mfrom>>4) );

            if(oboard[mto]) strcat(sbuf," X ");
            else strcat(sbuf,"-");

            sprintf(sbuf2,"%c%c ", 'a'+(mto&7),'8'-(mto>>4&7));
            strcat(sbuf, sbuf2);
            Serial.print("\n\n"); Serial.print(sbuf);
            display.fillRect(0,0, 200,40, BLACK);
            display.setCursor( 0, 0 );
            display.print(sbuf);


            sprintf(sbuf, " \nDEBUG: %d to %d  \n", mfrom, mto);
            Serial.print(sbuf);
            //display.print(sbuf);
            sprintf(sbuf,"  EPsq: %c%c (%3d)\n",
                    'a'+(Rootep&7), '8'-(Rootep>>4&7), Rootep);
            Serial.print(sbuf);
            //display.print(sbuf);
            sprintf(sbuf,"  rmvP: %c%c (%3d)\n\n",
                    'a'+(rmvP&7), '8'-(rmvP>>4&7), rmvP);
            Serial.print(sbuf);
            //display.print(sbuf);
         }
         else {
            Serial.print("\n\nILLEGAL!\n");
         }
      }
   }
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

      display.fillScreen(BLACK);
      ls(filelist, filecount, selectfilenr);
      markPos(cursorfilenr, cursorfilenr);
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
         display.setTextSize(1);

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
         ls(filelist, filecount, selectfilenr);
         markPos(cursorfilenr, cursorfilenr);
      }
   } // if(!strEndsWith(...))

}


bool Explorer() {

   // default cursorfilenr=0;

   // btn down 5
   if (btn00==5
         &&  cursorfilenr <= filecount) {
      if (cursorfilenr < filecount-1) {
         markPos(cursorfilenr, cursorfilenr+1);
         cursorfilenr++;
      }
   }

   // btn up 2
   else if (btn00==2
            && cursorfilenr >= 0) {
      markPos(cursorfilenr, cursorfilenr-1);
      if (cursorfilenr >= 0) cursorfilenr--;
   }

   if(cursorfilenr>=0) {
      //if (MENULEVEL==1) fileMarked=filelist[cursorfilenr];       else
      fileMarked="";
   }

   // btn right 4
   if (btn00==4
         && cursorfilenr > 0) {
      if(selectfilenr==cursorfilenr) {
         // selectfilenr= -1;      fileSelected= "";
         ExplorerExecFunc();
      }
      //ls(filelist, filecount, selectfilenr);
      else if(selectfilenr!=cursorfilenr) {
         selectfilenr=cursorfilenr;
         fileSelected=filelist[selectfilenr];
      }
      ls(filelist, filecount, selectfilenr);

   }

   // btn left 3
   else if (btn00==3) {
      if(selectfilenr==cursorfilenr) {
         // ExplorerExecFunc();
         selectfilenr= -1;   // unselect
         fileSelected= "";
      } // if(selectfilenr==cursorfilenr)
      ls(filelist, filecount, selectfilenr);

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
         display.println("     (Top/ESC)     "); display.setTextColor(BLUE);   display.println();
         display.println("      (2: Up)      "); display.setTextColor(LIME);   display.println();
         display.print  (" (Le/OK) ");           display.setTextColor(RED);
         display.println(          " (Ri/tag)"); display.setTextColor(YELLOW); display.println();
         display.println("      (5: Dn)      ");
         display.println(); display.setTextColor(WHITE);
         display.println();
         display.println(" press TOP Btn to proceed");
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
      display.setTextSize(1);
      if(newMenuLevel) {
         display.fillScreen(COLOR_BGND);
         readDirectory(SdPath, 0);
         ls(filelist, filecount, selectfilenr);
         markPos(cursorfilenr, cursorfilenr);
         newMenuLevel=false;
      }
      else if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL = 0;
            newMenuLevel=true;
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
         drawAllTSbuttons();
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
   String StrBuf;

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
   char cbuf[4]="   ";
   TSbutton0.initButton(&display, display.width()-30, 20,  60,28,  CYAN, BLUE, YELLOW, "ESC", 2);
   cbuf[0] = '2'; cbuf[2] = 24;
   TSbutton1.initButton(&display, display.width()-30, 84,  60,28,  CYAN, BLUE, YELLOW, cbuf,  2);
   cbuf[0] = '3'; cbuf[2] = 27;
   TSbutton2.initButton(&display, display.width()-30,148,  60,28,  CYAN, BLUE, YELLOW, cbuf, 2);
   cbuf[0] = '4'; cbuf[2] = 26;
   TSbutton3.initButton(&display, display.width()-30,212,  60,28,  CYAN, BLUE, YELLOW, cbuf, 2);
   cbuf[0] = '5'; cbuf[2] = 25;
   TSbutton4.initButton(&display, display.width()-30,276,  60,28,  CYAN, BLUE, YELLOW, cbuf, 2);

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
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
   Serial.println();
   Serial.println("try time Server");
   display.setTextColor(YELLOW);
   display.setCursor(0, tftline);
   display.print("try time Server");
   unsigned start = millis();
   while (!time(nullptr))   {
      delay(200);
      Serial.print(".");
      display.print(".");
      delay(200);
   }
   tftline+=15;

   delay(10);
   Serial.print("  Time: " + StrBuf);
   currenttime = time(nullptr);
   StrBuf=ctime(&currenttime);
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("Time: " + StrBuf);
   tftline+=15;
   delay(1);

   // DEBUG
   // while(1) delay(100);


   //----------------------------------------
   // Start the WiFiServer -> website
   wifiserver.begin();
   StrBuf="WiFiServer started, port " + (String)WIFIPORT;
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
   randomSeed( millis() + analogRead(A0) );


   //---------------------------------------------------------
   // RTOS std::thread
   //thread_1 = new std::thread(blinker_loop);

   delay(10);
   Serial.println();
   Serial.println("std::thread init done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("std::thread init done!");
   tftline+=15;
   delay(10);

   
   //---------------------------------------------------------
   // end of setup
   Serial.println();
   Serial.println("setup(): done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("setup(): done!");
   tftline+=15;
   delay(2000);

   // DEBUG
   // while(1) delay(100);

   display.fillScreen(COLOR_BGND);
   // drawAllTSbuttons(); // for debug <<<<<<<<<<

}


//=====================================================================
//=====================================================================
void loop() {
   char str1[128], str2[128];

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
      Serial.print("read MPU6050: ");
   }
   read_MPU6050(fyaw, fpitch, froll);
   pitch = (int16_t)fpitch;
   roll = -(int16_t)froll;
   delay(2);    // 3

   // ads1115 readings
   read_ADS0();

   // debug
   if (DEBUG) {
      sprintf(str1, "%-4d %-4d y=%-4d x=%-4d p=%-4d r=%-4d",
              adc00, adc01, adc02, adc03, pitch, roll);
      Serial.print((String)str1); Serial.println();

      display.fillRect( 0, display.height()-14, display.width()-13, 14, BLACK);
      display.setTextColor(RED);
      display.setTextSize(2);
      display.setFont();
      display.setCursor(0, display.height()-14);
      display.print((String)str1);
   }
   delay(20);
   
   showAppsMenu();  
   // debug
   uint8_t DEBUGSELECTFILE=0;
   if(DEBUGSELECTFILE) {
      display.fillRect( 0, display.height()-30, display.width(), 16, BLACK);
      display.setCursor(0, display.height()-30);
      display.setTextColor(YELLOW);
      display.print(fileSelected);
   }
   delay(30);

}

/*
   references:
   https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/pinouts

   https://github.com/TKJElectronics/KalmanFilter
   https://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/
   https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/SimpleWiFiServer/SimpleWiFiServer.ino
   https://charbase.com/block/emoticons
   https://github.com/espressif/arduino-esp32/issues/863#issuecomment-575923300


*/
