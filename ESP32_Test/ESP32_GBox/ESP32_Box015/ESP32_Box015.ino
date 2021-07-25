// ESP32, boxed
// Adafruit_HX8357 touchscreen feat.:
//    Adafruit_STMPE610
//    Adafruit_ImageReader
//    SD
// ads1115
// PCF8574
// MPU6050
// analog joystick
// analog 5x button pad

// ver 1.5

// change log:
// 1.5: variable IPs, read config from SD
// 1.4: MT fixes, msleep(), SD config.txt
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

#include <thread>
#include <freertos/task.h>

#define msleep(t) std::this_thread::sleep_for(std::chrono::milliseconds(t))

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
int LED1pin  =  LED_BUILTIN;
uint8_t OUT1 =  LOW;


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

String SSID     = "WLAN-";
String PASSWORD = "1865-4657";
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






//=====================================================================

void LED_writeScreen(int LED_pin_STATE) {
   //digitalWrite(LED_pin, LED_pin_STATE);
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

      //display.setTextColor(WHITE);
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
   msleep(1);  // <<<<<<<<<<<<<<<< new 0.7

   //-------------------------------------------

   if (ts.touched()) {
      // Empty the touchscreen buffer
      while ( ! ts.bufferEmpty() ) {
         p = ts.getPoint();
      }
      ts.writeRegister8(STMPE_INT_STA, 0xFF);    // clear the 'touched' interrupts
      msleep(2);
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
      display.print(list[cnt]);
   }
   Serial.println("print list[] done!\n");
}

//-------------------------------------------

void markPos(int old, int cnt) {
   display.fillRect ( 0+240*(old%2), 16*(old/2), 14, 14, COLOR_BGND);
   display.setTextColor(COLOR_TEXT);
   display.setCursor( 0+240*(cnt%2), 16*(cnt/2));
   COLOR_TEXT = RED;
   display.print(">");
   COLOR_TEXT = WHITE;
   msleep(200);
}



//-------------------------------------------

void showOptionsWindow() {

   // btn top/esc
   if(TFTMODE==0 || TFTMODE==4 || TFTMODE==5) {
      COLOR_TEXT = WHITE;
      //display.fillScreen(COLOR_BGND);
      display.setTextColor(COLOR_TEXT);
      display.setCursor(  0,  20 );
      display.print("page # ");  display.print(TFTMODE);
      display.print(" press ESC button to scroll \nthrough menu options!");
      msleep(1);
   }
   if (btn00==1) {
      TFTMODE += 1;
      if(TFTMODE>5) TFTMODE=0;
      display.fillScreen(COLOR_BGND);
      drawAllTSbuttons();

      // load option main windows

      if (TFTMODE==1) {                     // show menu
         String mItems[6]= { "0 instructions ",
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
         msleep(1);
      }


      if(TFTMODE==2)  // btn top/esc
      {
         // TFTMODE==2
         readDirectory(SdPath, 0);
         ls(filelist, filecount, selectfilenr);
         markPos(cursorfilenr, cursorfilenr);
         msleep(1);
      }

      if (TFTMODE==3) {                     // show date+time
         time_refreshDisplay();
         msleep(1);
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
         msleep(1);
      }

      // btn up 2
      else if (btn00==2
               && cursorfilenr >= 0) {
         markPos(cursorfilenr, cursorfilenr-1);
         if (cursorfilenr >= 0) cursorfilenr--;
         msleep(1);
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
         msleep(1);
      }

      // btn left 3
      else if (btn00==3) {
         // to do...
      }

   }


   if (TFTMODE==3) {
      // show date+time
      time_refreshDisplay();
      msleep(1);
   }

   msleep(1);

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
      msleep(500);
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

                  msleep(1);

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
            msleep(1);

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
      msleep(1);
      Serial.println("Client Disconnected.");
   }
   msleep(1);

}


//------------------------------------
void handleNotFound() {

   msleep(1);
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
   msleep(1000);
   Serial.println("Serial started!");

   //LED_pin=LED_BUILTIN;  // default: LED_pin=LED_BUILTIN
   pinMode(LED_pin, OUTPUT);
   pinMode(LED1pin, OUTPUT);

   Wire.begin();
   Wire.setClock(400000);
   msleep(10);


   //---------------------------------------------------------
   Adafruit_HX8357_ini(3);  // init function in <TFT_HX3857.h>:: TFT, Touch, SD
   msleep(10);

   COLOR_BGND = BLACK;
   COLOR_TEXT = WHITE;
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
   msleep(10);

   //---------------------------------------------------------
   // TS buttons
   //
   TSbutton1.initButton(&display, display.width()-30, 20,  60,30,  CYAN, BLUE, YELLOW, "Btn1", 2);
   TSbutton2.initButton(&display, display.width()-30,100,  60,30,  CYAN, BLUE, YELLOW, "Btn2", 2);
   TSbutton3.initButton(&display, display.width()-30,180,  60,30,  CYAN, BLUE, YELLOW, "Btn3", 2);
   TSbutton4.initButton(&display, display.width()-30,260,  60,30,  CYAN, BLUE, YELLOW, "Btn4", 2);

   drawAllTSbuttons();

   Serial.println("ts buttons setup done!");
   Serial.println();
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("ts buttons setup done!");
   tftline+=15;
   msleep(10);

   //---------------------------------------------------------
   // SD
   // SD.begin(); // by TFT lib
   display.setCursor(0, tftline);
   msleep(10);
   if( !SDioerr ) {
      Serial.println("SD.begin(SD_CS) failed!");
      Serial.println();
      display.setTextColor(RED);
      display.print("SD.begin(SD_CS) failed!");
      display.setTextColor(WHITE);
      msleep(2000); // delay here
   }
   else {
      SdPath = SD.open("/");
      Serial.println("SD setup done!\n");
      display.setTextColor(WHITE);
      display.print("SD setup done!");

      SD_File = SD.open("/config.txt");
      if (!SD_File) {
         Serial.print("The text file cannot be opened");
      }
      else {
         String buf1="", bufold="";
         Serial.println("reading SD config.txt...");
         while (SD_File.available()) {
            
            buf1 = SD_File.readStringUntil('\r');
            SD_File.readStringUntil('\n');
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
   msleep(10);


   //---------------------------------------------------------
   //i2c devices

   ads0.begin(0x48);
   Serial.println("i2c+ads1115 setup done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("i2c+ads1115 setup done!");
   tftline+=15;
   msleep(10);

   init_MPU6050();
   msleep(100);
   Serial.println("MPU6050 setup done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("MPU6050 setup done!");
   tftline+=15;
   msleep(10);


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

   Serial.println("try WiFi, IP " + LOCALIP.toString());
   display.setTextColor(YELLOW);
   display.setCursor(0, tftline);
   display.print("try WiFi");
   msleep(100);

   WiFi.mode(WIFI_STA);
   WiFi.config(LOCALIP, GATEWAY, SUBNET, GATEWAY, GATEWAY);
   char ssid[64], pwd[64];
   strcpy(ssid, SSID.c_str() ); 
   strcpy(pwd, PASSWORD.c_str() ); 
   WiFi.begin( ssid, pwd );
   // Wait for connection
   while (WiFi.status() != WL_CONNECTED) {
      msleep(500);
      Serial.print(".");
      display.print(".");
   }
   tftline+=15;
   msleep(500);
   StrBuf="  WiFi SSID " + (String)SSID;
   Serial.println();
   Serial.println(StrBuf);
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print(StrBuf);
   tftline+=15;
   msleep(100);

   StrBuf="  IP address: " + WiFi.localIP().toString();
   Serial.println(StrBuf);
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print(StrBuf);
   tftline+=15;
   msleep(100);

   //---------------------------------------------------------
   //init and get the time
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
   Serial.println();
   Serial.println("try time Server");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("try time Server");
   unsigned start = millis();
   while (!time(nullptr))
   {
      msleep(500);
      Serial.print(".");
      display.print(".");
      msleep(500);
   }
   tftline+=15;
   Serial.println("Time: " + StrBuf);
   currenttime = time(nullptr);
   StrBuf=ctime(&currenttime);
   display.setTextColor(YELLOW);
   display.setCursor(0, tftline);
   display.print("Time: " + StrBuf);
   display.setTextColor(WHITE);
   tftline+=15;
   msleep(100);

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
   msleep(100);


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
   msleep(100);
   Serial.println();
   StrBuf="WebServer started, port " + (String)WEBPORT;
   Serial.println(StrBuf);
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print(StrBuf);
   tftline+=15;
   msleep(100);

   // DEBUG
   //while(1) delay(100);


   //---------------------------------------------------------
   // RTOS std::thread
   thread_1 = new std::thread(blinker_loop);

   msleep(10);
   Serial.println();
   Serial.println("std::thread init done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("std::thread init done!");
   tftline+=15;
   msleep(10);

   //---------------------------------------------------------
   // end of setup
   Serial.println();
   Serial.println("setup(): done!\n");
   display.setTextColor(WHITE);
   display.setCursor(0, tftline);
   display.print("setup(): done!");
   tftline+=15;
   msleep(3000);

   // DEBUG
   // while(1) delay(100);

   display.fillScreen(COLOR_BGND);
   drawAllTSbuttons();

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
   msleep(1);


   /*
      // WebServer for clients
      webserver.handleClient();
      msleep(1);
   */


   // touch screen buttons
   GetTSbuttons();
   msleep(5);

   // IMU readings
   if(DEBUG) {
      Serial.print("read MPU6050: ");
   }
   read_MPU6050(fyaw, fpitch, froll);
   pitch = (int16_t)fpitch;
   roll = -(int16_t)froll;
   msleep(5);    // 3

   // ads1115 readings
   int adcraw;
   adcraw=adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0: port BUG!!
   adc00 = mapConstrain(adc00);
   btn00 = btnstate00(adc00);
   msleep(5);

   adc01 = ads0.readADC_SingleEnded(1);  // ADS1115 port A1
   adc01 = mapConstrain(adc01);  // 4Btn pad raw
   msleep(5);

   adc02 = ads0.readADC_SingleEnded(2);  // ADS1115 port A2
   adc02 = mapConstrain(adc02);  // Poti
   msleep(5);
   adc03 = ads0.readADC_SingleEnded(3);  // ADS1115 port A3
   adc03 = mapConstrain(adc03);  // Poti
   msleep(5);

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
   msleep(1);
   showOptionsWindow();
   msleep(10);

   uint8_t DEBUGSELECTFILE=0;
   // debug
   if(DEBUGSELECTFILE) {
      display.fillRect( 0, display.height()-30, display.width(), 16, BLACK);
      display.setCursor(0, display.height()-30);
      COLOR_TEXT = YELLOW;
      display.setTextColor(COLOR_TEXT);
      display.print(fileSelected);
   }

   msleep(1);
}

/*
   references:
   https://github.com/TKJElectronics/KalmanFilter
   https://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/
   https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/SimpleWiFiServer/SimpleWiFiServer.ino
   https://charbase.com/block/emoticons


*/
