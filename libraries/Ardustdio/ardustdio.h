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
int     readDirectory(File dir, int numLevel);
long    fprintf_( File * stream, const char fmtstr[], ... );     // see ANSI C: fprintf()
long    fscanf_ ( File * stream, const char fmtstr[], ... ) ;    // see ANSI C: fscanf()
char  * fgets_  ( char * str, int32_t num, File * stream );      // see ANSI C: fgets()

File    fopen_  ( char * filename, const char * mode)            // see ANSI C: fopen()
int16_t fclose_ ( File SDfile)                                   // see ANSI C: fclose()  
int16_t remove_ ( char * filename) {                             // see ANSI C: remove()

*/

//------------------------------------------------------------
//------------------------------------------------------------


#define fileIO_ERR_CREATE    -1
#define fileIO_ERR_OPEN      -2
#define fileIO_ERR_REMOVE    -4
#define fileIO_ERR_WRITE     -8
#define fileIO_ERR_READ      -16

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

#define E_NOERRstr              "fileIO OK" 
#define EPERMstr                "Operation not permitted" 
#define ENOENTstr               "No such file or directory"
#define EIOstr                  "I/O error" 
#define ENXIOstr                "No such device or address"
#define EBADFstr                "Bad file number" 
#define EACCESstr               "Permission denied" 
#define EFAULTstr               "Bad address" 
#define EEXISTstr               "File exists" 
#define ENODEVstr               "No such device" 
#define EMFILEstr               "Too many open files" 
#define EROFSstr                "Read-only file system" 
#define ETIMEDOUTstr            "Connection timed out" 
#define ENOMEDIUMstr            "No medium found" 
#define EMEDIUMTYPEstr          "Wrong medium type" 




//------------------------------------------------------------



String filelist[64];
int filecount = 0;

File SdPath;


//=================================================================
int  readDirectory(File dir, int numLevel) {
   while (true) {

      File entry =  dir.openNextFile();
      if (! entry) {
         // no more files
         break;
      }
      for (uint8_t i = 0; i < numLevel; i++) {
         //Serial.print('\t');
      }

      //Serial.print(entry.name());
      filelist[filecount] = (String)entry.name();

      if (entry.isDirectory()) {
         //Serial.println("/");
         filelist[filecount] += (String)"/";
         //Serial.println(filelist[filecount]);

         //display.setCursor(20 + 240 * (filecount % 2), 16 * (filecount / 2));
         //display.println(filelist[filecount]);

         filecount++;
         filelist[filecount] = filelist[filecount] = (String)entry.name() + "/..";
         //Serial.println(filelist[filecount]);

         //display.setCursor(20 + 240 * (filecount % 2), 16 * (filecount / 2));
         //display.println(filelist[filecount]);
         filecount++;
         readDirectory(entry, numLevel + 1);
      } else {
         // files have sizes, directories do not
         //Serial.println(filelist[filecount]);
         //Serial.print("\t\t");
         //Serial.println(entry.size(), DEC);
           
         //display.setCursor(20 + 240 * (filecount % 2), 16 * (filecount / 2));
         //display.println(filelist[filecount]);

         filecount++;
      }
      entry.close();
   }
   return filecount;
}

//------------------------------------------------------------

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
#endif
//------------------------------------------------------------
//------------------------------------------------------------