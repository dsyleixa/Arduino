/*
   SD_filelists
   https://www.arduino.cc/en/Tutorial/filelists

   This example shows how print out the files in a
   directory on a SD card

   The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

   created   Nov 2010
   by David A. Mellis
   modified 9 Apr 2012
   by Tom Igoe
   modified 2 Feb 2014
   by Scott Fitzgerald

   This example code is in the public domain.
*/

#include <SPI.h>
#include <SD.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"      // tft display 3.5"
#include <Adafruit_STMPE610.h>    // ts touch screen
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



Adafruit_HX8357   display = Adafruit_HX8357(TFT_CS, TFT_DC);


String filelist[64];
int flcnt = 0;

File SdPath;


//=================================================================
void readDirectory(File dir, int numLevel) {
   while (true) {

      File entry =  dir.openNextFile();
      if (! entry) {
         // no more files
         break;
      }
      for (uint8_t i = 0; i < numLevel; i++) {
         //Serial.print('\t');
      }

      //Serial.print(entry.name());
      filelist[flcnt] = (String)entry.name();

      if (entry.isDirectory()) {
         //Serial.println("/");
         filelist[flcnt] += (String)"/";
         //Serial.println(filelist[flcnt]);

         //display.setCursor(20 + 240 * (flcnt % 2), 16 * (flcnt / 2));
         //display.println(filelist[flcnt]);

         flcnt++;
         filelist[flcnt] = filelist[flcnt] = (String)entry.name() + "/..";
         //Serial.println(filelist[flcnt]);

         //display.setCursor(20 + 240 * (flcnt % 2), 16 * (flcnt / 2));
         //display.println(filelist[flcnt]);
         flcnt++;
         readDirectory(entry, numLevel + 1);
      } else {
         // files have sizes, directories do not
         //Serial.println(filelist[flcnt]);
         //Serial.print("\t\t");
         //Serial.println(entry.size(), DEC);
           
         //display.setCursor(20 + 240 * (flcnt % 2), 16 * (flcnt / 2));
         //display.println(filelist[flcnt]);

         flcnt++;
      }
      entry.close();
   }
}



//=================================================================
void setup() {
   Serial.begin(115200);
   delay(2000);

   // TFT
   display.begin(HX8357D);
   display.setRotation(1);
   display.fillScreen(HX8357_BLACK);
   display.setTextColor(HX8357_WHITE);
   display.setTextSize(2);

   Serial.print("Initializing SD card...");

   if (!SD.begin(SD_CS)) {
      Serial.println("initialization failed!");
      while (1);
   }
   Serial.println("initialization done.");

   SdPath = SD.open("/");

   readDirectory(SdPath, 0);
   Serial.println("print files done!\n");

   Serial.println("print filelist[]:");
   for (int cnt = 0; cnt < flcnt; cnt++) {
      if (cnt < 100) Serial.print(" ");
      if (cnt < 10)  Serial.print(" ");
      Serial.println((String)cnt + "  " + filelist[cnt]);
      display.setCursor(20+240*(cnt%2), 16*(cnt/2)); 
      display.println(filelist[cnt]);
   }
   Serial.println("print filelist[] done!\n");
}


//=================================================================
void loop() {
   // nothing happens after setup finishes.
}
