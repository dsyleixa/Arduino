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

#if defined(ESP8266)
#define TFT_CS   0
#define TFT_DC   15
#define SD_CS    2

#elif defined(ESP32)
#define TFT_CS   15
#define TFT_DC   33
#define SD_CS    14

#elif defined(TEENSYDUINO)
#define TFT_DC   10
#define TFT_CS   4
#define SD_CS    8

#elif defined(ARDUINO_STM32_FEATHER)
#define TFT_DC   PB4
#define TFT_CS   PA15
#define SD_CS    PC5

#elif defined(ARDUINO_NRF52_FEATHER) // BSP 0.6.5 and higher!
#define TFT_DC   11
#define TFT_CS   31
#define SD_CS    27

#elif defined(ARDUINO_MAX32620FTHR) || defined(ARDUINO_MAX32630FTHR)
#define TFT_DC   P5_4
#define TFT_CS   P5_3
#define STMPE_CS P3_3
#define SD_CS    P3_2

#else // Anything else!
#define TFT_CS   9
#define TFT_DC   10
#define SD_CS    5 // 4

#endif



#include <SPI.h>
#include <SD.h>

String filelist[64];
int flcnt=0;

File SDroot;


void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    
    Serial.print(entry.name());
    filelist[flcnt]=(String)entry.name();    
    
    if (entry.isDirectory()) {      
      Serial.println("/.");
      filelist[flcnt]+=(String)"/.";
      flcnt++;
      filelist[flcnt]=filelist[flcnt]=(String)entry.name()+"/..";
      flcnt++;
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
      flcnt++;
    }
    entry.close();
  }
}




void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  delay(2000);

  Serial.print("Initializing SD card...");

  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  SDroot = SD.open("/");

  printDirectory(SDroot, 0);
  Serial.println("print files done!\n");
  
  Serial.println("print filelist[]:");
  for (int i=0; i<flcnt; i++) {
   if(i<100) Serial.print(" ");
   if(i<10)  Serial.print(" ");
   Serial.println((String)i+"  "+filelist[i]);
  }
  Serial.println("print filelist[] done!\n");

  
}

void loop() {
  // nothing happens after setup finishes.
}
