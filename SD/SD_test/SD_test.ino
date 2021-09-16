/*
SD card: fprintf_() und fscanf_()
ver 1.01
*/

#include <SPI.h>
#include <SD.h>
#include <stdarg.h>
#include <stdio.h>
#include <ardustdio.h>


// SD Card
#define SD_CSpin 4
File myFile;
char fname[64];

char sdata[128];
char sbuf[128];





void setup()
{
  int32_t  p, i, cnt;
  char     sval[20];
  int32_t  ival, n, m;
  float   fval, x, y;
  // alternativ, ohne jeden Effekt: float   fval, x, y;

  pinMode(SD_CSpin, OUTPUT); 
  Serial.begin(9600);
 
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
    fprintf_(&myFile, "%d %d %f %f\n",  1, 2, PI, 4.567890);   
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
 
 
 
 
 
  // re-open the file for reading:
  Serial.println();
 
  myFile = SD.open(fname);
  if (myFile) {
    sprintf(sbuf,"#: reading %s ",fname);
    Serial.println(sbuf); 
   
    // read from the file until there's nothing else in it:
    i=0;
    cnt=0; 
    strcpy(sdata, "");
   
    //---------------------------------------------------------------------------------
    cnt = fscanf_(&myFile, "%d %d %f %f", &m, &n, &x, &y);
    //---------------------------------------------------------------------------------
                           
    Serial.println("# nach Aufruf cnt=fscanf_ im Hauptprogramm");   
       // Testausgabe:
       Serial.print("returned cnt="); Serial.println(cnt);   
       Serial.println();
       Serial.println("returned reformatted variables m,n,x,y:");
       Serial.println(m);
       Serial.println(n);
       Serial.println(x);
       Serial.println(y);

   
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
