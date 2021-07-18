// ESP32, boxed
// analog Button Pad (10bit):
//           up=0-2
// left=88-89     right=32-33
//         dn=166-168
//       bottom=350-352

//=====================================================================
#include <TFT_HX8357.h>
/*
       #include "Adafruit_GFX.h"
       #include "Adafruit_HX8357.h"
       #include <color16bit.h>  // 16 bit color defs
       #include <Fonts/FreeSans9pt7b.h>
       #include <Fonts/FreeMono12pt7b.h>
       #include <Fonts/FreeMono9pt7b.h>
       Adafruit_HX8357_ini(n);
*/


int COLOR_BGND = BLACK;
int COLOR_TEXT = WHITE;

//=====================================================================
#include <Wire.h>
#include <ardustdio.h>
#include <arduarray.h>
#include <stringEx.h>

//---------------------------------------------------------------------
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads0(0x48);

static uint16_t adc00, adc01, adc02, adc03;

//---------------------------------------------------------------------
// MPU6050
#include "data\MPU6050Kalman.h"
int8_t   yaw, pitch, roll;
int16_t  accx, accy, accz;

//=====================================================================



//---------------------------------------------------------------------
Adafruit_GFX_Button TSbutton1, TSbutton2, TSbutton3;
//---------------------------------------------------------------------
static int  cnt =-1;
static bool FILELIST = false;

void GetTSbuttons() {
   char str1[30], str2[30];

   //-------------------------------------------
   // Touchscreen readings
   // My new changes
   //-------------------------------------------
   Serial.println("touchscreen");

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

   Serial.print("X = "); Serial.print(p.x);
   Serial.print("\tY = "); Serial.print(p.y);
   Serial.print("\tPressure = "); Serial.println(p.z);
   Serial.println();
   sprintf(str1, "x=%3d", p.x); sprintf(str2, "y=%3d", p.y);

   display.setTextColor(RED);

   //display.fillRect(display.width()-80,40, 80,40, COLOR_BKGR);  // x1,y1, dx,dy
   //display.setCursor(display.width()-75,40);    display.print(buf);
   //display.setCursor(display.width()-75,60);    display.print(buf1);

   //-------------------------------------------
   // check if a TSbutton was pressed
   //-------------------------------------------
   TSbutton1.press(TSbutton1.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton2.press(TSbutton2.contains(p.y, p.x)); // tell the TSbutton it is pressed
   TSbutton3.press(TSbutton3.contains(p.y, p.x)); // tell the TSbutton it is pressed

   //-------------------------------------------
   // process TSbutton states+changes
   //-------------------------------------------

   Serial.println("buttons");

   if (TSbutton1.justReleased() ) {
      TSbutton1.drawButton();
      TSbutton2.drawButton();
      TSbutton3.drawButton();
   }
   else if (TSbutton1.justPressed()) {
      TSbutton1.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton2.justReleased() ) {
      TSbutton1.drawButton();
      TSbutton2.drawButton();
      TSbutton3.drawButton();
   }
   else if (TSbutton2.justPressed()) {
      TSbutton2.drawButton(true); // draw invert!
   }

   //-------------------------------------------

   else if (TSbutton3.justReleased() ) {
      TSbutton1.drawButton();
      TSbutton2.drawButton();
      TSbutton3.drawButton();

   }
   else if (TSbutton3.justPressed()) {
      TSbutton3.drawButton(true); // draw invert!
   }

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

void ls() {
   Serial.println("print filelist[]:");
   for (int cnt = 0; cnt < filecount; cnt++) {
      if (cnt < 100) Serial.print(" ");
      if (cnt < 10)  Serial.print(" ");
      Serial.println((String)cnt+"  "+filelist[cnt]);
      display.setCursor(20+240*(cnt%2), 16*(cnt/2));
      display.println(filelist[cnt]);
   }
   Serial.println("print filelist[] done!\n");
}

//-------------------------------------------

void markPos(int old, int cnt) {
   COLOR_TEXT = YELLOW;
   display.fillRect ( 0+240*(old%2), 16*(old/2), 14, 14, COLOR_BGND);
   display.setTextColor(COLOR_TEXT);
   display.setCursor( 0+240*(cnt%2), 16*(cnt/2));
   display.print(">");
   delay(300);
}

//-------------------------------------------

void showScreenFileMenu() {
   if (isInRange( adc00, 346, 356 ) ) {
      FILELIST = !FILELIST;
      delay(1);
      adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
      delay(1);
      adc00 = mapConstrain(adc00);
      delay(100);
      display.fillScreen(COLOR_BGND);
      COLOR_TEXT = WHITE;
      display.setTextColor(COLOR_TEXT);
      if(FILELIST) {
         readDirectory(SdPath, 0);
         ls();
         markPos(cnt, cnt);
      }
   }
   else if (isInRange( adc00, 162, 172 ) && FILELIST && cnt <= filecount) {
      if (cnt < filecount-1) {
         markPos(cnt, cnt+1);
         cnt++;
      }
      delay(1);
      adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
      delay(1);
      adc00 = mapConstrain(adc00);
   }
   else if (isInRange( adc00, 0, 10 ) && FILELIST && cnt >= 0) {
      markPos(cnt, cnt-1);
      if (cnt >= 0) cnt--;
      delay(1);
      adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
      delay(1);
      adc00 = mapConstrain(adc00);
   }
}


//=====================================================================
//=====================================================================
void setup() {
   Serial.begin(115200);
   delay(500);

   //---------------------------------------------------------
   Adafruit_HX8357_ini(1);  // init function in lib <display_HX3857.h>

   COLOR_BGND = BLACK;
   COLOR_TEXT = RED;
   display.fillScreen(COLOR_BGND);
   display.setTextColor(COLOR_TEXT);
   display.setTextSize(2);
   //display.setFont(&FreeMono9pt7b);
   Serial.println("display Setup done!");
   Serial.println();
   display.setCursor(0, 10); display.print("display Setup done!");

   //---------------------------------------------------------

   //TS buttons
   TSbutton1.initButton(&display, display.width()-37,  20, 76, 40, CYAN, BLUE, YELLOW, "Btn1", 2);
   TSbutton1.drawButton();

   TSbutton2.initButton(&display, display.width()-37, 100, 76, 40, CYAN, BLUE, YELLOW, "Btn2", 2);
   TSbutton2.drawButton();

   TSbutton3.initButton(&display, display.width()-37, 180, 76, 40, CYAN, BLUE, YELLOW, "Btn3", 2);
   TSbutton3.drawButton();

   //---------------------------------------------------------

   Serial.print("Initializing SD card...");

   if (!SD.begin(SD_CS)) {
      Serial.println("initialization failed!");
      while (1);
   }
   Serial.println("initialization done.");
   SdPath = SD.open("/");

   //---------------------------------------------------------

   Wire.begin();
   Wire.setClock(400000);
   ads0.begin();
   Serial.println("i2c ads1115 Setup done!");
   Serial.println();
   display.setTextColor(COLOR_TEXT);
   display.setCursor(0, 30); display.print("i2c ads1115 Setup done!");

   delay(1000);
   display.fillScreen(COLOR_BGND);

}


//=====================================================================
//=====================================================================

void loop() {
   char str1[30], str2[30];


   //GetTSbuttons();


   adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
   delay(1);
   adc01 = ads0.readADC_SingleEnded(1);  // ADS1115 port A1
   delay(1);
   adc02 = ads0.readADC_SingleEnded(2);  // ADS1115 port A2
   delay(1);
   adc03 = ads0.readADC_SingleEnded(3);  // ADS1115 port A3


   adc00 = mapConstrain(adc00);
   adc01 = mapConstrain(adc01);
   adc02 = 1023-mapConstrain(adc02);
   adc03 = 1023-mapConstrain(adc03);

   sprintf(str1, "ad00=%-4d ad01=%-4d ", adc00, adc01);
   sprintf(str2, "ad02=%-4d ad03=%-4d ", adc02, adc03);
   Serial.print((String)str1+str2); Serial.println();

   display.fillRect( 0, display.height()-14, display.width(), 14, BLACK);
   COLOR_TEXT = RED;
   display.setTextColor(COLOR_TEXT);
   display.setTextSize(2);
   display.setFont();
   display.setCursor(0, display.height()-14);
   display.print((String)str1+str2);

   showScreenFileMenu();


   delay(10);
}
