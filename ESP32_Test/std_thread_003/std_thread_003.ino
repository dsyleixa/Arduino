// std::thread for ESP32, Arduino IDE

#include <Arduino.h>
#include <thread>
#include <chrono>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

const auto one_sec = std::chrono::seconds
{
    1
};

void counter_loop() {
    thread_local uint32_t counter = 0;
    while(true) {
        Serial.println((String)"counter_loop: " + counter);
        std::this_thread::sleep_for(one_sec);
        counter++;
    }
}

void blinker_loop() {
    thread_local uint32_t counter = 0;
    while(true) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println((String)"blinker_loop (HIGH) counter: "+ counter);
        std::this_thread::sleep_for(one_sec);
        
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println((String)"blinker_loop (LOW) counter: "+ counter);
        std::this_thread::sleep_for(one_sec);
        
        counter++;
    }
}


std::thread *thread_1;
std::thread *thread_2;


void setup() {
  Serial.begin(115200);
  //debug
  delay(1000);
  thread_1 = new std::thread(counter_loop);
  thread_2 = new std::thread(blinker_loop);
}


uint32_t main_loop_counter = 0;
void loop() {
    main_loop_counter++;
    Serial.println((String)"main loop: " + main_loop_counter);
    delay(5000);
}
