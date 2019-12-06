#include <Wire.h>
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads0(0x48);

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(1);
  ads0.begin();
}


void loop() {
  char str[100];
  uint16_t adc00, adc01, adc02, adc03;

  adc00 = ads0.readADC_SingleEnded(0);  // ADS1115 port A0
  delay(1);
  adc01 = ads0.readADC_SingleEnded(1);  // ADS1115 port A1
  delay(1);
  adc02 = ads0.readADC_SingleEnded(2);  // ADS1115 port A2
  delay(1);
  adc03 = ads0.readADC_SingleEnded(3);  // ADS1115 port A3
  delay(1);
  
  adc00 = map(adc00, 0, 17530,  0, 1023);
  adc01 = map(adc01, 0, 17530,  0, 1023);
  adc02 = map(adc02, 0, 17530,  0, 1023);
  adc03 = map(adc03, 0, 17530,  0, 1023);

  sprintf(str, "adc00=%5d  adc01=%5d  adc02=%5d  adc03=%5d", adc00, adc01, adc02, adc03);
  Serial.println(str);


  // analog Button Pad (10bit):
  //           up=0-1
  // left=88-89     right=32-33
  //         dn=166-168
  //       bottom=350-352


  delay(100);

}
