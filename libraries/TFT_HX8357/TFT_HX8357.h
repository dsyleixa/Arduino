#pragma once 


// TFT_HX3857 instance and inits 
// ver 0.4  // incl SD

#include <SPI.h>
#include <SD.h>
#include <ardustdio.h>

//----------------------------------------------------
// Featherwing Adafruit_HX8357 graph lib settings
//----------------------------------------------------
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"      // display 3.5"
#include <Adafruit_STMPE610.h>    // touch screen
#include <Adafruit_ImageReader.h> // SD Image 



#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
 
#elif defined ESP32        // ESP32
   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14   
 
#elif defined TEENSYDUINO
   #define TFT_DC   10
   #define TFT_CS   4
   #define STMPE_CS 3
   #define SD_CS    8
  
#elif defined ARDUINO_STM32_FEATHER
   #define TFT_DC   PB4
   #define TFT_CS   PA15
   #define STMPE_CS PC7
   #define SD_CS    PC5
  
#elif defined ARDUINO_FEATHER52
   #define STMPE_CS 30
   #define TFT_CS   13
   #define TFT_DC   11
   #define SD_CS    27

#elif  defined(ARDUINO_MAX32620FTHR) || defined(ARDUINO_MAX32630FTHR)
   #define TFT_DC   P5_4
   #define TFT_CS   P5_3
   #define STMPE_CS P3_3
   #define SD_CS    P3_2

// Something else!          // standard pinout
#elif  defined (__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined (__AVR_ATmega328P__) || defined(ARDUINO_SAMD_ZERO) || defined(__SAMD51__)   
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5

 // default 
#else
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
#endif

#define TFT_RST -1

int SDioerr=0;


//----------------------------------------------------
// TFT + TS + BMP Image: create instance
//----------------------------------------------------
Adafruit_HX8357  display = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

Adafruit_STMPE610  ts = Adafruit_STMPE610(STMPE_CS);

// display aspect ratio
float TFTaspratio=1.5;


//----------------------------------------------------
// fonts
//----------------------------------------------------
#include <Fonts/FreeMono8pt7b.h>
#include <Fonts/FreeSans9pt7b.h>            
#include <Fonts/FreeMono12pt7b.h>              
#include <Fonts/FreeMono9pt7b.h>



//----------------------------------------------------
// touch screen buttons
//----------------------------------------------------
TS_Point p;

// Touch Calibration
int TS_MINX = 100;
int TS_MAXX = 3800;
int TS_MINY = 100;
int TS_MAXY = 3750;



//----------------------------------------------------
// Adafruit_Image
//----------------------------------------------------
Adafruit_ImageReader reader;         // Class w/image-reading functions
Adafruit_Image       img;            // An image loaded into RAM
int32_t              BMP_width  = 0, // BMP image dimensions
                     BMP_height = 0;
ImageReturnCode      img_stat;       // Status from image-reading functions



//----------------------------------------------------
// 16bit colors
//----------------------------------------------------
#include <color16bit.h>


//----------------------------------------------------
// Adafruit_HX8357_ini
//----------------------------------------------------
void Adafruit_HX8357_ini(char orientation) {  // call in setup() !
  uint32_t clock;
  clock=HX8357D; 
  
  // TFT
  display.begin(HX8357D);  
  display.setRotation(orientation);
  display.fillScreen(BLACK);
  display.setTextSize(2);
  TFTaspratio = display.width()/(float)display.height(); 
  Serial.println("Adafruit_HX8357_ini: Display started");  


  //---------------------------------------------------------
  // TS
  if (!ts.begin()) {
    Serial.println("Adafruit_HX8357_ini: Couldn't start touchscreen controller");
    delay(2000); // Loop here
  }
  Serial.println("Adafruit_HX8357_ini: Touchscreen started");  

  
  // SD
    Serial.print(F("Adafruit_HX8357_ini: Initializing SD card... "));
  SDioerr=SD.begin(SD_CS);
  if( !SDioerr ) {
    Serial.println("SD.begin(SD_CS) failed!");
    Serial.println();
    delay(1000); // delay here 
  }
  Serial.println("SD OK.");
  Serial.println(); 
}




