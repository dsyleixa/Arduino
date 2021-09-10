/*
       UART communication
       send/receive byte array (64 bytes)

       Arduino  slave
       ( Arduino Due + Mega;  for small AVR use SoftwareSerial ! )

       log:
       course, speed, nSat, time_t -> sendarray
       time, date, nGPSsat: int8_t
       INVALID: -1001
       GPD speed, course => int16_t *32
       debug values arrays BYTE0/1
       GPS ublox thread: Lat, Lng-> sendarray DOUBL0/1
*/

#define  vers  "uartmux0005"

// (C) Helmut Wunder (HaWe) 2015
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.6.1 - 1.6.5)
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/


#include <SPI.h>
#include <Scheduler.h>

#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>


#define  INVALID  -1001
#define  INVALCH   -127

time_t utc_time=INVALID, local_time=INVALID;
time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss)
{
  tmElements_t tmSet;
  tmSet.Year = YYYY - 1970;
  tmSet.Month = MM;
  tmSet.Day = DD;
  tmSet.Hour = hh;
  tmSet.Minute = mm;
  tmSet.Second = ss;
  return makeTime(tmSet);         //convert to time_t
}

TimeChangeRule CEST = { "CEST", Last, Sun, Mar, 2, 120 };     //Central European Summer Time
TimeChangeRule CET = { "CET ", Last, Sun, Oct, 3, 60 };       //Central European Time
Timezone CE(CEST, CET);
TimeChangeRule *tcr;        //pointer to the time change rule, use to get the TZ abbrev



const uint32_t SERIALbaud = 115200;
const uint32_t UARTbaud = 115200;

const uint32_t GPSbaud = 9600;
#include <TinyGPS++.h>
TinyGPSPlus gps;   // TinyGPS++ object
double         fGPSlat=INVALID, fGPSlng=INVALID;
float          fGPSspeed=INVALID, fGPScourse=INVALID;
int16_t        dspeedx32, dcoursex32;     //  (int16_t)(fvar*32)
int8_t         dday=INVALCH, dmonth=INVALCH;
int16_t        dyear=INVALID;
int8_t         dhour=INVALCH, dmin=INVALCH, dsec=INVALCH, dcsec=INVALCH, nGPSsat=INVALCH;


//=====================================================================================



#define SYNCSLOT      0  // start sync signal of this Msg: bsync  (e.g. 0xff/255)
#define CKSSLOT       1  // chksum this Msg
#define BYTE0         2  // byte 0      // byte: 8-bit => 8 digital bits 0-7
#define BYTE1         3  // byte 1      // byte: 8-bit => 8 digital bits 8-15
#define LONG0         4  // long 0      // 3x long: 32-bit/4byte
#define LONG1         8  // long 1
#define LONG2        12  // long 2      // GPS time_t
#define DOUBL0       16  // double0     // GPS Longit double 64bit/8byte
#define DOUBL1       24  // double1     // GPS Latit. double 64bit/8byte
#define FLOAT0       32  // float0      // 3x float 32bit/4byte
#define FLOAT1       36  // float1
#define FLOAT2       40  // float2
#define ANA0         44  // analog 0    // 9x analog  16-bit/2byte
#define ANA1         46  // analog 1    
#define ANA2         48  // analog 2
#define ANA3         50  // analog 3
#define ANA4         52  // analog 4
#define ANA5         54  // analog 5
#define ANA6         56  // analog 6
#define ANA7         58  // analog 7    //  GPS dcoursex32
#define ANA8         60  // analog 8    //  GPS dspeedx32
#define BYTE2        62  // byte 2      //  GPS n satel.
#define TERM         63  // terminating //  1 byte custom 

//=====================================================================================

#define  MSGSIZE 64

uint8_t  bsync = 255;
uint8_t  bterm = 254;
uint8_t  sendbuf[MSGSIZE],
         recvbuf[MSGSIZE],
         cbuf[MSGSIZE];




//************************************************************************************
// bit and byte and pin operations
//************************************************************************************
// convert byte arrays to int

inline int16_t  ByteArrayToInt16(uint8_t  barray[], uint8_t  slot) {
  return ((barray[slot + 1] << 8) + (barray[slot]));
}

inline long  ByteArrayToInt32(uint8_t  barray[], uint8_t  slot) {
  return ( (barray[slot + 3] << 24) + (barray[slot + 2] << 16) + (barray[slot + 1] << 8) + barray[slot] );
}

//------------------------------------------------------------------------------
// convert int to byte arrays

inline void Int16ToByteArray(int16_t vint16,  uint8_t barray[],  uint8_t slot) {
  memcpy(barray + slot * sizeof(char), &vint16, sizeof(int16_t)); // copy int16 to array
}

inline void Int32ToByteArray(int32_t vint32,  uint8_t barray[],  uint8_t slot) {
  memcpy(barray + slot * sizeof(char), &vint32, sizeof(int32_t)); // copy int32 to array
}


//------------------------------------------------------------------------------
// convert float/double to byte arrays

void DoubleToByteArray(double fvar, uint8_t * barray,  uint8_t slot) {
  union {
    double  f;
    uint8_t b8[sizeof(double)];
  } u;
  u.f = fvar;
  memcpy(barray + slot * sizeof(char), u.b8, sizeof(double));
}


double ByteArrayToDouble(uint8_t * barray, uint8_t  slot) {
  union {
    double  f;
    uint8_t b8[sizeof(double)];
  } u;
  memcpy( u.b8, barray + slot * sizeof(char), sizeof(double));
  return u.f;
}




void FloatToByteArray(float fvar, uint8_t * barray,  uint8_t slot) {
  union {
    double  f;
    uint8_t b4[sizeof(float)];
  } u;
  u.f = fvar;
  memcpy(barray + slot * sizeof(char), u.b4, sizeof(float));
}



double ByteArrayToFloat(uint8_t * barray, uint8_t  slot) {
  union {
    double  f;
    uint8_t b4[sizeof(float)];
  } u;
  memcpy( u.b4, barray + slot * sizeof(char), sizeof(float));
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


void displayvalues(char * caption, uint8_t barray[]) {
  int cnt;
  char sbuf[128];

  sprintf(sbuf, "%s ", caption);
  Serial.println(sbuf);
  for (cnt = 0; cnt < 8; ++cnt) {
    if (cnt % 8 == 0) Serial.println();
    sprintf(sbuf, "%3d ", barray[cnt]);      // print on TFT
    Serial.print(sbuf);                      // Print sendbufue to the Serial Monitor
  }
  Serial.println();

}


//=====================================================================================
// GET SENSOR VALUES FOR UART COMM
//=====================================================================================

void buildSendArray() {

  DoubleToByteArray(fGPSlat, sendbuf, DOUBL0);     // store double Latitude
  DoubleToByteArray(fGPSlng, sendbuf, DOUBL1);     // store double Longitude
  Int16ToByteArray(dcoursex32, sendbuf, ANA7);  // GPS course*32
  Int16ToByteArray(dspeedx32, sendbuf, ANA8);   // GPS speed*32
  sendbuf[BYTE2] = nGPSsat;                        // GPS n satellites
  Int32ToByteArray(utc_time, sendbuf, LONG2); // GPS local_time

  // finish sendbuf;
  sendbuf[0] = bsync;
  sendbuf[1] = calcchecksum(sendbuf);

}





//=====================================================================================
// serial transmission

uint8_t calcchecksum(uint8_t barray[]) {
  int32_t  sum = 0;
  for (int i = 2; i < MSGSIZE; ++i) sum += (barray[i]);
  return (sum & 0x00ff);
}

#define  checksumOK(barray)  (calcchecksum(barray)==barray[1])


// ================================================================
// addToBuffer and receive function courtesy of chucktodd

bool addToBuffer( uint8_t buf[], uint8_t *cnt, uint16_t timeout) {
  bool inSync = *cnt > 0;
  unsigned long start = millis();

  while ((*cnt < MSGSIZE) && (millis() - start < timeout)) {
    if (Serial1.available()) { // grab new char, test for sync char, if so start adding to buffer
      buf[*cnt] = (uint8_t)Serial1.read();
      if (inSync) *cnt += 1;
      else {
        if (buf[*cnt] == bsync) {
          inSync = true;
          *cnt += 1;
        }
      }
    }
  }
  return (*cnt == MSGSIZE);
}


//=====================================================================================

bool receive(uint8_t * buf, uint16_t timeout, uint8_t *cnt) {     // by passing cnt in and out,
  // i can timeout and still save a partial buffer, so a resync costs less (less data lost)

  bool inSync = false;
  unsigned long start = millis();
  uint8_t * p;  // pointer into buf for reSync operation
  bool done = false;

  do {
    done = addToBuffer(buf, cnt, timeout); // if this return false, a timeout has occured, and the while will exit.
    if (done) { // do checksumOK test of buffer;
      done = checksumOK(buf);
      if (!done) {      // checksumOK failed, scan buffer for next sync char
        p = (uint8_t*)memchr((buf + 1), bsync, (MSGSIZE - 1));

        if (p) { // found next sync char, shift buffer content, refill buffer
          *cnt = MSGSIZE - (p - buf); // count of characters to salvage from this failure
          memcpy(buf, p, *cnt); //cnt is now where the next character from Serial is stored!
        }
        else *cnt = 0; // whole buffer is garbage
      }
    }
  } while (!done && (millis() - start < timeout));

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
// loopTFTdisplay:  TFT Display
//=====================================================================================

void loopTFTdisplay() {
  char     sbuf[128];

  yield();
  delay(50);
  yield();
}



//=====================================================================================
// loopUARTcomm:  Serial1: UART Muxer
//=====================================================================================

void loopUARTcomm ()
{
  char     sbuf[128],  resOK;
  static   uint8_t cnt = 0;
  uint8_t  chk;
  volatile static double   ftest;

  //.....................................
  //     Receive from master

  memset(cbuf, 0, sizeof(cbuf));

  resOK = receive ( cbuf, 10000, &cnt);

  if ( resOK ) {                                     // byte 0 == syncbyte ?
    cnt = 0;

    //displayvalues(60, "Received...:", cbuf);
    chk = (byte)calcchecksum(cbuf);
    memcpy(recvbuf, cbuf, sizeof(cbuf));
  }
  else {
    Serial.println("\n UART MASTER RECIEVE ERROR\n");
  }
  yield();




  //.....................................
  //   send to master

  // clear send-buf
  memset(sendbuf, 0, sizeof(sendbuf));

  // debug: test values to send back!
  if (resOK) {
    sendbuf[BYTE0] = recvbuf[BYTE0] + 1;                  // change [BYTE0] to send back
    sendbuf[BYTE1] = recvbuf[BYTE1] + 1;                  // change [BYTE1] to send back
  }

  // get values to send
  buildSendArray();

  //...cheerio...!
  for (uint8_t i = 0; i < MSGSIZE; i++) {
    Serial1.write(sendbuf[i]);                      // Send values to the master
  }

  // debug
  // ftest=ByteArrayToDouble(sendbuf, DOUBL0);
  sprintf(sbuf, "recieve: %4d %4d   send: %4d %4d  Lat%13.8f  Lng%13.8f  %6.1f  %4.1f  %2d",
          recvbuf[BYTE0],recvbuf[BYTE1],sendbuf[BYTE0],sendbuf[BYTE1],fGPSlat,fGPSlng,fGPScourse,fGPSspeed,nGPSsat);
  Serial.println(sbuf);
  yield();

}



//=====================================================================================
// loopGPSread:   Serial2: GPS
//=====================================================================================

void loopGPSread() {
  char sbuf[128];
  volatile static uint32_t millisav;

  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read()))
      getGPSvalues();
    millisav = millis();
  }
  yield();

  if (millis() - millisav > 5000 && gps.charsProcessed() < 10)  {
    Serial.println("No GPS detected: check wiring.");
    millisav = millis() - 3000;
    yield();
  }

  yield();
}


//=====================================================================================
// SETUP
//=====================================================================================

void setup() {
  char sbuf[128];
  int32_t  i = 0;

  // Serial
  Serial.begin(SERIALbaud);       // USB terminal
  Serial.print("\n\n Starting...");  Serial.println(vers); Serial.println();
  Serial.print("Serial started: ");  Serial.println(SERIALbaud);

  Serial1.begin(UARTbaud);                              // RX-TX UART
  while (Serial1.available())  Serial1.read();          // clear buffer
  Serial.print("Serial1 started: "); Serial.println(UARTbaud);

  Serial2.begin(GPSbaud);                               // UART GPS
  Serial.print("Serial2 started: "); Serial.println(GPSbaud);
  Serial.println("GPS modul started...");
  Serial.print("   Testing TinyGPS++ library...:  version="); Serial.print(TinyGPSPlus::libraryVersion());
  Serial.println();

  Serial.println("Starting threads...:");
  Scheduler.startLoop(loopUARTcomm);
  Serial.println("   loopUARTcomm UART Muxer task started ");

  Scheduler.startLoop(loopGPSread);
  Serial.println("   loopGPSread GPS task started ");
  Serial.println();

  Serial.println("setup(): done.");
  Serial.println();

}







//=====================================================================================
void getGPSvalues()
{
  char sbuf[128];

  //----------------------------
  if (gps.location.isValid())
  {
    fGPSlat = (double)gps.location.lat();
    fGPSlng = (double)gps.location.lng();
    sprintf(sbuf, "GPS data: Lat: %+014.9f , Lng: %+014.9f ", fGPSlat, fGPSlng );
    Serial.print(sbuf);
  }
  else {
    fGPSlat = INVALID;
    fGPSlng = INVALID;
    Serial.print("GPS data:  Location:  INVALID      ");
  }

  //----------------------------
  if (gps.speed.isValid())
  {
    fGPSspeed = (double)gps.speed.kmph();
    sprintf(sbuf, "speed: %5.1f ", fGPSspeed );
    Serial.print(sbuf);
  }
  else  {
    fGPSspeed = INVALID;
    Serial.print(" Speed: INVALID ");
  }
  dspeedx32 = (int16_t)(fGPSspeed * 32.0); // *32


  //----------------------------
  if (gps.course.isValid())
  {
    fGPScourse = (double)gps.course.deg();
    sprintf(sbuf, "course: %5.1f ", fGPScourse );
    Serial.print(sbuf);
  }
  else {
    fGPScourse = INVALID;
    Serial.print(" course: INVALID ");
  }
  dcoursex32 = (int16_t)(fGPScourse * 32.0); // *32


  //----------------------------
  bool gpsdatevalid=false;
  if (gps.date.isValid())
  {     
    gpsdatevalid=true;
    dday = (int8_t)gps.date.day();
    dmonth = (int8_t)gps.date.month();
    dyear = (int16_t)gps.date.year();
    sprintf(sbuf, " Date: %02d/%02d/%04d ", dday, dmonth, dyear);
    Serial.print(sbuf);
  }
  else {
    dday=INVALCH; dmonth=INVALCH; dyear=INVALID;
    Serial.print( " Date: INVALID  ");
  }

  //----------------------------
  bool gpstimevalid=false;
  if (gps.time.isValid())
  {
    gpstimevalid=true;
    dhour = (int8_t)gps.time.hour();
    dmin = (int8_t)gps.time.minute();
    dsec = (int8_t)gps.time.second();
    dcsec = (int8_t)gps.time.centisecond();
    sprintf(sbuf, " Time: %02d:%02d:%02d ", dhour, dmin, dsec);
    Serial.print(sbuf);
  }
  else {
    dhour=INVALCH; dmin=INVALCH; dsec=INVALCH;
    Serial.print( " Time: INVALID  ");
  }

  if( gpstimevalid && gpsdatevalid ){
    utc_time = tmConvert_t(dyear,dmonth,dday,dhour,dmin,dsec); // 
    local_time = CE.toLocal(utc_time, &tcr); 
    setTime(local_time);    
    sprintf(sbuf, "\n LocalTime: %02d:%02d:%02d ",  (int)hour(),(int)minute(),(int)second() );
    Serial.print(sbuf);    
    sprintf(sbuf, " utc=%ld, LocalDate: %02d.%02d.%4d  UTC%-+2d", utc_time,
             (int)day(),(int)month(),(int)year(),(local_time-utc_time)/3600);    
    Serial.println(sbuf);
  }
  else {      
    utc_time=INVALID; local_time=INVALID;      
  }

  //----------------------------
  if (gps.satellites.isValid()) {
    nGPSsat = (int8_t)gps.satellites.value();
    sprintf(sbuf, " Satel.=% 2d ", nGPSsat);
    Serial.print(sbuf);
  }
  else {
    nGPSsat = INVALCH;
    Serial.print(" SatSignal: INVALID ");
  }

  Serial.println();
}



//=====================================================================================

//=====================================================================================
