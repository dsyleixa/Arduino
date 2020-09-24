// CMPS11 calibration example
// description see below

#include <Wire.h>

#define ADDRESS 0x60

void setup(){
  Wire.begin();
  Serial.begin(115200);
  calibrate();
}

void loop(){
}

void calibrate(){
  Serial.println("Calibration Mode");
  delay(2000);  //2 second before starting
  Serial.println("Start");

  Wire.beginTransmission(ADDRESS);
  Wire.write(0); //command register
  Wire.write(0xF0);
  Wire.endTransmission();
  delay(25);

  Wire.beginTransmission(ADDRESS);
  Wire.write(0); //command register
  Wire.write(0xF5);
  Wire.endTransmission();
  delay(25);

  Wire.beginTransmission(ADDRESS);
  Wire.write(0); //command register
  Wire.write(0xF6);  // 
  Wire.endTransmission();
  delay(20000);

  Wire.beginTransmission(ADDRESS);
  Wire.write(0); //command register
  Wire.write(0xF8);
  Wire.endTransmission();
  Serial.println("done");
}

/*
 * 
Documentation
http://www.robot-electronics.co.uk/htm/cmps11i2c.htm 

Please do not do this until you have I2C communication fully working. 
I would recommend evaluating the CMPS11 performance first before implementing this function. 
Its purpose is to remove sensor gain and offset of both magnetometer and accelerometer 
and achieves this by looking for maximum sensor outputs. 
First of all you need to enter the calibration mode by sending a 3 byte sequence of 

0xF0,0xF5 and then 0xF6 

to the command register, these MUST be sent in 3 separate I2C frames, 
you cannot send them all at once. There MUST be a minimum of 20ms between each I2C frame. 
An I2C frame is [start sequence] [I2C address] [register address] [command byte] [stop sequence]. 
The LED will then extinguish and the CMPS11 should now be rotated in all directions in 3 dimensions, 
if a new maximum for any of the sensors is detected then the LED will flash, 
when you cannot get any further LED flashes in any direction then exit the calibration mode 
with a command of 

0xF8. 

Please make sure that the CMPS11 is not located near to ferrous objects as this will distort 
the magnetic field and induce errors in the reading. 
While calibrating rotate the compass slowly. 
Remember the axis of the magnetic field is unlikely to be horizontal, it dips into the earth 
at an angle which varies depending on your location. At our offices in the UK it dips into the earth 
at 67 degrees and that is the orientation each axis of the compass needs to be to find the maximums. 
You need to find both positive and negative maximums for each axis so there are 6 points to calibrate. 
The accelerometer is also calibrated at the same time, so the module should also be positioned horizontal, 
inverted, and on all 4 sides to calibrate the 6 accelerometer points. Each accelerometer point needs 
to be stable for 200mS for its reading to be used for calibration. 
This delay is deliberate so that light taps to the module do not produces disruptive accelerometer readings 
which would mess up the pitch and roll angles. There is no delay for the magnetic points. 
The performance of the module is directly related to how well you perform calibration 
so do this slowly and carefully. 

To enter the horizontal calibration mode by sending a 3 byte sequence of 
0xF0,0xF5 and then 0xF7 
to the command register, these MUST be sent in 3 separate I2C frames, 
you cannot send them all at once. There MUST be a minimum of 20ms between each I2C frame. 
The LED will then extinguish and the CMPS11 should now be rotated in all directions on a horizontal plane, 
if a new maximum for any of the sensors is detected then the LED will flash, 
when you cannot get any further LED flashes in any direction 
then exit the calibration mode with a command of 0xF8. 
Please make sure that the CMPS11 is not located near to ferrous objects as this will distort the magnetic field 
and induce errors in the reading. 
While calibrating rotate the compass slowly. 
Only the X and Y magnetometer axis are calibrated in this mode.

*/
 */

