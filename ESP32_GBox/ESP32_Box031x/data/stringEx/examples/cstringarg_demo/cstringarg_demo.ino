#include <stringEx.h>

char haystack[1024];
char sarg[64];

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("testing cstringarg()...");
  Serial.println();
  
 
  strcpy(haystack,"&varA=a;&varFf=5678.432;&varEi=0;&varC=c c;&varB=bb;?varDi= 1234 ");
  
  cstringarg(haystack, "varA", sarg);  // a
  Serial.print(">"); Serial.print(sarg); Serial.println("<");    
  cstringarg(haystack, "varB", sarg);  // bb
  Serial.print(">"); Serial.print(sarg); Serial.println("<"); 
  cstringarg(haystack, "varC", sarg);  // c c
  Serial.print(">"); Serial.print(sarg); Serial.println("<"); 
  cstringarg(haystack, "varDi", sarg); // _1234_ 
  Serial.print(">"); Serial.print(sarg); Serial.println("<"); 
  cstringarg(haystack, "varEi", sarg); // 0
  Serial.print(">"); Serial.print(sarg); Serial.println("<");
  cstringarg(haystack, "varFf", sarg); // 5678.432
  Serial.print(">"); Serial.print(sarg); Serial.println("<");
  
  cstringarg(haystack, "varZZZ", sarg);                        // not exists
  Serial.print(">"); Serial.print(sarg);   Serial.println("<");            
  Serial.println();

}

void loop() {
  // put your main code here, to run repeatedly:

}
