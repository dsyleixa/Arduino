
/*
   UART communication
   send/receive byte array (64 bytes)
   *     
   Raspberry Pi  master
   ver 0667nrm2
 
 */
 
// (C) Helmut Wunder (HaWe) 2015
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: gcc C/C++
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>


#include <wiringPi.h>
#include <wiringSerial.h>

#define  byte  uint8_t

char   * uart  =  "/dev/ttyAMA0";
int    Serial1;

 
//*************************************************************//*************************************************************
//             bits, bytes, pins & arrays                      //               bits, bytes, pins & arrays
//*************************************************************//*************************************************************




//************************************************************************************
// bit and byte and pin operations
//************************************************************************************
// convert byte arrays to int

inline int16_t  ByteArrayToInt16(uint8_t  barray[], uint8_t  slot) {
  return ((barray[slot + 1] << 8) + (barray[slot]));
}

inline long  ByteArrayToInt32(uint8_t  barray[], uint8_t  slot) {
  return ( (barray[slot+3]<<24) + (barray[slot+2]<<16) + (barray[slot+1]<<8) + barray[slot] );
}

//------------------------------------------------------------------------------
// convert int to byte arrays

inline void Int16ToByteArray(int16_t vint16,  uint8_t barray[],  uint8_t slot) {
  memcpy(barray+slot*sizeof(char), &vint16, sizeof(int16_t));    // copy int16 to array
}

inline void Int32ToByteArray(int32_t vint32,  uint8_t barray[],  uint8_t slot) {
  memcpy(barray+slot*sizeof(char), &vint32, sizeof(int32_t));    // copy int32 to array
}


//------------------------------------------------------------------------------
// convert float/double to byte arrays



void DoubleToByteArray(double fvar, uint8_t * barray,  uint8_t slot) {
   union {
     double  f;
     uint8_t b8[sizeof(double)];
   } u;
   u.f = fvar;
   memcpy(barray+slot*sizeof(char), u.b8, sizeof(double));  
}


double ByteArrayToDouble(uint8_t * barray, uint8_t  slot) {
   union {
     double  f;
     uint8_t b8[sizeof(double)];
   } u;
   memcpy( u.b8, barray+slot*sizeof(char), sizeof(double)); 
   return u.f;
}




void FloatToByteArray(float fvar, uint8_t * barray,  uint8_t slot) {
    union {
     double  f;
     uint8_t b4[sizeof(float)];
   } u;
   u.f = fvar;
   memcpy(barray+slot*sizeof(char), u.b4, sizeof(float));   
}



double ByteArrayToFloat(uint8_t * barray, uint8_t  slot) {
   union {
     double  f;
     uint8_t b4[sizeof(float)];
   } u;
   memcpy( u.b4, barray+slot*sizeof(char), sizeof(float)); 
   return u.f;
}

//-------------------------------------------------------------

#define ifinrange(val,lo,hi)  if((val>=lo)&&(val<=hi))



//-------------------------------------------------------------
// read+write bits in numbers
/*
// already implemented by Arduino
#define bitRead(source, bit) ( ((source) >> (bit)) & 0x01 )
#define bitSet (source, bit) ( (source) |= (1UL << (bit)) )
#define bitClear(source, bit) ( (source) &= ~(1UL << (bit)) )
#define bitWrite(source, bit, bitvalue) ( bitvalue ? bitSet(source, bit) : bitClear(source, bit) )
*/
//-------------------------------------------------------------



//*************************************************************
//                  RC + Mux msg array structure
//*************************************************************


#define SYNCSLOT      0  // start sync signal of this Msg: bsync=0xff (255)
#define CKSSLOT       1  // chksum this Msg
#define BYTE0         2  // byte 0     // byte: 8-bit => 8 digital bits 0-7
#define BYTE1         3  // byte 1     // byte: 8-bit => 8 digital bits 8-15
#define ENC0          4  // motorenc 0 // 10 encoders: 32-bit
#define ENC1          8  // motorenc 1
#define ENC2         12  // motorenc 2
#define ENC3         16  // motorenc 3
#define ENC4         20  // motorenc 4
#define ENC5         24  // motorenc 5
#define ENC6         28  // motorenc 6
#define ENC7         32  // motorenc 7
#define ENC8         36  // motorenc 8
#define ENC9         40  // motorenc 9
#define ANA0         44  // analog 0   // 9 analog: 16-bit
#define ANA1         46  // analog 1   // analog 0+1 = joystick for drive
#define ANA2         48  // analog 2
#define ANA3         50  // analog 3
#define ANA4         52  // analog 4
#define ANA5         54  // analog 5
#define ANA6         56  // analog 6
#define ANA7         58  // analog 7
#define ANA8         60  // analog 8
#define BYTE2        62  // byte 2     // byte: 8-bit => 8 digital bits 16-23
#define TERM         63  // terminating: heart beat signal 


// optional:
#define LONG0         4  // long 0      // 3x long: 32-bit/4byte
#define LONG1         8  // long 1
#define LONG2        12  // long 2
#define DOUBL0       16  // double0     // 2x double 64bit/8byte
#define DOUBL1       24  // double1
#define FLOAT0       32  // float0      // 3x float 32bit/4byte
#define FLOAT1       36  // float1
#define FLOAT2       40  // float2


// alias
#define BYTE3        58    // byte 3     
#define BYTE4        59    // byte 4 
#define BYTE5        60    // byte 5 
#define BYTE6        61    // byte 6  



//*************************************************************
// msg array read / write
//*************************************************************




//=====================================================================================
// debug monitor

void displayvalues(char * caption, uint8_t array[]) {
  int cnt;
  char sbuf[128];
 
  sprintf(sbuf, "%s ", caption);
  printf(sbuf); printf("\n");
  for(cnt=0; cnt<8; ++cnt) {
    sprintf(sbuf, "%3d ", array[cnt]);      // print on TFT
    printf(sbuf);
  }   
  printf("\n");
 
}

//=====================================================================================
//=====================================================================================
// serial TCP


const   uint8_t  MSGSIZE=64;
const   uint8_t  bsync= 0xFF;         // transmission control bytes: begin of msg signal : bsync=0xFF (255)
uint8_t  sendbuf[MSGSIZE];
uint8_t  recvbuf[MSGSIZE];


uint8_t calcchecksum(uint8_t array[]) {
  int32_t  sum=0;
  for(int i=2; i<MSGSIZE; ++i) sum+=(array[i]);
  return (sum & 0x00ff);
}

bool checksumOK(uint8_t array[]){
return (calcchecksum(array)==array[1]);
}

// ================================================================
// addToBuffer and receive function courtesy of chucktodd

bool addToBuffer( uint8_t buf[], uint8_t *cnt, uint16_t timeout){
bool inSync = *cnt>0;
unsigned long start=millis();
while((*cnt<MSGSIZE)&&(millis()-start<timeout)){
  if( serialDataAvail( Serial1 ) ) { // grab new char, test for sync char, if so start adding to buffer
    buf[*cnt] = (uint8_t)serialGetchar( Serial1 );
    if(inSync) *cnt += 1;
    else{
     if(buf[*cnt]==0xFF){
       inSync = true;
       *cnt +=1;
       }
     }
    }
  }
return (*cnt==MSGSIZE);
}


//=====================================================================================

bool receive(uint8_t * buf, uint16_t timeout, uint8_t *cnt){ // by passing cnt in and out,
// i can timeout and still save a partial buffer, so a resync costs less (less data lost)

bool inSync=false;
unsigned long start=millis();
uint8_t * p;  // pointer into buf for reSync operation
bool done=false;

do{
  done = addToBuffer(buf,cnt,timeout); // if this return false, a timeout has occured, and the while will exit.
  if(done){                         // do checksumOK test of buffer;
    done=checksumOK(buf);
    if(!done){                      // checksumOK failed, scan buffer for next sync char
       p = (uint8_t*)memchr((buf+1),0xff,(MSGSIZE-1));   
       
       
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


void loop()
{
  char     sbuf[128],  resOK;   
  static   uint8_t cnt=0;
  uint8_t  cbuf[MSGSIZE], chk;

 
  //   send to slave
 
  //Serial.println();
  sendbuf[0]=bsync;
  sendbuf[1]=calcchecksum(sendbuf);
 
  for(uint8_t i=0; i<MSGSIZE; i++) {                     //
     serialPutchar( Serial1, sendbuf[i]);                // Send values to the slave       
  }       
  //serialFlush( Serial1 );                              // clear output buffer
 
  sprintf(sbuf, "send : %4d %4d     ", sendbuf[60], sendbuf[61]);
  printf(sbuf);

 
  //     Receive from slave

  memset(cbuf, 0, sizeof(cbuf));
   
  resOK = receive ( cbuf, 10000,&cnt);
 
  if( resOK ) {                         //  receive ok?
     cnt=0;

     memcpy(recvbuf, cbuf, sizeof(cbuf));
     
     // debug
     sprintf(sbuf, "received: %4d %4d    \n ", recvbuf[60], recvbuf[61]);
     printf(sbuf);
 
      memset(sendbuf, 0, sizeof(sendbuf));    // clear send buf
     // debug: test values to send back!
      sendbuf[60]=recvbuf[60]+10;                      // change [60] to send back
      sendbuf[61]=recvbuf[61]+20;                      // change [61] to send back
       
       
  }
 
}


//=====================================================================================

 
int main() {
    unsigned long timesav;
    char  sbuf[128];

     
    printf("initializing..."); printf("\n");
   
    // UART Serial com port
    Serial1 = serialOpen (uart, 115200); // for Arduino code compatibility reasons
   
    while(1) { loop(); }
   
    serialClose( Serial1);
   

    exit(0);
}


