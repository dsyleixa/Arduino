// Button Press Lib Testcode
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose only by personal written permission 
// by the author.

// ButtonClass.cpp


#include <ButtonClass.h>


tButton btn1;
tButton btn2;


void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(13, OUTPUT); // debug
  
  Serial.println("Serial started\n");
  btn1.init(4, INPUT_PULLUP);  // <<< adjust
  btn2.init(5, INPUT_PULLUP);  // <<< adjust
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