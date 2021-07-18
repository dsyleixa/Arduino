// Adafruit_ImageReader test for 3.5" TFT FeatherWing. Demonstrates loading
// images to the screen, to RAM, and how to query image file dimensions.
// OPEN THE ARDUINO SERIAL MONITOR WINDOW TO START PROGRAM.
// Requires three BMP files in root directory of SD card:
// adabot.bmp, parrot.bmp and wales.bmp.

#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_HX8357.h>      // Hardware-specific library
#include <Adafruit_ImageReader.h> // Image-reading functions

// Pin definitions for 2.4" TFT FeatherWing vary among boards...

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
#define SD_CS    5
#endif

Adafruit_HX8357      tft    = Adafruit_HX8357(TFT_CS, TFT_DC);
Adafruit_ImageReader reader;     // Class w/image-reading functions
Adafruit_Image       imgRAM;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;

//------------------------------------------------------------
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

//------------------------------------------------------------
void setup(void) {

   ImageReturnCode stat; // Status from image-reading functions

   Serial.begin(115200);
#if !defined(ESP32)
   delay(2000);       // Wait for Serial Monitor before continuing
#endif

   tft.begin();          // Initialize screen

   Serial.print(F("Initializing SD card..."));
   if (!SD.begin(SD_CS)) {
      Serial.println(F("    SD access failed!"));
      while (true); // Loop here forever
   }
   Serial.println(F("    SD OK!"));

   File  root = SD.open("/");
   printDirectory(root, 0);
   Serial.println("   SD print files done!\n");



   // Fill screen blue. Not a required step, this just shows that we're
   // successfully communicating with the screen.
   tft.fillScreen(HX8357_BLUE);

   // Load full-screen BMP file 'adabot.bmp' at position (0,0) (top left).
   // Notice the 'reader' object performs this, with 'tft' as an argument.
   Serial.println(F("Loading 'adabot.bmp' to screen..."));
   stat = reader.drawBMP("/adabot.bmp", tft, 0, 0);
   // (Absolute path isn't necessary on most devices, but something
   // with the ESP32 SD library seems to require it.)
   reader.printStatus(stat);   // How'd we do?

   // Query the dimensions of image 'parrot.bmp' WITHOUT loading to screen:
   Serial.println(F("Querying parrot.bmp image size..."));
   stat = reader.bmpDimensions("/parrot.bmp", &width, &height);
   reader.printStatus(stat);   // How'd we do?
   if (stat == IMAGE_SUCCESS) { // If it worked, print image size...
      Serial.print(F("Image dimensions: "));
      Serial.print(width);
      Serial.write('x');
      Serial.println(height);
   }

   // Load small BMP 'wales.bmp' into a GFX canvas in RAM. This should
   // fail gracefully on AVR and other small devices, meaning the image
   // will not load, but this won't make the program stop or crash, it
   // just continues on without it.
   Serial.println(F("Loading 'wales.bmp' to canvas..."));
   stat = reader.loadBMP("/wales.bmp", imgRAM);
   reader.printStatus(stat); // How'd we do?

   tft.fillScreen(HX8357_BLUE);
   
   // Load full-screen BMP file 'Lahntal.bmp' at position (0,0) (top left).
   // Notice the 'reader' object performs this, with 'tft' as an argument.
   Serial.println(F("Loading 'Lahntal.bmp' to screen..."));
   stat = reader.drawBMP("/Lahntal.bmp", tft, 0, 0);
   // (Absolute path isn't necessary on most devices, but something
   // with the ESP32 SD library seems to require it.)
   reader.printStatus(stat);   // How'd we do?
   
   tft.fillScreen(HX8357_BLUE);
   
   // Load full-screen BMP file 'Gossfeld.bmp' at position (0,0) (top left).
   // Notice the 'reader' object performs this, with 'tft' as an argument.
   Serial.println(F("Loading 'Gossfeld.bmp' to screen..."));
   stat = reader.drawBMP("/Gossfeld.bmp", tft, 0, 0);
   // (Absolute path isn't necessary on most devices, but something
   // with the ESP32 SD library seems to require it.)
   reader.printStatus(stat);   // How'd we do?

   delay(3000); // Pause 2 seconds before moving on to loop()
}


//------------------------------------------------------------
void loop() {

   delay(1000); // Pause 2 sec.

}
