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

// ver 1.9

// change log:
// 1.9  TS btn 
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

#define delay(t) std::this_thread::sleep_for(std::chrono::milliseconds(t))

std::thread *thread_1;
std::thread *thread_2;
std::thread *thread_3;

volatile static int8_t THREADRUN=1;

//=====================================================================
#include <TFT_HX8357.h>
/*
       #include "Adafruit_GFX.h"
       #include "Adafruit_HX8357.h"
       #include <color16bit.h>  // 16 bit color defs
       #include <Fonts/FreeSans9pt7b.h>
       #include <Fonts/FreeMono12pt7b.h>
       #include <Fonts/FreeMono9pt7b.h>
       Adafruit_HX8357_ini(orientation);
*/

// SD
File SD_File;

static int  MENULEVEL=0;
int COLOR_BGND = BLACK;


//=====================================================================
#include <Wire.h>
#include <ardustdio.h>
#include <arduarray.h>
#include <stringEx.h>


static bool     LED_pin_STATE=false;
int LED_pin  =  LED_BUILTIN;
int LED1pin  =  LED_BUILTIN;
uint8_t OUT1 =  LOW;


//---------------------------------------------------------------------
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads0;

static uint16_t adc00, adc01, adc02, adc03, adcraw;
static uint16_t btn00=0, btn01=0;

void read_ADS0() {
   adcraw=adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
   adc00 = mapConstrain(adc00);
   btn00 = btnstate00(adc00);
   delay(5);

   adc01 = ads0.readADC_SingleEnded(1);  // ADS1115 port A1
   adc01 = mapConstrain(adc01);  // 4Btn pad raw
   delay(5);

   adc02 = ads0.readADC_SingleEnded(2);  // ADS1115 port A2
   adc02 = mapConstrain(adc02);  // Poti
   delay(5);

   adc03 = ads0.readADC_SingleEnded(3);  // ADS1115 port A3
   adc03 = mapConstrain(adc03);  // Poti
   delay(5);

}

//---------------------------------------------------------------------
// MPU6050
#include "data\MPU6050Kalman.h"
// MPU6050dmp6noIRQ
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"


int8_t   yaw, pitch, roll;
float    fyaw, fpitch, froll;
int16_t  accx, accy, accz;


//=====================================================================
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
   // in <arduArray.h>
   int32_t  mapConstrain
   (int32_t val, int valmin=0, int valmax=17600, int newmin=0, int newmax=1023);
*/
//-------------------------------------------

//=============================================================
// analog 5-Button Pad (10bit):
// 0 NONE=900-1022
//          1 esc=330-360
//          2 up= 150-180
// 3 left=20-40     4 right=70-100
//          5 dn=1023-1024 // ESP32 port bug
//
//=============================================================
int32_t btnstate00(int val) {
   if (isInRange( adc00,  900, 1021 ) ) return 0; // NONE
   if (isInRange( adc00,  310,  360 ) ) return 1; // ESC
   if (isInRange( adc00,  140,  180 ) ) return 2; // up
   if (isInRange( adc00,   20,   40 ) ) return 3; // left
   if (isInRange( adc00,   70,  100 ) ) return 4; // right
   if (isInRange( adc00, 1022, 1025 ) ) return 5; // down
   if (isInRange( adc00, 0, 1 ) )       return 5; // down
}


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






//=====================================================================

void LED_writeScreen(int LED_pin_STATE) {
   //digitalWrite(LED_pin, LED_pin_STATE);
   if(LED_pin_STATE)
      display.fillCircle(display.width()-6, display.height()-6,6, LIGHTYELLOW);
   else display.fillCircle(display.width()-6, display.height()-6,6, DARKBLUE);
}

//=====================================================================

Adafruit_GFX_Button TSbutton0, TSbutton1, TSbutton2, TSbutton3, TSbutton4;
//---------------------------------------------------------------------

void drawAllTSbuttons() {
   TSbutton0.drawButton();
   TSbutton1.drawButton();
   TSbutton2.drawButton();
   TSbutton3.drawButton();
   TSbutton4.drawButton();
}

//-------------------------------------------

void GetTSbuttons() {
   char str1[30], str2[30];

   //-------------------------------------------
   // Touchscreen readings
   // My new changes
   //-------------------------------------------
   if(DEBUG) {
      Serial.println("touchscreen");
   }

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

   if(DEBUG) {
      Serial.print("X = "); Serial.print(p.x);
      Serial.print("\tY = "); Serial.print(p.y);
      Serial.print("\tPressure = "); Serial.println(p.z);
      Serial.println();
      sprintf(str1, "x=%3d", p.x); sprintf(str2, "y=%3d", p.y);

      //display.setTextColor(WHITE);
      //display.fillRect(display.width()-80,40, 80,40, COLOR_BKGR);  // x1,y1, dx,dy
      //display.setCursor(display.width()-75,40);    display.print(buf);
      //display.setCursor(display.width()-75,60);    display.print(buf1);
   }
   //-------------------------------------------
   // check if a TSbutton was pressed
   //-------------------------------------------
   TSbutton0.press(TSbutton0.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton1.press(TSbutton1.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton2.press(TSbutton2.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton3.press(TSbutton3.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton4.press(TSbutton4.contains(p.y, p.x)); // tell the TSbutton it is pressed

   //-------------------------------------------
   // process TSbutton states+changes
   //-------------------------------------------
   if(DEBUG) {
      Serial.println("buttons");
   }

   if (TSbutton0.justReleased() ) {
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton0.justPressed()) {
      TSbutton0.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton1.justReleased() ) {
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton1.justPressed()) {
      TSbutton1.drawButton(true); // draw invert!
   }

   //-------------------------------------------


   else if (TSbutton2.justReleased() ) {
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton2.justPressed()) {
      TSbutton2.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton3.justReleased() ) {
      drawAllTSbuttons();
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // <<<< new 1.8c clear the 'touched' interrupts
   }
   else if (TSbutton3.justPressed()) {
      TSbutton3.drawButton(true); // draw invert!
   }


   else if (TSbutton4.justReleased() ) {
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

   p.x = p.y =-1;
}



static int    cursorfilenr=-1, selectfilenr=-1;
static String fileMarked="", fileSelected="";

void ls(String *list, int size, int selectnr) {
   Serial.println("print list[]:");
   for (int cnt = 0; cnt < size; cnt++) {
      if (cnt < 100) Serial.print(" ");
      if (cnt < 10)  Serial.print(" ");
      Serial.println((String)cnt+"  "+list[cnt]);

      int COLOR_TEXT;
      if(cnt==selectnr) COLOR_TEXT=YELLOW;
      else COLOR_TEXT=WHITE;
      display.setTextColor(COLOR_TEXT);
      display.setCursor(20+240*(cnt%2), 16*(cnt/2));
      display.print(list[cnt]);
   }
   Serial.println("print list[] done!\n");
}

//-------------------------------------------

void markPos(int old, int cnt) {
   display.fillRect ( 0+240*(old%2), 16*(old/2), 14, 14, COLOR_BGND);
   display.setTextColor(RED);
   display.setCursor( 0+240*(cnt%2), 16*(cnt/2));
   display.print(">");
   display.setTextColor(WHITE);
   delay(200);
}

//===========================================================
// PONG
//===========================================================
#define MAXDISX 480.0
#define MAXDISY 320.0
#define CENTDIX MAXDISX/2.0
#define CENTDIY MAXDISY/2.0
#define CENTRECT (MAXDISY-RECHIGH)/2.0
float BALLRAD = 6.0;
float RECHIGH = 100.0;
float RECWIDX = 5.0;
float RECOFFS = 50.0;

float Ballx=CENTDIX, Bally=CENTDIY, lRposy=CENTRECT, rRposy=CENTRECT;
float oldbx=CENTDIX, oldby=CENTDIY, oldlRy=CENTRECT, oldrRy=CENTRECT;


//-------------------------------------------
void PaintBall(float x, float y, bool bIn=true) {
   display.fillCircle(oldbx, oldby, BALLRAD, COLOR_BGND);
   if(bIn) display.fillCircle(x, y, BALLRAD, LIGHTBLUE);
   else display.fillCircle(x, y, BALLRAD, RED);
   oldbx=x; oldby=y;
}
//-------------------------------------------
void PaintLRec(float y)  {
   display.fillRect(RECOFFS, oldlRy, RECWIDX, RECHIGH, COLOR_BGND);
   display.fillRect(RECOFFS, y, RECWIDX, RECHIGH, WHITE);
   oldlRy=y;
}
//-------------------------------------------
void PaintRRec(float y)  {
   display.fillRect(MAXDISX-RECWIDX-RECOFFS, oldrRy, RECWIDX, RECHIGH, COLOR_BGND);
   display.fillRect(MAXDISX-RECWIDX-RECOFFS, y, RECWIDX, RECHIGH, WHITE);
   oldrRy=y;
}


//-------------------------------------------

int Pong() {
   bool PONGACTIVE=true;
   bool BallOff = false;
   bool side=1, StopMode=false;
   float lRvy=1, rRvy=1, RvMax=1, RvMin=1, dvtemp;     // racket speed params
   float Bvx=1, Bvy=1, BvMin=1, BvMax=1, BvDynMax=1;   // ball speed params
   int ScoreL=0, ScoreR=0;


   display.setTextColor(YELLOW);
   display.setCursor(  0, 100 );   display.print("Pong started");
   display.setTextColor(YELLOW);
   display.setCursor(  0, 80 );   display.print("Quit: press Btn on Buttonpad 2 ");
   delay(500);
   display.fillScreen(COLOR_BGND);

   PaintBall(Ballx, Bally);
   delay(100);

   RvMax=40.0;           // Racket max speed
   RvMin=BALLRAD;        // Racket min speed
   BvMin=8.0;            // ball speed rand min (xy)
   BvMax=1.5*BvMin;      // ball speed rand max (xy)
   BvDynMax=1.5*BvMax;   // ball dynamic speed max (y)
   Bvx=random(BvMin,BvMax);
   Bvy=random(BvMin/2,BvMax);

   while(PONGACTIVE) {
      // Scores
      //display.fillRect(150, 0, 200, 10, COLOR_BGND);
      display.setTextColor(YELLOW);
      display.setCursor(  150, 5 );   display.print(ScoreL);
      display.setCursor(  300, 5 );   display.print(ScoreR);


      // read all Buttons
      read_ADS0();

      // left Racket move
      if(lRvy==0) dvtemp=RvMin;
      else if(lRvy<=3) dvtemp=RvMin;
      else dvtemp = 10;

      if(adc02>600)      lRvy= lRvy -dvtemp;   // Joystick (left)
      else if(adc02<400) lRvy= lRvy +dvtemp;
      else {
         lRvy=0;   // Rack middle=500
      }
      if(lRvy<-RvMax) lRvy=-RvMax;
      else if(lRvy>RvMax) lRvy=RvMax;

      // right Racket move
      if(rRvy==0) dvtemp=RvMin;
      else if(rRvy<=3) dvtemp=BALLRAD;
      else dvtemp = 10;

      if(btn00==2)      rRvy= rRvy -dvtemp;   // Btn Pad (right)
      else if(btn00==5) rRvy= rRvy +dvtemp;
      else {
         rRvy=0;   // Rack middle
      }
      if(rRvy<-RvMax) rRvy=-RvMax;
      else if(rRvy>RvMax) rRvy=RvMax;

      // debug
      /* 
         display.fillRect( 100, MAXDISY-15, 300, 15, COLOR_BGND);
         display.setTextColor(GREEN);
         display.setCursor(  150, MAXDISY-15 );   display.print((int)lRvy);
         display.setCursor(  300, MAXDISY-15 );   display.print((int)rRvy);
      */  
   
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

      if(lRposy>MAXDISY-RECHIGH-1) lRposy=MAXDISY-RECHIGH-1;
      if(lRposy<0) lRposy=0;
      if(rRposy>MAXDISY-RECHIGH-1) rRposy=MAXDISY-RECHIGH-1;
      if(rRposy<0) rRposy=0;

      PaintLRec(lRposy);
      PaintRRec(rRposy);
      delay(1);

      Ballx=Ballx+Bvx; // ball x move vector
      Bally=Bally+Bvy; // ball x move vector

      // bottom/top reflexion
      if(Bally>MAXDISY-BALLRAD-1) {
         Bally=MAXDISY-BALLRAD-1;
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
            Ballx=RECOFFS+RECWIDX+BALLRAD;
            Bvy=Bvy + (lRvy/3);
            Bvx=-Bvx;
         }
         else if(Ballx<=BALLRAD+1)  { // debug: in Game => to off
            BallOff=true;
            ScoreR+=1;
            //Ballx=BALLRAD; Bvx=-Bvx;
         }

         else
            // right Racket
            if( Ballx<MAXDISX-RECOFFS+2*BALLRAD && Ballx>=MAXDISX-RECOFFS-RECWIDX-BALLRAD    
                  && Bally>=rRposy && Bally<=rRposy+RECHIGH )
            {
               Ballx=MAXDISX-RECOFFS-RECWIDX-BALLRAD;
               Bvy=Bvy + (rRvy/3);
               Bvx=-Bvx;
            }
            else if(Ballx>MAXDISX-BALLRAD-1) { // debug: in Game => to off
               BallOff=true;
               ScoreL+=1;
               //Ballx=MAXDISX-BALLRAD-1; Bvx=-Bvx;
            }

      if(Bvy>BvDynMax)  Bvy=BvDynMax;
      if(Bvy<-BvDynMax) Bvy=-BvDynMax;

      if(BallOff) {
         PaintBall(Ballx, Bally, false);
         delay(150);
         PaintBall(Ballx, Bally, true);
         delay(150);
         PaintBall(Ballx, Bally, false);
         delay(150);
         side=!side;
         if(!side) {   // Level++ !!    // increase Score level by -- Racket size
            if(RECHIGH>=3*BALLRAD) RECHIGH-=0.5;
         }
         if( (ScoreL+ScoreR)%10==0 ) {  // increase Score level by ++ ball speed
            if(BvMin<10) BvMin+=0.3;
            if(BvMax<16) BvMax+=0.3;
         }         
         
         Bvx=random(BvMin,BvMax);
         if(!side) Bvx=-Bvx;
         Bvy=random(BvMin/2,BvMax);
         if(random(2)) Bvy=-Bvy;
         BallOff=false;
         Ballx=CENTDIX+100-side*200;
         Bally=CENTDIY;
         lRvy=0;
         rRvy=0;
         
         lRposy=CENTRECT;          rRposy=CENTRECT;
         PaintBall(Ballx, Bally);
         PaintLRec(lRposy);
         PaintRRec(rRposy);
         delay(500);
         display.fillScreen(COLOR_BGND);
      }

      PaintBall(Ballx, Bally);
      delay(1);

      if (adc01>=400) {
         display.setTextColor(RED);
         display.setCursor(  0, 200 ); display.print("Pong terminated");
         delay(500);
         PONGACTIVE=false;
         btn00=1;
      }

      delay(1);
   }
}


//===========================================================
// PONG END
//===========================================================



void showOptionsWindow() {
   static uint32_t currentTime=millis();
   static bool newMenuLevel = true;

   if(MENULEVEL>5) MENULEVEL=0;

   if(MENULEVEL==0) {
      if(newMenuLevel) {
         drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         display.setTextColor(WHITE);
         display.setCursor(  0,  20 );
         display.print("page # ");  display.print(MENULEVEL);
         display.print(" press ESC button to scroll \nthrough menu options!");
         newMenuLevel=false;
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL += 1;
            newMenuLevel = true;
            display.fillScreen(COLOR_BGND);
            drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         }
      }
      return;
   }

   if (MENULEVEL==1) {                     // show menu
      String mItems[6]= { "0 instructions ",
                          "1 options menu (this)",
                          "2 SD file manager",
                          "3 Date + Time",
                          "4 PONG ",
                          "5 (void)"
                        };

      if(newMenuLevel) {
         drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         Serial.println("Menu"); // show menu window
         Serial.println(mItems[0]);
         Serial.println(mItems[1]);
         Serial.println(mItems[2]);
         Serial.println(mItems[3]);
         Serial.println(mItems[4]);

         int COLOR_TEXT = WHITE;
         display.setTextColor(COLOR_TEXT); display.setCursor(  0,  20 );
         display.println("Menu");
         display.setTextColor(COLOR_TEXT); display.setCursor( 40,  40 );
         display.println(mItems[0]);
         display.setTextColor(COLOR_TEXT); display.setCursor( 40,  60 );
         display.println(mItems[1]);
         display.setTextColor(COLOR_TEXT); display.setCursor( 40,  80 );
         display.println(mItems[2]);
         display.setTextColor(COLOR_TEXT); display.setCursor( 40, 100 );
         display.println(mItems[3]);
         display.setTextColor(COLOR_TEXT); display.setCursor( 40, 120 );
         display.println(mItems[4]);
         display.setTextColor(COLOR_TEXT); display.setCursor( 40, 140 );
         display.println(mItems[5]);
         newMenuLevel=false;
      }

      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL += 1;
            newMenuLevel = true;
            display.fillScreen(COLOR_BGND);
            drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         }
      }
      return;
   }


   if (MENULEVEL==2) {                       // SD file menu
      if(newMenuLevel)   {
         // MENULEVEL==2
         readDirectory(SdPath, 0);
         ls(filelist, filecount, selectfilenr);
         markPos(cursorfilenr, cursorfilenr);
         newMenuLevel=false;
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL += 1;
            newMenuLevel=true;
            display.fillScreen(COLOR_BGND);
         }

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
            if (MENULEVEL==1) fileMarked=filelist[cursorfilenr];
            else fileMarked="";
         }

         // btn right 4
         if (btn00==4
               && cursorfilenr >= 0) {
            if(selectfilenr!=cursorfilenr) {
               selectfilenr=cursorfilenr;
               fileSelected=filelist[selectfilenr];
            }
            else if(selectfilenr==cursorfilenr) {
               selectfilenr= -1;
               fileSelected= "";
            }
            ls(filelist, filecount, selectfilenr);
         }

         // btn left 3
         else if (btn00==3) {
            // to do...
         }
      }
      return;
   }


   if (MENULEVEL==3) {
      // show date+time
      if(newMenuLevel) {
         time_refreshDisplay();
         newMenuLevel=false;
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL += 1;
            newMenuLevel=true;
            display.fillScreen(COLOR_BGND);
         }
      }
      return;
   }

   if(MENULEVEL==4) {
      if(newMenuLevel) {
         display.fillScreen(COLOR_BGND);
         display.setTextColor(YELLOW);
         display.setCursor( 100,  60 );
         display.println("PONG");
         newMenuLevel=false;
         delay(500);
         Pong();
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL += 1;
            newMenuLevel=true;
            display.fillScreen(COLOR_BGND);
         }
      }
      return;
   }

   if(MENULEVEL==5) {
      if(newMenuLevel) {
         drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         display.setTextColor(WHITE);
         display.setCursor(  0,  20 );
         display.print("page # ");  display.print(MENULEVEL);
         display.print(" press ESC button to scroll \nthrough menu options!");
         newMenuLevel=false;
      }
      if(!newMenuLevel) {
         if (btn00==1) {
            MENULEVEL += 1;
            newMenuLevel = true;
            display.fillScreen(COLOR_BGND);
            // drawAllTSbuttons(); // enable TS-Buttons! <<<<<<<<<<
         }
      }
      return;
   }
}

//=====================================================================

void time_refreshDisplay() {
   static uint32_t printtimestamp=millis()+800;
   if(millis()-printtimestamp>1000) {
      displayLocalTime();
      printtimestamp=millis();
   }
}



//=====================================================================
void blinker_loop() {
   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio

   while(THREADRUN != 0) {
      LED_writeScreen(LED_pin_STATE);  // virual LED on TFT screen
      LED_pin_STATE=!LED_pin_STATE;
      delay(500);
   }
}

//=====================================================================
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

void handleRoot() {
   //handleClients();
}



//=====================================================================
//=====================================================================
void setup() {
   int tftline=10;
   String StrBuf;

   Serial.begin(230400);
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
   Adafruit_HX8357_ini(3);  // init function in <TFT_HX3857.h>:: TFT, Touch, SD
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
   TSbutton0.initButton(&display, display.width()-30, 20,  60,28,  CYAN, BLUE, YELLOW, "ESC", 2);
   TSbutton1.initButton(&display, display.width()-30, 84,  60,28,  CYAN, BLUE, YELLOW, "1 ^^", 2);
   TSbutton2.initButton(&display, display.width()-30,148,  60,28,  CYAN, BLUE, YELLOW, "2 <-", 2);
   TSbutton3.initButton(&display, display.width()-30,212,  60,28,  CYAN, BLUE, YELLOW, "3 ->", 2);
   TSbutton4.initButton(&display, display.width()-30,276,  60,28,  CYAN, BLUE, YELLOW, "4 vv", 2);

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

            buf1 = SD_File.readStringUntil('\r');
            if( SD_File.available() ) SD_File.readStringUntil('\n');
            if(bufold.startsWith((String)"SSID" ) ) { //myString.startsWith(myString2)
               SSID=buf1;
               Serial.println("SD-SSID value="+SSID+(String)"<");
            }
            if(bufold.startsWith((String)"PASSWORD" ) ) {
               PASSWORD=buf1;
               Serial.println("SD-PASSWORD value="+PASSWORD+(String)"<");
            }
            if(bufold.startsWith((String)"LOCALIP" ) ) {
               StringLOC_IP=buf1;
               Serial.println("SD-LOCALIP value="+StringLOC_IP+(String)"<");
            }
            if(bufold.startsWith((String)"GATEWAY" ) ) {
               StringGW_IP=buf1;
               Serial.println("SD-GATEWAY value="+StringGW_IP+(String)"<");
            }
            if(bufold.startsWith((String)"SUBNET" ) ) {
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
   delay(5000);

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
   delay(1);


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
   delay(5);    // 3

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

   showOptionsWindow();
   delay(10);

   uint8_t DEBUGSELECTFILE=0;
   // debug
   if(DEBUGSELECTFILE) {
      display.fillRect( 0, display.height()-30, display.width(), 16, BLACK);
      display.setCursor(0, display.height()-30);
      display.setTextColor(YELLOW);
      display.print(fileSelected);
   }

   delay(1);
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
