#include <ArduArray.h>


//--------------------------------------------------
//--------------------------------------------------
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nSerial started...\n");  
  
  int Test1[] = { 4, 2, 9 };
  double Test2[] = { 8.2, 7.1, 5.4, 9.6, 1.3 };

  Serial.println("bubblesort");
  bubblesort(Test1, 3);
  for(auto& n : Test1) {
    Serial.println(n);
  }
  Serial.println();
  
  bubblesort(Test2, 5);
  for(auto& n : Test2) {
    Serial.println(n);
  }
  Serial.println();
  
  Serial.println("shellsort");
  shellsort(Test1, 3);
  for(auto& n : Test1) {
    Serial.println(n);
  }
  Serial.println();
  
  shellsort(Test2, 5);
  for(auto& n : Test2) {
    Serial.println(n);
  }
  Serial.println();

  Serial.println("median of 5 int");
  fifoi[0][0]=0;
  fifoi[0][1]=2;
  fifoi[0][2]=4;
  fifoi[0][3]=3;
  fifoi[0][4]=1;
  Serial.println(medianOfi(0,5));

  Serial.println("    New=7; median of 5 int (value=1 at pos=[4] is out-shifted");
  Serial.println(medianNewOfi(7, 0, 5) );
  Serial.println("    New=10; median of 5 int (updated value=3 at pos=[4] is out-shifted");
  Serial.println(medianNewOfi(10, 0, 5) );
  Serial.println();

  Serial.println("Lowpass Filter");
  float oldAvrg=100.0;
  float fnew=200.0;
  Serial.println(lowpassFilt(fnew, oldAvrg, 0.5) );
  Serial.println(lowpassFilt(fnew, oldAvrg, 0.2) );
  
  
  
}

//--------------------------------------------------
//--------------------------------------------------
void loop() {
  // put your main code here, to run repeatedly:

}
