//PCF8574Exp.cpp
//Code fuer Arduino
//Author Retian
//Version 2.2


/*
Ansteuerung des I2C-Expander PCF8574

PCF8574Exp Name;

Beispiel siehe unter:
http://arduino-projekte.webnode.at/projekte/portexpander-pcf8574/

Funktionen siehe unter:
http://arduino-projekte.webnode.at/projekte/portexpander-pcf8574/funktionen/

*/

#include "Arduino.h"
#include "PCF8574Exp.h"
#include "Wire.h"


PCF8574Exp::PCF8574Exp()
{
}


//***************************************************************

void PCF8574Exp::begin(byte I2CAdd)
{
  _I2CAdd = I2CAdd;
  Wire.begin(_I2CAdd);

  LastByte = 0x00;
}


//***************************************************************

boolean PCF8574Exp::isReady()
{
  Wire.beginTransmission(_I2CAdd);
  Wire.write(_I2CAdd);
  if(Wire.endTransmission(true) == 0) return(true);
  else return(false);
}


//***************************************************************

void PCF8574Exp::writeByte(byte SByte)
{
  _SByte = SByte;

  Wire.beginTransmission(_I2CAdd);
  Wire.write(_SByte);
  Wire.endTransmission(true);
  LastByte = _SByte;
}



//***************************************************************

void PCF8574Exp::writeBit(byte BitNumber, bool BitWert)
{
  _BitNumber = BitNumber;
  _BitWert = BitWert;

  byte BitMask = 0x01 << _BitNumber;
  
  if(_BitWert == 1)
  {
    //Serial.print("BitMask Bit 1: ");
    //Serial.println(BitMask, BIN);

    Wire.beginTransmission(_I2CAdd);
    Wire.write(LastByte | BitMask);
    Wire.endTransmission(true);
    LastByte = LastByte | BitMask;
  }

  if(BitWert == 0)
  {
    BitMask = ~BitMask;
    //Serial.print("BitMask Bit 0: ");
    //Serial.println(BitMask, BIN);

    Wire.beginTransmission(_I2CAdd);
    Wire.write(LastByte & BitMask);
    Wire.endTransmission(true);
    LastByte = LastByte & BitMask;
  }
}

// alias:
void PCF8574Exp::digitalWrite(byte BitNumber, bool BitWert) {
   PCF8574Exp::writeBit(BitNumber, BitWert);
}




//***************************************************************

void PCF8574Exp::writeBitHigh(byte BitNumber)
{
  _BitNumber = BitNumber;
  byte BitMask = 0x01 << _BitNumber;

  Wire.beginTransmission(_I2CAdd);
  Wire.write(LastByte | BitMask);
  Wire.endTransmission(true);
  LastByte = LastByte | BitMask;
}



//***************************************************************

void PCF8574Exp::writeBitLow(byte BitNumber)
{
  _BitNumber = BitNumber;
  byte BitMask = ~(0x01 << _BitNumber);

  Wire.beginTransmission(_I2CAdd);
  Wire.write(LastByte & BitMask);
  Wire.endTransmission(true);
  LastByte = LastByte & BitMask;

  //Serial.print("LastByte nachher: ");
  //Serial.println(LastByte, BIN);
}



//***************************************************************

byte PCF8574Exp::readByte()
{ 
  Wire.requestFrom(int(_I2CAdd), 1, true);
  while(Wire.available() == 0);
  byte Wert = Wire.read();
  return(Wert);
}




//***************************************************************

byte PCF8574Exp::readBit(byte BitNumber)
{
  _BitNumber = BitNumber;
  byte BitMask = (0x01 << _BitNumber);

  Wire.requestFrom(int(_I2CAdd), 1, true);
  while(Wire.available() == 0);
  byte Wert = Wire.read();
  Wert = ((Wert & BitMask) >> _BitNumber);
  return(Wert);
}

// alias:
byte PCF8574Exp::digitalRead(byte BitNumber) {
  return PCF8574Exp::readBit(BitNumber);
}


//***************************************************************

byte PCF8574Exp::readBitpullup(byte BitNumber)
{
   _BitNumber = BitNumber;
  byte BitMask = 0x01 << _BitNumber;

  Wire.beginTransmission(_I2CAdd);
  Wire.write(LastByte | BitMask);
  Wire.endTransmission(true);
  LastByte = LastByte | BitMask;

  Wire.requestFrom(int(_I2CAdd), 1, true);
  while(Wire.available() == 0);
  byte Wert = Wire.read();
  Wert = ((Wert & BitMask) >> _BitNumber);
  return(Wert);
}

