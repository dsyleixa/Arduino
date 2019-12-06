/*
  Programmbeispiel PCF8574Exp:

  Testaufbau:
  http://files.arduino-projekte.webnode.at/200001618-8f0e490081/PCF8574%20Test_Steckplatine_s.jpg !
  
  Das Programmbeispiel schaltet eine LED bei Betaetigung eines Tasters 
  - entsprechend dem obigen Testaufbau - ein.
  
  Achtung: Pins, die als Eingaenge verwendet werden, muessen vor dem Einlesen 
  auf logisch 1 gesetzt werden
  - Bit-weise mit writeBit(pinNr, 1) oder writeBitHigh(pinNr) 
  - oder wenn alle Pins als Eingaenge verwendet werden - mit writeByte(0xFF) !!  
  
  Mehr Infos: http://arduino-projekte.webnode.at/projekte/portexpander-pcf8574/ !

*/

#include <PCF8574Exp.h>
#include <Wire.h>

PCF8574Exp I2CExp;
#define    PCF8574addr  0x20

void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  I2CExp.begin(PCF8574addr);    // I2C-Adresse des PCF8574  
  delay(100);                   //   
  if ( !I2CExp.isReady() )  {
     Serial.println();
     Serial.print("PCF I2C error at addr HEX x"); Serial.println(PCF8574addr,HEX);
     delay(10);      
  }
  I2CExp.writeByte(0xF); // (all pins INPUT HIGH)
  delay(1);
}

void loop() {
   I2CExp.writeBitHigh(1);     //Bit 1 ist Eingang und muss vor dem Einlesen auf 1 gesetzt werden
   if (I2CExp.readBit(1) == 0) {  // Wenn Taster gedrueckt, 
       I2CExp.writeBit(2, 0);     // dann LED ein
       Serial.println("pressed");
   }
   else {
      I2CExp.writeBit(2, 1); //Wenn Taster nicht gedrueckt, LED aus
      Serial.println("up");
   }
   delay(100);
}
