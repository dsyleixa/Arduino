// TFT Menu 
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose: only 
// by personal written permission by the author.

// default TFT: OLED 128x64, compatible to Adafruit (R) Libs
// in this example: using an ESP8266 NodeMCU 1.0 board
// using ButtonClass for button action (up, down, enter, single/double/long press) 

// history:
// 0.0.2  Exec function, variant values
// 0.0.1  tMenu new list

// menu example: change variant values
// ver 0.0.2


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

 
char * mlist1[10] = {
                     "M1 main menu",  // 0
                     "L01",           // 1                     
                     "itest  ",       // 2
                     " +1 !",         // 3
                     " -1 !",         // 4
                     "L05",           // 5
                     "L06",           // 6
                     "L07",           // 7
                     "L08",           // 8
                     "L09"            // 9
}; 

// create instance
tMenu menu1(10,11, (char**)mlist1, &menu1, 1);  
//  (numEntries,lineLength,  menu_list,  preMenu, menu-ID)

//-----------------------------------------------------------------------

tMenu * actMenu = &menu1;  // menu handle  

//-----------------------------------------------------------------------
// execute what to do at either line

char buf[20];
volatile int  itest=1000;

char * ExecList1(int menuline){
  sprintf(menu1.list[2], "%d", itest);   // refresh value
  
  switch (menuline) {
     case  0:    ; break;
     case  1:    ; break;
     case  2:    ; break;  // display int value
     case  3:    {  
                   itest+=1;                      // change value
                   menu1.IntToLine(itest, 2);  // refresh screen
                   break;
                 }
     case  4:    {  
                   itest-=1;                      // change value
                   menu1.IntToLine(itest, 2);  // refresh screen
                   break;
                 }
     case  5:    ; break;
     case  6:    ; break;
     case  7:    ; break;
     case  8:    ; break;
     case  9:    ; break;  
  }     
}


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
  //delay(3000); // wait for Serial()
  Serial.println("Serial started");
  
  btnUp.init(2, INPUT_PULLUP);
  btnDown.init(3, INPUT_PULLUP);
  btnEnter.init(4, INPUT_PULLUP);    
  
  // Start i2c Wire (SDA, SCL)
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

  sprintf(buf, "%d", itest); 

  //display default setting
  actMenu = &menu1;  //     
  actMenu->mdisplay();   
  delay(500);
  // display actual itest variable value
  actMenu->IntToLine(itest, 2);
  
  Serial.println("\n\n");
}


//-----------------------------------------------------------------------
// loop
//-----------------------------------------------------------------------
void loop() {
    int32_t ID;
    int16_t line;      

    line = actMenu->checkbtn(btnUp.click(), btnDown.click(), btnEnter.click() ); 
    ID = actMenu->ID;    
        
    if(line!=-1) {      // double click btnEnter: do if selected             
       // execute menu lines
       if( ID==1 )  {    
           ExecList1(line);  
           // debug
           // Serial.print("itest="); Serial.println(itest);
           Serial.print("mline="); Serial.println(actMenu->list[2]);
       }      
    }       
}

// end of file
