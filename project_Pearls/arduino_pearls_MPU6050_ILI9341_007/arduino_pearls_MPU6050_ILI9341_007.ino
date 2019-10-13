// MPU6050
// ILI9341 Touch
// 32-bit float

// sources
//
// MPU6050:  
// Kalman.h  
//    Source: https://github.com/TKJElectronics/KalmanFilter  
//    no Gyro yaw
//    no IRQ Pin       
// TFT ILI9341 
// TFT Lib: Adafruit_ILI9341 link: https://github.com/adafruit/Adafruit_ILI9341 //
// Touch Lib: XPT2046   link: https://github.com/spapadim/XPT2046 //
// based on Nailbuster  link: https://nailbuster.com/?page_id=341 //

// Adafruit Arduino Libraries (c) www.adafruit.com  
// Adafruit invests time and resources providing this open source code,
// please support Adafruit and open-source hardware by purchasing
// products from Adafruit!
// Written by Limor Fried/Ladyada for Adafruit Industries.
// MIT license, all text above must be included in any redistribution

// (C) dsyleixa 2018
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.6.1 ... 1.8.5)
// protected under the friendly 
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/ //
//
// Alle Codes wurden zur Verfügung gestellt in der Hoffnung, dass sie nützlich sind, 
// Irrtümer vorbehalten, Benutzung auf eigenes Risiko,
// ohne Anspruch auf Schadenersatz, Garantie oder Gewährleistung 
// für irgendwelche eventuellen Schäden, die aus ihrer Benutzung entstehen könnten.
//
// unabhängig hiervon gelten die Lizenz-rechtlichen Besimmungen der Original-Autoren


/*
 * Wiring
 
Touch           Arduino Pin        nodeMCU
-------------------------------------------
T_IRQ           3                   (D1)
T_DO            MISO-pin  ICSP 3.3V
T_DIN           MOSI-pin  ICSP 3.3V
T_CS            4                   (D2)
T-CLK           SCK-pin   ICSP 3.3V

TFT
------
SDO(MISO)       MISI-pin  ICSP 3.3V (D6)
LED             3.3V 
SCK             SCK-pin   ICSP 3.3V (D5)
SDI(MOSI)       MOSI-pin  ICSP 3.3V (D7)
DC              9                   (D4)
Reset           Reset-pin Board
CS              10                  (D8)
GND             GND  
VCC             3.3V

SD
------
SD_SCK          SCK-pin   ICSP 3.3V
SD_MISO         MISO-pin  ICSP 3.3V
SD_MOSI         MOSI-pin  ICSP 3.3V
SD_CS           5                   (D3)

 
*/

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

//-------------------------------------------------------------------------
// MPU6050

#include <Kalman.h> // Source: https://github.com/TKJElectronics/KalmanFilter

//#define RESTRICT_PITCH // Comment out to restrict roll to ±90deg instead - please read: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf

Kalman kalmanX; // Create the Kalman instances
Kalman kalmanY;

/* IMU Data */
float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
int16_t tempRaw;

float gyroXangle, gyroYangle; // Angle calculate using the gyro only
float compAngleX, compAngleY; // Calculated angle using a complementary filter
float kalAngleX, kalAngleY; // Calculated angle using a Kalman filter

uint32_t timer;
uint8_t i2cData[14]; // Buffer for I2C data

// TODO: Make calibration routine


//-------------------------------------------------------------------------
// IMU settings

signed char pitch, roll;
int   accx, accy, accz;
float anglef;
const float GYRO_IDLE = 2.8;

//-------------------------------------------------------------------------
// TFT libs

#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>

#define BLACK       ILI9341_BLACK
#define DARKGREY    ILI9341_DARKGREY 
#define WHITE       ILI9341_WHITE
#define RED         ILI9341_RED
#define YELLOW      ILI9341_YELLOW
#define CYAN        ILI9341_CYAN
#define DARKCYAN    ILI9341_DARKCYAN
#define BLUE        ILI9341_BLUE
#define GREENYELLOW ILI9341_GREENYELLOW

// Modify the following two lines to match your hardware
// Also, update calibration parameters below, as necessary

// For the esp shield, these are the defaults:
#define TFT_DC  9 //  nodeMCU: 2        // <<<<<<<<<<<<<<<<<<<<<< adjusted
#define TFT_CS 10 //  nodeMCU: 15       // <<<<<<<<<<<<<<<<<<<<<< adjusted

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(/*cs=4*/ 4, /*irq=5*/ 3); // <<<<<<<<<<<<<<<<<<<<<< adjusted

Adafruit_GFX_Button button1, button2;   // create virtual screen buttons

// calib values determined by calibration program
int  vi1=0;      // 2000;
int  vj1=255;    //  248;
int  vi2=204;   //  224;
int  vj2=1792;   // 2000;


//---------------------------------------------------------

int16_t  RADMAX=4;

int16_t canvasx=236, canvasy=236;
int16_t offsx=RADMAX, offsy=320-240-RADMAX;

uint16_t COLOR_BKGR=WHITE; //  

typedef struct {
  float    posx=0.0, posy=0.0,
           oldx=0.0, oldy=0.0;
  float    accx=0.0, accy=0.0;      
  float    damp=1.0;          
  float    speedx=0.0, speedy=0.0;              // <<<<<<<<<<<<<<< test
  int16_t  ix=0, iy=0, oldix=0, oldiy=0, r=RADMAX;
  uint16_t color=WHITE; 
} pearl;


#define  MAXPEARLARR  120
pearl pearls[MAXPEARLARR];
int16_t  MAXPEARLS;


//-------------------------------------------------------------------------
void paint_pearl(int nr) { 
   
       
   // draw on canvas
   // if(pearls[nr].ix+offsx<=canvasx && pearls[nr].iy+offsx<=canvasy   && pearls[nr].ix>=0 && pearls[nr].iy>=0 )
   if(pearls[nr].ix < canvasx  && pearls[nr].iy < canvasy   && pearls[nr].ix  > 0 && pearls[nr].iy  > 0 )
   {
      tft.fillCircle(pearls[nr].oldix+offsx, pearls[nr].oldiy+offsy, pearls[nr].r+1, COLOR_BKGR);
      tft.fillCircle(pearls[nr].ix+offsx, pearls[nr].iy+offsy, pearls[nr].r, pearls[nr].color);

   }     
}

void reset_allpearls() {
   uint16_t nr, R, G, B;
   canvasx=240-1, canvasy=240-1;
   offsx=RADMAX, offsy=320-240;

   if(MAXPEARLS<20) RADMAX=8; 
   else if(MAXPEARLS<40) RADMAX=6;
   else if(MAXPEARLS<70) RADMAX=5;
   else RADMAX=4;
   for (nr=0; nr<MAXPEARLS; nr++) {
      pearls[nr].ix=random(RADMAX+2, canvasx-RADMAX-3);
      pearls[nr].iy=random(RADMAX+2, canvasy-RADMAX-3);
      
      pearls[nr].posx=(float)pearls[nr].ix;
      pearls[nr].posy=(float)pearls[nr].iy;
      
      pearls[nr].accx=0.0;
      pearls[nr].accy=0.0;      
      pearls[nr].damp=0.90;          // <<<<<<<<<<<<<<< test
      pearls[nr].speedx=0;
      pearls[nr].speedy=0;              
      pearls[nr].r=RADMAX;
      R = random(0,170);
      G = random(0,170);
      B = random(0,170);
      pearls[nr].color= (R>>3<<11) | (G>>2<<5) | (B>>3);

   }
}
//-------------------------------------------------------------------------


void update_pearl(int nr) {
   float dx, dy, x, y;
   int16_t i;
   bool coll=false;
   float dt;

   dt=0.50 + MAXPEARLS/150;
   
   pearls[nr].oldx=pearls[nr].posx;
   pearls[nr].oldy=pearls[nr].posy;
   pearls[nr].oldix=(int16_t)round(pearls[nr].oldx);
   pearls[nr].oldiy=(int16_t)round(pearls[nr].oldy);
    
   pearls[nr].accx=  roll;
   pearls[nr].accy=  pitch;                 
   pearls[nr].oldx=pearls[nr].posx;
   pearls[nr].oldy=pearls[nr].posy;
      
   if( abs(pearls[nr].accx) > GYRO_IDLE || abs(pearls[nr].accy) > GYRO_IDLE) {   
      pearls[nr].speedx= (pearls[nr].speedx + pearls[nr].accx *dt) ;
      pearls[nr].speedy= (pearls[nr].speedy + pearls[nr].accy *dt) ;   
   }
   pearls[nr].speedx *= pearls[nr].damp;
   pearls[nr].speedy *= pearls[nr].damp;
   
   dx =  pearls[nr].speedx *dt;
   dy =  pearls[nr].speedy *dt;

   x=pearls[nr].posx;
   y=pearls[nr].posy;

     
   if(dx>0 && pearls[nr].posx+pearls[nr].r >= canvasx) dx=0;
      else
      if(dx<0 && pearls[nr].posx-pearls[nr].r <= 0) dx=0;
      
   if(dy>0 && pearls[nr].posy+pearls[nr].r >= canvasy) dy=0;
      else
      if(dy<0 && pearls[nr].posy-pearls[nr].r <= 0) dy=0;   
      
   x += dx;        
   y += dy;    
      
   pearls[nr].speedx *= pearls[nr].damp;
   pearls[nr].speedy *= pearls[nr].damp; 

   if(x<RADMAX) x=RADMAX; if(x>canvasx-RADMAX) x=canvasx-RADMAX;
   if(y<RADMAX) y=RADMAX; if(y>canvasy-RADMAX) y=canvasy-RADMAX;

   for (i=0; i<MAXPEARLS; i++) {
         if (
            i!=nr &&  ( (abs(pearls[i].ix - (int16_t)round(x))<=RADMAX  
                  && abs(pearls[i].iy - (int16_t)round(y))<=RADMAX)  ) )
         {                         
            coll=true;       
            break;
         }
    }
    if(coll) { 
            pearls[i].speedx=  pearls[nr].speedx*pearls[nr].damp ;
            pearls[i].speedy=  pearls[nr].speedy*pearls[nr].damp ;
            pearls[nr].speedx= -pearls[nr].speedx*pearls[nr].damp/2;
            pearls[nr].speedy= -pearls[nr].speedy*pearls[nr].damp/2;
            x=pearls[nr].posx;
            y=pearls[nr].posy; 
            pearls[nr].speedx*= -1 * pearls[nr].damp;  
            pearls[nr].speedy*= -1 * pearls[nr].damp;             
    }

    pearls[nr].posx=x;
    pearls[nr].posy=y;

     // calc new TFT pos
    pearls[nr].ix=(int16_t)round(pearls[nr].posx);
    pearls[nr].iy=(int16_t)round(pearls[nr].posy);  
    

     
    
}



//-------------------------------------------------------------------------
static void calibratePoint(uint16_t x, uint16_t y, uint16_t &vi, uint16_t &vj) {
  // Draw cross
  
  
  tft.drawFastHLine(x - 8, y, 16,ILI9341_YELLOW);
  tft.drawFastVLine(x, y - 8, 16,ILI9341_YELLOW);
  while (!touch.isTouching()) {
    delay(10);
  }
  touch.getRaw(vi, vj);
  // Erase by overwriting with black
  tft.drawFastHLine(x - 8, y, 16, 0);
  tft.drawFastVLine(x, y - 8, 16, 0);
}

void calibrate() {
  uint16_t x1, y1, x2, y2;
  uint16_t vi1, vj1, vi2, vj2;
  touch.getCalibrationPoints(x1, y1, x2, y2);
  calibratePoint(x1, y1, vi1, vj1);
  delay(1000);
  calibratePoint(x2, y2, vi2, vj2);
   
  touch.setCalibration(vi1, vj1, vi2, vj2);
  
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(2);
  tft.println("Calibration Params");
  tft.println("");
  tft.setTextSize(3);
  tft.println(vi1);  // e.g. 127
  tft.println(vj1);  // e.g. 848
  tft.println(vi2);  // e.g. 1864
  tft.println(vj2);  // e.g. 255
}











//-------------------------------------------------------------------------
void calibrate_default() {
   tft.fillScreen(COLOR_BKGR); 
   //touch.setCalibration(0, 127, 2047, 1536);   
   touch.setCalibration(1896, 127, 127, 1856);  //old tft
}













//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void setup() {
  int i;
  Wire.begin();
  Serial.begin(115200);
  delay(1000);
  Serial.println("Serial started");

  init_MPU6050();

    
  tft.begin();
  tft.setTextSize(2);
  
  touch.begin(tft.width(), tft.height());  // Must be done before setting rotation
  tft.fillScreen(BLACK);
  tft.println("Calibration");
  delay(1000);
  tft.fillScreen(BLACK);
  //calibrate();
  calibrate_default();
  delay(2000);  
   
  //Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
  tft.fillScreen(COLOR_BKGR);
  
  // create button
  button1.initButton(&tft, 40, 20, 70, 40, DARKCYAN, BLUE, GREENYELLOW, "Reset", 2);
  button1.drawButton();  
  // create button
  button2.initButton(&tft, 200, 20, 70, 40, DARKCYAN, BLUE, GREENYELLOW, "B/W", 2);
  button2.drawButton();  

  randomSeed(analogRead(A0) );
  MAXPEARLS=random(1,MAXPEARLARR-1);
  reset_allpearls();

  for (i=0; i<MAXPEARLS; i++) {
     paint_pearl(i);
  }
}


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
static uint16_t prev_x = 0xffff, prev_y = 0xffff;

//-------------------------------------------------------------------------


void loop() {
  static uint16_t x=100, y=100;
  int i;
  char buf[20]="", buf1[20]="";
  float fyaw, fpitch, froll;


  read_MPU6050(fyaw, fpitch, froll);
  anglef=fyaw;
  pitch=(int8_t)fpitch;
  roll = -(int8_t)froll;
  
  delay(3);

  for (i=0; i<MAXPEARLS; i++) {
     update_pearl(i);
  }
  for (i=0; i<MAXPEARLS; i++) {
     paint_pearl(i);
  }
  
  //debug
  /*
  sprintf(buf, "d%3d r%+3d p%+3d n%-3d", (int)anglef, roll, pitch, MAXPEARLS);
  Serial.println();
  Serial.println(buf); 
  tft.setTextColor(RED); 
  tft.fillRect(0,40, 239,30, COLOR_BKGR);  // x1,y1, dx,dy    
  tft.setCursor(0,45);    tft.println(buf);   
   */
  
  if (touch.isTouching()) {
    touch.getPosition(x, y); 
    if(x>tft.width()-1  && x<0xffff) x=tft.width()-1;
    if(y>tft.height()-1 && y<0xffff) y=tft.height()-1;  

    //while (touch.isTouching());
    
    //debug
    /*
    Serial.println();
    sprintf(buf, "x=%3d y=%3d",x,y);      
    Serial.println(buf);    Serial.println();
    sprintf(buf, "x=%3d",x); sprintf(buf1, "y=%3d",y);
    tft.setTextColor(RED); 
    tft.fillRect(80,0, 80,60, COLOR_BKGR);  // x1,y1, dx,dy 
    tft.setCursor(85,0);     
    tft.setCursor(85,20); 
    tft.println(buf);
    tft.println(buf1);
    */
       
    delay(1);   
    
    prev_x = x;
    prev_y = y;
    
  }   
  else   {
    prev_x = prev_y = 0xffff;
  }  
  
  button1.press(button1.contains(x, y)); // tell the button it is pressed
  button2.press(button2.contains(x, y)); // tell the button2 it is pressed

  delay(1);
  
// now we can ask the buttons if their state has changed
  
  
  if (button1.justReleased()) {
     tft.fillScreen(COLOR_BKGR);
     button1.drawButton(); // draw normal
     button2.drawButton(); // draw normal
     MAXPEARLS=random(1,MAXPEARLARR-1);
     reset_allpearls();
      x = y = 0xffff;
  }
  if (button1.justPressed()) {
     button1.drawButton(true); // draw invert!
      x = y = 0xffff;
  }

  
   if (button2.justPressed()) {
     button2.drawButton(true); // draw invert!
      x = y = 0xffff;
  }
  if (button2.justReleased()) {
     if(COLOR_BKGR==WHITE) COLOR_BKGR=BLACK;
     else COLOR_BKGR=WHITE;
     
     tft.fillScreen(COLOR_BKGR);
        
     button1.drawButton(); // draw normal
     button2.drawButton(); // draw normal
      x = y = 0xffff;
  }

  delay(1);
}



//-------------------------------------------------------------------------
// MPU6050 functions
void init_MPU6050() {   
  i2cData[0] = 7; // Set the sample rate to 1000Hz - 8kHz/(7+1) = 1000Hz
  i2cData[1] = 0x00; // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
  i2cData[2] = 0x00; // Set Gyro Full Scale Range to ±250deg/s
  i2cData[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g
  while (i2cWrite(0x19, i2cData, 4, false)); // Write to all four registers at once
  while (i2cWrite(0x6B, 0x01, true)); // PLL with X axis gyroscope reference and disable sleep mode

  while (i2cRead(0x75, i2cData, 1));
  if (i2cData[0] != 0x68) { // Read "WHO_AM_I" register
    Serial.print(F("Error reading sensor"));
    while (1);  }

  delay(100); // Wait for sensor to stabilize

  /* Set kalman and gyro starting angle */
  while (i2cRead(0x3B, i2cData, 6));
  accX = (int16_t)((i2cData[0] << 8) | i2cData[1]);
  accY = (int16_t)((i2cData[2] << 8) | i2cData[3]);
  accZ = (int16_t)((i2cData[4] << 8) | i2cData[5]);

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
  
#ifdef RESTRICT_PITCH // Eq. 25 and 26  // default: not defined
  float roll  = atan2(accY, accZ) * RAD_TO_DEG;
  float pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
#else // Eq. 28 and 29                  // default: defined
  float roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  float pitch = atan2(-accX, accZ) * RAD_TO_DEG;
#endif

  kalmanX.setAngle(roll); // Set starting angle
  kalmanY.setAngle(pitch);
  gyroXangle = roll;
  gyroYangle = pitch;
  compAngleX = roll;
  compAngleY = pitch;

  timer = micros();  
}


void read_MPU6050(float &yaw, float &pitch, float &roll) {
  /* Update all the values */
  while (i2cRead(0x3B, i2cData, 14));
  accX = (int16_t)((i2cData[0] << 8) | i2cData[1]);
  accY = (int16_t)((i2cData[2] << 8) | i2cData[3]);
  accZ = (int16_t)((i2cData[4] << 8) | i2cData[5]);
  tempRaw = (int16_t)((i2cData[6] << 8) | i2cData[7]);
  gyroX = (int16_t)((i2cData[8] << 8) | i2cData[9]);
  gyroY = (int16_t)((i2cData[10] << 8) | i2cData[11]);
  gyroZ = (int16_t)((i2cData[12] << 8) | i2cData[13]);;

  float dt = (float)(micros() - timer) / 1000000; // Calculate delta time
  timer = micros();

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
  
  #ifdef RESTRICT_PITCH // Eq. 25 and 26          // default: not defined
  roll  = atan2(accY, accZ) * RAD_TO_DEG;
  pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
  #else // Eq. 28 and 29                          // default: not defined
  roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  pitch = atan2(-accX, accZ) * RAD_TO_DEG;
  #endif

  float gyroXrate = gyroX / 131.0; // Convert to deg/s
  float gyroYrate = gyroY / 131.0; // Convert to deg/s

#ifdef RESTRICT_PITCH            // default: not defined
  // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if ((roll < -90 && kalAngleX > 90) || (roll > 90 && kalAngleX < -90)) {
    kalmanX.setAngle(roll);
    compAngleX = roll;
    kalAngleX = roll;
    gyroXangle = roll;
  } else                        // default: not defined
    kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter

  if (abs(kalAngleX) > 90)
    gyroYrate = -gyroYrate; // Invert rate, so it fits the restriced accelerometer reading
  kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt);
#else
  // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if ((pitch < -90 && kalAngleY > 90) || (pitch > 90 && kalAngleY < -90)) {
    kalmanY.setAngle(pitch);
    compAngleY = pitch;
    kalAngleY = pitch;
    gyroYangle = pitch;
  } else
    kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt); // Calculate the angle using a Kalman filter

  if (abs(kalAngleY) > 90)
    gyroXrate = -gyroXrate; // Invert rate, so it fits the restriced accelerometer reading
  kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter
#endif

  gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter
  gyroYangle += gyroYrate * dt;
  //gyroXangle += kalmanX.getRate() * dt; // Calculate gyro angle using the unbiased rate
  //gyroYangle += kalmanY.getRate() * dt;

  compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * roll; // Calculate the angle using a Complimentary filter
  compAngleY = 0.93 * (compAngleY + gyroYrate * dt) + 0.07 * pitch;

  // Reset the gyro angle when it has drifted too much
  if (gyroXangle < -180 || gyroXangle > 180)
    gyroXangle = kalAngleX;
  if (gyroYangle < -180 || gyroYangle > 180)
    gyroYangle = kalAngleY;
}




//-------------------------------------------------------------------------
// EOF
//-------------------------------------------------------------------------
