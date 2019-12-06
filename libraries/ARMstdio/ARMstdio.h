// enables sprintf feat. float and dtostrf

 
#pragma once

#if defined (SAM)
   #include "avr/dtostrf.h"
#endif
 
void ARMstdio_ini() {  
   #if defined (SAM)
      asm(".global _printf_float");
   #endif
}
 