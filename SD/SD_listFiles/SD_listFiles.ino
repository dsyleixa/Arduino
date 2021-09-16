/*
  SD_Listfiles
  https://www.arduino.cc/en/Tutorial/listfiles 

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




File root;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("print files done!");
}

void loop() {
  // nothing happens after setup finishes.
}

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
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
