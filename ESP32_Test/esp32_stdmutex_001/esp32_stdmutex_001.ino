// std::thread for ESP32, Arduino IDE

// ver 0.0.6 fibonacci

#include <Arduino.h>
#include <thread>
#include <freertos/task.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

const auto one_sec = std::chrono::seconds { 1 };

#include <mutex>

std::mutex print_mutex;

std::thread *thread_1;
std::thread *thread_2;


void mtxPrintln(String str) // String: Arduino API  // C++: std::string
{
    print_mutex.lock();
    Serial.println(str);
    print_mutex.unlock();
}

void loop1() {
    thread_local uint32_t counter = 0, i=0;  
    vTaskPrioritySet(NULL,0); //set Priority
    
    while(true) {
        // do stuff     
    }
}

void loop2() {
    thread_local uint32_t counter = 0, i=0;  
    vTaskPrioritySet(NULL,0); //set Priority
    
    while(true) {
        // do stuff     
    }
}



void setup() {
  Serial.begin(115200);
  delay(1000);
 
  thread_1 = new std::thread(loop1);
  thread_2 = new std::thread(loop2);
}


void loop() {
    delay(500);
    static uint32_t main_loop_counter = 0;
    mtxPrintln((String)"main loop: " + main_loop_counter);
    delay(500);
    main_loop_counter++;
}
