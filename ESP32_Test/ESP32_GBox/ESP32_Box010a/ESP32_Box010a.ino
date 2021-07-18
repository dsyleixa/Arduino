// ESP32, boxed
// Adafruit_HX8357 touchscreen feat.:
//    Adafruit_STMPE610
//    Adafruit_ImageReader
// ads1115
// PCF8574
// MPU6050
// analog joystick
// analog 5x button pad

// ver 1.0a

// change log:
// 1.0: console GameBoy-like
// 0.9: WiFi  + Webserver + Time tm * timeinfo
// 0.8: WiFi  + Webserver example HelloServer
// 0.7: TFT debug line, TSbutton1-4, MPU6050
// 0.6: SD ls() + file select
// 0.3: ads1115


static bool DEBUG=true;

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
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads0(0x48);

static uint16_t adc00, adc01, adc02, adc03;

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



int32_t  mapConstrain(int32_t val, int valmin=0, int valmax=17550, int newmin=0, int newmax=1023) {  
  
  val=map(val, valmin, valmax,  newmin, newmax);
  val=constrain(val, newmin, newmax);
  
  return (int32_t)val;
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


//=====================================================================
// analog 5-Button Pad (10bit):
//           up=0-2
// left=88-89     right=32-33
//         dn=166-168
//       bottom=350-352
//=====================================================================


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
      delay(10);
      adc00 = mapConstrain(adc00);

   Serial.print("adc00=");
   Serial.println(adc00);
   if (isInRange( adc00, 330, 360 ) ) {     // btn top/esc
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


      if(TFTMODE==2)
      {
         // TFTMODE==2, btn dn
         readDirectory(SdPath, 0);
         ls(filelist, filecount, selectfilenr);
         markPos(cursorfilenr, cursorfilenr);
      }

      if (TFTMODE==3) {                     // show date+time
         time_refreshDisplay();
      }

      adc00 = ads0.readADC_SingleEnded(0);  // ADS1115  A0
      delay(10);
      adc00 = mapConstrain(adc00);
   }


   if (TFTMODE==2) {                       // SD file menu
      // btn down
      if (isInRange( adc00, 0, 20 )  
            &&  cursorfilenr <= filecount) {
         if (cursorfilenr < filecount-1) {
            markPos(cursorfilenr, cursorfilenr+1);
            cursorfilenr++;
         }
         delay(1);
         adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
         delay(10);
         adc00 = mapConstrain(adc00);
      }

      // btn up
      else if (isInRange( adc00, 150, 180 )  
               && cursorfilenr >= 0) {
         markPos(cursorfilenr, cursorfilenr-1);
         if (cursorfilenr >= 0) cursorfilenr--;
         delay(1);
         adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
         delay(10);
         adc00 = mapConstrain(adc00);
      }

      if(cursorfilenr>=0) {
         if (TFTMODE==1) fileMarked=filelist[cursorfilenr];
         else fileMarked="";
      }

      // btn right
      if (isInRange( adc00, 70, 100 )  
            && cursorfilenr >= 0) {
         if(selectfilenr!=cursorfilenr) {
            selectfilenr=cursorfilenr;
            fileSelected=filelist[selectfilenr];
         }
         else if(selectfilenr==cursorfilenr) {
            selectfilenr= -1;
            fileSelected= "";
         }
         delay(1);
         adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
         delay(10);
         adc00 = mapConstrain(adc00);
         ls(filelist, filecount, selectfilenr);
      }
   }


   if (TFTMODE==3) {                          // show date+time
      time_refreshDisplay();
   }

}

//=====================================================================

void LED_blink() {
   static uint32_t blinktimestamp=millis();
   if(millis()-blinktimestamp>1000) {
      LED_pin_STATE=!LED_pin_STATE;
      LED_writeScreen(LED_pin, LED_pin_STATE);
      blinktimestamp=millis();
   }
}

//---------------------------------------------------------

void time_refreshDisplay() {
   static uint32_t printtimestamp=millis()+800;
   if(millis()-printtimestamp>1000) {
      displayLocalTime();
      printtimestamp=millis();
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



   //---------------------------------------------------------
   Adafruit_HX8357_ini(3);  // init function in lib <display_HX3857.h>
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
   SD.begin();
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
   //i2c Wire

   Wire.begin();
   Wire.setClock(400000);
   delay(100);

   ads0.begin();
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
   adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
   delay(5);
   adc01 = ads0.readADC_SingleEnded(1);  // ADS1115 port A1
   delay(5);
   adc02 = ads0.readADC_SingleEnded(2);  // ADS1115 port A2
   delay(5);
   adc03 = ads0.readADC_SingleEnded(3);  // ADS1115 port A3
   delay(1);

   adc00 = mapConstrain(adc00);  // 5Btn pad
   adc01 = mapConstrain(adc01);  // 4Btn pad
   adc02 = mapConstrain(adc02);  // Poti
   adc03 = mapConstrain(adc03);  // Poti


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
      sprintf(str1, "0=%-4d %-4d %-4d %-4d   p=%-4d r=%-4d",
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
 * references: 
 * https://github.com/TKJElectronics/KalmanFilter
 * https://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/ 
 * 
*/
