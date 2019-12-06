// TFT Menu 
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose: only 
// by personal written permission by the author.

// default TFT: OLED 128x64, compatible to Adafruit (R) Libs
// in this example: using an ESP8266 NodeMCU 1.0 board
// using ButtonClass for button action (up, down, enter, single/double/long press) 

// history:
// 0.0.3  exec functions
// 0.0.2  3 nested menus 
// 0.0.1  tMenu new list

// simple menu example 
// ver 0.0.3

/*
ESP digital pins:          
digital  GPIO     default         
   D0     16       WAKE           
   D1      5       I2C SCL        
   D2      4       I2C SDA         
   D3      0       FLASH/LED      
   D4      2       TX1            
   D5     14       SPI SCK        
   D6     12       SPI MISO       
   D7     13       SPI MOSI        
   D8     15       MTD0 PWM       
   D9      3       UART RX0       
   D10     1       UART TX0     
*/

// i2c
#include <Wire.h>         // Incl I2C comm, but needed for not getting compile error


//-----------------------------------------------------------------------
// display driver
//-----------------------------------------------------------------------
#include <Adafruit_GFX.h>   // https://github.com/adafruit/Adafruit-GFX-Library 
// Adafruit Arduino OLED driver
#include <Adafruit_SSD1306.h> // https://github.com/esp8266/arduino 

#include <Fonts/FreeSans9pt7b.h>             // optional
#include <Fonts/FreeMono12pt7b.h>            // optional  
 #include <Fonts/FreeMono9pt7b.h>     // used here by default
//#include <Fonts/FreeMonoBold7pt7b.h>    // optional, recommended

// Pin definitions

#define OLED_RESET 10  // GPIO10=D12 Pin RESET signal (virtual)

//Adafruit_SSD1306 display();  // old Adafruit lib (tested OK)
 Adafruit_SSD1306 display(128, 64, &Wire); // new Adafruit lib

 
//-----------------------------------------------------------------------
// OLED menu
//-----------------------------------------------------------------------
#include <MenuClass.h>


//--------------------------------------------
// menu line lists + menu instances
//--------------------------------------------
 
char * mlist1[10] = {
                     "M1 main menu",
                     "L01",
                     "menu02 >",
                     "LEDBI ON !",
                     "LEDBI OFF!",
                     "L05",
                     "L06",
                     "L07",
                     "L08",
                     "L09"
}; 

// create instance
tMenu menu1(10,11, (char**)mlist1, &menu1, 1);  
//   numEntries, lineLength,  menu_list,  preMenu, menu-ID


//--------------------------------------------
char * mlist12[5]= { 
                     "back <",
                     "1",
                     "2",
                     "3",
                     "menu024      >"
};

// create instance
tMenu menu12(5,11, (char**)mlist12, &menu1, 12);  
//   numEntries, lineLength,  menu_list,  preMenu, menu-ID
 
char * mlist124[3] = { 
                       "back <",
                       "Yes",
                       "No"
}; 

//--------------------------------------------
// create instance
tMenu menu124(3,11, (char**)mlist124, &menu12, 124); 
//   numEntries, lineLength,  menu_list,  preMenu, menu-ID


//-------------------------------------------- 
// create handle (alias)
tMenu * actMenu = &menu1;


//--------------------------------------------
// menu execute functions
//--------------------------------------------

char * ExecList1(int menuline){
  switch (menuline) {
     case  0:    ; break;
     case  1:    ; break;
     case  2:    actMenu = &menu12; actMenu->mdisplay(); break;
                 // actMenu refresh for new menu ID
     case  3:    digitalWrite(LED_BUILTIN, HIGH); break; // 
     case  4:    digitalWrite(LED_BUILTIN, LOW); break;  // 
     case  5:    ; break;
     case  6:    ; break;
     case  7:    ; break;
     case  8:    ; break;
     case  9:    ; break;
  }     
  return "";              
}; 

//--------------------------------------------
char * ExecList12(int menuline){
  switch (menuline) {
     case  0:    actMenu = actMenu->preMenu; actMenu->mdisplay(); break;
                 // actMenu refresh for new menu ID
     case  1:    return "1"; break;
     case  2:    return "2"; break;
     case  3:    return "3"; break;
     case  4:    actMenu = &menu124;actMenu->mdisplay(); break;
                 // actMenu refresh for new menu ID
  }        
  return "";           
}; 

//--------------------------------------------
char * ExecList124(int menuline){
  switch (menuline) {
     case  0:    actMenu = actMenu->preMenu; actMenu->mdisplay(); break;
                 // actMenu refresh for new menu ID
     case  1:    Serial.println("Yes"); return "Yes"; break;
     case  2:    Serial.println("No"); return "No"; break;          
  }                                  
  return "";        
}; 


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

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  //  
  
  btnUp.init( 2, INPUT_PULLUP);
  btnDown.init( 3, INPUT_PULLUP);
  btnEnter.init( 4, INPUT_PULLUP);    
  
  // Start Wire (SDA, SCL)
  Wire.begin();

  // SSD1306 Init  
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // old Adafruit lib
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, false); // new Adafruit lib
  
  display.setRotation(2);  
  display.fillScreen(0x0000);  // Clear the buffer.

  // text display tests
  display.setTextSize(1);
  display.setFont();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Hello, world!");  
  display.display();
  delay(500);

  //--------------------------------------
  // test +  debug
  display.setFont(&FreeMono9pt7b);   // default 
  //display.setFont(&FreeMonoBold7pt7b);   // optional, recommended
  
  display.fillScreen(0x0000); 
  display.display(); 

  Serial.println("menu init:");

  actMenu = &menu1;  //   
  actMenu->mdisplay();   

  actMenu = &menu12;  //   
  actMenu->mdisplay();  

  actMenu = &menu124;  //   
  actMenu->mdisplay(); 

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
       
       // debug lines
       sprintf(buf, "select: line=%d ID=%d  contents=%s", line, ID, actMenu->list[line]); 
       Serial.println(buf);

       // execute menu lines
       if( ID==1 )  { ExecList1(line);  }  // re menu line from mlist1      
       else
       if( ID==12)  { ExecList12(line); }  // re menu line from mlist12      
       else
       if( ID==124) { ExecList124(line); } // re menu line from mlist124     
    }       
}

// end of file
