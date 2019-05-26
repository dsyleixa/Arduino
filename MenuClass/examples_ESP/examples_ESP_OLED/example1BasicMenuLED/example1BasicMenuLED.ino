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

//Adafruit_SSD1306 display();  // old Adafruit lib (tested OK)
 Adafruit_SSD1306 display(128, 64, &Wire); // new Adafruit lib

 
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
  
  btnUp.init(D6, INPUT_PULLUP);
  btnDown.init(D3, INPUT_PULLUP);
  btnEnter.init(D4, INPUT_PULLUP);    
  
  // Start i2c Wire (SDA, SCL)
  //Wire.begin(ESPSDA,ESPSCL);  // !!!!!! adjust if necessary !!!!!!!! 
  Wire.begin();

  // OLED SSD1306 Init  
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // old Adafruit lib
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, false); // new Adafruit lib  
  display.setRotation(2);  
  display.fillScreen(0x0000);  
  display.setTextSize(1);
  display.setFont();
  display.setTextColor(WHITE);

  // text display debug tests
  display.setCursor(0,0);
  display.print("Hello, world!");  // debug
  display.display();
  delay(500);

  // use display menu font
  display.setFont(&FreeMono9pt7b);    
  display.fillScreen(0x0000);
  display.display(); 

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
