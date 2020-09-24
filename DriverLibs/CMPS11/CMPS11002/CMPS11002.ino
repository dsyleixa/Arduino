/******************************************************************
*                   Arduino CMPS11 example code                   *
*                     CMPS11 running I2C mode                     *
*                     by James Henderson, 2012                    *
*******************************************************************/
// verändert & angepasst für UTFT-Displays von Helmut Wunder, 2015
// (C) CMPS10/CMPS11 code by James Henderson
// (C) UTFT Display Driver Lib by Henning Karlsen
// keine freie Verwendung für kommerzielle Zwecke, 
// Code frei zur privaten Nutzung gemäss
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
// Urheberrechte von  J. Henderson und H. Karlsen bleiben unberührt


#include <UTFTQD.h>
#include <Wire.h>
#define  ADDR_CMPS11  0x60                                // Defines address of CMPS11




//=====================================================================================
// TFT LCD
//=====================================================================================
#define  UTFT_CSpin    52    // <<<<<<<< adjust!

// set LCD TFT type
int16_t  LCDTYPE   =   -1;
#define  __UTFT__       4  // Henning Karlsen UTFT 2.2-2.4" 220x176 - 320x240 lib
                           // http://henningkarlsen.com/electronics/library.php?id=51

//--------------------------------------------------------------------------------------------------
//UTFT   myGLCD(Model, SDA=MISO, SCL, CS,         RESET,  RS)    // Due: 3 exposed SS pins: 4,10,52
  UTFT   myGLCD(QD220A,   50,    49,  UTFT_CSpin,  0,     51);   // A0->Vc (LED), A4->BoardReset
extern   uint8_t SmallFont[];
//--------------------------------------------------------------------------------------------------
int16_t  fontwi= 8,  fonthi=10;

int16_t  _curx_, _cury_;                    // last x,y cursor pos on TFT screen

void lcdcls()  {                                                         
   if(LCDTYPE==__UTFT__) { myGLCD.clrScr();  _curx_ =0;  _cury_ =0; }                           
}

void lcdprintxy(int16_t x, int16_t y, char * str) {
   if(LCDTYPE==__UTFT__) { myGLCD.print(str,x,y); _curx_=x+strlen(str)*fontwi; _cury_=y; }
}



//=====================================================================================

void setup() {
   char sbuf[128];
   Serial.begin(115200); 
   Wire.begin();                                      // Conects I2C   
   
   // TFT LCD
   Serial.println();
   LCDTYPE = __UTFT__ ;                               // set LCD-Type
   
   Serial.println("init LCD...");   
   myGLCD.InitLCD();   
   myGLCD.setFont(SmallFont);
   lcdcls();
      
   sprintf(sbuf, "CMPS11 Example V: %d", soft_ver()); // Display software version of the CMPS11
   lcdprintxy( 0, 0, sbuf);   

}


//=====================================================================================

int soft_ver(){
   int data;                                      // Software version of  CMPS11 is read into data and then returned
 
   Wire.beginTransmission(ADDR_CMPS11);
   // Values of 0 being sent with write masked as a byte to be not misinterpreted as NULL 
   // (bug in arduino 1.0)
   Wire.write((byte)0);                           // Sends the register we wish to start reading from
   Wire.endTransmission();

   Wire.requestFrom(ADDR_CMPS11, 1);              // Request byte from CMPS11
   while(Wire.available() < 1);
   data = Wire.read();         
 
   return(data);
}


//=====================================================================================
void display_data(float b,  int p, int r){    // pitch and roll (p, r) are recieved as ints (signed values)
  char sbuf[128];          
 
  sprintf(sbuf, "heading =  %6.1f", b);
  lcdprintxy( 0,20, sbuf);

  sprintf(sbuf, "Pitch =  %6d", p);
  lcdprintxy( 0,30, sbuf);

  sprintf(sbuf, "Roll  =  %6d", r);
  lcdprintxy( 0,40, sbuf);
 
}


//=====================================================================================

void loop(){
   uint8_t  HdgHibyte, HdgLobyte;           // HdgHibyte and HdgLobyte store high and low bytes of the heading 
   int8_t   pitch, roll;                    // Stores pitch and roll values of CMPS11 (signed char value)
 
   float    fheading;                       // Stores full heading (float)
   char     sbuf[128];
 
   Wire.beginTransmission(ADDR_CMPS11);     //starts communication with CMPS11
   Wire.write(2);                           //Sends the register we wish to start reading from
   Wire.endTransmission();

   Wire.requestFrom(ADDR_CMPS11, 4);        // Request 4 bytes from CMPS11
   while(Wire.available() < 4);             // Wait for bytes to become available
   HdgHibyte = Wire.read();         
   HdgLobyte = Wire.read();           
   pitch = Wire.read();             
   roll = Wire.read();             

   fheading = ( (float)(HdgHibyte<<8) + (float)HdgLobyte )/10.0;  // heading (float)
 
   display_data(fheading, pitch, roll);     // Display data to the LCD
 
   delay(10);   

}
//=====================================================================================
//=====================================================================================
