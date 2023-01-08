// std::thread plus mutex for ESP32, Arduino IDE

#include <Arduino.h>
#include <thread>
#include <freertos/task.h>
#include <mutex>

const auto one_sec = std::chrono::seconds { 1 };

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

std::mutex serial_mutex;


void loop1() {
    thread_local uint32_t counter = 0, i=0;  
    vTaskPrioritySet(NULL,0); //set Priority
    
    while(true) 
        serial_mutex.lock();
        Serial.println((String)"this is loop1, counter="+counter);
        serial_mutex.unlock();
        counter++;
        // do stuff     
        delay(10);
    }
}

void loop2() {
    thread_local uint32_t counter = 0, i=0;  
    vTaskPrioritySet(NULL,0); //set Priority
    
    while(true) {
        serial_mutex.lock();
        Serial.println((String)"this is loop2, counter="+counter);
        serial_mutex.unlock();
        counter++;
        // do stuff     
        delay(10);     
    }
}



void setup() {
  Serial.begin(115200);
  delay(1000);

  std::thread thread_1 (loop1);
  std::thread thread_2 (loop2);

}


void loop() {
    static uint32_t main_loop_counter = 0;
    serial_mutex.lock();
    Serial.println((String)"\nthis is main loop, main_loop_counter="+main_loop_counter );
    serial_mutex.unlock();
    delay(10);
    main_loop_counter++;
}
