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

// An die Pins 2 und 3 ist der Encoder angeschlossen
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
* Wird aufgerufen, wenn der entsprechende Interrupt
* ausgelöst wird
*
*************************************************************/
ISR(TIMER1_COMPA_vect) {
  altAB0 <<= 2;
  altAB0 &= B00001100;
  altAB0 |= (digitalRead(encoder0A) << 1) | digitalRead(encoder0B);
  encoderWert0 += schrittTab[altAB];
  
  
  altAB1 <<= 2;
  altAB1 &= B00001100;
  altAB1 |= (digitalRead(encoder1A) << 1) | digitalRead(encoder1B);
  encoderWert1 += schrittTab[altAB];
  
  
  
  
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
  pinMode(encoder0A, INPUT_PULLUP);
  pinMode(encoder0B, INPUT_PULLUP);
  
  noInterrupts(); // Jetzt keine Interrupts
  TIMSK1 |= (1<<OCIE1A);  // Timer 1 Output Compare A Match Interrupt Enable

  TCCR1A = 0; // "Normaler" Modus

  // WGM12: CTC-Modus einschalten (Clear Timer on Compare match) 
  //        Stimmen OCR1A und Timer überein, wird der Interrupt
  //        ausgelöst
  // Bit CS12 und CS10 setzen = Vorteiler: 1024
  TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);

  // Frequenz = 16000000 / 1024 / 15 = rd. 1042Hz
  // Überlauf bei 14, weil die Zählung bei 0 beginnt OCR1A = 14;
  OCR1A = 4; 
  
  interrupts(); // Interrupts wieder erlauben

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


