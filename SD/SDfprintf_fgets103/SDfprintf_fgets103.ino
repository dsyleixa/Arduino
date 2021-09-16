/*
SD card: fprintf_() und fgets_()
ver 1.01
*/


#include <SPI.h>
#include <SD.h>



// SD Card
#define SD_CSpin 38  //  <<<<<<<<<<<<<<<<<<<<<< adjust !!

File SDfile;
char fname[64];

char sdata[128];
char sbuf[128];



//******************************************************************************************** 

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


//******************************************************************************************** 

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


//******************************************************************************************** 


File fopen_(char * filename, const char * mode) {
   int16_t  IOerror=0;
   File     tfile;
 
   if(mode=="w") {                               // remove/rewrite
      IOerror = SD.remove(filename);
      tfile  = SD.open(filename, FILE_WRITE);  
   }  
   else
   if(mode=="a") {                               // append at EOF
      tfile  = SD.open(filename, FILE_WRITE);  
   } 
   else
   if(mode=="r") {
      tfile  = SD.open(filename, FILE_READ);      // open at beginning of file
   } 
   return tfile;
  
}


//******************************************************************************************** 

int16_t   fclose_(File tfile) { tfile.close(); return 0; }


//******************************************************************************************** 



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
    sprintf(sbuf,"#: %s already exists ... to be removed ...",fname);
    Serial.println(sbuf);   
  }
    
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.  
  SDfile = fopen_(fname, "w");
  
  if (SDfile) {
    // if the file opened okay, write to it, then close file:
    sprintf(sbuf,"#: Writing strings to %s ...:",fname); 
    Serial.println(sbuf);
    sprintf(sbuf, "%s\n%d\n%d\n%f\n%f\n", "Teststring", 1, 2, PI, 4.567890);
    Serial.println(sbuf);
    
    //---------------------------------------------------------------------------------
    // write data to file
    fprintf_(&SDfile, "%s\n%d\n%d\n%f\n%f\n", "Teststring", 1, 2, PI, 4.567890);    
    //---------------------------------------------------------------------------------
   
    // close the file:
    fclose_(SDfile);
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
  SDfile = fopen_(fname, "r");
  if (SDfile) {
    sprintf(sbuf,"#: reading %s ",fname);
    Serial.println(sbuf);  
    
    // read from the file until there's nothing else in it:
    i=0;
    cnt=1;   
    
    while (SDfile.available()) {
      strcpy(sdata, ""); 
      fgets_ ( sdata, 20, &SDfile );  
      Serial.print(cnt); Serial.print(": string raw="); Serial.print(sdata); 
      Serial.println("rueckformatiert:");
      if (cnt==1) {Serial.print("str  ="); Serial.println(sdata); }
      if (cnt==2) {Serial.print("int  ="); Serial.println(atoi(sdata) ); }
      if (cnt==3) {Serial.print("int  ="); Serial.println(atoi(sdata) ); }
      if (cnt==4) {Serial.print("float="); Serial.println(atof(sdata) ); }
      if (cnt==5) {Serial.print("float="); Serial.println(atof(sdata) ); }
      ++cnt;
      Serial.println();
    }  
    
    // close the file:
    fclose_(SDfile);        
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
