// Library: stringEx.h
// extended C-string manipulation
//
// (C) dsyleixa 2015-2019
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.6.1 - 1.8.9)
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/   //   




#ifndef  STRINGEX_H
#define STRINGEX_H


//------------------------------------------------------------

char * sprintDouble();
char * millis_to_strF();
char * strinsert();
char * strdelnpos();
char * strpatch();
char * substr();
int16_t  strchpos();
int16_t  strstrpos();
char * stradd();
char * cstringarg();



//----------------------------------------------------------------------------
// cstring functions
//----------------------------------------------------------------------------

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

char * millis_to_strF(uint32_t ms, char * str) {
  uint32_t  Days = 0;
  uint32_t  Hours = 0;
  uint32_t  Mins = 0;
  uint32_t  Secs = 0;

  Secs  = ms / 1000;
  Mins  = Secs / 60;
  Hours = Mins / 60;
  Days  = Hours / 24;
  Secs  = Secs - (Mins * 60);
  Mins  = Mins - (Hours * 60);
  Hours = Hours - (Days * 24);

  sprintf(str, "%ld.%02ld:%02ld:%02ld", Days, Hours, Mins, Secs);
  return str;
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

int16_t  strstrpos(char * haystack,  char * needle)   // find 1st occurance of substr in str
{
   char *p = strstr(haystack, needle);
   if (p) return p - haystack;
   return -1;   // Not found = -1.
}



//-------------------------------------------------------

const int  MAXLEN = 1024;
const int  TOKLEN = 64;

//-------------------------------------------------------

char * cstringarg( char* haystack, char* vname, char* sarg ) {  // search for &vname=sarg or ?vname=sarg, returns sarg  
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
      if( (ch=='&')||(ch==';')||(ch=='\0') ||(ch=='\n') ||(ch=='\r')
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
#endif
//------------------------------------------------------------
//------------------------------------------------------------