// Mersenne Twister TT800
// Quelle: https://de.wikipedia.org/wiki/Mersenne-Twister


#include <stdint.h>

//const uint32_t  N_TT800 = 25;
//const uint32_t  M_TT800 =  7;

#define  N_TT800   25 
#define  M_TT800    7 

/* 
 * Initialisiere den Vektor mit Pseudozufallszahlen.
 */


// Erweiterung: 
// random entropy injection REI
uint32_t   REI = 0;


static void  TT800_vector_init (uint32_t* const p, const int len)
{
  const uint32_t  mult  = 509845221ul;
  const uint32_t  add   =         3ul;
  uint32_t   seed1 = 9;
  uint32_t   seed2 = 3402 + REI;   // <<< put random entropy injection REI
  int        i;

  for (i = 0; i < len; i++) 
  {
    seed1  = seed1 * mult + add;
    seed2 *= seed2 + 1;
    p[i]   = seed2 + (seed1 >> 10);
  }
}

/* 
 * Berechne neuen Zustandsvektor aus altem Zustandsvektor
 * Dies erfolgt periodisch alle N_TT800=25 Aufrufe von TT800().
 *
 * Der folgende Code stellt eine optimierte Version von
 *   ...
 *   for (i = 0; i < N_TT800; i++) 
 *     Proc2 (p[i], p[(i+M_TT800) % N_TT800]);
 *   ...
 * mit
 *   void Proc2 (uint32_t &a0, uint32_t aM)
 *   {
 *     a0 = aM ^ (a0 >> 1) ^ A[a0 & 1];
 *   }
 * dar, in der die (zeitintensiven) Modulo N_TT800-Operationen vermieden werden.
 */

static void  TT800_vector_update (uint32_t* const p)
{
  static const uint32_t  A[2] = { 0, 0x8ebfd028 };
  int                    i = 0;

  for (; i < N_TT800-M_TT800; i++)
     p[i] = p[i+(M_TT800  )] ^ (p[i] >> 1) ^ A[p[i] & 1];
  for (; i < N_TT800  ; i++)
     p[i] = p[i+(M_TT800-N_TT800)] ^ (p[i] >> 1) ^ A[p[i] & 1];
}



/*
 * Die eigentliche Funktion:
 * - beim 1. Aufruf wird der Vektor mittels TT800_vector_init() initialisiert.
 * - beim 1., 26., 51., ... Aufruf wird ein neuer Vektor mittels TT800_vector_update berechnet.
 * - bei jedem Aufruf wird eine Zufallszahl aus dem Vektor ausgelesen und noch einem Tempering unterzogen.
 */

uint32_t randTT800() 
{
   static uint32_t  vektor [N_TT800];  /* Zustandsvektor */
   static int       idx = N_TT800+1;   /* Auslese-Index; idx >= N_TT800: neuer Vektor muss berechnet werden, */
                                 /* idx=N_TT800+1: Vektor muss überhaupt erst mal initialisiert werden */
   uint32_t         e;

   if (idx >= N_TT800) 
   {
     if (idx > N_TT800) 
     {
        //REI = analogRead(A0);   // <<< get random entropy injection REI
        TT800_vector_init (vektor, N_TT800);
     }
      
     TT800_vector_update (vektor);
     idx = 0;
   }

   e  = vektor [idx++];
   e ^= (e <<  7) & 0x2b5b2500;             /* Tempering */
   e ^= (e << 15) & 0xdb8b0000;
   e ^= (e >> 16);
   return e;  // e min= 26089 max=4294700568
}



void setup() {


   Serial.begin(115200);
   delay(1000);
   Serial.println();
   Serial.println("Serial() started");
   srand(millis());

   uint32_t i, imax=0, imin=1000000, j;
   
   for (j=0; j<100000; j++) {
      i=randTT800()%RAND_MAX;
      if(imax<i) imax=i;
      if(imin>i) imin=i;
      Serial.println((String)j+": " +i +" imin="+imin + " imax="+imax);
   }

   Serial.println((String)"\n RAND_MAX=" + RAND_MAX ); // imin=26089 imax=2147469030
   Serial.println("\n finished!");



}

void loop() {

}
