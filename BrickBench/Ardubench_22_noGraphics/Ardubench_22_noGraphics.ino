// HaWe Brickbench
// benchmark test for SoCs and MCUs
// PL: GCC,Arduino
// Autor: (C) dsyleixa 2013-2019
//
// freie Verwendung für private Zwecke
// f�r kommerzielle Zwecke nur nach schriftlicher Genehmigung durch den Autor.
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
// version 2.2a  (2019-04-15)
// no graphics
// change log:
// 2.2.   testing both 32fp and 64fp 
// 2.1.1. 32bit fp tests vs. 64bit double (ARM/32bit cores, optional)
//        low-level bitRead/Write vs. digitalRead/Write (AVR cores, optional) 
// 2.1 GPIO r/w
// 2.0 loop counts


#include "SPI.h"



unsigned long runtime[10];

#define tpin1  11  // GPIO test pins digitalRead
#define tpin2  12  // GPIO test pins digitalWrite
#define tpin3  13  // GPIO test pins digitalWrite
void TFTprint(char sbuf[], int16_t x, int16_t y) {
  
}

int a[500], b[500], c[500], t[500];

//--------------------------------------------
// Mersenne Twister
//--------------------------------------------

unsigned long randM(void) {
   
   const int M = 7;
   const unsigned long A[2] = { 0, 0x8ebfd028 };

   static unsigned long y[25];
   static int index = 25+1;

   if (index >= 25) {
     int k;
     if (index > 25) {
        unsigned long r = 9, s = 3402;
        for (k=0 ; k<25 ; ++k) {
          r = 509845221 * r + 3;
          s *= s + 1;
          y[k] = s + (r >> 10);
        }
     }
     for (k=0 ; k<25-M ; ++k)
        y[k] = y[k+M] ^ (y[k] >> 1) ^ A[y[k] & 1];
     for (; k<25 ; ++k)
        y[k] = y[k+(M-25)] ^ (y[k] >> 1) ^ A[y[k] & 1];
     index = 0;
   }

   unsigned long e = y[index++];
   e ^= (e << 7) & 0x2b5b2500;
   e ^= (e << 15) & 0xdb8b0000;
   e ^= (e >> 16);
   return e;
}

//--------------------------------------------
// Matrix Algebra
//--------------------------------------------

// matrix * matrix multiplication (matrix product)
 
 void MatrixMatrixMult(int N, int M, int K, double *A, double *B, double *C) {
   int i, j, s;
   for (i = 0; i < N; ++i) {
      for (j = 0; j < K; ++j) {
         C[i*K+j] = 0;
         for (s = 0; s < M; ++s) {
            C[i*K+j] = C[i*K+j] + A[i*N+s] * B[s*M+j];
         }
      }
   }
}


// matrix determinant

double MatrixDet(int N, double A[]) {
   int i, j, i_count, j_count, count = 0;
   double Asub[N - 1][N - 1], det = 0;

   if (N == 1)
      return *A;
   if (N == 2)
      return ((*A) * (*(A+1+1*N)) - (*(A+1*N)) * (*(A+1)));

   for (count = 0; count < N; count++) {
      i_count = 0;
      for (i = 1; i < N; i++) {
         j_count = 0;
         for (j = 0; j < N; j++) {
            if (j == count)
               continue;
            Asub[i_count][j_count] = *(A+i+j*N);
            j_count++;
         }
         i_count++;
      }
      det += pow(-1, count) * A[0+count*N] * MatrixDet(N - 1, &Asub[0][0]);
   }
   return det;
}



//--------------------------------------------
// shell sort
//--------------------------------------------

void shellsort(int size, int* A)
{
  int i, j, increment;
  int temp;
  increment = size / 2;

  while (increment > 0) {
    for (i = increment; i < size; i++) {
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) {
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }

    if (increment == 2)
       increment = 1;
    else
       increment = (unsigned int) (increment / 2.2);
  }
}

//--------------------------------------------
// gnu quick sort
// (0ptional)
//--------------------------------------------

int compare_int (const int *a, const int *b)
{
  int  temp = *a - *b;

  if (temp > 0)          return  1;
  else if (temp < 0)     return -1;
  else                   return  0;
}

// gnu qsort:
// void qsort (void *a , size_a count, size_a size, compare_function)
// gnu qsort call for a[500] array of int:
// qsort (a , 500, sizeof(a), compare_int)



//--------------------------------------------
// benchmark test procedures
//--------------------------------------------


int test_Int_Add() { // 10,000,000 int +,-
   int i=1, j=11, k=112, l=1111, m=11111, n=-1, o=-11, p=-111, q=-1112, r=-11111;
   unsigned long x;
   volatile long s=0;
   for(x=0;x<5000000;x++) {
     s+=i; s+=j; s+=k; s+=l; s+=m; s+=n; s+=o; s+=p; s+=q; s+=r;
   }
   return s; // debug
}


//--------------------------------------------
long test_Int_Mult() { // 2,000,000 int *,/
  int  x;
  unsigned long y;
  volatile long s;

  for(y=0;y<500000;y++) {
    s=1;
    for(x=1;x<=10;x++) { s*=x;}
    for(x=10;x>0;--x) { s/=x;}
  }
  return s; // debug
}


#define PI  M_PI

//--------------------------------------------
double test_fp64_math() { // 2,000,000 fp (double) mult, transcend.
  volatile double s=(double)PI;
  unsigned long y;

  for(y=0;y<500000UL;y++) {
     s*=sqrt(s);
     s=sin(s);
     s=exp(s);
     s=s*s;
  }
  return s;
}

//--------------------------------------------
float test_fp32_math() { // 2,000,000 32bit float mult, transcend.
  volatile float s=(float)PI;
  unsigned long y;

  for(y=0;y<500000UL;y++) {
     s*=sqrtf(s);
     s=sinf(s);
     s=expf(s);
     s=s*s;
  }
  return s;
}


//--------------------------------------------
long test_rand_MT() { // 2,500,000 PRNGs
  volatile unsigned long s;
  unsigned long y;

  for(y=0;y<2500000;y++) {
     s=randM()%10001;
  }
  return s;
}

//--------------------------------------------
double test_matrix_math() { // 150,000 2D Matrix algebra (mult, det)
   unsigned long x;

  double A[2][2], B[2][2], C[2][2];
  double S[3][3], T[3][3];
  unsigned long s;

 for(x=0;x<50000;++x) {

    A[0][0]=1;   A[0][1]=3;
    A[1][0]=2;   A[1][1]=4;

    B[0][0]=10;  B[0][1]=30;
    B[1][0]=20;  B[1][1]=40;

    MatrixMatrixMult(2, 2, 2, A[0], B[0], C[0]);  

    A[0][0]=1;   A[0][1]=3;
    A[1][0]=2;   A[1][1]=4;
    
    MatrixDet(2, A[0]);                           

    S[0][0]=1;   S[0][1]=4;  S[0][2]=7;
    S[1][0]=2;   S[1][1]=5;  S[1][2]=8;
    S[2][0]=3;   S[2][1]=6;  S[2][2]=9;

    MatrixDet(3, S[0]);                           

    s=(S[0][0]*S[1][1]*S[2][2]);
  }
  return s;
}


//--------------------------------------------
// for array copy using void *memcpy(void *dest, const void *src, size_t n);

long test_Sort() { // 1500  sort of random array[500]
  unsigned long s;
  int y, i;

  int t[500];
 
  for(y=0;y<500;++y) {
    memcpy(t, a, sizeof(a));
    shellsort(500, t);
   
    memcpy(t, a, sizeof(b));
    shellsort(500, t);
   
    memcpy(t, a, sizeof(c));
    shellsort(500, t);
  }
  return y;
}


//--------------------------------------------
int32_t test_GPIO() {   // 6,000,000 GPIO r/w
   volatile static bool w=false, r;
   uint32_t y;
   for (y=0; y<2000000; y++) {      
         digitalWrite(tpin1, w);
         w=!w;
         r=digitalRead(tpin3);
         digitalWrite(tpin2, w&!r);      
   }
   return 1;
}

/*
//--------------------------------------------
int32_t test_GPIO_AVR() {  // 6,000,000 GPIO bit r/w
   volatile static bool w=false, r;
   uint32_t y;
   for (y=0; y<2000000; y++) {     
         bitWrite(PORTB, PB5, w);
         w=!w;
         r = bitRead(PINB, PB7);
         bitWrite(PORTB, PB6, w&!r);   // optional:  bitWrite(PORTB, PB6, w&r);   
   }
   return 1; // debug
}
*/

    
//--------------------------------------------
inline void displayValues() {

  char buf[120];
  //tft.fillScreen(0x0000); // clrscr()

    sprintf (buf, "%3d %9ld  int_Add",    0, runtime[0]); TFTprint(buf, 0, 9);
    sprintf (buf, "%3d %9ld  int_Mult",   1, runtime[1]); TFTprint(buf, 0,18);
    sprintf (buf, "%3d %9ld  fp32_op",    2, runtime[2]); TFTprint(buf, 0,27);
    sprintf (buf, "%3d %9ld  fp64_op",    3, runtime[3]); TFTprint(buf, 0,36);
    sprintf (buf, "%3d %9ld  randomize",  4, runtime[4]); TFTprint(buf, 0,45);
    sprintf (buf, "%3d %9ld  matrx_algb", 5, runtime[5]); TFTprint(buf, 0,54);
    sprintf (buf, "%3d %9ld  arr_sort",   6, runtime[6]); TFTprint(buf, 0,63);
    sprintf (buf, "%3d %9ld  GPIO_togg",  7, runtime[7]); TFTprint(buf, 0,72);
    //sprintf (buf, "%3d %9ld  Graphics",   8, runtime[8]); TFTprint(buf, 0,80);
}

//--------------------------------------------
int32_t test_TextOut(){
  int  y;
  char buf[120];
 
  for(y=0;y<10;++y) {   
    //tft.fillScreen(0x0000); // clrscr()
    sprintf (buf, "%3d %9d  int_Add",    y, 1000);  TFTprint(buf, 0, 9);
    sprintf (buf, "%3d %9d  int_Mult",   0, 1010);  TFTprint(buf, 0,18);
    sprintf (buf, "%3d %9d  fp32_op",    0, 1032);  TFTprint(buf, 0,27);
    sprintf (buf, "%3d %9d  fp64_op",    0, 1064);  TFTprint(buf, 0,36);
    sprintf (buf, "%3d %9d  randomize",  0, 1040);  TFTprint(buf, 0,45);
    sprintf (buf, "%3d %9d  matrx_algb", 0, 1050);  TFTprint(buf, 0,54);
    sprintf (buf, "%3d %9d  GPIO_togg",  0, 1060);  TFTprint(buf, 0,63);
    sprintf (buf, "%3d %9d  Graphics",   0, 1070);  TFTprint(buf, 0,72);
    sprintf (buf, "%3d %9d  testing...", 0, 1080);  TFTprint(buf, 0,80);
  }
  return y;
}


//--------------------------------------------
int32_t test_graphics(){
  int y;
  char buf[120];
 
 
  for(y=0;y<10;++y) {
    //tft.fillScreen(0x0000);
    sprintf (buf, "%3d", y);  TFTprint(buf, 0,80); // outcomment for downwards compatibility

    /*
    tft.drawCircle(50, 40, 10, 0xFFFF);
    tft.fillCircle(30, 24, 10, 0xFFFF);
    tft.drawLine(10, 10, 60, 60, 0xFFFF);
    tft.drawLine(50, 20, 90, 70, 0xFFFF);
    tft.drawRect(20, 20, 40, 40, 0xFFFF);
    tft.fillRect(65, 25, 20, 30, 0xFFFF);
    tft.drawCircle(70, 30, 15, 0xFFFF); 
    */
  }
  return y;
}


//--------------------------------------------
long test(){
 unsigned long time0, x, y;
  double s;
  char  buf[120];
  int   i;
  float f;

  Serial.println("init test arrays");

  for(y=0;y<500;++y) {
    a[y]=randM()%30000; b[y]=randM()%30000; c[y]=randM()%30000;
  }

  Serial.println("start test");
  //tft.println("start test");
  delay(10);

  
  
  time0= millis();

  s=test_Int_Add();
  runtime[0]=millis()-time0;
  sprintf (buf, "%3d %9ld  int_Add",    0, runtime[0]); 
  Serial.println( buf);
  //tft.println( buf);

  time0=millis();
  s=test_Int_Mult();
  runtime[1]=millis()-time0;
  sprintf (buf, "%3d %9ld  int_Mult",   1, runtime[1]); 
  Serial.println( buf);
  //tft.println( buf);


  time0=millis();
  s=test_fp32_math();
  runtime[2]=millis()-time0;
  sprintf (buf, "%3d %9ld  fp32_ops",   2, runtime[2]); 
  Serial.println( buf);  
  //tft.println( buf);
  //debug  // Serial.println(s);


  time0=millis();
  s=test_fp64_math();
  runtime[3]=millis()-time0;
  sprintf (buf, "%3d %9ld  fp64_ops",   3, runtime[3]); 
  Serial.println( buf);  
  //tft.println( buf);
  //debug  // Serial.println(s);
  

  time0=millis();
  s=test_rand_MT();
  runtime[4]=millis()-time0;
  sprintf (buf, "%3d %9ld  randomize",  4, runtime[4]); 
  Serial.println( buf);
  //tft.println( buf);
 

  time0=millis();
  s=test_matrix_math();
  runtime[5]=millis()-time0;
  sprintf (buf, "%3d %9ld  matrx_algb", 5, runtime[5]); 
  Serial.println( buf);
  //tft.println( buf);
 

  time0=millis();
  s=test_Sort();
  runtime[6]=millis()-time0;
  sprintf (buf, "%3d %9ld  arr_sort",   6, runtime[6]); 
  Serial.println( buf);
  //tft.println( buf);


  // GPIO R/W toggle test
  //Serial.println("GPIO_toggle test");
  time0=millis();
  s=test_GPIO();
  runtime[7]=millis()-time0;
  sprintf (buf, "%3d %9ld  GPIO_toggle", 7, runtime[7]); 
  Serial.println( buf);
  //tft.println( buf);


  
  // lcd display text / graphs
  time0=millis();
  /*
  s=test_TextOut();  
  s=test_graphics();
  */
  runtime[8]=millis()-time0;
  sprintf (buf, "%3d %9ld  Graphics   ", 8, runtime[8]); 
  Serial.println( buf); 
  //tft.println( buf);
  
  Serial.println();
 
  y = 0;
  for (x = 0; x < 9; ++x) {
      y += runtime[x];
  }
 
  displayValues();
  sprintf (buf, "runtime ges.:  %-9ld ", y);
  Serial.println( buf);   TFTprint(buf, 0,90);
 
  x=50000000.0/y;
  sprintf (buf, "benchmark:     %-9ld ", x);
  Serial.println( buf);   TFTprint(buf, 0,100);


  return 1;
}

//--------------------------------------------
void setup() {
 
  Serial.begin(115200);
  Serial.println("starting Serial()");
  while(!Serial);

  // Setup the LCD
  /*
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF); tft.setTextSize(1);
  */
  Serial.println("tft started");

  pinMode(tpin1, INPUT_PULLUP);
  pinMode(tpin2, OUTPUT);
  pinMode(tpin3, OUTPUT);


  char  buf[120];
  test(); 
  sprintf (buf, "Ende HaWe brickbench");   
  Serial.println( buf);
  TFTprint(buf, 0, 110);
}

void loop() {

}



/* 

 

test design:
  0   int_Add     50,000,000 int +,- plus counter
  1   int_Mult    10,000,000 int *,/  plus counter
  2   fp32_ops    2,000,000 fp32 mult, transc.  plus counter
  3   fp64_ops    2,000,000 fp64 mult, transc.  plus counter (if N/A: 32bit)
  4   randomize   2,500,000 Mersenne PRNG (+ * & ^ << >>)
  5   matrx_algb  150,000 2D Matrix algebra (mult, det)
  6   arr_sort    1500 shellsort of random array[500]
  7   GPIO toggle 6,000,000 toggle GPIO r/w  plus counter
  8   Graphics    10*8 textlines + 10*8 shapes + 20 clrscr

.


Vergleichswerte (für Raspi noch nicht komplett durchgeführt):

Arduino MEGA + ILI9225 + Karlson UTFT + Arduino GPIO-r/w
  0     90244  int_Add
  1    237402  int_Mult
  2    163613  fp32_ops(float)
  3    163613  fp32_ops(float=double)
  4    158567  randomize
  5     46085  matrx_algb
  6     23052  arr_sort
  7     41569  GPIO toggle
  8     62109  Graphics   
runtime ges.:  986254
benchmark:     51




Arduino MEGA + ILI9225 + Karlson UTFT + Register bitRead/Write
  0     90238  int_Add
  1    237387  int_Mult
  2    163602  fp32_ops (float)
  3    163602  fp32_ops (float=double)
  4    158557  randomize
  5     45396  matrx_algb
  6     23051  arr_sort
  7      4528  GPIO_toggle bit r/w
  8     62106  Graphics   
runtime ges.:  948467
benchmark:     53  


Arduino MEGA + adafruit_ILI9341 Hardware-SPI  Arduino GPIO r/w
  0     90244  int_Add
  1    237401  int_Mult
  2    163612  fp32_ops (float)
  3    163612  fp32_ops (float=double)
  4    158725  randomize
  5     46079  matrx_algb
  6     23051  arr_sort
  7     41947  GPIO toggle
  8      6915  Graphics   
runtime ges.:  931586
benchmark:     54  
 
 



Arduino/Adafruit M0 + adafruit_ILI9341 Hardware-SPI  
  0      7746  int_Add
  1     15795  int_Mult
  2     89054  fp32_ops
  3    199888  fp64_ops(double)
  4     17675  randomize
  5     18650  matrx_algb
  6      6328  arr_sort
  7      9944  GPIO_toggle
  8      6752  Graphics 
runtime ges.:  371832
benchmark:     134 



Arduino DUE + adafruit_ILI9341 Hardware-SPI  
  0      4111  int_Add
  1      1389  int_Mult
  2     29124  fp32_ops(float)
  3     57225  fp64_ops(double)
  4      3853  randomize
  5      4669  matrx_algb
  6      2832  arr_sort
  7     11859  GPIO_toggle
  8      6142  Graphics   
runtime ges.:  121204 
benchmark:     413    



Arduino/Adafruit M4 + adafruit_HX8357 Hardware-SPI  
  0      2253  int_Add
  1       872  int_Mult
  2      2773  fp32_ops
  3     24455  fp64_ops
  4      1680  randomize
  5      1962  matrx_algb
  6      1553  arr_sort
  7      2395  GPIO_toggle
  8      6521  Graphics   
runtime ges.:  44464     
benchmark:     1124   



Vergleich Raspberry Pi 2:

Raspi 2 (v1): 4x 900MHz,  GPU 400MHz, no CPU overclock, full-HD, openVG:
  0     384  int_Add
  1     439  int_Mult
  2     (N/A, ca. 441)  fp32_ops(float)
  3     441  fp64_ops(double)
  4     399  randomize
  5     173  matrx_algb
  6     508  arr_sort
  7     823  GPIO_toggle
  8    2632  graphics
runtime ges.: ca.  6240
benchmark: ca.     8000


*/
 
