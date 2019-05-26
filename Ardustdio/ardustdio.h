// Library: ardustdio.h
// kind of stdio.h functionality for files plus extended C-string manipulation
//
// (C) Helmut Wunder (HaWe) 2015
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.6.1 - 1.6.3)
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/   //   




#ifndef  ARDUSTDIO_H
#define ARDUSTDIO_H
// #include <SD.h>
/*


long     fprintf_( File * stream, const char fmtstr[], ... );     // see ANSI C: fprintf()
long     fscanf_ ( File * stream, const char fmtstr[], ... ) ;    // see ANSI C: fscanf()
char   * fgets_  ( char * str, int32_t num, File * stream );      // see ANSI C: fgets()

File     fopen_  ( char * filename, const char * mode)            // see ANSI C: fopen()
int16_t  fclose_ ( File SDfile)                                   // see ANSI C: fclose()  
int16_t  remove_ ( char * filename) {                             // see ANSI C: remove()

char  * ftoa( char * str, double f, int16_t precision );          // converts float to string by precision (digits)

char  * strinsert( char * source, char * sub, int pos );          // insert a substr into source at pos
char  * strdelnpos(char * source, int16_t  pos, int16_t  sublen); // delete a substr in source at pos
char  * strpatch ( char * source, char * sub, int pos );          // patch source by substr at pos
char  * substr  (  char * source, char * sub, int pos, int len );    // get substring of source at pos by length
int16_t strchpos(  char * str, char ch );                         // find 1st occurence of char ch in string str
int16_t strstrpos( char * haystack,  char * needle)            // find 1st occurance of needle in haystack
char  * stradd(char * s, int n, ...);  // "adds strings" s=sumstring , n=number_in_list, ... = var string list

char * cstringarg( char* haystack, char* vname, char* sarg );

char * sprintDouble(char* s, double val, int width, int prec, bool sign); // substitute to sprintf for floats on AVR

void    delayms(unsigned long ms)            // bug-free thread-safe delay





*/

//------------------------------------------------------------
//------------------------------------------------------------


#define fileIO_ERR_CREATE    -1
#define fileIO_ERR_OPEN      -2
#define fileIO_ERR_REMOVE    -4
#define fileIO_ERR_WRITE     -8
#define fileIO_ERR_READ      -16

#define E_NOERR               0 
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



//------------------------------------------------------------
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

 


char * ftoa(char * str, double f, int16_t precision) { // convert float to string by precision (digits)
  int32_t  p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
  int32_t  intnum, decimal;
  
  char * sret = str;

  if(precision>8) precision=8;
  
  intnum = (long)f;
  itoa( intnum, str, 10);
  
  while ( *str != '\0') str++;
  *str++ = '.';
  
  decimal = abs( (long)( (f - intnum ) * p[precision]) );  
  itoa( decimal, str, 10);

  return sret;
}

//------------------------------------------------------------


char *  ftos(char*str, int len, int prec, double var) {
   int16_t l, p;
   
   dtostrf(var,len,prec,str);
    
   l = strlen(str); 
   p = strchpos(str, '.');  

   if (l>len && p<len ) {  
      dtostrf( var,len, max(0,len-p-1), str );  
   } 
   else
   if ((p<=0 || p>=len) && l>len) { 
      p= max(0,len-6); 
      l= strlen( dtostre( var, str, p, 0 ) );
      if(l<len && p==0) {
        strcat(str, " ");  
      }
   }    
   return str;   
}


//------------------------------------------------------------

char * strinsert(char * source, char * sub, int16_t  pos) {  // insert a substr into a string at pos
   int   srclen, sublen;
   char  * sret = source;

   srclen = strlen(source);
   sublen = strlen(sub);

   if( pos>srclen ) pos=srclen;
   memmove( source+pos+sublen, source+pos, sublen+srclen-pos );
   memcpy ( source+pos, sub, sublen );

   source[srclen+sublen]= '\0';
   return sret;
   
}  


//------------------------------------------------------------

char * strdelnpos( char * source, int16_t  pos, int16_t  sublen ) {  // delete a substr in string at pos
   int   srclen;
   char  * sret = source;

   srclen = strlen(source);

   if( pos > srclen ) return sret;
   if( sublen > srclen-pos ) sublen = srclen-pos;
   memmove( source+pos, source+pos+sublen, srclen+sublen);
   source[srclen-sublen]= '\0';
   return sret;   
}   
 


//------------------------------------------------------------

char * strpatch( char * source, char * sub, int16_t  pos )  {  // patch string by substr at pos
   int16_t srclen, sublen;
   char  * sret = source;
   
   srclen = strlen(source);
   sublen = strlen(sub);
   
   if( pos+sublen > srclen) return sret;   // size/position error
   memcpy(source+pos, sub, sublen);    

   return sret;     
}



//------------------------------------------------------------

char  * substr  ( char * source, char * sub, int16_t  pos, int16_t len ) { // get substr of source at pos by length 
   char   *sret = sub;

   if ( (pos+len) >  strlen(source) ) len = strlen(source)-pos;  // cut away if too long
   sub = strncpy(sub, source+pos, len);
   sub[len] = '\0';
  
   return sret;  
}




//------------------------------------------------------------

int16_t strchpos( char * str, char ch ) {       // find 1st occurence of char ch in string str
   int16_t  len, i=-1;
   
   len=strlen(str);
   if(len==0) return i;
   for(i=0; i<len; ++i) {
     if(str[i]==ch) break;   
   }   
   return i;
}   




//------------------------------------------------------------


int16_t  strstrpos(char * haystack,  char * needle)   // find 1st occurance of substr in str
{
   char *p = strstr(haystack, needle);
   if (p) return p - haystack;
   return -1;   // Not found = -1.
}


//------------------------------------------------------------

char * stradd(char * s, int n, ...)  // "adds strings" s=sumstring , n=number_in_list, ... = var string list
{
    va_list vlst;    
    int i;
    
    char * bufptr;
    char * retptr = s;
    
    va_start(vlst, n);
    for (i=1; i<=n; ++i)   {
       bufptr = va_arg(vlst, char *);
       strcat(s, bufptr);
    }
    va_end(vlst);	
    return retptr;    
}

//------------------------------------------------------------

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


//------------------------------------------------------------

char * sprintDouble(char* s, double val, int width, int prec, bool sign)  {   
   char sbuf[20] ="\0";
   strcpy(s, "\0");
   dtostrf(val, width, prec, s);
   if(sign && val>0) {
     for (int i=width-1; i>=0; i--) {
       if(s[i]==' ') {s[i]='+'; break;}
     }
   }
   return s;
}

//------------------------------------------------------------

void delayms(unsigned long ms)
{
    uint32_t start = micros();

    while (ms > 0) {
        yield();
        while ( ms > 0 && (micros() - start) >= 1000) {
            ms--;
            start += 1000;
        }
    }
}


 


#endif
//------------------------------------------------------------
//------------------------------------------------------------