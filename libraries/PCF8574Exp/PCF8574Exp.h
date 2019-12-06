//PCF8574Exp.h

#ifndef PCF8574EXP_h
#define PCF8574EXP_h

#include "Arduino.h"


class PCF8574Exp
{
  public:
    PCF8574Exp();
    void begin(byte I2CAdd);
    void writeByte(byte SByte);
    byte readByte();
    byte readBit(byte BitNumber);
    byte digitalRead(byte BitNumber);

    byte readBitpullup(byte BitNumber);
    boolean isReady();
    void writeBit(byte BitNumber, bool BitWert);
    void digitalWrite(byte BitNumber, bool BitWert);

    void writeBitHigh(byte BitNumber);
    void writeBitLow(byte BitNumber);
        
  private:
    byte _I2CAdd;
    byte LastByte;
    byte _SByte;
    byte _BitNumber;
    byte _BitWert;

     
};

#endif