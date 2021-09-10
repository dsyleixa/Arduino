/*       
       UART communication
       send/receive byte array (64 bytes)
       
       Arduino Due slave
       ver uartmux0002
       
 */

 
// (C) Helmut Wunder (HaWe) 2015
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.6.1 - 1.6.5)
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/


#include <SPI.h>
#include <Scheduler.h>

#include <TinyGPS++.h>
const uint32_t  GPSbaud = 9600;
TinyGPSPlus gps;   // The TinyGPS++ object

const uint32_t  UARTbaud = 115200;

//=====================================================================================



#define SYNCSLOT      0  // start sync signal of this Msg: bsync  (e.g. 0xff/255)
#define CKSSLOT       1  // chksum this Msg
#define BYTE0         2  // byte 0      // byte: 8-bit => 8 digital bits 0-7
#define BYTE1         3  // byte 1      // byte: 8-bit => 8 digital bits 8-15
#define INT0          4  // int0        // 3x long: 32-bit/4byte
#define INT1          8  // int1
#define INT2         12  // int2
#define DOUBL0       16  // double0     // 2x double 64bit/8byte
#define DOUBL1       24  // double1
#define FLOAT0       32  // float0      // 3x float 32bit/4byte
#define FLOAT1       36  // float1
#define FLOAT2       40  // float2
#define ANA0         44  // analog 0    // 9x ananog (ADC) 16-bit/2byte
#define ANA1         46  // analog 1    
#define ANA2         48  // analog 2
#define ANA3         50  // analog 3
#define ANA4         52  // analog 4
#define ANA5         54  // analog 5
#define ANA6         56  // analog 6
#define ANA7         58  // analog 7
#define ANA8         60  // analog 8
#define BYTE2        62  // byte 2      //  1 byte custom
#define TERM         63  // terminating //  1 byte custom 

//=====================================================================================

const    uint8_t  MSGSIZE=64;
uint8_t  bsync=255;
uint8_t  bterm=254;
uint8_t  sendbuf[MSGSIZE];
uint8_t  recvbuf[MSGSIZE];




//************************************************************************************
// bit and byte and pin operations
//************************************************************************************
// convert byte arrays to int

inline int16_t  ByteArrayToInt16(uint8_t  array[], uint8_t  slot) {
  return ((array[slot + 1] << 8) + (array[slot]));
}

inline long  ByteArrayToInt32(uint8_t  array[], uint8_t  slot) {
  return ( (array[slot+3]<<24) + (array[slot+2]<<16) + (array[slot+1]<<8) + array[slot] );
}

//------------------------------------------------------------------------------
// convert int to byte arrays

inline void Int16ToByteArray(int16_t vint16,  uint8_t *array,  uint8_t slot) {
  memcpy(array+slot*sizeof(char), &vint16, sizeof(int16_t));    // copy int16 to array
}

inline void Int32ToByteArray(int32_t vint32,  uint8_t *array,  uint8_t slot) {
  memcpy(array+slot*sizeof(char), &vint32, sizeof(int32_t));    // copy int32 to array
}

inline void FloatToByteArray(float vfloat32,  uint8_t *array,  uint8_t slot) {   
  memcpy(array+slot*sizeof(char), &vfloat32, sizeof(float));    // copy float to array
}

//------------------------------------------------------------------------------
// convert float/double to byte arrays

void double2bytebuf(double fvar, uint8_t * bytearr) {
   union {
     double  f;
     uint8_t b8[8];
   } u;
   u.f = fvar;
   memcpy(bytearr, u.b8, 8);  
}

double bytebuf2double(uint8_t * bytearr) {
   union {
     double  f;
     uint8_t b8[8];
   } u;
   memcpy(u.b8, bytearr, 8);
   return u.f;
}

//------------------------------------------------------------------------------
// read+write bits in numbers
/*
#define bitRead(source, bit) ( ((source) >> (bit)) & 0x01 )
#define bitSet (source, bit) ( (source) |= (1UL << (bit)) )
#define bitClear(source, bit) ( (source) &= ~(1UL << (bit)) )
#define bitWrite(source, bit, bitvalue) ( bitvalue ? bitSet(source, bit) : bitClear(source, bit) )
*/


//------------------------------------------------------------------------------------

int16_t  toggleup(int16_t  nr,  int16_t  max) {
  if ( nr < (max - 1) ) ++nr;
  else nr = 0;
  return nr;
}


//------------------------------------------------------------------------------------

#define sensortouch(pinHIGH) !digitalRead(pinHIGH)

//------------------------------------------------------------------------------------


void displayvalues(char * caption, uint8_t array[]) {
  int cnt;
  char sbuf[128];
 
  sprintf(sbuf, "%s ", caption);  
  Serial.println(sbuf);
  for(cnt=0; cnt<8; ++cnt) {
    if(cnt%8==0) Serial.println();
    sprintf(sbuf, "%3d ", array[cnt]);      // print on TFT
    Serial.print(sbuf);                      // Print sendbufue to the Serial Monitor
  }   
  Serial.println(); 
 
} 


//=====================================================================================
// serial transmission

uint8_t calcchecksum(uint8_t array[]) {
  int32_t  sum=0;
  for(int i=2; i<MSGSIZE; ++i) sum+=(array[i]);
  return (sum & 0x00ff);
}

#define  checksumOK(array)  (calcchecksum(array)==array[1]) 


// ================================================================
// addToBuffer and receive function courtesy of chucktodd

  bool addToBuffer( uint8_t buf[], uint8_t *cnt, uint16_t timeout){
  bool inSync = *cnt>0;
  unsigned long start=millis();
  
  while((*cnt<MSGSIZE)&&(millis()-start<timeout)){
     if(Serial1.available()){ // grab new char, test for sync char, if so start adding to buffer
        buf[*cnt] = (uint8_t)Serial1.read();
        if(inSync) *cnt += 1;   
        else {
           if(buf[*cnt]==bsync) {
             inSync = true;
             *cnt +=1;
           }
        }
     }
   }
   return (*cnt==MSGSIZE);
}


//=====================================================================================

bool receive(uint8_t * buf, uint16_t timeout, uint8_t *cnt) {     // by passing cnt in and out,
// i can timeout and still save a partial buffer, so a resync costs less (less data lost)

   bool inSync=false;
   unsigned long start=millis();
   uint8_t * p;  // pointer into buf for reSync operation
   bool done=false;

   do {
     done = addToBuffer(buf,cnt,timeout); // if this return false, a timeout has occured, and the while will exit.
     if(done) { // do checksumOK test of buffer;
       done=checksumOK(buf);
       if(!done) {       // checksumOK failed, scan buffer for next sync char
          p = (uint8_t*)memchr((buf+1),bsync,(MSGSIZE-1));         
       
          if(p){ // found next sync char, shift buffer content, refill buffer
             *cnt = MSGSIZE -(p-buf); // count of characters to salvage from this failure
             memcpy(buf,p,*cnt); //cnt is now where the next character from Serial is stored!
          }
          else *cnt=0; // whole buffer is garbage
       }
     }
   } while(!done&&(millis()-start<timeout));

   return done; // if done then buf[] contains a sendbufid buffer, else a timeout occurred
}




//=====================================================================================
// L O O P (reserved)
//=====================================================================================
void loop() {

   yield();
   delay(100);
   yield();

}


//=====================================================================================
// L O O P 2:  TFT Display
//=====================================================================================

void loop2() {
   char     sbuf[128]; 
   
   yield();
   delay(50);
   yield();
}



//=====================================================================================
// L O O P 4:  Serial1: UART Muxer
//=====================================================================================
void loop4 ()
{ 
  char     sbuf[128],  resOK;   
  static   uint8_t cnt=0; 
  uint8_t  cbuf[MSGSIZE], chk;
 

  //     Receive from master 

  memset(cbuf, 0, sizeof(cbuf)); 
   
  resOK = receive ( cbuf, 10000,&cnt);
 
  if( resOK ) {                                      // byte 0 == syncbyte ?
    cnt=0;

    //displayvalues(60, "Received...:", cbuf);
     chk=(byte)calcchecksum(cbuf);     
     memcpy(recvbuf, cbuf, sizeof(cbuf));
 
        // change values to send back!
        memcpy(sendbuf, recvbuf, sizeof(sendbuf));   // copy inbuf to outbuf
        sendbuf[4]+=1;                               // change [6] to send back 
        sendbuf[6]+=1;                               // change [6] to send back    
  }

  yield();
 
  //   send to master 
 
  //Serial.println(); 
  sendbuf[0]=bsync;
  sendbuf[1]=calcchecksum(sendbuf);
  for(uint8_t i=0; i<MSGSIZE; i++) {       
     Serial1.write(sendbuf[i]);                      // Send value to the Rx Arduino       
  }       
  //Serial1.flush();                                 // clear output buffer
  //displayvalues(20, "Transmitted...: ", sendbuf);
  sprintf(sbuf, "recieve: %4d %4d     send: %4d %4d", recvbuf[4], recvbuf[6], sendbuf[4], sendbuf[6]);
  Serial.println(sbuf); 

  yield();
 
}



//=====================================================================================
// L O O P 5:   Serial2: GPS
//=====================================================================================
void loop5() {
char sbuf[128];
  volatile static uint32_t millisav;
  
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read()))
      displayInfo();
      millisav=millis();
  }
  yield();
  
  if (millis()-millisav > 5000 && gps.charsProcessed() < 10)  {
    Serial.println("No GPS detected: check wiring.");
    millisav=millis()-3000;
    yield();
  }

  delay(50); 
  yield();
}


//=====================================================================================



void setup() {
   char sbuf[128];   
   int32_t  i=0;

               
   // Serial
   Serial.begin(115200);   // USB terminal
 
   Serial1.begin(UARTbaud);                    // RX-TX UART
   while(Serial1.available())  Serial1.read();  // clear output buffer

   Serial2.begin(GPSbaud);

   Serial.println("DeviceExample.ino");
   Serial.println("A simple demonstration of TinyGPS++ with an attached GPS module");
   Serial.print("Testing TinyGPS++ library v. "); 
   Serial.println(TinyGPSPlus::libraryVersion());
   Serial.println("by Mikal Hart");
   Serial.println();
  
 
   sprintf(sbuf, "Rx slave, BAUD= %ld", UARTbaud );

   // Add "loopX"  to scheduling.
   
   //Scheduler.startLoop(loop2); sprintf(sbuf, "MT display task started " ); 

   //Scheduler.startLoop(loop3);  sprintf(sbuf, "loop3:  task started ");
   
   Scheduler.startLoop(loop4);  sprintf(sbuf, "loop4 UART Muxer task started ");

   Scheduler.startLoop(loop5);  sprintf(sbuf, "loop4 GPS task started ");

   sprintf(sbuf, "setup(): done.");
   Serial.println(); Serial.println(sbuf);   

}







//=====================================================================================
void displayInfo()
{  
  char sbuf[128]; 
  double    fLatt, fLong, fspeed, fcourse, ftest=12.345678;
  uint16_t  dday, dmonth, dyear, dhour, dmin, dsec, dcsec, nsat;

  //----------------------------
  if (gps.location.isValid())
  { 
    fLatt= (double)gps.location.lat();
    fLong= (double)gps.location.lng(); 
    sprintf(sbuf, "Latt: %+013.8f , Long: %+013.8f ", fLatt, fLong ); 
    Serial.print(sbuf);
  }
  else { Serial.print("Location:  INVALID      ");}

  //----------------------------
  if (gps.speed.isValid())
  { 
    fspeed= (double)gps.speed.kmph();
    sprintf(sbuf, "speed: %5.1f ", fspeed ); 
    Serial.print(sbuf);
  }
  else  { Serial.print("Speed:INVALID"); }


  //----------------------------
  if (gps.course.isValid())
  { 
    fcourse= (double)gps.course.deg();
    sprintf(sbuf, "course: %5.1f ", fcourse ); 
    Serial.print(sbuf);
  }
  else { Serial.print("course:INVALID"); }


  //----------------------------
  if (gps.date.isValid())
  {     
    dday=gps.date.day();
    dmonth=gps.date.month();    
    dyear=gps.date.year();
    sprintf(sbuf, " Date: %02d/%02d/%04d", dday, dmonth, dyear); 
    Serial.print(sbuf);
  }
  else { Serial.print(" Date:  INVALID     "); }

  //----------------------------
  if (gps.time.isValid())
  {
    dhour=gps.time.hour();
    dmin= gps.time.minute();
    dsec= gps.time.second();
    dcsec=gps.time.centisecond();
    nsat =gps.satellites.value();
    sprintf(sbuf, " Time: %02d:%02d:%02d,%03d Satellites=%02d", dhour, dmin, dsec, dcsec, nsat); 
    Serial.print(sbuf);
  }
  else { Serial.print(" Time: INVALID     "); }

  Serial.println();
}



//=====================================================================================

//=====================================================================================
