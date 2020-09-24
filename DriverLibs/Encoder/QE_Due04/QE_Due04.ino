/************************************************************
*
* Demo-Programm zur Auswertung eines händisch betriebenen
* Drehencoders (Quadraturencoder) mit dem Arduino im 
* Timer-Interrupt mit einer Abfragefrequenz von rd. 1kHz
*
* Kann von jederman frei verwendet werden, aber bitte den 
* Hinweis: "Entnommen aus http://www.meinDUINO.de" einfügen
*
************************************************************/

#include <DueTimer.h>
 
//  Encoder Pins
#define encoder0A 22
#define encoder0B 23

#define encoder1A 26
#define encoder1B 27

// Globale Variablen zur Auswertung in der
// Interrupt-Service-Routine (ISR)
volatile int8_t   altAB0 = 0,       altAB1 = 0;
volatile int32_t  encoderWert0 = 0, encoderWert1 = 0;

// Die beiden Schritt-Tabellen für volle oder 1/4-Auflösung
// 1/1 Auflösung
//int8_t schrittTab[16] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; 

// 1/2 Auflösung
int8_t schrittTab[16] = {0, 0,0,0,1,0,0,-1, 0,0,0,1,0,0,-1,0}; 


// 1/4 Auflösung
//int8_t schrittTab[16] = {0,0,0,0,0,0,0,-1,0,0,0,0,0,1,0,0}; 


/*************************************************************
*
* Interrupt Service Routine
*
*************************************************************/

void myHandler() {

  altAB0 <<= 2;
  altAB0 &= B00001100;
  altAB0 |= (digitalRead(encoder0A) << 1) | digitalRead(encoder0B);
  encoderWert0 += schrittTab[altAB0];
  
  
  altAB1 <<= 2;
  altAB1 &= B00001100;
  altAB1 |= (digitalRead(encoder1A) << 1) | digitalRead(encoder1B);
  encoderWert1 += schrittTab[altAB1];  
    
}

/*************************************************************
* 
* void setup()
*
* Wird einmal beim Programmstart ausgeführt
*
*************************************************************/
void setup() {
  pinMode(encoder0A, INPUT_PULLUP);
  pinMode(encoder0B, INPUT_PULLUP);
  pinMode(encoder1A, INPUT_PULLUP);
  pinMode(encoder1B, INPUT_PULLUP);
  
     
  Timer1.attachInterrupt(myHandler);
  Timer1.start(200); // Calls every ...µs

  Serial.begin(115200);
}


/*************************************************************
* 
* void loop()
*
* Wird immer wieder durchlaufen
*
*************************************************************/
void loop() {
 
  while(true) {
    Serial.print(encoderWert0); Serial.print("     "); Serial.print(encoderWert1); Serial.println("     ");
    delay(100);
  }
}


