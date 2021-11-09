// std::thread for ESP32, Arduino IDE

// ver 0.0.7 fibonacci, GPIO, blink, sort und main counter

#include <Arduino.h>
#include <thread>
#include <freertos/task.h>
#include <esp_task.h>

volatile static bool THREADRUN = true;


//--------------------------------------------------------------------
void loop2() {
   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio   
   
   while(THREADRUN) {      
      // do stuff
   }
}

//--------------------------------------------------------------------

void loop1() {
   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio   
   
   while(THREADRUN) {      
      // do stuff
   }
}

//--------------------------------------------------------------------

void loop0() {
   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio 
   
   while(THREADRUN) {      
      // do stuff
   }
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
void setup() {
   Serial.begin(115200);

   std::thread thread_0 (loop0);
   std::thread thread_1 (loop1);
   std::thread thread_2 (loop2);
   
   thread_0.join();
   thread_1.join();
   thread_2.join();
   
   Serial.println((String)"\n all threads joined, program terminated\n");  // <<<<<<  
}


//--------------------------------------------------------------------
void loop() { // empty placeholder for Arduino compatibility
}
