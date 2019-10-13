// rolling  balls and pearls
// 32-bit float

// sources
// MPU6050:
// Kalman.h
//    Source: https://github.com/TKJElectronics/KalmanFilter
//    no Gyro yaw
//    no IRQ Pin
//
// TFT HX8357:  Adafruit_HX8357.h
// Touchscreen: Adafruit_STMPE610.h
// Adafruit Arduino Libraries (c) www.adafruit.com  
// Adafruit invests time and resources providing this open source code,
// please support Adafruit and open-source hardware by purchasing
// products from Adafruit!
// Written by Limor Fried/Ladyada for Adafruit Industries.
// MIT license, all text above must be included in any redistribution

// (C) dsyleixa 2018, 2019
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.8.8)
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



//#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>


// TFT drivers
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>

//----------------------------------------------------
// fonts
//----------------------------------------------------
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>


//----------------------------------------------------
// TFT shield pins
//----------------------------------------------------
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

#elif defined ARDUINO_FEATHER52
#define STMPE_CS 30
#define TFT_CS   13
#define TFT_DC   11
#define SD_CS    27

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

//----------------------------------------------------
// TFT + TS create instance
//----------------------------------------------------
Adafruit_HX8357   tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610  ts = Adafruit_STMPE610(STMPE_CS);


//----------------------------------------------------
// screen canvas window + colors
//----------------------------------------------------
int16_t  RADMAX = 4;
int16_t  offsx = RADMAX, offsy = 0;
int16_t  canvasx = 400 - 1, canvasy = 320 - offsy - 1;

#define BLACK       HX8357_BLACK
#define DARKGREY    HX8357_DARKGREY
#define WHITE       HX8357_WHITE
#define RED         HX8357_RED
#define YELLOW      HX8357_YELLOW
#define CYAN        HX8357_CYAN
#define BLUE        HX8357_BLUE

uint32_t COLOR_BKGR = WHITE; // 

//=========================================================================
// ADS1015 ADC
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads0(0x48);
Adafruit_ADS1115 ads1(0x49);

//=========================================================================
// MPU6050

signed char pitch, roll;
int accx, accy, accz;


#include <Kalman.h> // Source: https://github.com/TKJElectronics/KalmanFilter
//#define RESTRICT_PITCH // Comment out to restrict roll to ±90deg instead - please read: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf
Kalman kalmanX; // Create the Kalman instances
Kalman kalmanY;

//  IMU Data
float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
int16_t tempRaw;

const uint8_t IMUAddress = 0x68;   // AD0 is logic low on the PCB
const uint16_t I2C_TIMEOUT = 1000; // Used to check for errors in I2C communication

float gyroXangle, gyroYangle; // Angle calculate using the gyro only
float compAngleX, compAngleY; // Calculated angle using a complementary filter
float kalAngleX, kalAngleY;   // Calculated angle using a Kalman filter

uint32_t timer;
uint8_t i2cData[14]; // Buffer for I2C data

// TODO: Make calibration routine



//-------------------------------------------------------------------------
uint8_t i2cWrite(uint8_t registerAddress, uint8_t data, bool sendStop) {
  return i2cWrite(registerAddress, &data, 1, sendStop); // Returns 0 on success
}

//-------------------------------------------------------------------------
uint8_t i2cWrite(uint8_t registerAddress, uint8_t *data, uint8_t length, bool sendStop) {
  Wire.beginTransmission(IMUAddress);
  Wire.write(registerAddress);
  Wire.write(data, length);
  uint8_t rcode = Wire.endTransmission(sendStop); // Returns 0 on success
  if (rcode) {
    Serial.print(F("i2cWrite failed: "));
    Serial.println(rcode);
  }
  return rcode; // See: http://arduino.cc/en/Reference/WireEndTransmission
}

//-------------------------------------------------------------------------
uint8_t i2cRead(uint8_t registerAddress, uint8_t *data, uint8_t nbytes) {
  uint32_t timeOutTimer;
  Wire.beginTransmission(IMUAddress);
  Wire.write(registerAddress);
  uint8_t rcode = Wire.endTransmission(false); // Don't release the bus
  if (rcode) {
    Serial.print(F("i2cRead failed: "));
    Serial.println(rcode);
    return rcode; // See: http://arduino.cc/en/Reference/WireEndTransmission
  }
  Wire.requestFrom(IMUAddress, nbytes, (uint8_t)true); // Send a repeated start and then release the bus after reading
  for (uint8_t i = 0; i < nbytes; i++) {
    if (Wire.available())
      data[i] = Wire.read();
    else {
      timeOutTimer = micros();
      while (((micros() - timeOutTimer) < I2C_TIMEOUT) && !Wire.available());
      if (Wire.available())
        data[i] = Wire.read();
      else {
        Serial.println(F("i2cRead timeout"));
        return 5; // This error value is not already taken by endTransmission
      }
    }
  }
  return 0; // Success
}

//-------------------------------------------------------------------------
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
#ifdef RESTRICT_PITCH // Eq. 25 and 26
  roll  = atan2(accY, accZ) * RAD_TO_DEG;
  pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
#else // Eq. 28 and 29
  roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  pitch = atan2(-accX, accZ) * RAD_TO_DEG;
#endif

  float gyroXrate = gyroX / 131.0; // Convert to deg/s
  float gyroYrate = gyroY / 131.0; // Convert to deg/s

#ifdef RESTRICT_PITCH
  // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if ((roll < -90 && kalAngleX > 90) || (roll > 90 && kalAngleX < -90)) {
    kalmanX.setAngle(roll);
    compAngleX = roll;
    kalAngleX = roll;
    gyroXangle = roll;
  } else
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

  //#define NO_KALMAN
#ifdef NO_KALMAN
  gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter
  gyroYangle += gyroYrate * dt;
#else
  // Calculate gyro angle using the unbiased rate
  gyroXangle += kalmanX.getRate() * dt;
  gyroYangle += kalmanY.getRate() * dt;
#endif

  compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * roll; // Calculate the angle using a Complimentary filter
  compAngleY = 0.93 * (compAngleY + gyroYrate * dt) + 0.07 * pitch;

  // Reset the gyro angle when it has drifted too much
  if (gyroXangle < -180 || gyroXangle > 180)
    gyroXangle = kalAngleX;
  if (gyroYangle < -180 || gyroYangle > 180)
    gyroYangle = kalAngleY;
}

//-------------------------------------------------------------------------
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
    while (1);
  }

  delay(100); // Wait for sensor to stabilize

  /* Set kalman and gyro starting angle */
  while (i2cRead(0x3B, i2cData, 6));
  accX = (int16_t)((i2cData[0] << 8) | i2cData[1]);
  accY = (int16_t)((i2cData[2] << 8) | i2cData[3]);
  accZ = (int16_t)((i2cData[4] << 8) | i2cData[5]);

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf 
#ifdef RESTRICT_PITCH // Eq. 25 and 26
  float roll  = atan2(accY, accZ) * RAD_TO_DEG;
  float pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
#else // Eq. 28 and 29
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



//=========================================================================
//-------------------------------------------------------------------------
// balls + pearls
//-------------------------------------------------------------------------

typedef struct {
  float    posx = 0.0, posy = 0.0,
           oldx = 0.0, oldy = 0.0;
  float    accx = 0.0, accy = 0.0;
  float    damp = 1.0;
  float    speedx = 0.0, speedy = 0.0;          // <<<<<<<<<<<<<<< test
  int16_t  ix = 0, iy = 0, oldix = 0, oldiy = 0, r = RADMAX;
  uint16_t color = WHITE;
} tPearl;


#define  MAXPEARLARR  150
tPearl   pearls[MAXPEARLARR];
int16_t  NumberOfPearls;

const float GYRO_IDLE = 2.8;


//-------------------------------------------------------------------------
void paint_pearl(int nr) {
  // draw on canvas
  // if(pearls[nr].ix+offsx<=canvasx && pearls[nr].iy+offsx<=canvasy   && pearls[nr].ix>=0 && pearls[nr].iy>=0 )

  if (pearls[nr].ix < canvasx  && pearls[nr].iy < canvasy   && pearls[nr].ix  > 0 && pearls[nr].iy  > 0 )
  {
    tft.fillCircle(pearls[nr].oldix + offsx, pearls[nr].oldiy + offsy, pearls[nr].r + 1, COLOR_BKGR);
    tft.fillCircle(pearls[nr].ix + offsx, pearls[nr].iy + offsy, pearls[nr].r, pearls[nr].color);

  }
}

//-------------------------------------------------------------------------
void reset_allpearls() {
  uint16_t nr, R, G, B;

  if (NumberOfPearls < 20) RADMAX = 8;
  else if (NumberOfPearls < 40) RADMAX = 6;
  else if (NumberOfPearls < 70) RADMAX = 5;
  else RADMAX = 4;
  for (nr = 0; nr < NumberOfPearls; nr++) {
    pearls[nr].ix = random(RADMAX + 2, canvasx - RADMAX - 3);
    pearls[nr].iy = random(RADMAX + 2, canvasy - RADMAX - 3);

    pearls[nr].posx = (float)pearls[nr].ix;
    pearls[nr].posy = (float)pearls[nr].iy;

    pearls[nr].accx = 0.0;
    pearls[nr].accy = 0.0;
    pearls[nr].damp = 0.9;        // <<<<<<<<<<<<<<< test
    pearls[nr].speedx = 0;
    pearls[nr].speedy = 0;
    pearls[nr].r = RADMAX;
    R = random(0, 170);
    G = random(0, 170);
    B = random(0, 170);
    pearls[nr].color = (R >> 3 << 11) | (G >> 2 << 5) | (B >> 3);

    //pearls[nr].vis=true;
  }
}

//-------------------------------------------------------------------------
void update_pearl(int nr) {
  float dx, dy, x, y;
  int16_t i;
  bool coll = false;
  float dt;

  dt = 0.5 + (float)NumberOfPearls / (2 * (float)MAXPEARLARR);

  pearls[nr].oldx = pearls[nr].posx;
  pearls[nr].oldy = pearls[nr].posy;
  pearls[nr].oldix = (int16_t)round(pearls[nr].oldx);
  pearls[nr].oldiy = (int16_t)round(pearls[nr].oldy);

  pearls[nr].accx =  roll;
  pearls[nr].accy =  pitch;
  pearls[nr].oldx = pearls[nr].posx;
  pearls[nr].oldy = pearls[nr].posy;

  if ( abs(pearls[nr].accx) > GYRO_IDLE || abs(pearls[nr].accy) > GYRO_IDLE) {
    pearls[nr].speedx = (pearls[nr].speedx + pearls[nr].accx * dt) ;
    pearls[nr].speedy = (pearls[nr].speedy + pearls[nr].accy * dt) ;
  }
  pearls[nr].speedx *= pearls[nr].damp;
  pearls[nr].speedy *= pearls[nr].damp;

  dx =  pearls[nr].speedx * dt;
  dy =  pearls[nr].speedy * dt;

  x = pearls[nr].posx;
  y = pearls[nr].posy;

  // test, debug
  if (dx > 0 && pearls[nr].posx + pearls[nr].r >= canvasx) { dx = 0; }
  else if (dx < 0 && pearls[nr].posx - pearls[nr].r <= 0)  { dx = 0; }

  if (dy > 0 && pearls[nr].posy + pearls[nr].r >= canvasy) { dy = 0;}
  else if (dy < 0 && pearls[nr].posy - pearls[nr].r <= 0)  { dy = 0;}

  x += dx;
  y += dy;

  pearls[nr].speedx *= pearls[nr].damp;
  pearls[nr].speedy *= pearls[nr].damp;

  if (x < RADMAX) x = RADMAX; if (x > canvasx - RADMAX) x = canvasx - RADMAX;
  if (y < RADMAX) y = RADMAX; if (y > canvasy - RADMAX) y = canvasy - RADMAX;

  for (i = 0; i < NumberOfPearls; i++) {
    if (
      i != nr &&  ( (abs(pearls[i].ix - (int16_t)round(x)) <= RADMAX
                     && abs(pearls[i].iy - (int16_t)round(y)) <= RADMAX)  ) )
    {
      coll = true;
      break;
    }
  }
  if (coll) {
    pearls[i].speedx =  pearls[nr].speedx * pearls[nr].damp ;
    pearls[i].speedy =  pearls[nr].speedy * pearls[nr].damp ;
    pearls[nr].speedx = -pearls[nr].speedx * pearls[nr].damp / 2.0;
    pearls[nr].speedy = -pearls[nr].speedy * pearls[nr].damp / 2.0;
    x = pearls[nr].posx;
    y = pearls[nr].posy;
    pearls[nr].speedx *= -1 * pearls[nr].damp;
    pearls[nr].speedy *= -1 * pearls[nr].damp;
  }

  pearls[nr].posx = x;
  pearls[nr].posy = y;

  // calc new TFT pos
  pearls[nr].ix = (int16_t)round(pearls[nr].posx);
  pearls[nr].iy = (int16_t)round(pearls[nr].posy);
}



//=========================================================================
// touch screen buttons

// Touch Calibration
int TS_MINX = 100;
int TS_MAXX = 3800;
int TS_MINY = 100;
int TS_MAXY = 3750;

Adafruit_GFX_Button button1, button2, button3 ;



//=========================================================================
// setup
//=========================================================================
void setup() {
  int i;

  Wire.begin();

  Serial.begin(115200);
  delay(1000);

  init_MPU6050();
  
  ads0.begin();
  //ads0.setGain(GAIN_TWOTHIRDS);

  tft.begin(HX8357D);
  tft.setRotation(1);
  tft.fillScreen(COLOR_BKGR);
  tft.setTextSize(2);
  //tft.setFont(&FreeMono9pt7b);


  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");


  // create button
  button1.initButton(&tft, tft.width() - 37, 20, 76, 40, CYAN, BLUE, YELLOW, "Random", 2);
  button1.drawButton();

  button2.initButton(&tft, tft.width() - 37, 90, 76, 40, CYAN, BLUE, YELLOW, "Enter", 2);
  button2.drawButton();

  button3.initButton(&tft, tft.width() - 37, 220, 76, 40, CYAN, BLUE, YELLOW, "B/W", 2);
  button3.drawButton();

  randomSeed(analogRead(A0) + (int)millis() );
  NumberOfPearls = random(1, MAXPEARLARR - 1);
  reset_allpearls();

  for (i = 0; i < NumberOfPearls; i++) {
    paint_pearl(i);
  }
}


TS_Point p;

//=========================================================================
// loop
//=========================================================================

void loop() {
  int i;
  float fyaw, fpitch, froll;
  char buf[20] = "", buf1[20] = "";
  static int _MODE_ = 1;
  static uint32_t lastmillis = 0;
  int PotiRX, PotiRY, Touchpad;

  //-------------------------------------------
  // Potentiometer  analogRead(), ads0.readADC_SingleEnded()
  Serial.println("analogRead");
  Touchpad = analogRead(A2);
  Touchpad = map(Touchpad, 0,4095,  0,1023);
  // PotiRX = (1023 - analogRead(A0));
  // PotiRY = (1023 - analogRead(A1));  
  //map(value, fromLow, fromHigh, toLow, toHigh) 
  
  //PotiRX=0; PotiRY=0;  
  //PotiRX = ads0.readADC_SingleEnded(0);
  //PotiRX = map(PotiRX, 115,17303,  0,1023);
  //delay(1);
  PotiRY = ads0.readADC_SingleEnded(1); 
  //PotiRY = map(PotiRY, 115,17303,  1023,0);
  PotiRY = map(PotiRY, 115,17303,  MAXPEARLARR-1,0);
  if(PotiRY<3) PotiRY=3; 
    
  //monitor + debug
  Serial.println(String("PotiRX=")+PotiRX + "  PotiRY="+PotiRY + + "  Touchpad="+Touchpad);
  tft.setTextColor(RED);
  tft.fillRect(tft.width()-62, 115, 100, 80, COLOR_BKGR); //  
  tft.setCursor(tft.width()-60, 117);    
  //tft.print(PotiRX);
  //tft.setCursor(tft.width()-60, 137);     
  tft.print(PotiRY);
  tft.setCursor(tft.width()-60, 157); 
  tft.print(Touchpad);
  delay(1);  // yield
  
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
  sprintf(buf, "x=%3d", p.x); sprintf(buf1, "y=%3d", p.y);

  tft.setTextColor(RED);
  
  //tft.fillRect(tft.width()-80,40, 80,40, COLOR_BKGR);  // x1,y1, dx,dy
  //tft.setCursor(tft.width()-75,40);    tft.print(buf);
  //tft.setCursor(tft.width()-75,60);    tft.print(buf1);   

  //-------------------------------------------
  // check if a button was pressed
  //-------------------------------------------
  button1.press(button1.contains(p.y, p.x)); // tell the button it is pressed
  button2.press(button2.contains(p.y, p.x)); // tell the button it is pressed
  button3.press(button3.contains(p.y, p.x)); // tell the button it is pressed

  //-------------------------------------------
  // process button states + changes
  //-------------------------------------------
  
  Serial.println("buttons");
  
  if (button1.justReleased() ) {
    
    _MODE_ = 1;
    tft.fillScreen(COLOR_BKGR);
    NumberOfPearls = random(1, MAXPEARLARR - 1);
    reset_allpearls();
    button1.drawButton();
    button2.drawButton();
    button3.drawButton();    
  }
  
  if (button1.justPressed()) {
    button1.drawButton(true); // draw invert!
  }
  //-------------------------------------------  
  
  
  if (button2.justReleased() ) {
    _MODE_ = 2;
    tft.fillScreen(COLOR_BKGR);
    NumberOfPearls = PotiRY;
    reset_allpearls();
    button1.drawButton();
    button2.drawButton();
    button3.drawButton();
  }
  
  if (button2.justPressed()) {
  button2.drawButton(true); // draw invert!
  }
  //-------------------------------------------
  
  
  if (button3.justReleased() ) {
    _MODE_ = 2;
    if(COLOR_BKGR==WHITE) COLOR_BKGR = BLACK;
    else COLOR_BKGR = WHITE;
    
    tft.fillScreen(COLOR_BKGR);
    button1.drawButton();
    button2.drawButton();
    button3.drawButton();
  } 
  
  if (button3.justPressed()) {
    button3.drawButton(true); // draw invert!
  }

  //-------------------------------------------

  if (ts.touched()) {
    // Empty the touchscreen buffer  
    while ( ! ts.bufferEmpty() ) {        
       p = ts.getPoint();
    }
    ts.writeRegister8(STMPE_INT_STA, 0xFF);    // clear the 'touched' interrupts    
  }
  p.x = p.y = -1; 

  //-------------------------------------------
  // action!
  //-------------------------------------------
  Serial.println("action");
  
  if (_MODE_ < 3) {
    // IMU readings
    Serial.println("read MPU6050");
    read_MPU6050(fyaw, fpitch, froll);
    pitch = (int16_t)fpitch;
    roll = -(int16_t)froll;
    Serial.println("MPU6050 ok");
    delay(1);    // 3

    // apply IMU to balls'n'pearls
    Serial.println("update pearls");
    for (i = 0; i < NumberOfPearls; i++) {
      update_pearl(i);
    }
    Serial.println("paint pearls");
    for (i = 0; i < NumberOfPearls; i++) {
      paint_pearl(i);
    }
    sprintf(buf, "y%3d p%+3d r%+3d n%-3d", (int16_t)fyaw, pitch, roll, NumberOfPearls);
    Serial.println();
    Serial.println(buf);
    //tft.setTextColor(RED);
    //tft.fillRect(0,40, 239,30, COLOR_BKGR);  // x1,y1, dx,dy
    //tft.setCursor(0,45);    tft.println(buf);
  }



  
  else if (_MODE_ == 3) {
     if(millis()-lastmillis >= 999) {
        //displayTime(); // display the real-time clock data on Monitor and LCD 
        Serial.println("mode 3 test");
        lastmillis = millis();
        // debug: reset mode
        _MODE_=2;
     }
  }
  
  //------------------------------------------- 
  
  delay(1);  // yield

}


//-------------------------------------------------------------------------
// EOF
//-------------------------------------------------------------------------
