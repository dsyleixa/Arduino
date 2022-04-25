// Arduino  Brickbench
// benchmark test for SoCs and MCUs
// PL: GCC,Arduino
// Autor: (C) dsyleixa 2013-2019
//


// version 2.2  (2019-04-15)

// for TFT Adafruit HX8357 (Featherwing)
// change log:
// 2.2. G only TFT graphics test, no calculations

// 2.2.   testing both 32fp and 64fp 
// 2.1.1. 32bit fp tests vs. 64bit double (ARM/32bit cores, optional)
//        low-level bitRead/Write vs. digitalRead/Write (AVR cores, optional) 
// 2.1 GPIO r/w
// 2.0 loop counts


#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>

#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>


#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
 
#elif defined ESP32
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

// Something else!
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

Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);



#define  TimerMS() millis()

unsigned long runtime[10];


void TFTprint(char sbuf[], int16_t x, int16_t y) {
  tft.setCursor(x, y);
  tft.print(sbuf);
}


    
//--------------------------------------------
inline void displayValues() {

  char buf[120];
  tft.fillScreen(0x0000); // clrscr()

    sprintf (buf, "%3d %9ld  int_Add",    0, runtime[0]); TFTprint(buf, 0, 9);
    sprintf (buf, "%3d %9ld  int_Mult",   1, runtime[1]); TFTprint(buf, 0,18);
    sprintf (buf, "%3d %9ld  fp32_op",    2, runtime[2]); TFTprint(buf, 0,27);
    sprintf (buf, "%3d %9ld  fp64_op",    3, runtime[3]); TFTprint(buf, 0,36);
    sprintf (buf, "%3d %9ld  randomize",  4, runtime[4]); TFTprint(buf, 0,45);
    sprintf (buf, "%3d %9ld  matrx_algb", 5, runtime[5]); TFTprint(buf, 0,54);
    sprintf (buf, "%3d %9ld  arr_sort",   6, runtime[6]); TFTprint(buf, 0,63);
    sprintf (buf, "%3d %9ld  GPIO_togg",  7, runtime[7]); TFTprint(buf, 0,72);
    sprintf (buf, "%3d %9ld  Graphics",   8, runtime[8]); TFTprint(buf, 0,80);
}

//--------------------------------------------
int32_t test_TextOut(){
  int  y;
  char buf[120];
 
  for(y=0;y<10;++y) {   
    tft.fillScreen(0x0000); // clrscr()
    sprintf (buf, "%3d %9d  int_Add",    y, 1000);  TFTprint(buf, 0, 9);
    sprintf (buf, "%3d %9d  int_Mult",   0, 1010);  TFTprint(buf, 0,18);
    sprintf (buf, "%3d %9d  fp32_op",    0, 1032);  TFTprint(buf, 0,27);
    sprintf (buf, "%3d %9d  fp64_op",    0, 1064);  TFTprint(buf, 0,36);
    sprintf (buf, "%3d %9d  randomize",  0, 1040);  TFTprint(buf, 0,45);
    sprintf (buf, "%3d %9d  matrx_algb", 0, 1050);  TFTprint(buf, 0,54);
    sprintf (buf, "%3d %9d  GPIO_togg",  0, 1060);  TFTprint(buf, 0,63);
    sprintf (buf, "%3d %9d  Graphics",   0, 1070);  TFTprint(buf, 0,72);
    sprintf (buf, "%3d %9d  testing...", 0, 1080);  TFTprint(buf, 0,80);
  }
  return y;
}


//--------------------------------------------
int32_t test_graphics(){
  int y;
  char buf[120];
 
 
  for(y=0;y<10;++y) {
    tft.fillScreen(0x0000);
    sprintf (buf, "%3d", y);  TFTprint(buf, 0,80); // outcomment for downwards compatibility

    tft.drawCircle(50, 40, 10, 0xFFFF);
    tft.fillCircle(30, 24, 10, 0xFFFF);
    tft.drawLine(10, 10, 60, 60, 0xFFFF);
    tft.drawLine(50, 20, 90, 70, 0xFFFF);
    tft.drawRect(20, 20, 40, 40, 0xFFFF);
    tft.fillRect(65, 25, 20, 30, 0xFFFF);
    tft.drawCircle(70, 30, 15, 0xFFFF); 

  }
  return y;
}


//--------------------------------------------
long test(){
 unsigned long time0, x, y;
  double s;
  char  buf[120];
  int   i;
  float f;

  

  Serial.println("start test");
  tft.println("start test");
  delay(10);

  // computational benchmark tests 0...7 skipped

  // lcd display text / graphs
  time0=TimerMS();
  s=test_TextOut();  
  s=test_graphics();
  runtime[8]=TimerMS()-time0;
  sprintf (buf, "%3d %9ld  Graphics   ", 8, runtime[8]); 
  Serial.println( buf); 
  tft.println( buf);

  Serial.println();
 
  y = 0;
  for (x = 0; x < 9; ++x) {
      y += runtime[x];
  }
 
  displayValues();
  sprintf (buf, "runtime ges.:  %-9ld ", y);
  Serial.println( buf);   TFTprint(buf, 0,90);
 
  x=50000000.0/y;
  sprintf (buf, "benchmark:     %-9ld ", x);
  Serial.println( buf);   TFTprint(buf, 0,100);


  return 1;
}

//--------------------------------------------
void setup() {
 
  Serial.begin(115200);
  Serial.println("starting Serial()");
  while(!Serial);

  // Setup the LCD
  //tft.begin(HX8357D);
  tft.begin();  // for ESP32 == tft.begin(20000000);
  tft.setRotation(1);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF); tft.setTextSize(1);
  Serial.println("tft started");



  char  buf[120];
  test(); 
  sprintf (buf, "Ende brickbench");   
  Serial.println( buf);
  TFTprint(buf, 0, 110);
}

void loop() {

}



/* 

test design:
  0   int_Add     50,000,000 int +,- plus counter
  1   int_Mult    10,000,000 int *,/  plus counter
  2   fp32_ops    2,500,000 fp32 mult, transc.  plus counter
  3   fp64_ops    2,500,000 fp64 mult, transc.  plus counter (if N/A: 32bit)
  4   randomize   2,500,000 Mersenne PRNG (+ * & ^ << >>)
  5   matrx_algb  150,000 2D Matrix algebra (mult, det)
  6   arr_sort    1500 shellsort of random array[500]
  7   GPIO toggle 6,000,000 toggle GPIO r/w  plus counter
  8   Graphics    10*8 textlines + 10*8 shapes + 20 clrscr

.


Vergleichswerte (für Raspi noch nicht komplett durchgeführt):

Arduino MEGA + ILI9225 + Karlson UTFT + Arduino GPIO-r/w
  0     90244  int_Add
  1    237402  int_Mult
  2    163613  fp32_ops(float)
  3    163613  fp32_ops(float=double)
  4    158567  randomize
  5     46085  matrx_algb
  6     23052  arr_sort
  7     41569  GPIO toggle
  8     62109  Graphics   
runtime ges.:  986254
benchmark:     51




Arduino MEGA + ILI9225 + Karlson UTFT + Register bitRead/Write
  0     90238  int_Add
  1    237387  int_Mult
  2    163602  fp32_ops (float)
  3    163602  fp32_ops (float=double)
  4    158557  randomize
  5     45396  matrx_algb
  6     23051  arr_sort
  7      4528  GPIO_toggle bit r/w
  8     62106  Graphics   
runtime ges.:  948467
benchmark:     53  


Arduino MEGA + adafruit_ILI9341 Hardware-SPI  Arduino GPIO r/w
  0     90244  int_Add
  1    237401  int_Mult
  2    163612  fp32_ops (float)
  3    163612  fp32_ops (float=double)
  4    158725  randomize
  5     46079  matrx_algb
  6     23051  arr_sort
  7     41947  GPIO toggle
  8      6915  Graphics   
runtime ges.:  931586
benchmark:     54  
 
 



Arduino/Adafruit M0 + adafruit_ILI9341 Hardware-SPI  
  0      7746  int_Add
  1     15795  int_Mult
  2     89054  fp32_ops
  3    199888  fp64_ops(double)
  4     17675  randomize
  5     18650  matrx_algb
  6      6328  arr_sort
  7      9944  GPIO_toggle
  8      6752  Graphics 
runtime ges.:  371832
benchmark:     134 



Arduino DUE + adafruit_ILI9341 Hardware-SPI  
  0      4111  int_Add
  1      1389  int_Mult
  2     29124  fp32_ops(float)
  3     57225  fp64_ops(double)
  4      3853  randomize
  5      4669  matrx_algb
  6      2832  arr_sort
  7     11859  GPIO_toggle
  8      6142  Graphics   
runtime ges.:  121204 
benchmark:     413    



Arduino/Adafruit M4 + adafruit_HX8357 Hardware-SPI  
  0      2253  int_Add
  1       872  int_Mult
  2      2773  fp32_ops (float)
  3     24455  fp64_ops (double)
  4      1680  randomize
  5      1962  matrx_algb
  6      1553  arr_sort
  7      2395  GPIO_toggle
  8      4600  Graphics   
runtime ges.:  39864
benchmark:     1254 




Arduino/Adafruit ESP32 + adafruit_HX8357 Hardware-SPI 
  0      2308  int_Add
  1       592  int_Mult
  2      1318  fp32_ops
  3     14528  fp64_ops
  4       825  randomize
  5      1101  matrx_algb
  6       687  arr_sort
  7       972  GPIO_toggle
  8      3053  Graphics   
runtime ges.:  25384     
benchmark:     1969 





Vergleich Raspberry Pi 2:

Raspi 2 (v1): 4x 900MHz,  GPU 400MHz, no CPU overclock, full-HD, openVG:
  0     384  int_Add
  1     439  int_Mult
  2     (N/A, ca. 441)  fp32_ops(float)
  3     441  fp64_ops(double)
  4     399  randomize
  5     173  matrx_algb
  6     508  arr_sort
  7     823  GPIO_toggle
  8    2632  graphics
runtime ges.: ca.  6240
benchmark: ca.     8000 

*/
 
