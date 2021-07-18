// ESP32, boxed
// Adafruit_HX8357 touchscreen feat.:
//    Adafruit_STMPE610
//    Adafruit_ImageReader
// ads1115
// PCF8574
// MPU6050
// analog joystick
// analog 5x button pad

// ver 1.0b

// change log:
// 1.0: console GameBoy-like; a,b: ads1115 buttonpads fixed
// 0.9: WiFi  + Webserver + Time tm * timeinfo
// 0.8: WiFi  + Webserver example HelloServer
// 0.7: TFT debug line, TSbutton1-4, MPU6050
// 0.6: SD ls() + file select
// 0.3: ads1115


static bool DEBUG=true;

#include <thread>
#include <freertos/task.h>

std::thread *thread_1;
std::thread *thread_2;

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

static int  TFTMODE=0;
int COLOR_BGND = BLACK;
int COLOR_TEXT = WHITE;


//=====================================================================
#include <Wire.h>
#include <ardustdio.h>
#include <arduarray.h>
#include <stringEx.h>


static bool     LED_pin_STATE=false;
int LED_pin  =  LED_BUILTIN;

//---------------------------------------------------------------------
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads0;

static uint16_t adc00, adc01, adc02, adc03;
static uint16_t btn00=0, btn01=0;

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

const char* ssid = "WLAN-3YA7LD";
const char* password = "18658446694458594657";

IPAddress local_ip(192, 168,  2, 202);
IPAddress gateway (192, 168,  2,  1);
IPAddress subnet  (255, 255, 255, 0);

WebServer server(8008);

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
   if (isInRange( adc00,  330,  360 ) ) return 1; // ESC
   if (isInRange( adc00,  150,  180 ) ) return 2; // up
   if (isInRange( adc00,   20,   40 ) ) return 3; // left
   if (isInRange( adc00,   70,  100 ) ) return 4; // right
   if (isInRange( adc00, 1022, 1025 ) ) return 5; // down
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
   display.setTextColor(COLOR_TEXT);
}



//------------------------------------
void handleRoot() {
   char buffer[80];

   getLocalTime(buffer);

   LED_writeScreen(LED_BUILTIN, 1);
   server.send(200, "text/plain",
               (String)"hello from esp32!\n" + buffer);

   delay(100);
   LED_writeScreen(LED_BUILTIN, 0);
}

//------------------------------------
void handleNotFound() {
   LED_writeScreen(LED_BUILTIN, 1);
   String message = "File Not Found\n\n";
   message += "URI: ";
   message += server.uri();
   message += "\nMethod: ";
   message += (server.method() == HTTP_GET) ? "GET" : "POST";
   message += "\nArguments: ";
   message += server.args();
   message += "\n";
   for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
   }
   server.send(404, "text/plain", message);
   LED_writeScreen(LED_BUILTIN, 0);
}



//=====================================================================

void LED_writeScreen(int LED_pin, int LED_pin_STATE) {
   digitalWrite(LED_pin, LED_pin_STATE);
   if(LED_pin_STATE)
      display.fillCircle(display.width()-6, display.height()-6,6, LIGHTYELLOW);
   else display.fillCircle(display.width()-6, display.height()-6,6, DARKBLUE);
}

//=====================================================================

Adafruit_GFX_Button TSbutton1, TSbutton2, TSbutton3, TSbutton4;
//---------------------------------------------------------------------

void drawAllTSbuttons() {
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

      display.setTextColor(RED);

      //display.fillRect(display.width()-80,40, 80,40, COLOR_BKGR);  // x1,y1, dx,dy
      //display.setCursor(display.width()-75,40);    display.print(buf);
      //display.setCursor(display.width()-75,60);    display.print(buf1);
   }
   //-------------------------------------------
   // check if a TSbutton was pressed
   //-------------------------------------------
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

   if (TSbutton1.justReleased() ) {
      drawAllTSbuttons();
   }
   else if (TSbutton1.justPressed()) {
      TSbutton1.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton2.justReleased() ) {
      drawAllTSbuttons();
   }
   else if (TSbutton2.justPressed()) {
      TSbutton2.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton3.justReleased() ) {
      drawAllTSbuttons();
   }
   else if (TSbutton3.justPressed()) {
      TSbutton3.drawButton(true); // draw invert!
   }


   else if (TSbutton4.justReleased() ) {
      drawAllTSbuttons();
   }
   else if (TSbutton4.justPressed()) {
      TSbutton4.drawButton(true); // draw invert!
   }
   delay(1);  // <<<<<<<<<<<<<<<< new 0.7

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

      if(cnt==selectnr) COLOR_TEXT=YELLOW;
      else COLOR_TEXT=WHITE;
      display.setTextColor(COLOR_TEXT);
      display.setCursor(20+240*(cnt%2), 16*(cnt/2));
      display.println(list[cnt]);
   }
   Serial.println("print list[] done!\n");
}

//-------------------------------------------

void markPos(int old, int cnt) {
   COLOR_TEXT = RED;
   display.fillRect ( 0+240*(old%2), 16*(old/2), 14, 14, COLOR_BGND);
   display.setTextColor(COLOR_TEXT);
   display.setCursor( 0+240*(cnt%2), 16*(cnt/2));
   display.print(">");
   delay(200);
}



//-------------------------------------------

void showOptionsWindow() {
   adc00 = ads0.readADC_SingleEnded(0);  // ADS1115  A0
   adc00 = mapConstrain(adc00);
   btn00 = btnstate00(adc00);
   delay(100);


   Serial.print("adc00=");
   Serial.println(adc00);

   // btn top/esc
   if (btn00==1) {
      TFTMODE += 1;
      if(TFTMODE>=5) TFTMODE=0;
      display.fillScreen(COLOR_BGND);
      drawAllTSbuttons();
      COLOR_TEXT = WHITE;
      display.setTextColor(COLOR_TEXT);

      // load option main windows

      if (TFTMODE==1) {                     // show menu
         String mItems[6]= { "0 ESC ",
                             "1 options menu (this)",
                             "2 SD file manager",
                             "3 Date + Time",
                             "4 (void)",
                             "5 (void)"
                           };

         Serial.println("Menu"); // show menu window
         Serial.println(mItems[0]);
         Serial.println(mItems[1]);
         Serial.println(mItems[2]);
         Serial.println(mItems[3]);
         Serial.println(mItems[4]);
         //display.fillScreen(COLOR_BGND);
         drawAllTSbuttons();
         COLOR_TEXT = WHITE;
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
         delay(10);
      }


      if(TFTMODE==2)  // btn top/esc
      {
         // TFTMODE==2
         readDirectory(SdPath, 0);
         ls(filelist, filecount, selectfilenr);
         markPos(cursorfilenr, cursorfilenr);
      }

      if (TFTMODE==3) {                     // show date+time
         time_refreshDisplay();
      }
   }


   if (TFTMODE==2) {                       // SD file menu
      // btn down 5
      if (btn00==5
            &&  cursorfilenr <= filecount) {
         if (cursorfilenr < filecount-1) {
            markPos(cursorfilenr, cursorfilenr+1);
            cursorfilenr++;
         }
         delay(1);
      }

      // btn up 2
      else if (btn00==2
               && cursorfilenr >= 0) {
         markPos(cursorfilenr, cursorfilenr-1);
         if (cursorfilenr >= 0) cursorfilenr--;
         delay(1);
      }

      if(cursorfilenr>=0) {
         if (TFTMODE==1) fileMarked=filelist[cursorfilenr];
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
         delay(1);
      }

      // btn left 3
      else if (btn00==3) {
         // to do...
      }
   }


   if (TFTMODE==3) {                          // show date+time
      time_refreshDisplay();
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
   while(true) {         
      LED_writeScreen(LED_pin, LED_pin_STATE);  
      LED_pin_STATE=!LED_pin_STATE;
      delay(500);
   }   
}

//=====================================================================
void touchscreen_loop() {
   while(true) {      
      delay(1);
   }   
}



//=====================================================================
//=====================================================================
void setup() {
   int tftline=10;

   Serial.begin(230400);
   delay(1000);
   Serial.println("setup(): Serial started!");

   //LED_pin=LED_BUILTIN;  // default: LED_pin=LED_BUILTIN
   pinMode(LED_pin, OUTPUT);

   Wire.begin();
   Wire.setClock(400000);
   delay(100);





   //---------------------------------------------------------
   Adafruit_HX8357_ini(3);  // init function in <TFT_HX3857.h>:: TFT, Touch, SD
   delay(100);

   COLOR_BGND = BLACK;
   COLOR_TEXT = WHITE;
   display.fillScreen(COLOR_BGND);
   display.setTextColor(WHITE);
   display.setTextSize(2);
   //display.setFont(&FreeMono9pt7b);
   Serial.println("setup(): display setup done!");
   Serial.println();
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("setup(): display setup done!");
   tftline+=15;
   delay(100);

   //---------------------------------------------------------
   //TS buttons
   // TS
   TSbutton1.initButton(&display, display.width()-30, 20,  60,30,  CYAN, BLUE, YELLOW, "Btn1", 2);
   TSbutton2.initButton(&display, display.width()-30,100,  60,30,  CYAN, BLUE, YELLOW, "Btn2", 2);
   TSbutton3.initButton(&display, display.width()-30,180,  60,30,  CYAN, BLUE, YELLOW, "Btn3", 2);
   TSbutton4.initButton(&display, display.width()-30,260,  60,30,  CYAN, BLUE, YELLOW, "Btn4", 2);

   drawAllTSbuttons();

   Serial.println("setup(): ts buttons setup done!");
   Serial.println();
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("setup(): ts buttons setup done!");
   tftline+=15;
   delay(100);

   //---------------------------------------------------------
   // SD
   //SD.begin(); // by TFT lib
   display.setCursor(0, tftline);
   delay(10);
   if( !SDioerr ) {
      Serial.println("SD.begin(SD_CS) failed!");
      Serial.println();
      display.setTextColor(RED);
      display.print("setup(): SD.begin(SD_CS) failed!");
      delay(2000); // delay here
   }
   else {
      SdPath = SD.open("/");
      Serial.println("setup(): SD setup done!\n");
      display.setTextColor(WHITE);
      display.print("setup(): SD setup done!");
   }
   tftline+=15;
   delay(100);


   //---------------------------------------------------------
   //i2c devices

   ads0.begin(0x48);
   Serial.println("setup(): i2c+ads1115 setup done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("setup(): i2c+ads1115 setup done!");
   tftline+=15;
   delay(100);

   init_MPU6050();
   Serial.println("setup(): MPU6050 setup done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("setup(): MPU6050 setup done!");
   tftline+=15;
   delay(100);


   //-----------------------------------------------------
   // connecting to router
   //-----------------------------------------------------
   Serial.print("try WiFi, localIP ");
   Serial.println(local_ip);
   Serial.println();
   display.setTextColor(YELLOW);
   display.setCursor(0, tftline);
   display.print("try WiFi, localIP ");
   display.print(local_ip);
   tftline+=15;

   WiFi.mode(WIFI_STA);
   WiFi.config(local_ip, gateway, subnet, gateway, gateway);
   WiFi.begin(ssid, password);
   delay(100);
   // Wait for connection
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
   }
   delay(500);

   Serial.println();
   Serial.print("  Connected to ");
   Serial.print(ssid);
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("  Connected to ");
   display.print(ssid);
   tftline+=15;
   delay(100);

   Serial.print("  IP address: ");
   Serial.println(WiFi.localIP());
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("  IP address: ");
   display.print(WiFi.localIP());
   tftline+=15;
   delay(100);

   //---------------------------------------------------------
   //init and get the time
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
   delay(100);

   Serial.println();
   Serial.println("\nWaiting for time");
   unsigned start = millis();
   while (!time(nullptr))
   {
      Serial.print(".");
      delay(1000);
   }
   Serial.println("Time...");
   delay(500);
   currenttime = time(nullptr);
   String StrBuf=ctime(&currenttime);
   Serial.print(StrBuf);

   display.setTextColor(YELLOW);
   display.setCursor(0, tftline);
   display.print("  Time started: ");
   display.print(StrBuf);
   display.setTextColor(WHITE);
   tftline+=15;
   delay(100);


   //---------------------------------------------------------
   // WebServer

   server.on("/", handleRoot);
   server.on("/inline", []() {
      server.send(200, "text/plain", "this works as well");
   });
   server.onNotFound(handleNotFound);

   server.begin();
   delay(100);

   Serial.println();
   Serial.println("  HTTP server started!");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline); display.print("  HTTP server started!");
   tftline+=15;
   delay(1);

   //---------------------------------------------------------
   // RTOS std::thread
   thread_1 = new std::thread(blinker_loop);
   thread_2 = new std::thread(touchscreen_loop);
   delay(1);
   Serial.println();
   Serial.println("std::thread init done!\n");
   display.setCursor(0, tftline); display.print("std::thread init done!");
   tftline+=15;
   delay(1);

   //---------------------------------------------------------
   // end of setup
   Serial.println();
   Serial.println("setup(): done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline); display.print("setup(): done!");
   tftline+=15;
   delay(3000);

   display.fillScreen(COLOR_BGND);
   drawAllTSbuttons();

}


//=====================================================================
//=====================================================================
void loop() {
   char str1[128], str2[128];

   // web server
   server.handleClient();

   // time server
   if(DEBUG) {
      //time_refreshDisplay();
   }

   // touch screen buttons
   GetTSbuttons();

   // ads1115 readings
   int adcraw;
   adcraw=adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0: port BUG!!
   adc00 = mapConstrain(adc00);
   btn00 = btnstate00(adc00);
   delay(10);

   adc01 = ads0.readADC_SingleEnded(1);  // ADS1115 port A1
   adc01 = mapConstrain(adc01);  // 4Btn pad raw
   delay(10);

   adc02 = ads0.readADC_SingleEnded(2);  // ADS1115 port A2
   adc02 = mapConstrain(adc02);  // Poti
   delay(10);
   adc03 = ads0.readADC_SingleEnded(3);  // ADS1115 port A3
   adc03 = mapConstrain(adc03);  // Poti
   delay(10);


   // IMU readings
   if(DEBUG) {
      Serial.print("read MPU6050: ");
   }
   read_MPU6050(fyaw, fpitch, froll);
   pitch = (int16_t)fpitch;
   roll = -(int16_t)froll;
   delay(1);    // 3


   // debug
   if (DEBUG) {
      sprintf(str1, "%-4d %-4d y=%-4d x=%-4d p=%-4d r=%-4d",
              adc00, adc01, adc02, adc03, pitch, roll);
      Serial.print((String)str1); Serial.println();

      display.fillRect( 0, display.height()-14, display.width()-13, 14, BLACK);
      COLOR_TEXT = RED;
      display.setTextColor(COLOR_TEXT);
      display.setTextSize(2);
      display.setFont();
      display.setCursor(0, display.height()-14);
      display.print((String)str1);
   }

   showOptionsWindow();

   // debug
   if (DEBUG) {
      display.fillRect( 0, display.height()-30, display.width(), 16, BLACK);
      //COLOR_TEXT = WHITE;
      //display.setTextColor(COLOR_TEXT);
      display.setCursor(0, display.height()-30);
      //display.print(fileMarked);
      COLOR_TEXT = YELLOW;
      display.setTextColor(COLOR_TEXT);
      //display.setCursor(20+240, display.height()-30);
      display.print(fileSelected);
   }


   delay(10);
}

/*
   references:
   https://github.com/TKJElectronics/KalmanFilter
   https://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/

*/
