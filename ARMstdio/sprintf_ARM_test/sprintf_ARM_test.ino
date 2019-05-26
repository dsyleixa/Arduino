
#include <ARMstdio.h>


void setup() {
    ARMstdio_ini();  
    char buf[40];  

    Serial.begin(115200);
    delay(1500); // for native USB ports
    Serial.println("print Floating Point on Arduino");
    Serial.print("regular Serial.print(1.2345) ");
    Serial.println(1.2345);

    Serial.print("sprintf(2.3456) ");
    sprintf(buf, "%7.4f", 2.3456);
    Serial.println(buf);

    Serial.print("dtostrf(3.4567) ");
    dtostrf(3.456, 7, 4, buf);
    Serial.println(buf);
}

void loop()
{
}
