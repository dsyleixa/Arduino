// Arduino COM to Borland C++ Builder form 
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose: only 
// by personal written permission by the author.

// default TFT: OLED 128x64, compatible to Adafruit (R) Libs
// in this example: using an ESP8266 NodeMCU 1.0 board
// using ButtonClass for button action (up, down, enter, single/double/long press) 

// history:
// 0.0.3  send/receive strings
// 0.0.2  receiving strings, pattern: &varname1=value1;
// 0.0.1  receiving simple Serial char 

// simple Rx/Tx example 
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
    
    cstringarg(cbuf, "LEDBI", cval); // LEDBI: LED_BUILTIN    
    if(strlen(cval)>0) {          
      writeDPin(LED_BUILTIN, atoi(cval) );       
    }  
    
    // unrolled for debugging 
    /*
    cstringarg(cbuf, "DPIN0", cval); // caution!    
    if(cval!="") {          
      writeDPin(0, atoi(cval));      
    }    
    cstringarg(cbuf, "DPIN1", cval); // caution!   
    if(cval!="") {          
      writeDPin(1, atoi(cval));      
    }  
    */
     
    cstringarg(cbuf, "DPIN2", cval); //      
    if(strlen(cval)>0)) {          
      writeDPin(2, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN3", cval); //  
    if(strlen(cval)>0) {          
      writeDPin(3, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN4", cval); //      
    if(strlen(cval)>0) {          
      writeDPin(4, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN5", cval); //      
    if(strlen(cval)>0) {          
      writeDPin(5, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN6", cval); //      
    if(strlen(cval)>0) {          
      writeDPin(6, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN7", cval); //    
    if(strlen(cval)>0) {          
      writeDPin(7, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN8", cval); //    
    if(strlen(cval)>0) {          
      writeDPin(8, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN9", cval); //      
    if(strlen(cval)>0) {          
      writeDPin(9, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN10", cval); //      
    if(strlen(cval)>0) {          
      writeDPin(10, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN11", cval); //      
    if(strlen(cval)>0) {          
      writeDPin(11, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN12", cval); //      
    if(strlen(cval)>0) {          
      writeDPin(12, atoi(cval));      
    }  
 
    cstringarg(cbuf, "DPIN13", cval); //        
    if(strlen(cval)>0) {          
      writeDPin(13, atoi(cval));      
    }  

    cstringarg(cbuf, "DPIN14", cval); // caution!       
    if(strlen(cval)>0) {          
      writeDPin(14, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN15", cval); // caution!       
    if(strlen(cval)>0) {          
      writeDPin(15, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN16", cval); // caution!       
    if(strlen(cval)>0) {          
      writeDPin(16, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN17", cval); // caution!       
    if(strlen(cval)>0) {          
      writeDPin(17, atoi(cval));      
    }  
    cstringarg(cbuf, "DPIN18", cval); // caution!       
    if(strlen(cval)>0) {          
      writeDPin(18, atoi(cval));      
    }        
    cstringarg(cbuf, "DPIN19", cval); // caution!       
    if(strlen(cval)>0) {          
      writeDPin(19, atoi(cval));      
    } 
    cstringarg(cbuf, "DPIN20", cval); // caution!    
    if(strlen(cval)>0) {          
      writeDPin(20, atoi(cval));      
    }   
    
    
    inputString="";
    stringComplete = false;

    //delay
    delay(10);
  }
  //-------------------------------------------------------------
  // send
  
  int i0, i1, i2, i3, i4, i5, i6, i7, i8;
  double d0, d1, d2, d3, d4, d5;
  
  i0=analogRead(A0);
  i1=analogRead(A1);
  i2=analogRead(A2);
  i3=analogRead(A3);
  i4=444;
  i5=5555;
  i6=-66666;
  i7=777777;
  i8=8888888;

  d0=0;
  d1=10.1; 
  d2=-20.22;
  d3=300.333;
  d4=-4000.4444;
  d5=50000.55555;

  sprintf(cbuf, "&INT0=%d;&INT1=%d;&INT2=%d;&INT3=%d;&INT4=%d;&INT5=%d;&INT6=%d;&INT7=%d;&INT8=%d;&DBL0=%f;&DBL1=%f;&DBL2=%f;&DBL3=%f;&DBL4=%f;&DBL5=%f;", 
                 i0,i1,i2,i3,i4,i5,i6,i7,i8,d0,d1,d2,d3,d4,d5);
                 
  Serial.println(cbuf);
  strcpy(cbuf, "");  

  //delay
  delay(10); 
}

// end of file
