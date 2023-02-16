void setup() {

   int count[10], r;

   Serial.begin(115200);
   delay(1000);
   Serial.println();
   Serial.println("Serial() started");
   srand(millis());

   for (int j=0; j<6; j++) {
      for (int i=0; i<10; i++) {
         count[i]=0;
      }
      for (int i=0; i<100; i++) {
         r=rand()%10;
         count[r]+=1;
      }
      for (int i=0; i<10; i++) {
         Serial.print(i);
         Serial.print(": ");
         Serial.println(count[i]);
      }

      Serial.println();
   }


}

void loop() {

}
