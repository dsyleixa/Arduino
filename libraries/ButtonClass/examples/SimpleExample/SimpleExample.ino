// Button Press Lib Testcode
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose only by personal written permission 
// by the author.



#include <ButtonClass.h>


tButton btn1;
tButton btn2;


void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(13, OUTPUT); // debug
  
  Serial.println("Serial started\n");
  Serial.println("press either button by 1 short click, double click, or long press! \n");
  btn1.init(2, INPUT_PULLUP);  // <<< adjust
  btn2.init(3, INPUT_PULLUP);  // <<< adjust
}

//------------------------------------------------------------------------

void loop() {
  int8_t  btn;

  btn=btn1.click();
  if(btn) {
    Serial.print("btn1.click()=");
    Serial.println(btn);
  }
  btn=btn2.click();
  if(btn) {
    Serial.print("btn2.click()=");
    Serial.println(btn);
  }
  // delay(10);  // debug
}
