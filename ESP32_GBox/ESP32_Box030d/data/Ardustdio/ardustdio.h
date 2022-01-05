// Library: ardustdio.h
// kind of stdio.h functionality for files plus extended C-string manipulation
//
// (C) dsyleixa 2015-2019
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.6.1 - 1.6.3)
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/   //   




#ifndef  ARDUSTDIO_H
#define ARDUSTDIO_H

// #include <SD.h>


/*
int     readDirectory(File dir, int dirLevel);
long    fprintf_( File * stream, const char fmtstr[], ... );     // see ANSI C: fprintf()
long    fscanf_ ( File * stream, const char fmtstr[], ... ) ;    // see ANSI C: fscanf()
char  * fgets_  ( char * str, int32_t num, File * stream );      // see ANSI C: fgets()

File    fopen_  ( char * filename, const char * mode)            // see ANSI C: fopen()
int16_t fclose_ ( File SDfile)                                   // see ANSI C: fclose()  
int16_t remove_ ( char * filename) {                             // see ANSI C: remove()

bool SdExist(fs::FS &fs)                                         // SD card test, unbuffered


*/

//------------------------------------------------------------
//------------------------------------------------------------


#define fileIO_ERR_CREATE    -40
#define fileIO_ERR_OPEN      -41
#define fileIO_ERR_REMOVE    -42
#define fileIO_ERR_WRITE     -43
#define fileIO_ERR_READ      -44


#define E_NOERR               0  // fileIO OK 
#define EPERM                 1  // Operation not permitted 
#define ENOENT                2  // No such file or directory 
#define EIO                   5  // I/O error 
#define ENXIO                 6  // No such device or address
#define EBADF                 9  // Bad file number 
#define EACCES               13  // Permission denied 
#define EFAULT               14  // Bad address 
#define EEXIST               17  // File exists 
#define ENODEV               19  // No such device 
#define EMFILE               24  // Too many open files 
#define EROFS                30  // Read-only file system 
#define ETIMEDOUT           110  // Connection timed out 
#define ENOMEDIUM           123  // No medium found 
#define EMEDIUMTYPE         124  // Wrong medium type 

#define EUNKNOWN            127  // Unknown error

//------------------------------------------------------------

String STRERROR(int num) {
   if(num==0)  return    "fileIO OK"; 
   if(num==1)  return    "Operation not permitted"; 
   if(num==2)  return    "No such file or directory"; 
   if(num==5)  return    "I/O error";  
   if(num==6)  return    "No such device or address"; 
   if(num==9)  return    "Bad file number";   
   if(num==13)  return   "Permission denied";  
   if(num==14)  return   "Bad address"; 
   if(num==17)  return   "File exists"; 
   if(num==19)  return   "No such device"; 
   if(num==24)  return   "Too many open files"; 
   if(num==30)  return   "Read-only file system"; 
   if(num==110)  return  "Connection timed out"; 
   if(num==123)  return  "No medium found"; 
   if(num==124)  return  "Wrong medium type"; 
   if(num==127)  return   "Unknown error";

   if(num==-40)  return   "Error create file"; 
   if(num==-41)  return   "Error open file"; 
   if(num==-42)  return   "Error remove file"; 
   if(num==-43)  return   "Error write file"; 
   if(num==-44)  return   "Error read file";    

   return "Unknown error";
}




//------------------------------------------------------------

File SdPath;

volatile int filecount = 0;
int maxFsEntries=128;

//String filelist[maxFsEntries];
//String *filelist = new String[128];
std::vector<String> filelist(maxFsEntries);


//=================================================================
int  readDirectory(File dir, int dirLevel) {   
   if(filecount==0) {       // <<<<<< shifted, NEW
      filelist[0]="/";   
      filecount++;
   }

   while (true) {
      if(filecount>=maxFsEntries-1) {
         filelist.push_back("");
         maxFsEntries++;
      }
      File entry =  dir.openNextFile();
      if (! entry) {
         // no more files
         break;
      }

      //Serial.print(entry.name());
      filelist[filecount] = (String)entry.name();
      
      if (!entry.isDirectory()) {
         filecount++;
      }
      else if (entry.isDirectory()) {
         filelist[filecount] += (String)"/"; 
         filecount++;
         readDirectory(entry, dirLevel + 1);
      } 
      
      entry.close();
   }
   return filecount;
}


//=================================================================






#if defined (__arm__)  // ARM Cortex compatible

int32_t  fscanf_ ( File * stream, const char fmtstr[], ... ) { // see C: fscanf() 
   const  int32_t   MAXSTRSIZE = 1024;
   char   str[MAXSTRSIZE];
   va_list   args; 
   int32_t   i=0, cnt=0;
   int16_t   chr, argcnt=0;
   char      argstart, ch;
 
   va_start(args, fmtstr); 
   strcpy(str, "");
   while ( (stream->available()) && (i < MAXSTRSIZE-1) ) {   
      chr = stream->read() ;
      
      if (chr>=0 && chr!='\n') { // additionally limit to number of arguments! <
           ch = (char)chr;  
           str[i] = ch;     
           ++i;
      }
      else break;     
   }  
   str[++i] = '\0';                                                         
   cnt = vsscanf( str, fmtstr, args ); 
   va_end(args);

   return cnt;
}
#endif

//------------------------------------------------------------

int32_t fprintf_ ( File * stream, const char fmtstr[], ... ) { // see C: fprintf()
   const  int32_t   MAXSTRSIZE = 1024;
   char   str[MAXSTRSIZE];
   va_list   args;
   int32_t   num;
   
   va_start( args, fmtstr );
   num = vsnprintf(str, sizeof(str), fmtstr, args);
   stream->print(str);
   va_end( args );

   return num;
}

//------------------------------------------------------------



char * fgets_ ( char * str, int32_t num, File * stream ) { // see ANSI C: fgets()
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

//------------------------------------------------------------

File fopen_(char * filename, const char * mode) {     // see ANSI C: fopen()
   int16_t  IOresult=0;
   File     file_ ;     // can't be initialized to NULL !
 
   if(mode=="w") {                                 // remove/rewrite
      IOresult = SD.remove(filename);              // success==TRUE, failed==FALSE
      file_    = SD.open(filename, FILE_WRITE); 
   } 
   else
   if(mode=="a") {                                 // append at EOF
      file_  = SD.open(filename, FILE_WRITE); 
   }
   else
   if(mode=="r") {
      file_  = SD.open(filename, FILE_READ);       // open at beginning of file
   }
   return file_; 
}


//------------------------------------------------------------
//------------------------------------------------------------

int16_t   fclose_(File SDfile) {                   // see ANSI C: fclose()
   SDfile.close();
   return E_NOERR ;
}   

//------------------------------------------------------------

int16_t  remove_ (char * filename) {               // see ANSI C: remove()
   int16_t  IOresult=0;

   if (SD.exists(filename) ) {       
       IOresult=SD.remove(filename);       
       // removed: success ?
       if (IOresult) return E_NOERR;              // SD file remove OK
       else  return fileIO_ERR_REMOVE;            // SD file remove failed   
    }
    else    return ENOENT;                        // SD file name not found 
}


//------------------------------------------------------------
//------------------------------------------------------------


bool SdExist(fs::FS &fs){
    Serial.println("Creating foo.txt file to detect the SD card");
    if(fs.open("/fooo.txt","w")) 
    {
        fs.remove("/fooo.txt");
        Serial.println("Succesfuly created and removed.");
        Serial.println("SD card detected");
        return true;
    }
    else 
    {
        Serial.println("Error in creating file");
        Serial.println("SD card not detected");
        return false;
    }
}


//------------------------------------------------------------
#endif
//------------------------------------------------------------
//------------------------------------------------------------