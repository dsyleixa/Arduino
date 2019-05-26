#include <Wire.h>
#include "Nunchuk.h"

void setup() {
    
    Serial.begin(115200);
    delay(2000);
    Serial.println("Serial() started");
    Wire.begin();
    Wire.setClock(100000);
    Serial.println("Wire() started");
    
    // nunchuk_init_power(); // A1 and A2 is power supply    
    // scanning for Nunchuk address 0x52
    Serial.println("Scanning for Nunchuk"); 
    Wire.beginTransmission(0x52);
    int error = Wire.endTransmission();     
    if (error == 0)        {   
      Serial.println("Nunchuk found at 0x52"); 
    }
    else {
      Serial.print("Nunchuk error: "); 
      Serial.println(error); 
    }   
    Serial.println(); Serial.println();
    
    // nunchuk_init
    nunchuk_init();
}


void loop() {
    if (nunchuk_read()) {
        // Work with nunchuk_data
        nunchuk_print(true);
    }
    delay(10);
} 
