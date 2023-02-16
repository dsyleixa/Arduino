// Mersenne Twister TT800
// Quellen: https://de.wikipedia.org/wiki/Mersenne-Twister
// https://de-academic.com/dic.nsf/dewiki/945690


#include <stdint.h>

unsigned randTT800(void) {
   const int N = 25;
   const int M = 7;
   const unsigned A[2] = { 0, 0x8ebfd028 };
 
   static unsigned y[25];
   static int index = N+1;
 
   if (index >= N) {
     int k; 
     if (index > N) {
        unsigned r = 9, s = 3402;
        for (k=0 ; k<N ; ++k) {
          r = 509845221 * r + 3;
          s *= s + 1;
          y[k] = s + (r >> 10);
        }
     }
     for (k=0 ; k<N-M ; ++k)
        y[k] = y[k+M] ^ (y[k] >> 1) ^ A[y[k] & 1];
     for (; k<N ; ++k)
        y[k] = y[k+(M-N)] ^ (y[k] >> 1) ^ A[y[k] & 1];
     index = 0;
   }
 
   unsigned e = y[index++];
   e ^= (e << 7) & 0x2b5b2500;
   e ^= (e << 15) & 0xdb8b0000;
   e ^= (e >> 16);
   return e;
 }



void setup() {


   Serial.begin(115200);
   delay(1000);
   Serial.println();
   Serial.println("Serial() started");
   srand(millis());

   uint32_t i, imax=0, imin=1000000, j;
   
   for (j=0; j<50000; j++) {
      i=randTT800(); //randTT800()%RAND_MAX;
      if(imax<i) imax=i;
      if(imin>i) imin=i;
      Serial.println((String)j+": " +i +" imin="+imin + " imax="+imax);
   }

   Serial.println((String)"\n RAND_MAX=" + RAND_MAX ); 
   Serial.println("\n finished!");

   //  9999: 405758574 imin=26089 imax=4294580691
   // 49999: 3409695116 imin=26089 imax=4294941586



}

void loop() {

}
