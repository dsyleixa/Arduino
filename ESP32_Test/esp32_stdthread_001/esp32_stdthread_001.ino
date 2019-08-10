// std::thread for ESP32, Arduino IDE

// ver 0.0.6 fibonacci

#include <Arduino.h>
#include <thread>
#include <freertos/task.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

const auto one_sec = std::chrono::seconds { 1 };



int fibbonacci(int n) {
   if(n == 0){
      return 0;
   } else if(n == 1) {
      return 1;
   } else {
      return (fibbonacci(n-1) + fibbonacci(n-2));
   }
}



void blinker_loop() {
    thread_local uint32_t counter = 0;
    Serial.println((String)"blinker_loop Current priority :" + uxTaskPriorityGet(NULL));
    vTaskPrioritySet(NULL,0);//set Priority
    Serial.println((String)"blinker_loop Current priority :" + uxTaskPriorityGet(NULL));
    while(true) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println((String)"blinker_loop (HIGH) counter: "+ counter);
        std::this_thread::sleep_for(one_sec);
        
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println((String)"blinker_loop (LOW) counter: "+ counter);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        
        counter++;
    }
}


void fibonacci_loop() {
    thread_local uint32_t counter = 0, i=0;
    Serial.println((String)"fibonacci_loop Current priority :" + uxTaskPriorityGet(NULL));
    vTaskPrioritySet(NULL,0);//set Priority
    Serial.println((String)"fibonacci_loop Current priority :" + uxTaskPriorityGet(NULL));
    while(true) {
        for(i=30; i<41; i++) {    // limits: test, debug
          Serial.println( (String)"Fibbonacci of "+i+"="+fibbonacci(i));            
        }        
        Serial.println((String)"fibonacci_loop counter: "+counter);  
        counter++;      
    }
}



std::thread *thread_1;
std::thread *thread_2;


void setup() {
  Serial.begin(115200);
  delay(1000);
  // vTaskPrioritySet(NULL, 3); no effect
  
  thread_1 = new std::thread(blinker_loop);
  thread_2 = new std::thread(fibonacci_loop);
}


void loop() {
    delay(500);
    static uint32_t main_loop_counter = 0;
    Serial.println((String)"main loop: " + main_loop_counter);
    delay(5000);
    main_loop_counter++;
}
