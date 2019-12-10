// TFT Menu 
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose: only 
// by personal written permission by the author.

// default TFT: OLED 128x64, compatible to Adafruit (R) Libs
// in this example: using an ESP8266 NodeMCU 1.0 board
// using ButtonClass for button action (up, down, enter, single/double/long press) 

// history:
// 0.0.1  tMenu new list

// simple menu example 
// ver 0.0.1


// i2c
#include <Wire.h>         // Incl I2C comm, but needed for not getting compile error


//-----------------------------------------------------------------------
// Display driver
//-----------------------------------------------------------------------
#include <Adafruit_GFX.h>   // https://github.com/adafruit/Adafruit-GFX-Library 
#include <Adafruit_SSD1306.h> // https://github.com/esp8266/arduino 

//#include <Fonts/FreeSans9pt7b.h>             // optional
//#include <Fonts/FreeMono12pt7b.h>            // optional  
#include <Fonts/FreeMono9pt7b.h>         // used here
//#include <Fonts/FreeMonoBold7pt7b.h>   // customized, recommended for OLED 

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

 
//-----------------------------------------------------------------------
// Display menu
//-----------------------------------------------------------------------
#include <MenuClass.h>

 
char * mlist1[7] = {
                     "Menu1 L0",
                     "LED ON !",
                     "LED OFF!", 
                     "L3", 
                     "L4"}; 

tMenu menu1(5,11, (char**)mlist1, &menu1, 1);  
//      numEntries, lineLength,  menu_list,  preMenu, menu-ID

tMenu * actMenu = &menu1;  // for convenience, not required here


//-----------------------------------------------------------------------
// ButtonClass buttons
//-----------------------------------------------------------------------
#include <ButtonClass.h>

// 3 buttons for menu control
tButton btnUp;
tButton btnDown;
tButton btnEnter;



//-----------------------------------------------------------------------
// setup
//-----------------------------------------------------------------------
void setup(void)
{
  // Start Serial

  Serial.begin(115200);
  delay(3000); // wait for Serial()
  Serial.println("Serial started");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !LOW);  // inverted LED_BUILTIN signal logic
  
  btnUp.init(2, INPUT_PULLUP);
  btnDown.init(3, INPUT_PULLUP);
  btnEnter.init(4, INPUT_PULLUP);    
  
  // Start i2c Wire (SDA, SCL)
  //Wire.begin(ESPSDA,ESPSCL);  // !!!!!! adjust if necessary !!!!!!!! 
  Wire.begin();

  
  
  tft.setTextSize(2);
  
  tft.setFont();
  tft.setTextColor(WHITE);

  // text display debug tests
  tft.setCursor(0,0);
  tft.print("Hello, world!");  // debug
  tft.display();
  delay(500);

  // use display menu font
  tft.setFont(&FreeMono9pt7b);    
  tft.fillScreen(0x0000);
 

  Serial.println("menu init:");
  actMenu = &menu1;  //   
  actMenu->mdisplay();   
  Serial.println("\n\n");
}


//-----------------------------------------------------------------------
// loop
//-----------------------------------------------------------------------
void loop() {
    int32_t ID;
    int16_t line;   
    char buf[20];

    line = actMenu->checkbtn(btnUp.click(), btnDown.click(), btnEnter.click() ); 
    ID = actMenu->ID;
    
    if(line!=-1) {      // double click btnEnter: do if selected
       sprintf(buf, "select: line=%d ID=%d  contents=%s", line, ID, actMenu->list[line]); 
       Serial.println(buf);

       if(line==0) {  // menu line 0
         // do something
       }
       else
       if(line==1) {  // menu line 1
         digitalWrite(LED_BUILTIN, !HIGH);  // inverted LED_BUILTIN signal logic
       }
       else
       if(line==2) {  // menu line 2
         digitalWrite(LED_BUILTIN, !LOW);   // inverted LED_BUILTIN signal logic
       }
       else
       if(line==3) {  // menu line 3
         // do something
       }
       else
       if(line==4) {  // menu line 4
         // do something
       }
    }       
}

// end of file
