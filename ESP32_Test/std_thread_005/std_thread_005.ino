// std::thread for ESP32, Arduino IDE

// ver 0.0.5 fibonacci

#include <Arduino.h>
#include <thread>
#include <esp_pthread.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

const auto one_sec = std::chrono::seconds { 1 };


//-------------------------------------------------
int fibbonacci(int n) {
   if(n == 0){
      return 0;
   } else if(n == 1) {
      return 1;
   } else {
      return (fibbonacci(n-1) + fibbonacci(n-2));
   }
}


//-------------------------------------------------
void fibonacci_loop() {
    thread_local uint32_t counter = 0, i=0;
    while(true) {
        feedLoopWDT();
        Serial.println((String)"fibonacci_loop counter: "+counter);  
        for(i=30; i<41; i++) {    // limits: test, debug
          Serial.println( (String)"Fibbonacci of "+i+"="+fibbonacci(i));            
        }                
        counter++;      
    }
}


//-------------------------------------------------
void blinker_loop() {
    thread_local uint32_t counter = 0;
    while(true) {
        feedLoopWDT();
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println((String)"blinker_loop (HIGH) counter: "+ counter);
        std::this_thread::sleep_for(one_sec);
        
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println((String)"blinker_loop (LOW) counter: "+ counter);
        std::this_thread::sleep_for(one_sec);
        
        counter++;
    }
}


//-------------------------------------------------
std::thread *thread_1;
std::thread *thread_2;

void setup() {
  Serial.begin(115200);
  //debug
  delay(1000);
  
  esp_pthread_cfg_t cfg;
  esp_pthread_get_cfg(&cfg);
  cfg.prio = 1;
  esp_pthread_set_cfg(&cfg);
  
  thread_2 = new std::thread(fibonacci_loop);  
  thread_1 = new std::thread(blinker_loop);


  feedLoopWDT();
}


//-------------------------------------------------
void loop() {
    static uint32_t main_loop_counter = 0;
    
    feedLoopWDT();
    Serial.println((String)"main loop: " + main_loop_counter);
    delay(5000);
    main_loop_counter++;
}

//-------------------------------------------------
// EOF
