// ardumath.h

#pragma once

/*
dynamic 2D array conversion:
https://www.raspberrypi.org/forums/viewtopic.php?f=33&t=257284&p=1570347#p1570069 

2-dim array static:
double array2D[LINE][ROW]; 

2-dim dynamic arrays for C++:
double(*array2D)[LINE] = new double[ROW][LINE]; // rearranged!

2-dim dynamic arrays for ISO C:     
static double (*array2D)[LINE] = NULL;          // rearranged!
array2D= malloc((LINE) * sizeof(double[ROW]));
*/


template <typename variant>  void bubblesort(variant *array, int length);
template<typename variant>   void shellsort(variant *array, int length);


//********************************************************************************
// map, constrain
//********************************************************************************

int32_t  mapConstrain(int32_t val, int vmin=0, int vmax=1023 ) {  
  val=map(val, 0, 17550,  vmin, vmax);
  val=constrain(val, vmin, vmax);
  return (int32_t)val;
}


//********************************************************************************
// bit and byte and pin operations
//********************************************************************************
// convert byte arrays to int16

inline int16_t  ByteArrayToInt16(uint8_t  barray[], uint8_t  slot) {
  return ((barray[slot + 1] << 8) + (barray[slot]));
}

inline long  ByteArrayToInt32(uint8_t  barray[], uint8_t  slot) {
  return ( (barray[slot+3]<<24) + (barray[slot+2]<<16) + (barray[slot+1]<<8) + barray[slot] );
}

//------------------------------------------------------------------------------
// convert int to byte arrays

inline void Int16ToByteArray(int16_t vint16,  uint8_t barray[],  uint8_t slot) {
  memcpy(barray+slot*sizeof(char), &vint16, sizeof(int16_t));    // copy int16 to array
}

inline void Int32ToByteArray(int32_t vint32,  uint8_t barray[],  uint8_t slot) {
  memcpy(barray+slot*sizeof(char), &vint32, sizeof(int32_t));    // copy int32 to array
}


//------------------------------------------------------------------------------
// convert float/double to byte arrays

void DoubleToByteArray(double fvar, uint8_t * barray,  uint8_t slot) {
   union {
     double  f;
     uint8_t b8[sizeof(double)];
   } u;
   u.f = fvar;
   memcpy(barray+slot*sizeof(char), u.b8, sizeof(double));  
}


double ByteArrayToDouble(uint8_t * barray, uint8_t  slot) {
   union {
     double  f;
     uint8_t b8[sizeof(double)];
   } u;
   memcpy( u.b8, barray+slot*sizeof(char), sizeof(double)); 
   return u.f;
}


void FloatToByteArray(float fvar, uint8_t * barray,  uint8_t slot) {
    union {
     double  f;
     uint8_t b4[sizeof(float)];
   } u;
   u.f = fvar;
   memcpy(barray+slot*sizeof(char), u.b4, sizeof(float));   
}


double ByteArrayToFloat(uint8_t * barray, uint8_t  slot) {
   union {
     double  f;
     uint8_t b4[sizeof(float)];
   } u;
   memcpy( u.b4, barray+slot*sizeof(char), sizeof(float)); 
   return u.f;
}


//------------------------------------------------------------------------------
// read+write bits in numbers
// included already in Arduino libs by default
//------------------------------------------------------------------------------
/*
#define bitRead(source, bit) ( ((source) >> (bit)) & 0x01 )
#define bitSet (source, bit) ( (source) |= (1UL << (bit)) )
#define bitClear(source, bit) ( (source) &= ~(1UL << (bit)) )
#define bitWrite(source, bit, bitvalue) ( bitvalue ? bitSet(source, bit) : bitClear(source, bit) )
*/

//---------------------------------------------------------
// Bubble Sort
//---------------------------------------------------------

template <typename variant>  void bubblesort(variant *array, int length) {
  variant tmp;

  for (int i=0; i<length-1; i++)     {
    for (int j=0; j<length-i-1; j++)  {
      if (array[j] > array[j+1])       {
        tmp = array[j];
        array[j] = array[j+1];
        array[j+1] = tmp;
      }
    }
  }
}

//---------------------------------------------------------
// Shell Sort
//---------------------------------------------------------

template<typename variant>  void shellsort(variant *array, int length) {
  int   i, j, increment;   
  variant temp;

  increment = length / 2;
  while (increment > 0) {
    for (i = increment; i < length; i++) {
      j = i;
      temp = array[i];
      while ((j >= increment) && (array[j-increment] > temp)) {
        array[j] = array[j - increment];
        j = j - increment;
      }
      array[j] = temp;
    }
    if (increment == 2)
       increment = 1;
    else
       increment = (unsigned int) (increment / 2.2);
  }
}

//---------------------------------------------------------
// Median Filter
//---------------------------------------------------------

#define MEDMAX 5  // Median depth: 5 each (MEDMAX default)

// private FiFo Buffers: 8 variants, Median depth: 5 each (MEDMAX default)
int     fifoi[8][MEDMAX];
double  fifod[8][MEDMAX];


void fifopush(int _new, int varnr) {
  for (int i=MEDMAX-1; i>=1; i--) {
     fifoi[varnr][i] = fifoi[varnr][i-1];
  }
  fifoi[varnr][0] = _new;
}

void fifopush(double _new, int varnr) {
  for (int i=MEDMAX-1; i>=1; i--) {
     fifod[varnr][i] = fifod[varnr][i-1];
  }
  fifod[varnr][0] = _new;
}

//---------------------------------------------------------

int medianOfi(int varnr, int depth) {
  int temp[depth];  
  for (int i=0; i<depth; i++)  temp[i] = fifoi[varnr][i];
  bubblesort( temp, depth );  
  return temp[depth/2];
}

double  medianOfd(int varnr, int depth) {
  double  temp[depth];
  for (int i=0; i<depth; i++)  temp[i] = fifod[varnr][i];
  bubblesort( temp, depth );
  return temp[depth/2];
}

//---------------------------------------------------------

int medianNewOfi(int _new, int varnr, int depth) {
  fifopush(_new, varnr);
  return medianOfi(varnr, depth);
}

double medianNewOfd(double _new, int varnr, int depth) {
  fifopush(_new, varnr);
  return medianOfd(varnr, depth);
}


 
//---------------------------------------------------------
//  Tiefpass = lowpassFilter
//---------------------------------------------------------

template<typename variant>  double lowpassFilt(variant NewVal, variant Avrg, double FFW) { // FFW=Filter-Factor Weight
   
   return (double)(NewVal*FFW + Avrg*(1.0-FFW)) ;
}


//---------------------------------------------------------
//EOF
//---------------------------------------------------------
