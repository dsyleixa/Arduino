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
  int adc;  
 
  //adc = analogRead(A0); Serial.print("A0="); Serial.println(adc);
  //adc = analogRead(A1); Serial.print("A1="); Serial.println(adc);
  //adc = analogRead(A2); Serial.print("A2(12)="); Serial.println(adc);
  //adc = analogRead(A2); Serial.print("A2(10)="); Serial.println(adc/4);

  adc = ads0.readADC_SingleEnded(2);  // ADS1115 port A2
  Serial.print("ADS(2,16bit)="); Serial.print(adc);
  adc = map(adc, 0,17530,  0,1023);
  Serial.print("     ADS(2,10bit)="); Serial.println(adc);

  // analog Button Pad (10bit):
  //        up=1
  // left=89    right=33
  //     dn=167-168
  //   bottom=351-352
  

  delay(50);

}
