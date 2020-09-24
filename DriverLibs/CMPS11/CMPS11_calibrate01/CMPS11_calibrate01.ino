// CMPS11 calibration example
// description see below
//
// Button pBtn press to start/finish calibration


#include <Wire.h>

#define ADDR_CMPS11 0x60


#define pBtn  11


// CMPS11
#define CMPS11_ADDRESS 0x60  // Address of CMPS11 shifted right one bit for arduino wire library
#define ANGLE_8  1           // Register to read 8bit angle from

unsigned char high_byte, low_byte, angle8;
char pitch, roll;
unsigned int angle16;
double anglef;
int accx, accy, accz;



int read_cmps11() {
  byte b, bbuf, b10,b11,b12,b13,b14,b15,b16,b17,b18,b19,b20,b21;
  
  Wire.beginTransmission(CMPS11_ADDRESS);  //starts communication with CMPS11
  Wire.write(ANGLE_8);                     //Sends the register we wish to start reading from
  Wire.endTransmission();
 
  // Request 21 bytes from the CMPS11
  // this will give us the 8 bit bearing, 
  // both bytes of the 16 bit bearing, pitch and roll
  Wire.requestFrom(CMPS11_ADDRESS, 21);       
  
  while(Wire.available() < 21);        // Wait for all bytes to come back

  angle8 = Wire.read();       // 1; Read back the first 5 bytes
  high_byte = Wire.read();
  low_byte = Wire.read();
  pitch = Wire.read();
  roll = Wire.read();         // 5    
    
    bbuf=Wire.read();
    bbuf=Wire.read();
    bbuf=Wire.read();
    bbuf=Wire.read();
    
  b10= Wire.read();           // 10
  b11= Wire.read();
  b12= Wire.read(); 
  b13= Wire.read();
  b14= Wire.read();
  b15= Wire.read();
  
  b16= Wire.read();          // 16  
  b17= Wire.read();           
  b18= Wire.read();
  b19= Wire.read();
  b20= Wire.read();
  b21= Wire.read();          // 21
  
  accx= (int16_t)((b16<<8) + b17);
  accy= (int16_t)((b18<<8) + b19);
  accz= (int16_t)((b20<<8) + b21);
 
  angle16 = (high_byte<<8) + low_byte; // Calculate 16 bit angle
  anglef=angle16/10.0;

  return 0;
}




void setup(){
  pinMode( pBtn, INPUT_PULLUP); // start buton
  pinMode(13, OUTPUT);       // built-in LED 
  
  Wire.begin();
  Serial.begin(115200);
  delay(1000);
  Serial.println("Init...");
  
  calibrate();
}



void loop(){
  char sbuf[100];
  
  read_cmps11();
    
  Serial.print("roll: ");               // Display roll data
  Serial.print(roll, DEC);
  
  Serial.print("   pitch: ");          // Display pitch data
  Serial.print(pitch, DEC);

  sprintf(sbuf, "%6.2f ", anglef);
  Serial.println((String)"   angle full: "+sbuf);     // Display 16 bit angle with decimal place

  sprintf(sbuf, "  accx=%+6d ", accx);   Serial.println(sbuf);
  sprintf(sbuf, "  accy=%+6d ", accy);   Serial.println(sbuf);
  sprintf(sbuf, "  accz=%+6d ", accz);   Serial.println(sbuf);
  
  Serial.println();
  delay(100);                           // Short delay before next loop
}



void calibrate(){
  Serial.println("Calibration Mode: start/finish: press button ");

  digitalWrite(13, LOW);
  while(digitalRead(pBtn));  
  if( !digitalRead(pBtn)) {
    digitalWrite(13, HIGH);
    delay(100);
    while( !digitalRead(pBtn));
  }
  digitalWrite(13, LOW);
  Serial.println("Starting...");
  delay(1000);   
  
  Serial.println();
  Serial.println("Begin: rotate into each direction (3D rotation)");
  Serial.println("until LED stops flashing!");
  Serial.println("Then fix it to the ground at either edge for 2sec each!");
  Serial.println("finish: again press button ");
  delay(2000); // 2 sec before starting
  
  Wire.beginTransmission(ADDR_CMPS11);
  Wire.write(0); //command register
  Wire.write(0xF0);
  Wire.endTransmission();
  delay(25);

  Wire.beginTransmission(ADDR_CMPS11);
  Wire.write(0); //command register
  Wire.write(0xF5);
  Wire.endTransmission();
  delay(25);

  Wire.beginTransmission(ADDR_CMPS11);
  Wire.write(0); //command register
  Wire.write(0xF6);  // horizontal: 0xf7  all: 0xf6
  Wire.endTransmission();
  delay(2000);

  while(digitalRead(pBtn));  
  if( !digitalRead(pBtn)) {
    digitalWrite(13, HIGH);
    delay(100);
    while( !digitalRead(pBtn));
  }
  digitalWrite(13, LOW);
  
  Serial.println("Finish...");
  delay(500);

  Wire.beginTransmission(ADDR_CMPS11);
  Wire.write(0); //command register
  Wire.write(0xF8);
  Wire.endTransmission();
  
  Serial.println("...done!");
}



/*
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

