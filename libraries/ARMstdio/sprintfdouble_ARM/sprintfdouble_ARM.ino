
#include <ARMstdio.h>


void setup() {
    ARMstdio_ini();  

    char buf[40];  

    Serial.begin(115200);
    delay(2000); // for native USB ports
    
    Serial.println("print Floating Point on Arduino");
    Serial.print("regular Serial.print(1.23456789012) : ");
    Serial.println(1.23456789012);

    Serial.print("sprintf 1.23456789012 %10.8f  : ");
    sprintf(buf, "%10.8f", 1.23456789012);
    Serial.println(buf);

    Serial.print("dtostrf 1.23456789012  10,8   : ");
    dtostrf(1.23456789012, 10, 8, buf);
    Serial.println(buf);
}

void loop()
{
    char buf[40];
    
    delay(2000);  
    
    Serial.println("print Floating Point on Arduino");
    Serial.print("regular Serial.print(1.23456789012) : ");
    Serial.println(1.23456789012);

    Serial.print("sprintf 1.23456789012 %10.8f  : ");
    sprintf(buf, "%10.8f", 1.23456789012);
    Serial.println(buf);

    Serial.print("dtostrf 1.23456789012  10,8   : ");
    dtostrf(1.23456789012, 10, 8, buf);
    Serial.println(buf);

    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println();
    
  
}
