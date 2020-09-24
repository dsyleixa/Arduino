// CMPS11 calibration example
// description see below

#include <Wire.h>

#define ADDRESS 0x60

void setup(){
  Wire.begin();
  Serial.begin(115200);
  delay(1000);
  Serial.println("init done");
  delay(1000);
  calibrate();
}

void loop(){
}

void calibrate(){
  Serial.println("Re-cCalibration: Factory Mode");
  delay(1000);  // 1 second before starting
  Serial.println("Start");

  Wire.beginTransmission(ADDRESS);
  Wire.write(0); //command register
  Wire.write(0x20);
  Wire.endTransmission();
  delay(25);

  Wire.beginTransmission(ADDRESS);
  Wire.write(0); //command register
  Wire.write(0x2A);
  Wire.endTransmission();
  delay(25);

  Wire.beginTransmission(ADDRESS);
  Wire.write(0); //command register
  Wire.write(0x60);  // 
  Wire.endTransmission();
  delay(25);

  Serial.println("done");
  delay(1000);

}

/*

Restoring Factory Calibration
Should you need to revert to the factory calibration then write the following to the command register in 3 separate transactions with 20ms between each transaction: 
0x20,0x2A,0x60. 
These commands must be sent in the correct sequence to restore the calibration, additionally, No other command may be issued in the middle of the sequence. 
The sequence must be sent to the command register at location 0, which means 3 separate write transactions on the I2C bus. 

(A write transaction is [start sequence] [I2C address] [register address] [command byte] [stop sequence] then a 20mS delay).



 */

