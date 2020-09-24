/****************************************************************
*                  Arduino CMPS10 example code                  *
*                    CMPS10 running I2C mode                    *
*                    by James Henderson, 2012                   *
*****************************************************************/
#include <Wire.h>
#include <SoftwareSerial.h>

#define ADDRESS 0x60                                          // Defines address of CMPS10

#define LCD_RX              0x02                              // RX and TX pins used for LCD0303 serial port
#define LCD_TX              0x03
#define LCD03_HIDE_CUR      0x04
#define LCD03_CLEAR         0x0C
#define LCD03_SET_CUR       0x02

SoftwareSerial lcd03 =  SoftwareSerial(LCD_RX, LCD_TX);      // Defines software serial port for LCD03

void setup(){
  Wire.begin();                                               // Conects I2C
  lcd03.begin(9600);
  lcd03.write(LCD03_HIDE_CUR);
  lcd03.write(LCD03_CLEAR);
}

void loop(){
   byte highByte, lowByte, fine;              // highByte and lowByte store high and low bytes of the bearing and fine stores decimal place of bearing
   char pitch, roll;                          // Stores pitch and roll values of CMPS10, chars are used because they support signed value
   int bearing;                               // Stores full bearing
   
   Wire.beginTransmission(ADDRESS);           //starts communication with CMPS10
   Wire.write(2);                              //Sends the register we wish to start reading from
   Wire.endTransmission();

   Wire.requestFrom(ADDRESS, 4);              // Request 4 bytes from CMPS10
   while(Wire.available() < 4);               // Wait for bytes to become available
   highByte = Wire.read();           
   lowByte = Wire.read();            
   pitch = Wire.read();              
   roll = Wire.read();               
   
   bearing = ((highByte<<8)+lowByte)/10;      // Calculate full bearing
   fine = ((highByte<<8)+lowByte)%10;         // Calculate decimal place of bearing
   
   display_data(bearing, fine, pitch, roll);  // Display data to the LCD03
   
   delay(100);
}

void display_data(int b, int f, int p, int r){    // pitch and roll (p, r) are recieved as ints instead oif bytes so that they will display corectly as signed values.
  
  lcd03.write(LCD03_SET_CUR);                     // Set the LCD03 cursor position
  lcd03.write(1);  
  lcd03.print("CMPS10 Example V:");
  lcd03.print(soft_ver());                        // Display software version of the CMPS10
  
  delay(5);                                       // Delay to allow LCD03 to proscess data  
  
  lcd03.write(LCD03_SET_CUR);
  lcd03.write(21);  
  lcd03.print("Bearing = ");                      // Display the full bearing and fine bearing seperated by a decimal poin on the LCD03
  lcd03.print(b);                               
  lcd03.print(".");
  lcd03.print(f);
  lcd03.print("  ");

  delay(5);

  lcd03.write(LCD03_SET_CUR);                     // Display the Pitch value to the LCD03
  lcd03.write(41);
  lcd03.print("Pitch = ");
  lcd03.print(p);
  lcd03.print(" ");

  delay(5);
  
  lcd03.write(LCD03_SET_CUR);                     // Display the roll value to the LCD03
  lcd03.write(61);
  lcd03.print("Roll = ");
  lcd03.print(r);
  lcd03.print(" ");
}

int soft_ver(){
   int data;                                      // Software version of  CMPS10 is read into data and then returned
   
   Wire.beginTransmission(ADDRESS);
   // Values of 0 being sent with write need to be masked as a byte so they are not misinterpreted as NULL this is a bug in arduino 1.0
   Wire.write((byte)0);                           // Sends the register we wish to start reading from
   Wire.endTransmission();

   Wire.requestFrom(ADDRESS, 1);                  // Request byte from CMPS10
   while(Wire.available() < 1);
   data = Wire.read();           
   
   return(data);
}
