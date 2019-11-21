// Neural Network for Arduino

// Ref.:
// http://robotics.hobbizine.com/arduinoann.html
// www.cs.bham.ac.uk/~jxb/NN/nn.html

// 3-layer Backpropagation net

// modified, extended, and enhanced by dsylexia
// required MCUs: ESP8266, ESP32
// version 0.1.1

// (C) 2018 by dsyleixa
// (C) of processed 3rd party code: see original sources.
// This example code is in the public domain for private use, 3rd party code not affected.
// Use for professional or business purpose only by personal written permission
// by the author.



// change log:
// 0.1.1: input matrix=void => output array=void {0,0,0,0,0,0,0,0,0,0}
//        input matrix pattern="0" => output array={1,0,0,0,0,0,0,0,0,0}
// 0.1.0: debug mode



#include <math.h>


// debug mode: uncomment, regular mode: outcomment!
//#define  DEBUG


#define  REPORT_N   20    // for terminal log
int32_t  ReportInterval;  // for terminal log
uint32_t timestamp;       // for terminal log


/******************************************************************
   Network Configuration, customizable
 ******************************************************************/

const int MAX_PATTERNS = 40;  //
const int NUM_INPUTS   = 120;
const int NUM_HIDDEN   = 35;
const int NUM_OUTPUTS  = 10;


float LearningRate = 0.2;     // 0.3 vv lower oscillating
float Momentum     = 0.85;     // 0.8 ^^ lower local min
float InitialWeightMax = 0.5; // 0.5


#ifdef DEBUG
#define MAXLOOPS   3000
#define ThrSuccess 0.20
#else
#define MAXLOOPS   2147483647
#define ThrSuccess 1E-6*NUM_INPUTS*NUM_OUTPUTS
#endif


int   i, j, p, q, r;
int   RandomizedIndex[MAX_PATTERNS];
long  TrainingCycle;
float Rando;
float Error;
float Accum;


/******************************************************************
   Artificial Neuron
 ******************************************************************/

float HiddenActiv[NUM_HIDDEN];                          // Activation factors for neurons
float OutputActiv[NUM_OUTPUTS];
float HiddenWeights[NUM_INPUTS + 1][NUM_HIDDEN];        // Weight factors for neuron inputs
float OutputWeights[NUM_HIDDEN + 1][NUM_OUTPUTS];
float HiddenDelta[NUM_HIDDEN];                          // Delta=Divergence target/actual
float OutputDelta[NUM_OUTPUTS];
float ChangeHiddenWeights[NUM_INPUTS + 1][NUM_HIDDEN];  // change buffer for Weights
float ChangeOutputWeights[NUM_HIDDEN + 1][NUM_OUTPUTS];


/******************************************************************
   Training Test Patterns
 ******************************************************************/

#define  X      1
#define  TEN_0  0,0,0,0,0,0,0,0,0,0
#define  TEN_1  1,1,1,1,1,1,1,1,1,1

//-----------------------------------------------------------------
//  Input patterns
//-----------------------------------------------------------------

byte Input[MAX_PATTERNS + 1][NUM_INPUTS] = {
  { 0, 0, 0, 0, X, X, 0, 0, 0, 0,  // 0 ="0"
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, X, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, 0, 0, 0, 0,  // 1
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, 0, 0, 0, 0,  // 2
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, X, X, X, X, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, 0, 0, 0, 0,  // 3
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, X, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, 0, 0, 0, X, 0, 0,  // 4
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, X, 0, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, 0, X, 0, 0, 0,
    0, 0, X, X, X, X, X, X, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, X, X, X, X, X, X, 0, 0,  // 5
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, X, X, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, X, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, 0, 0, 0, 0,  // 6
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, X, X, X, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, X, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, X, X, X, X, X, X, 0, 0,  // 7
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, 0, 0, 0, 0,  // 8
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, X, X, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, X, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, 0, 0, 0, 0,  // 9
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, X, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, X, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, X, X, 0, 0, 0, 0, 0,  // 10 ="0"
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, X, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, X, X, 0, 0, 0, 0, 0,  // 11
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, X, X, 0, 0, 0, 0, 0,  // 12
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, X, X, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, X, X, 0, 0, 0, 0, 0,  // 13
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, X, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, 0, 0, X, 0, 0, 0,  // 14
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, X, 0, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, X, 0, 0, 0, X, 0, 0, 0, 0,
    0, X, X, X, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, X, X, X, X, X, X, 0, 0, 0,  // 15
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, X, X, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, X, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, X, X, 0, 0, 0, 0, 0,  // 16
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, X, X, X, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, X, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, X, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, X, X, X, X, X, X, 0, 0, 0,  // 17
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, 0, X, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, X, X, 0, 0, 0, 0, 0,  // 18
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, X, X, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, X, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, X, X, 0, 0, 0, 0, 0,  // 19
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, 0, X, 0, 0, 0,
    0, 0, 0, X, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,
    0, X, 0, 0, 0, 0, X, 0, 0, 0,
    0, 0, X, 0, 0, X, 0, 0, 0, 0,
    0, 0, 0, X, X, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, X, 0, 0, 0,  // 20="0"
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 21="1"
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, X, 0, 0, 0,  // 22="2"
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, X, 0, 0, 0,  // 23="3"
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 24="4"
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, X, 0, 0, 0,  // 25="5"
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, X, 0, 0, 0,   // 26="6"
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, X, 0, 0, 0,  // 27="7"
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, X, 0, 0, 0,  // 28="8"
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },

  { 0, 0, 0, 0, X, X, X, 0, 0, 0,  // 29="9"
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, X, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, 0, 0, 0, X, 0, 0,
    0, 0, 0, 0, X, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  { TEN_0 },                        // 30
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 31="1"
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0,
    0, 0, X, 0, 0, 0, 0, 0, 0, 0
  },
  { TEN_0 },
  { TEN_0 },
  { TEN_0 },
  { TEN_0 },
  { TEN_0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 37="1"
    0, 0, 0, 0, 0, X, X, 0, 0, 0,
    0, 0, 0, 0, 0, 0, X, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, X, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, X, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0  
  },

  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 38="1"
    0, 0, X, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, X, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 39="1"
    X, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, X, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }

};


//-----------------------------------------------------------------
//  Output patterns
//-----------------------------------------------------------------

byte Target[MAX_PATTERNS + 1][NUM_OUTPUTS] = {
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //0
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 }, //6
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 }, //9
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //10
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, //11
  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 }, //12
  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 }, //13
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 }, //16  )
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 }, //19
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //20
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, //21
  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 }, //22
  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 }, //23
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 }, //26  )
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 }, //29
  { TEN_0 }, //30
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, //31
  { TEN_0 },
  { TEN_0 },
  { TEN_0 },
  { TEN_0 },
  { TEN_0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }  //39

};



//-----------------------------------------------------------------
//  Test Inputs
//-----------------------------------------------------------------


byte TestInputC01[NUM_INPUTS] = {  // "1"
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

byte TestInputC02[NUM_INPUTS] = {  // "2"
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, X, 0, 0, 0, 0, 0, 0,
  0, 0, 0, X, 0, 0, 0, 0, 0, 0,
  0, 0, 0, X, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

byte TestInputC03[NUM_INPUTS] = {  // "3"
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

byte TestInputC06[NUM_INPUTS] = {   //  "6"
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, X, 0, 0, 0, 0, 0, 0,
  0, 0, 0, X, 0, 0, 0, 0, 0, 0,
  0, 0, 0, X, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, X, 0, 0, 0, X, 0, 0,
  0, 0, 0, X, 0, 0, 0, X, 0, 0,
  0, 0, 0, X, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};



byte TestInputT1[NUM_INPUTS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Test pattern T1 1, untrained
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, X, 0, 0, 0, 0, 0, 0, 0, 0,
  0, X, 0, 0, 0, 0, 0, 0, 0, 0,
  0, X, 0, 0, 0, 0, 0, 0, 0, 0,
  0, X, 0, 0, 0, 0, 0, 0, 0, 0,
  0, X, 0, 0, 0, 0, 0, 0, 0, 0,
  0, X, 0, 0, 0, 0, 0, 0, 0, 0,
  0, X, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

byte TestInputT9[NUM_INPUTS] =  // Test pattern T9 9, untrained
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, X, 0, 0, 0, X, 0, 0,
  0, 0, 0, X, 0, 0, 0, X, 0, 0,
  0, 0, 0, X, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, 0, 0, 0, X, 0, 0,
  0, 0, 0, 0, X, X, X, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/*  ^^^^^^^^^^^^^^^     End NetConfiguration      ^^^^^^^^^^^^^^^^
 *****************************************************************/



/******************************************************************
   Tools
 ******************************************************************/

const char* lnUp="^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
const char* lnDn="vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv";

//-----------------------------------------------------------------
//  tool: millis to cstring
//-----------------------------------------------------------------

char * millis_to_strF(int ms) {
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

  char str[20] = "";
  sprintf(str, "%d.%02d:%02d:%02d", Days, Hours, Mins, Secs);
  return str;
}


//-----------------------------------------------------------------
// tool: NN: calc Activation + Net-Error
//-----------------------------------------------------------------

void computeActErr() {
  //--------------------------------------------------
  //  Compute hidden layer activations
  //--------------------------------------------------

  for ( i = 0 ; i < NUM_HIDDEN ; i++ ) {
    Accum = HiddenWeights[NUM_INPUTS][i] ;
    for ( j = 0 ; j < NUM_INPUTS ; j++ ) {
      Accum += Input[p][j] * HiddenWeights[j][i] ;
    }
    HiddenActiv[i] = 1.0 / (1.0 + exp(-Accum)) ;
  }

  //--------------------------------------------------
  //  Compute output layer activations +  errors
  //--------------------------------------------------

  for ( i = 0 ; i < NUM_OUTPUTS ; i++ ) {
    Accum = OutputWeights[NUM_HIDDEN][i] ;
    for ( j = 0 ; j < NUM_HIDDEN ; j++ ) {
      Accum += HiddenActiv[j] * OutputWeights[j][i] ;
    }
    OutputActiv[i] = 1.0 / (1.0 + exp(-Accum)) ;
  }
}


//-----------------------------------------------------------------
// tool: NN statistics log to terminal monitor
//-----------------------------------------------------------------

void PrintStatistixx() {
  Serial.print ("TrainingCycle: "); Serial.print (TrainingCycle);
  if (TrainingCycle > 0) {
    Serial.print ("  Error=");      Serial.print (Error, 5);
    Serial.print ("  ThrSuccess="); Serial.print (ThrSuccess, 5);
    Serial.print ("  runtime (d:h:m:s)=");
    Serial.print (millis_to_strF(millis() - timestamp));
  }
}



//-----------------------------------------------------------------
// tool: NN Inputs/Outputs to terminal monitor
//-----------------------------------------------------------------

void PrintNetPattern()
{
  char buf[10];

  for ( p = 0 ; p < MAX_PATTERNS ; p++ ) {
    Serial.println();
    //Serial.print ("  Training Pattern: ");

    sprintf(buf, "%3d", p);
    Serial.print (buf);
    Serial.print (" In:");
    for ( i = 0 ; i < NUM_INPUTS ; i++ ) {
      if (!(i % 10)) {
        yield();
        Serial.println();
        Serial.print("    ");
      }
      Serial.print (Input[p][i], DEC);
    }
    Serial.print (" Targ: ");
    for ( i = 0 ; i < NUM_OUTPUTS ; i++ ) {
      Serial.print (Target[p][i], DEC);
      Serial.print ("");
    }

    computeActErr();

    Serial.print (" Out: ");
    for ( i = 0 ; i < NUM_OUTPUTS ; i++ ) {
      Serial.print (OutputActiv[i], 3);
      Serial.print (" ");
    }
    Serial.println();
    Serial.print ("        ROUNDED OUT: ");
    for ( i = 0 ; i < NUM_OUTPUTS ; i++ ) {
      Serial.print ((int)round(OutputActiv[i]));
      //Serial.print (" ");
    }
  }
  Serial.println();
  Serial.println(lnUp);
  PrintStatistixx();

  Serial.println();
#ifdef DEBUG
  Serial.println ("DEBUG MODE: ACTIVE! ");
#endif

}

/******************************************************************
   HAL (Heuristic Algorithmic Layer): Initialize
 ******************************************************************/

int initializeWeights() {
  //--------------------------------------------------
  //  Initialize HiddenWeights and ChangeHiddenWeights
  //--------------------------------------------------
  for ( i = 0 ; i < NUM_HIDDEN ; i++ ) {
    for ( j = 0 ; j <= NUM_INPUTS ; j++ ) {
      ChangeHiddenWeights[j][i] = 0.0 ;
      Rando = float(random(100)) / 100;
      HiddenWeights[j][i] = 2.0 * ( Rando - 0.5 ) * InitialWeightMax ;
    }
  }

  //--------------------------------------------------
  //  Initialize OutputWeights and ChangeOutputWeights
  //--------------------------------------------------
  for ( i = 0 ; i < NUM_OUTPUTS ; i ++ ) {
    for ( j = 0 ; j <= NUM_HIDDEN ; j++ ) {
      ChangeOutputWeights[j][i] = 0.0 ;
      Rando = float(random(100)) / 100;
      OutputWeights[j][i] = 2.0 * ( Rando - 0.5 ) * InitialWeightMax ;
    }
  }
}

/******************************************************************
   HAL: Backpropagation Learning
 ******************************************************************/



//******************************************************************
int BP_Learning() {
  initializeWeights();

  Serial.println();
  Serial.println("Initial/Untrained Outputs: ");

  PrintNetPattern();

  //--------------------------------------------------
  //  Begin training
  //--------------------------------------------------
  for ( TrainingCycle = 1 ; TrainingCycle < MAXLOOPS ; TrainingCycle++) {

    //--------------------------------------------------
    //  Randomize order of training patterns
    //--------------------------------------------------
RESTART:
    for ( p = 0 ; p < MAX_PATTERNS ; p++) {
      q = random(MAX_PATTERNS);
      r = RandomizedIndex[p] ;
      RandomizedIndex[p] = RandomizedIndex[q] ;
      RandomizedIndex[q] = r ;
    }
    Error = 0.0 ;

    //--------------------------------------------------
    //  Cycle through each training pattern in the randomized order
    //--------------------------------------------------
    for ( q = 0 ; q < MAX_PATTERNS ; q++ ) {
      p = RandomizedIndex[q];

      //--------------------------------------------------
      //  Compute hidden layer activations
      //--------------------------------------------------

      for ( i = 0 ; i < NUM_HIDDEN ; i++ ) {

        Accum = HiddenWeights[NUM_INPUTS][i] ;
        for ( j = 0 ; j < NUM_INPUTS ; j++ ) {
          Accum += Input[p][j] * HiddenWeights[j][i] ;
        }
        HiddenActiv[i] = 1.0 / (1.0 + exp(-Accum)) ;
      }

      //--------------------------------------------------
      //  Compute output layer activations + calculate errors
      //--------------------------------------------------
      for ( i = 0 ; i < NUM_OUTPUTS ; i++ ) {

        Accum = OutputWeights[NUM_HIDDEN][i] ;
        for ( j = 0 ; j < NUM_HIDDEN ; j++ ) {
          Accum += HiddenActiv[j] * OutputWeights[j][i] ;
        }
        OutputActiv[i] = 1.0 / (1.0 + exp(-Accum)) ;
        OutputDelta[i] = (Target[p][i] - OutputActiv[i]) * OutputActiv[i] * (1.0 - OutputActiv[i]) ;
        Error += 0.5 * (Target[p][i] - OutputActiv[i]) * (Target[p][i] - OutputActiv[i]) ;
      }

      //--------------------------------------------------
      //  Backpropagate errors to hidden layer
      //--------------------------------------------------

      for ( i = 0 ; i < NUM_HIDDEN ; i++ ) {
        Accum = 0.0 ;
        yield();
        for ( j = 0 ; j < NUM_OUTPUTS ; j++ ) {
          Accum += OutputWeights[i][j] * OutputDelta[j] ;
        }
        HiddenDelta[i] = Accum * HiddenActiv[i] * (1.0 - HiddenActiv[i]) ;
      }


      //--------------------------------------------------
      //  Update Inner --> Hidden Weights
      //--------------------------------------------------

      for ( i = 0 ; i < NUM_HIDDEN ; i++ ) {
        yield();
        ChangeHiddenWeights[NUM_INPUTS][i] = LearningRate * HiddenDelta[i] + Momentum * ChangeHiddenWeights[NUM_INPUTS][i] ;
        HiddenWeights[NUM_INPUTS][i] += ChangeHiddenWeights[NUM_INPUTS][i] ;
        for ( j = 0 ; j < NUM_INPUTS ; j++ ) {
          ChangeHiddenWeights[j][i] = LearningRate * Input[p][j] * HiddenDelta[i] + Momentum * ChangeHiddenWeights[j][i];
          HiddenWeights[j][i] += ChangeHiddenWeights[j][i] ;
        }
      }

      //--------------------------------------------------
      //  Update Hidden --> Output Weights
      //--------------------------------------------------

      for ( i = 0 ; i < NUM_OUTPUTS ; i ++ ) {

        ChangeOutputWeights[NUM_HIDDEN][i] = LearningRate * OutputDelta[i] + Momentum * ChangeOutputWeights[NUM_HIDDEN][i] ;
        OutputWeights[NUM_HIDDEN][i] += ChangeOutputWeights[NUM_HIDDEN][i] ;
        for ( j = 0 ; j < NUM_HIDDEN ; j++ ) {
          ChangeOutputWeights[j][i] = LearningRate * HiddenActiv[j] * OutputDelta[i] + Momentum * ChangeOutputWeights[j][i] ;
          OutputWeights[j][i] += ChangeOutputWeights[j][i] ;
        }
      }
    }

    //--------------------------------------------------
    //  Every (n) cycles send to terminal for display
    //--------------------------------------------------

    ReportInterval = ReportInterval - 1;
    if (ReportInterval == 0)
    {
      Serial.println();
      Serial.println(lnDn);
      PrintStatistixx();

      PrintNetPattern();

      if (TrainingCycle == 1)
      {
        ReportInterval = REPORT_N - 1;
      }
      else
      {
        ReportInterval = REPORT_N;
      }
    }


    //--------------------------------------------------
    // If (error rate < pre-determined threshold) => end
    //--------------------------------------------------

    // if captured in local minimum or oscillating:
    if ( Error > 1.0 && TrainingCycle > 3000 ) {
      initializeWeights();
      goto RESTART;
    }
    if ( Error > 0.6 && TrainingCycle > 20000 ) {
      initializeWeights();
      goto RESTART;
    }
    

    // success?
    if ( Error < ThrSuccess ) return 0 ;  // 0:  training OK: no err
  }
  return -1;              // -1: loop limit reached: no success
}





/******************************************************************
   HAL (Heuristic Algorithmic Layer): Recognize arbitrary patterns
 ******************************************************************/

void InputPatternRecognition(byte TestInput[NUM_INPUTS] ) {
  char buf[10];

  for (int i = 0; i < NUM_INPUTS; i++) {
    Input[MAX_PATTERNS][i] = TestInput[i];
  }

  Serial.println();
  Serial.print ("  Test Pattern ");

  Serial.print (" In:");
  for ( i = 0 ; i < NUM_INPUTS ; i++ ) {
    if (!(i % 10)) {
      yield();
      Serial.println();
      Serial.print("    ");
    }
    Serial.print (Input[MAX_PATTERNS][i], DEC);
  }

  computeActErr();

  Serial.print (" Out: ");
  for ( i = 0 ; i < NUM_OUTPUTS ; i++ ) {
    Serial.print (OutputActiv[i], 3);
    Serial.print (" ");
  }
  Serial.println();
  Serial.print ("       ROUNDED OUT: ");
  for ( i = 0 ; i < NUM_OUTPUTS ; i++ ) {
    Serial.print ((int)round(OutputActiv[i]));
    //Serial.print (" ");
  }

  Serial.println();
  Serial.println(lnUp);

}




volatile static int8_t StateMode, ModeLearn = 1, ModeDetect = 0, ModePause = 0;

/******************************************************************
   setup
 ******************************************************************/

void setup() {
  Serial.begin(115200);
  delay(1000);
  timestamp = millis();

  randomSeed(analogRead(A0));
  ReportInterval = 1;
  for ( p = 0 ; p < MAX_PATTERNS ; p++ ) {
    RandomizedIndex[p] = p ;
  }
}



/******************************************************************
   loop
 ******************************************************************/
void loop() {
  volatile static int8_t result = -1;

  //--------------------------------------------------
  // start Backpropagation Learning (BP)
  //--------------------------------------------------
  result = BP_Learning();

#ifdef DEBUG
  Serial.println ("DEBUG MODE: ACTIVE! ");
#endif

  if (result == 0) {               // 0:  training OK: no err
    // Error < ThrSuccess

    Serial.println ();
    Serial.println(lnDn);
    PrintStatistixx();
    PrintNetPattern();
    Serial.println ();
    Serial.println ();
    Serial.println ("Training Set Solved! ");
    Serial.println ("-------------------- ");
    Serial.println ();
    Serial.println ();

  }

  else if (result == -1) {      // -1: loop limit reached: no success

    Serial.println(lnDn);
    PrintStatistixx();
    PrintNetPattern();
    Serial.println ();
    Serial.println ();
    Serial.println ("loop limit reached - Training aborted! ");
    Serial.println ("-------------------------------------- ");
    Serial.println ();
    Serial.println ();

  }


  // Recognize test patterns:

  Serial.println ("TestInputC01 \"1\" ");  // Test, debug
  InputPatternRecognition( TestInputC01 );
  Serial.println ();
  Serial.println ();

  Serial.println ("TestInputC06 \"6\" ");  // Test, debug
  InputPatternRecognition( TestInputC06 );
  Serial.println ();
  Serial.println ();

  Serial.println ("TestInputT1, untrained 1");  // Test T1, debug
  InputPatternRecognition( TestInputT1 );
  Serial.println ();
  Serial.println ();

  Serial.println ("TestInputT9, untrained 9");  // Test T9, debug
  InputPatternRecognition( TestInputT9 );
  Serial.println ();
  Serial.println ();

#ifdef DEBUG
  Serial.println ("DEBUG MODE: ACTIVE! ");
#endif


  while (true) {        // debug: loop forever
    delay(1000); // training finished
  }

}
