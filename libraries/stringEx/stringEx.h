// Library: stringEx.h
// extended C-string manipulation
//
// (C) dsyleixa 2015-2019
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// Programming language: Arduino Sketch C/C++ (IDE 1.6.1 - 1.8.9)
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/   //   


// ver 0.9: cstringarg: SpaceDilimiterOption

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




char * ftoa(float f, char *str, uint8_t precision) {
  uint8_t i, j, divisor = 1;
  int8_t log_f;
  int32_t int_digits = (int)f;             //store the integer digits
  float decimals;
  char s1[12];

  memset(str, 0, sizeof(str));  
  memset(s1, 0, 10);

  if (f < 0) {                             //if a negative number 
    str[0] = '-';                          //start the char array with '-'
    f = abs(f);                            //store its positive absolute value
  }
  log_f = ceil(log10(f));                  //get number of digits before the decimal
  if (log_f > 0) {                         //log value > 0 indicates a number > 1
    if (log_f == precision) {              //if number of digits = significant figures
      f += 0.5;                            //add 0.5 to round up decimals >= 0.5
      itoa(f, s1, 10);                     //itoa converts the number to a char array
      strcat(str, s1);                     //add to the number string
    }
    else if ((log_f - precision) > 0) {    //if more integer digits than significant digits
      i = log_f - precision;               //count digits to discard
      divisor = 10;
      for (j = 0; j < i; j++) divisor *= 10;    //divisor isolates our desired integer digits 
      f /= divisor;                             //divide
      f += 0.5;                            //round when converting to int
      int_digits = (int)f;
      int_digits *= divisor;               //and multiply back to the adjusted value
      itoa(int_digits, s1, 10);
      strcat(str, s1);
    }
    else {                                 //if more precision specified than integer digits,
      itoa(int_digits, s1, 10);            //convert
      strcat(str, s1);                     //and append
    }
  }

  else {                                   //decimal fractions between 0 and 1: leading 0
    s1[0] = '0';
    strcat(str, s1);
  }

  if (log_f < precision) {                 //if precision exceeds number of integer digits,
    decimals = f - (int)f;                 //get decimal value as float
    strcat(str, ".");                      //append decimal point to char array

    i = precision - log_f;                 //number of decimals to read
    for (j = 0; j < i; j++) {              //for each,
      decimals *= 10;                      //multiply decimals by 10
      if (j == (i-1)) decimals += 0.5;     //and if it's the last, add 0.5 to round it
      itoa((int)decimals, s1, 10);         //convert as integer to character array
      strcat(str, s1);                     //append to string
      decimals -= (int)decimals;           //and remove, moving to the next
    }
  }
  return str;
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


const int  MAXLEN = 1024;
const int  TOKLEN = 64;

int16_t  strstrpos(char * haystack,  char * needle)   // find 1st occurance of substr in str
{                                                     // see String.indexOf(val, from), C++: std::string::find
   char *p = strstr(haystack, needle);
   if (p) return p - haystack;
   return -1;   // Not found = -1.
}


//------------------------------------------------------------

bool strEndsWith(char* haystack, char* theEnd) {
	int lenH=strlen(haystack), lenE=strlen(theEnd);
	
	//if( strstrpos( haystack, theEnd) == lenH-lenE ) return 1;
	for(int i=0; i<lenE; i++) {
		if (haystack[lenH-i-1]!=theEnd[lenE-i-1]) return 0;
		else
		if (i==lenE-1 && haystack[lenH-lenE]==theEnd[0] ) return 1;
	}
	return 0;
}




//-------------------------------------------------------
char * cstringarg( char* haystack, char* vname, char* sarg, bool SpaceDilimiterOption=true ) {  // <<<<<<<<<<<<<<<<<<<< NEW
  int i = 0, pos = -1;
  unsigned char  ch = 0xff;
  const char*  kini = "&";       // start of varname: '&'
  const char*  kin2 = "?";       // start of varname: '?'
  const char*  kequ = "=";       // end of varname, start of argument: '='
  char  needle[TOKLEN] = "";     // complete pattern:  &varname=abc1234 

  strcpy(sarg, "");
  strcpy(needle, kini);          // try 1st initial char '&'
  strcat(needle, vname);
  strcat(needle, kequ);               
  pos = strstrpos(haystack, needle);  // see String.indexOf(val, from), C++: std::string::find
  if (pos == -1) {
    needle[0] = kin2[0];                   // when search failed: try 2nd initial char '?'
    pos = strstrpos(haystack, needle);
    if (pos == -1) return sarg;
  }
  pos = pos + strlen(vname) + 2; // start of value = kini+vname+kequ
  while ( (ch != '&') && (ch != '\0') ) {
    ch = haystack[pos + i];
    if ( (ch == '&') || (ch == ';') || (ch == '\0') || (ch == '\n') 
         || ( SpaceDilimiterOption && (ch == ' ') ) // <<<<<<<<<<<<<<<<<<<<<<<<< break at Space (for login)!
         || (i + pos >= strlen(haystack)) || (i > TOKLEN - 1) ) {
      sarg[i] = '\0';
      return sarg;
    }
    if ( (ch != '&') ) {
      sarg[i] = ch;
      i++;
    }
  }
  return sarg;
}



//------------------------------------------------------------
#endif
//------------------------------------------------------------
//------------------------------------------------------------