/*       
       UART communication
       send/receive byte array (64 bytes)
       
       Arduino slave
       ( Arduino Due + Mega;  for small AVR use SoftwareSerial ! )     
       ver 0666nas2
       
 */

 
// (C) Helmut Wunder (HaWe) 2015
// freie Verwendung f�r private Zwecke
// f�r kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.6.1 - 1.6.5)
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/


 

//=====================================================================================

const  uint8_t  MSGSIZE=64;
const  uint8_t  bsync=255;
uint8_t  sendbuf[MSGSIZE];
uint8_t  recvbuf[MSGSIZE];


//=====================================================================================

const uint32_t UARTclock = 115200;

//=====================================================================================


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

#define bitRead(source, bit) ( ((source) >> (bit)) & 0x01 )
#define bitSet (source, bit) ( (source) |= (1UL << (bit)) )
#define bitClear(source, bit) ( (source) &= ~(1UL << (bit)) )
#define bitWrite(source, bit, bitvalue) ( bitvalue ? bitSet(source, bit) : bitClear(source, bit) )

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
// 

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
  if(done){ // do checksumOK test of buffer;
    done=checksumOK(buf);
    if(!done){// checksumOK failed, scan buffer for next sync char
       p = (uint8_t*)memchr((buf+1),0xff,(MSGSIZE-1));
       
       
       if(p){ // found next sync char, shift buffer content, refill buffer
         *cnt = MSGSIZE -(p-buf); // count of characters to salvage from this failure
         memcpy(buf,p,*cnt); //cnt is now where the next character from Serial is stored!
         }
       else *cnt=0; // whole buffer is garbage
       }
    }
   
  }while(!done&&(millis()-start<timeout));

return done; // if done then buf[] contains a sendbufid buffer, else a timeout occurred
}

//=====================================================================================
//=====================================================================================

void loop()
{
  char     sbuf[128],  resOK;   
  static   uint8_t cnt=0;
  uint8_t  cbuf[MSGSIZE], chk;
 

  //     Receive from master

  memset(cbuf, 0, sizeof(cbuf));
   
  resOK = receive ( cbuf, 10000,&cnt);
 
  if( resOK ) {                                      //    receive ok?
    cnt=0;

    //displayvalues(60, "Received...:", cbuf);
         
     memcpy(recvbuf, cbuf, sizeof(cbuf));
     
     memset(sendbuf, 0, sizeof(sendbuf));     
     // debug: test values to send back!
      sendbuf[60]=recvbuf[60]+1;                      // change [60] to send back
      sendbuf[61]=recvbuf[62]+1;                      // change [61] to send back

  }


 
  //   send to master
 
  //Serial.println();
  sendbuf[0]=bsync;
  sendbuf[1]=calcchecksum(sendbuf);
  for(uint8_t i=0; i<MSGSIZE; i++) {       
     Serial1.write(sendbuf[i]);                      // Send values to the master       
  }       
  //Serial1.flush();                                 // clear output buffer
  //displayvalues(20, "Transmitted...: ", sendbuf);
  sprintf(sbuf, "recieve: %4d %4d     send: %4d %4d", recvbuf[60], recvbuf[61], sendbuf[60], sendbuf[61]);
  Serial.println(sbuf);
 
}


//=====================================================================================



void setup() {
   char sbuf[128];   
   int32_t  i=0;

               
   // Serial
   Serial.begin(115200);   // USB terminal
 
   Serial1.begin(UARTclock);                    // RX-TX UART
   while(Serial1.available())  Serial1.read();  // clear output buffer
   
   sprintf(sbuf, "setup(): done.");
   Serial.println(); Serial.println(sbuf);   
 
   sprintf(sbuf, "Rx slave, BAUD= %ld", UARTclock );;

}

//=====================================================================================
