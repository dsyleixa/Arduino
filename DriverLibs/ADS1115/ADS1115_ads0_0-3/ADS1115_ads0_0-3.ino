#include <Wire.h>
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads0(0x48);


uint16_t adsMap(int32_t val, int vmax ) {  
  val=map(val, 0, 17550,  0, vmax);
  val=constrain(val, 0, vmax);
  return (uint16_t)val;
}


void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(1);
  ads0.begin();
  delay(1);
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
  
  adc00 = adsMap(adc00, 1023);
  adc01 = adsMap(adc01, 1023);
  adc02 = adsMap(adc02, 1023);
  adc03 = adsMap(adc03, 1023);

  sprintf(str, "adc00=%5d  adc01=%5d  adc02=%5d  adc03=%5d", adc00, adc01, adc02, adc03);
  Serial.println(str);


  // analog Button Pad (10bit):
  //           up=0-1
  // left=88-89     right=32-33
  //         dn=166-168
  //       bottom=350-352


  delay(100);

}
