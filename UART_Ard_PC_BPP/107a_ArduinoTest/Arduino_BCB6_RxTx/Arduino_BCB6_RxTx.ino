// Arduino COM to Borland C++ Builder form 
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose: only 
// by personal written permission by the author.

// default TFT: OLED 128x64, compatible to Adafruit (R) Libs
// in this example: using an ESP8266 NodeMCU 1.0 board
// using ButtonClass for button action (up, down, enter, single/double/long press) 

// history:
// 0.0.7  pin-loop, 6x out-pins OUTn(I/O/pwm), &_ALLPINS_=0 when BCB-close
// 0.0.6  output pins DPINn
// 0.0.3  send/receive strings
// 0.0.2  receiving strings, pattern: &varname1=value1;
// 0.0.1  receiving simple Serial char 

// BCB-Arduino: Arduino to Borland C++ Builder
// ver 0.0.3


// i2c
#include <Wire.h>   // Incl I2C comm, but needed for not getting compile error



// notice:
// on Mega2560 the anaolgWrite() pwm function works on all pins 2...13 (thus also for LED_BUILTIN) plus some evtra ones.
// on ATmega168 or ATmega328P the anaolgWrite() pwm function works on pins 3, 5, 6, 9, 10, and 11 
// thus (not for LED_BUILTIN).
// on Mega2560 the anaolgWrite() pwm function works on all pins 2...13 
// (thus also for LED_BUILTIN) plus some evtra ones.



//----------------------------------------------------------------------------
// tools
//----------------------------------------------------------------------------

#define iINVALID -29999

int16_t  strstrpos(char * haystack,  char * needle)   // find 1st occurance of substr in str
{
   char *p = strstr(haystack, needle);
   if (p) return p - haystack;
   return -1;   // Not found = -1.
}


//------------------------------------------------------------

#define TOKLEN 30
#define MSGLEN 1024


char * cstringarg( char* haystack, char* vname, char* sarg ) {
   int i=0, pos=-1;
   unsigned char  ch=0xff;
   const char*  kini = "&";       // start of varname: '&'
   const char*  kin2 = "?";       // start of varname: '?'
   const char*  kequ = "=";       // end of varname, start of argument: '='
   char  needle[TOKLEN] = "";     // complete pattern:  &varname=abc1234


   strcpy(sarg,"");
   strcpy(needle, kini);
   strcat(needle, vname);
   strcat(needle, kequ);
   pos = strstrpos(haystack, needle); 
   if(pos==-1) {
      needle[0]=kin2[0];
      pos = strstrpos(haystack, needle);
      if(pos==-1) return sarg;
   }
   pos=pos+strlen(vname)+2; // start of value = kini+vname+kequ   
   while( (ch!='&')&&(ch!='\0') ) {
      ch=haystack[pos+i];    
      if( (ch=='&')||(ch==';')||(ch==' ')||(ch=='\0') ||(ch=='\n')
        ||(i+pos>=strlen(haystack))||(i>TOKLEN-1) ) {
           sarg[i]='\0';
           return sarg;
      }       
      if( (ch!='&') ) {
          sarg[i]=ch;          
          i++;       
      }      
   } 
   return sarg;
}


void writeDPin(int pin, int ival) {
  pinMode(pin, OUTPUT);  // safety
  if(ival==0)   { digitalWrite(pin, LOW); }
  else
  if(ival==255) { digitalWrite(pin, HIGH); }
  else {       
     analogWrite(pin, ival); 
  }   
}
//----------------------------------------------------------------------------
// setup
//----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

}


//----------------------------------------------------------------------------
// loop
//----------------------------------------------------------------------------

String  inputString="";
char    cval[20];      // number as cstring
char    cbuf[MSGLEN];  // cstring buffer


void loop() {
  static bool stringComplete = false;

  //-------------------------------------------------------------
  // receive
  
  while (Serial.available() ) {
    char inChar = (char)Serial.read();
 
    inputString += inChar;
    if (inChar == '\n')  {
      stringComplete = true;
    }
  }

  if (stringComplete)  {
    inputString.toCharArray(cbuf, MSGLEN-1);
    
    // cstringarg( char* haystack, char* vname, char* sarg )
    // haystack pattern: &varname=1234abc;  delimiters &, \n, \0, SPACE, EOF 

    bool LED_BUILTIN_set = false;  
    
    cstringarg(cbuf, "LEDBI", cval); // LEDBI: LED_BUILTIN    
    if(strlen(cval)>0) {          
      writeDPin(LED_BUILTIN, atoi(cval) ); 
      LED_BUILTIN_set = true;      
    }  
 
    for(int o=2; o<=13; o++) {        // adjust output pins! 
                                      // avoid LED_BUILTIN call conflicts!
        char keyw[20];
        sprintf(keyw, "OUT%d", o);    // OUT2, OUT3, ...
        cstringarg(cbuf, keyw, cval); 
        if(strlen(cval)>0) {           
           writeDPin(o, atoi(cval));      
        } 
        cstringarg(cbuf, "_ALLPINS_", cval); 
        if(strlen(cval)>0) {        
           writeDPin(o, atoi(cval));      
        } 
    
    }

    /*
    for(int o=15; o<=16; o++) {        // adjust output pins for ESP!!
       char keyw[20];
       sprintf(keyw, "OUT%d", o);    // OUT2, OUT3, ...
       cstringarg(cbuf, keyw, cval); 
       if(strlen(cval)>0) {          
          writeDPin(o, atoi(cval));      
       } 
       cstringarg(cbuf, "_ALLPINS_", cval); 
       if(strlen(cval)>0) {        
           writeDPin(o, atoi(cval));      
       } 
    }
    */

    /*
    for(int o=22; o<54; o++) {        // adjust output pins for DUE & MEGA!!
       char keyw[20];
       sprintf(keyw, "OUT%d", o);    // OUT2, OUT3, ...
       cstringarg(cbuf, keyw, cval); 
       if(strlen(cval)>0) {          
          writeDPin(o, atoi(cval));      
       } 
       cstringarg(cbuf, "_ALLPINS_", cval); 
       if(strlen(cval)>0) {        
           writeDPin(o, atoi(cval));      
       } 
    }
    */
    
    inputString="";
    stringComplete = false;

    //delay
    delay(5);
  }
  //-------------------------------------------------------------
  // send
  
  int i0, i1, i2, i3, i4, i5, i6;
  int a0, a1, a2, a3, a4, a5;  // analog pins
  double f0, f1, f2, f3, f4, f5;
  //----------------------
  // debug

  #define _DEBUG_      // <<<<<<<<<<< comment/uncomment for testing !!
  #ifdef _DEBUG_
  
  i0=0;
  i1=1;
  i2=-22;
  i3=33;
  i4=-444;
  i5=5555;
  i6= B10101010*256;
  i6+=B10101010;


  f0=0;
  f1=10.1; 
  f2=-20.22;
  f3=300.333;
  f4=-4000.4444;
  f5=50000.55555;

  #endif
   
  // end debug
  //----------------------

  // analogRead UNO: A0...A5(w/o i2c); A0...A3 (with i2c)
  // MEGA, DUE: A0...A11
  a0=analogRead(A0);  
  a1=analogRead(A1);
  a2=analogRead(A2);
  a3=analogRead(A3);
  a4=analogRead(A4);  // caution UNO i2c!
  a5=analogRead(A5);

  // alternatives:
  a5=analogRead(A6);  // caution UNO!
  a5=analogRead(A7);
  a5=analogRead(A8);  // caution UNO!
  a5=analogRead(A9);
  a5=analogRead(A10);  // caution UNO!
  a5=analogRead(A11);

  char formatstr[MSGLEN];
  strcpy(formatstr, "&i0=%d;&i1=%d;&i2=%d;&i3=%d;&i4=%d;&i5=%d;&i6=%d;");
  strcat(formatstr, "&f0=%f;&f1=%f;&f2=%f;&f3=%f;&f4=%f;&f5=%f;");
  strcat(formatstr, "&a0=%d;&a1=%d;&a2=%d;&a3=%d;&a4=%d;&a5=%d;");

  sprintf(cbuf, formatstr, i0,i1,i2,i3,i4,i5,i6, f0,f1,f2,f3,f4,f5, a0,a1,a2,a3,a4,a5 );
                 
  Serial.println(cbuf);
  strcpy(cbuf, "");  

  //delay
  delay(20); 
}

// end of file
