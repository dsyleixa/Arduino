// Mersenne Twister TT800
// Quellen: https://de.wikipedia.org/wiki/Mersenne-Twister
// http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/tt800.c


#include <stdint.h>

#define N 25
#define M 7

//double
uint32_t
genrand()
{
    unsigned long y;
    static int k = 0;
    static unsigned long x[N]={ /* initial 25 seeds, change as you wish */
   0x95f24dab, 0x0b685215, 0xe76ccae7, 0xaf3ec239, 0x715fad23,
   0x24a590ad, 0x69e4b5ef, 0xbf456141, 0x96bc1b7b, 0xa7bdf825,
   0xc1de75b7, 0x8858a9c9, 0x2da87693, 0xb657f9dd, 0xffdc8a9f,
   0x8121da71, 0x8b823ecb, 0x885d05f5, 0x4e20cd47, 0x5a9ad5d9,
   0x512c0c03, 0xea857ccd, 0x4cc1d30f, 0x8891a8a1, 0xa6b7aadb
    };
    static unsigned long mag01[2]={ 
        0x0, 0x8ebfd028 /* this is magic vector `a', don't change */
    };
    if (k==N) { /* generate N words at one time */
      int kk;
      for (kk=0;kk<N-M;kk++) {
   x[kk] = x[kk+M] ^ (x[kk] >> 1) ^ mag01[x[kk] % 2];
      }
      for (; kk<N;kk++) {
   x[kk] = x[kk+(M-N)] ^ (x[kk] >> 1) ^ mag01[x[kk] % 2];
      }
      k=0;
    }
    y = x[k];
    y ^= (y << 7) & 0x2b5b2500; /* s and b, magic vectors */
    y ^= (y << 15) & 0xdb8b0000; /* t and c, magic vectors */
    y &= 0xffffffff; /* you may delete this line if word size = 32 */
/* 
   the following line was added by Makoto Matsumoto in the 1996 version
   to improve lower bit's corellation.
   Delete this line to o use the code published in 1994.
*/
    y ^= (y >> 16); /* added to the 1994 version */
    k++;
    //return( (double) y / (unsigned long) 0xffffffff);
    return y%RAND_MAX;
}



void setup() {


   Serial.begin(115200);
   delay(1000);
   Serial.println();
   Serial.println("Serial() started");
   srand(millis());

   uint32_t i, imax=0, imin=1000000, j;
   
   for (j=0; j<100000; j++) {
      i=genrand();  
      if(imax<i) imax=i;
      if(imin>i) imin=i;
      Serial.println((String)j+": " +i +" imin="+imin + " imax="+imax);
   }

   Serial.println((String)"\n RAND_MAX=" + RAND_MAX ); 
   Serial.println("\n finished!");

   //   9999:  709125572 imin=368610 imax=2147380779
   //  19999: 1023761597 imin=168463 imax=2147380779
   //  49999:  817979727 imin=168463 imax=2147459005





}

void loop() {

}
