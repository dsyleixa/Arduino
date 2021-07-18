// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class using DMP (MotionApps v6.12)
// 6/21/2012 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//      2020-02-09 - IRQs erased, calibration simplified by dsyleixa
//      2020-02-02 - DMP6 Simplified Based on MPU6050_DMP6_using_DMP_V6.12


/* ============================================
  I2Cdev device library code is placed under the MIT license
  Copyright (c) 2012 Jeff Rowberg
  ===============================================
*/


#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"

#include "Wire.h"

#define LED_PIN LED_BUILTIN  // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;


// *****************  Setup  *****************

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69

MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

// MPU control/status vars
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 gy;         // [x, y, z]            gyro sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };




// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties


  Serial.begin(115200);
  delay(1000);

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  Serial.println(F("keep the mpu in a horizontal position, don't move it...!"));
  
  mpu.initialize();

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  
  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  mpu.dmpInitialize();

  // We can now access raw data from the MPU

 
  Serial.println(F("Setting initial Offsets..."));
  Serial.println(F("Each MPU is unique and must have its own offsets."));
  Serial.println(F("The following Calibration Routine Generates offsets for you to replace the offsets represented here"));
  // supply your own gyro offsets here. 
  // NOTE: Your offsets are different than the ones shown below! In face Each MPU is different.

  //           X Accel  Y Accel  Z Accel   X Gyro   Y Gyro   Z Gyro
  //OFFSETS    -2102,   -3776,     836,    -345,     -31,      31

  mpu.setXAccelOffset(-2102);
  mpu.setYAccelOffset(-3776);
  mpu.setZAccelOffset(836);
  mpu.setXGyroOffset(-345);
  mpu.setYGyroOffset(-31);
  mpu.setZGyroOffset(31);
 
  Serial.println(F("Calibrating Offsets..."));
  // Calibration Time: generate offsets and calibrate our MPU6050
  Serial.println(F("Calibrating Accelerometer..."));
  mpu.CalibrateAccel(6);
  Serial.println(F("\n\nCalibrating Gyro..."));
  mpu.CalibrateGyro(6);
  Serial.println();
  Serial.println(F("Replace initial offsets with these offsets..."));
  mpu.PrintActiveOffsets();

  // turn on the DMP, now that it's ready
  Serial.println(F("Enabling DMP..."));
  mpu.setDMPEnabled(true);
  // Digital Motion Processing Firmware is now enabled

  // configure LED for output
  pinMode(LED_PIN, OUTPUT);
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {
  // read a packet from FIFO
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet

    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    Serial.print("ypr\t");
    Serial.print(ypr[0] * 180 / M_PI);
    Serial.print("\t");
    Serial.print(ypr[1] * 180 / M_PI);
    Serial.print("\t");
    Serial.print(ypr[2] * 180 / M_PI);
    
    Serial.println();

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
  }
}
