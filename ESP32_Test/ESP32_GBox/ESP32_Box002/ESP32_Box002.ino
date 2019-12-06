// ESP32, boxed
  // analog Button Pad (10bit):
  //           up=0-2
  // left=88-89     right=32-33
  //         dn=166-168
  //       bottom=350-352

//=====================================================================
#include <TFT_HX8357.h> 
/*  
    *  #include "Adafruit_GFX.h"
    *  #include "Adafruit_HX8357.h"  
    *  #include <color16bit.h>  // 16 bit color defs
    *  #include <Fonts/FreeSans9pt7b.h>
    *  #include <Fonts/FreeMono12pt7b.h>
    *  #include <Fonts/FreeMono9pt7b.h>
    *  Adafruit_HX8357_ini(n);
*/


//=====================================================================
#include <Wire.h>
//---------------------------------------------------------------------
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads0(0x48);
//---------------------------------------------------------------------
// MPU6050
#include "data\MPU6050Kalman.h"
int8_t   pitch, roll;
int16_t  accx, accy, accz;

//=====================================================================

int32_t  mapConstrain(int32_t val, int valmin=0, int valmax=17480, int newmin=0, int newmax=1023) {  
  val=map(val, valmin, valmax,  newmin, newmax);
  val=constrain(val, newmin, newmax);
  return (int32_t)val;
}




//---------------------------------------------------------------------
Adafruit_GFX_Button TSbutton1, TSbutton2, TSbutton3;
//---------------------------------------------------------------------

void GetTSbuttons() {
  char str1[30], str2[30];
  
  //-------------------------------------------  
  // Touchscreen readings
  // My new changes
  //-------------------------------------------
  Serial.println("touchscreen");
    
  if (ts.touched()) {
    // Empty the touchscreen buffer to ensure that the latest touchscreen interaction is held in the variable
    while ( ! ts.bufferEmpty() ) {
       p = ts.getPoint();
    }
    ts.writeRegister8(STMPE_INT_STA, 0xFF);    // clear the 'touched' interrupts    
  }
  // end my changes-------------------------------------------------
  

  // Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MAXX, TS_MINX, 0, tft.height());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
  
  Serial.print("X = "); Serial.print(p.x); 
  Serial.print("\tY = "); Serial.print(p.y);  
  Serial.print("\tPressure = "); Serial.println(p.z);
  Serial.println();
  sprintf(str1, "x=%3d", p.x); sprintf(str2, "y=%3d", p.y);

  tft.setTextColor(RED);
  
  //tft.fillRect(tft.width()-80,40, 80,40, COLOR_BKGR);  // x1,y1, dx,dy
  //tft.setCursor(tft.width()-75,40);    tft.print(buf);
  //tft.setCursor(tft.width()-75,60);    tft.print(buf1);   

  //-------------------------------------------
  // check if a TSbutton was pressed
  //-------------------------------------------
  TSbutton1.press(TSbutton1.contains(p.y, p.x)); // tell the TSbutton it is pressed
  TSbutton2.press(TSbutton2.contains(p.y, p.x)); // tell the TSbutton it is pressed
  TSbutton3.press(TSbutton3.contains(p.y, p.x)); // tell the TSbutton it is pressed

  //-------------------------------------------
  // process TSbutton states + changes
  //-------------------------------------------
  
  Serial.println("buttons");
  
  if (TSbutton1.justReleased() ) {
    TSbutton1.drawButton();
    TSbutton2.drawButton();
    TSbutton3.drawButton();    
     
  }
  else 
  if (TSbutton1.justPressed()) {
    TSbutton1.drawButton(true); // draw invert!
     
  }
  
    
  //-------------------------------------------  
  
  else  
  if (TSbutton2.justReleased() ) {   
    TSbutton1.drawButton();
    TSbutton2.drawButton();
    TSbutton3.drawButton();
     
  }
  else
  if (TSbutton2.justPressed()) {
    TSbutton2.drawButton(true); // draw invert!
     
  }


  //-------------------------------------------
  
  else  
  if (TSbutton3.justReleased() ) {
    TSbutton1.drawButton();
    TSbutton2.drawButton();
    TSbutton3.drawButton();
    
  } 
  else 
  if (TSbutton3.justPressed()) {
    TSbutton3.drawButton(true); // draw invert!
     
  }


  //-------------------------------------------
  
  if (ts.touched()) {
    // Empty the touchscreen buffer  
    while ( ! ts.bufferEmpty() ) {        
       p = ts.getPoint();
    }
    ts.writeRegister8(STMPE_INT_STA, 0xFF);    // clear the 'touched' interrupts  
    delay(2);  
  }
  
  p.x = p.y = -1; 


}








//=====================================================================
//=====================================================================
void setup() {  
  Serial.begin(115200);
  delay(500);

  //---------------------------------------------------------
  Adafruit_HX8357_ini(1);  // init function in lib <TFT_HX3857.h>  
  
  BACKGROUND = BLACK;
  tft.fillScreen(BACKGROUND);      
  tft.setTextSize(2);  

  delay(500);
  tft.setTextColor(WHITE); 
  //tft.setFont(&FreeMono9pt7b);  
  Serial.println("TFT Setup done!");
  Serial.println();
  tft.setCursor(0, 10); tft.print("TFT Setup done!");

  //---------------------------------------------------------
  tft.fillScreen(BACKGROUND);  
   
  //TS buttons
  TSbutton1.initButton(&tft, tft.width() - 37, 20, 76, 40, CYAN, BLUE, YELLOW, "Btn1", 2);
  TSbutton1.drawButton();

  TSbutton2.initButton(&tft, tft.width() - 37, 100, 76, 40, CYAN, BLUE, YELLOW, "Btn2", 2);
  TSbutton2.drawButton();

  TSbutton3.initButton(&tft, tft.width() - 37,180, 76, 40, CYAN, BLUE, YELLOW, "Btn3", 2);
  TSbutton3.drawButton();
   
    
  //---------------------------------------------------------
 
  Wire.begin();   
  Wire.setClock(400000);
 
  ads0.begin();
     
}


//=====================================================================
//=====================================================================

void loop() {
  char str1[30], str2[30];
  uint16_t adc00, adc01, adc02, adc03;

  GetTSbuttons();

   
  adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
  delay(1);
  adc01 = ads0.readADC_SingleEnded(1);  // ADS1115 port A1
  delay(1);
  adc02 = ads0.readADC_SingleEnded(2);  // ADS1115 port A2
  delay(1);
  adc03 = ads0.readADC_SingleEnded(3);  // ADS1115 port A3

  
  adc00 = mapConstrain(adc00);
  adc01 = mapConstrain(adc01);
  adc02 = 1023-mapConstrain(adc02);
  adc03 = 1023-mapConstrain(adc03);

  sprintf(str1, "ad00=%-4d ad01=%-4d ", adc00, adc01);
  sprintf(str2, "ad02=%-4d ad03=%-4d ", adc02, adc03);
  Serial.print((String)str1+str2); Serial.println();

  tft.fillRect( 0, tft.height()-14, tft.width(),14, BLACK);
  tft.setTextColor(WHITE);   
  tft.setTextSize(2);
  tft.setFont();
  tft.setCursor(0, tft.height()-14); 
  tft.print((String)str1+str2); 

  delay(10);

}
