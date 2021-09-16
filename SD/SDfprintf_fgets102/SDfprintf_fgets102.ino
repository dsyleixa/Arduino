/*
SD card: fprintf_() und fgets_()
ver 1.01
*/


#include <SPI.h>
#include <SD.h>



// SD Card
#define SD_CSpin 38  //  <<<<<<<<<<<<<<<<<<<<<< adjust !!

File myFile;
char fname[64];

char sdata[128];
char sbuf[128];



int32_t fprintf_ ( File * stream, const char fmtstr[], ... ) {
   char      str[1024];
   va_list   args;
   int32_t   num;
   
   va_start( args, fmtstr );
   num = vsnprintf(str, sizeof(str), fmtstr, args);
   stream->print(str);
   va_end( args );

   return num;
}




char * fgets_ ( char * str, int32_t num, File * stream ) {
  int32_t i = 0;

  strcpy(str, "");
  while ( (stream->available()) && (i < num-1) ) {
    int16_t ch = stream->read();
    if (ch < 0)     // end of file
      break;
    str[i++] = ch;
    if ('\n' == ch) // end of line
      break;
  }

  if (i) {          // room in buffer for terminating null
    str[i] = 0;
    return str;
  }
  else
  // return NULL;                  // buffer too small or immediate end of file
  { strcpy(str, ""); return str; } // alternative: return ""
}





void setup()
{
  int16_t  p, i, cnt;
  float    x;
  char     sval[20];
  int16_t  ival;
  double   fval;
  
  pinMode(SD_CSpin, OUTPUT);  
  Serial.begin(115200);
   
  sprintf(sbuf,"#: SD Initializing... ");
  Serial.println(sbuf);  

  while(!SD.begin(SD_CSpin) ) {
    sprintf(sbuf,"#: ...SD init failed ");
    Serial.println(sbuf); 
    delay(1000);
  }
  
  sprintf(sbuf,"#: ...SD OK !      ");
  Serial.println(sbuf);  
  strcpy(fname,"test.txt");
  
  if (SD.exists(fname) ) {
    sprintf(sbuf,"#: %s exists     ",fname);
    Serial.println(sbuf);  
    
    sprintf(sbuf,"#: Removing %s      ",fname);   
    Serial.println(sbuf); 
    
    SD.remove("test.txt");
    // removed: success ?
    if (SD.exists(fname) ) {
       sprintf(sbuf,"#: %s  exists     ",fname);  
       Serial.println(sbuf);  
    }
    else {
       sprintf(sbuf,"#: %s  N/A     ",fname);  
       Serial.println(sbuf);  
     }
  }
    
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.  
  myFile = SD.open(fname, FILE_WRITE);
  
  if (myFile) {
    // if the file opened okay, write to it, then close file:
    sprintf(sbuf,"#: Writing strings to %s ",fname); 
    Serial.println(sbuf);
    
    //---------------------------------------------------------------------------------
    // write data to file
    fprintf_(&myFile, "%s\n%d\n%d\n%f\n%f\n", "Teststring", 1, 2, PI, 4.567890);    
    //---------------------------------------------------------------------------------
   
    // close the file:
    myFile.close();
    sprintf(sbuf,"#: %s closed.   ",fname);
    Serial.println(sbuf);  
  }  
  else {
    // if the file didn't open, print an error:
    sprintf(sbuf,"#: error opening %s   ",fname);
    Serial.println(sbuf);      
  }
  
  Serial.println(); 
  // re-open the file for reading:
  myFile = SD.open(fname);
  if (myFile) {
    sprintf(sbuf,"#: reading %s ",fname);
    Serial.println(sbuf);  
    
    // read from the file until there's nothing else in it:
    i=0;
    cnt=1;   
    
    while (myFile.available()) {
      strcpy(sdata, ""); 
      fgets_ ( sdata, 20, &myFile );  
      Serial.print(cnt); Serial.print(": string raw="); Serial.println(sdata); 
      Serial.println("rueckformatiert:");
      if (cnt==1) {Serial.print("str  ="); Serial.println(sdata); }
      if (cnt==2) {Serial.print("int  ="); Serial.println(atoi(sdata) ); }
      if (cnt==3) {Serial.print("int  ="); Serial.println(atoi(sdata) ); }
      if (cnt==4) {Serial.print("float="); Serial.println(atof(sdata) ); }
      if (cnt==5) {Serial.print("float="); Serial.println(atof(sdata) ); }
      ++cnt;
    }  
    
    // close the file:
    myFile.close();        
    sprintf(sbuf,"#: %s closed. ",fname); 
    Serial.println(sbuf);      
  } else {
    // if the file didn't open, print an error:
    sprintf(sbuf,"#: error opening %s   ",fname);      
    Serial.println(sbuf);

  }
}

void loop()
{
  // nothing happens after setup
}
