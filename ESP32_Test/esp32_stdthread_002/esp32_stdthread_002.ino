// std::thread for ESP32, Arduino IDE

// ver 0.0.7 fibonacci, GPIO, blink, sort und main counter

#include <Arduino.h>
#include <thread>
#include <freertos/task.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

const auto one_sec = std::chrono::seconds { 1 };



uint32_t fibonacci(int n) {
   if(n == 0){
      return 0;
   } else if(n == 1) {
      return 1;
   } else {
      return (fibonacci(n-1) + fibonacci(n-2));
   }
}

void shellsort(int size, int* A)
{
  int i, j, increment;
  int temp;
  increment = size / 2;

  while (increment > 0) {
    for (i = increment; i < size; i++) {
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) {
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }

    if (increment == 2)
       increment = 1;
    else
       increment = (unsigned int) (increment / 2.2);
  }
}


#define tpin1  11  // GPIO test pins digitalWrite
#define tpin2  12  // GPIO test pins digitalWrite
#define tpin3  10  // GPIO test pins digitalRead

int32_t test_GPIO() {   //  
   volatile static bool w=false, r;
   
   uint32_t y;
   for (y=0; y<100000; y++) {      
         digitalWrite(tpin1, w);
         w=!w;
         r=digitalRead(tpin3);
         digitalWrite(tpin2, w&!r);      
   }
   return 1;
}



void blinker_loop() {
    thread_local uint32_t counter = 0;
    vTaskPrioritySet(NULL,0);//set Priority
    Serial.println((String)"blinker_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");
    while(true) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println((String)"\nblinker_loop (HIGH) counter: "+ counter+"\n");
        std::this_thread::sleep_for(one_sec);
        
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println((String)"\nblinker_loop (LOW) counter: "+ counter+"\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        
        counter++;
    }
}


void fibonacci_loop() {
    thread_local uint32_t counter = 0, i=0;
    vTaskPrioritySet(NULL,0);//set Priority
    Serial.println((String)"fibonacci_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");
    while(true) {
        for(i=25; i<41; i++) {    // limits: test, debug
          Serial.println( (String)"\nfibonacci of "+i+"="+fibonacci(i)+"\n");            
        }        
        Serial.println((String)"\nfibonacci_loop counter: "+counter+"\n");  
        counter++;      
    }
}


void sort_loop() {
    static int array[1000];
    
    thread_local uint32_t counter = 0, i=0;    
    vTaskPrioritySet(NULL,0);//set Priority
    Serial.println((String)"sort_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");
    while(true) {
        Serial.println((String)"\nsort_loop counter: "+counter+"\n");
        
        for(i=0; i<1000; i++) {     
           array[i]= random(0,65000);           
        }        
        shellsort(1000, array);
        for(i=0; i<1000; i+=50) { // abgekürzte Ausgabe  da Serial sehr langsam   
           Serial.println((String)i + "="+array[i]+"\n");           
        }    
        counter++;      
    }
}


void GPIO_loop() {
   
    thread_local uint32_t counter = 0, i=0;    
    vTaskPrioritySet(NULL,0);//set Priority
    Serial.println((String)"GPIO_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");
    while(true) {
        Serial.println((String)"\nGPIO_loop counter: "+counter);
        
        test_GPIO();

        Serial.println((String)"\ntpin1="+digitalRead(tpin1)+" tpin2="+digitalRead(tpin2)+" tpin3="+digitalRead(tpin3)+"\n");         
        counter++;      
    }
}





std::thread *thread_1;
std::thread *thread_2;
std::thread *thread_3;
std::thread *thread_4;


void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(tpin1, OUTPUT);
  pinMode(tpin2, OUTPUT);
  pinMode(tpin3, INPUT_PULLUP);
  
  thread_1 = new std::thread(blinker_loop);
  thread_2 = new std::thread(fibonacci_loop);
  thread_3 = new std::thread(sort_loop);
  thread_4 = new std::thread(GPIO_loop);
}


void loop() {    
    static uint32_t main_loop_counter = 0;
    Serial.println((String)"\nmain loop: " + main_loop_counter);
    delay(500);
    main_loop_counter++;
}
