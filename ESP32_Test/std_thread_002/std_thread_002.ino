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
        Serial.print("counter_loop: ");
        Serial.println(counter);
        std::this_thread::sleep_for(one_sec);
        counter++;
    }
}

void blinker_loop() {
    thread_local uint32_t counter = 0;
    while(true) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.print("blinker_loop (HIGH) counter: ");
        Serial.println(counter);
        std::this_thread::sleep_for(one_sec);
        
        digitalWrite(LED_BUILTIN, LOW);
        Serial.print("blinker_loop (LOW) counter: ");
        Serial.println(counter);
        std::this_thread::sleep_for(one_sec);
        counter++;
    }
}


std::unique_ptr<std::thread>  counter_loop_thread;
std::unique_ptr<std::thread>  blinker_loop_thread;

void setup() {
  Serial.begin(115200);
  //debug
  delay(2000);
  
  counter_loop_thread.reset(new std::thread(counter_loop));
  blinker_loop_thread.reset(new std::thread(blinker_loop));
}


uint32_t main_loop_counter = 0;
void loop() {
    main_loop_counter++;
    Serial.print("main loop: ");
    Serial.println(main_loop_counter);
    delay(10000);
}
