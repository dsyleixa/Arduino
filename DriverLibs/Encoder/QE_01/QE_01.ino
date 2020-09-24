/************************************************************
*
* Demo-Programm zur Auswertung eines händisch betriebenen
* Drehencoders (Quadraturencoder) mit dem Arduino im 
* Polling-Verfahren
*
* Kann von jederman frei verwendet werden, aber bitte den 
* Hinweis: "Entnommen aus http://www.meinDUINO.de" einfügen
*
************************************************************/

#define motenc1A 2
#define motenc1B 3
#define motor1_A 4
#define motor1_B 5
#define motor1_Speed 6

#define motenc2A 7
#define motenc2B 8
#define motor2_A 9
#define motor2_B 10
#define motor2_Speed 11
 

 
int8_t altAB = 0;

// 1/1 Auflösung
int8_t schrittTab[16] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; 

// 1/4 Auflösung
// int8_t schrittTab[16] = {0,0,0,0,0,0,0,-1,0,0,0,0,0,1,0,0}; 


int8_t readEncoder(int pinA, int pinB) {
  altAB <<= 2;
  altAB &= B00001100;
  altAB |= (digitalRead(pinA) << 1) | digitalRead(pinB);
  return(schrittTab[altAB]);
}

void setup() {
  /*
  pinMode(encoderA, INPUT);
  pinMode(encoderB, INPUT);
  */
  pinMode(motenc1A, INPUT_PULLUP);
  pinMode(motenc1B, INPUT_PULLUP);

  Serial.begin(115200);
}


void loop() {
  int wert = 0;
  int wertAlt = 0;
  
  while(true) {
    wert += readEncoder(motenc1A, motenc1B);

    if(wert != wertAlt) {
      Serial.println(wert);
      wertAlt = wert;
    }
  }
}
