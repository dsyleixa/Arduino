// Arduino COM to Borland C++ Builder form 
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose: only 
// by personal written permission by the author.

// history:
// 0.0.2  receiving strings, pattern: &varname1=value1;
// 0.0.1  receiving simple Serial char 

// simple menu example 
// ver 0.0.2


// i2c
#include <Wire.h>   // Incl I2C comm, but needed for not getting compile error



// notice:
// on ATmega168 or ATmega328P the anaolgWrite() pwm function works on pins 3, 5, 6, 9, 10, and 11 
// (thus not for LED_BUILTIN).
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
#define MSGLEN 256


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
int16_t ival;


void loop() {
  static bool stringComplete = false;
 
  while (Serial.available() ) {
    char inChar = (char)Serial.read();
 
    inputString += inChar;
    if (inChar == '\n')  {
      stringComplete = true;
    }
  }

  if (stringComplete)  {
    ival = iINVALID;
    inputString.toCharArray(cbuf, MSGLEN-1);
    
    // cstringarg( char* haystack, char* vname, char* sarg )
    // haystack pattern: &varname=1234abc,  delimiters &, \n, \0, SPACE, EOF 
    
    cstringarg(cbuf, "LEDBI", cval); // LEDBI: LED_BUILTIN    
    if(cval!="") {    
      pinMode(LED_BUILTIN, OUTPUT);  // safety
      ival=atoi(cval);
      if(ival==0)  { digitalWrite(LED_BUILTIN, LOW); }
      else
      if(ival==255) { digitalWrite(LED_BUILTIN, HIGH); }
      else {       
         analogWrite(LED_BUILTIN, ival); 
      }   
    }
    
    inputString = "";
    stringComplete = false;
  }

}

// end of file
