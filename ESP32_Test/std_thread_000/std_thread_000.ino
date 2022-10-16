// std::thread for ESP32, Arduino IDE

// thread loops can be exited by internal conditions (e.g. via break or return) 
// or all inner loops by setting THREADRUN=false at any location.

#include <Arduino.h>
#include <thread>
#include <freertos/task.h>
#include <esp_task.h>

volatile static bool THREADRUN = true;


//--------------------------------------------------------------------
void function2() {
   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio   
   Serial.println((String)"   function2 started"); 
   while(THREADRUN) {      
      // do stuff
   }
   Serial.println((String)"   function2 terminated"); 
}

//--------------------------------------------------------------------

void function1() {
   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio   
   Serial.println((String)"   function1 started"); 
   while(THREADRUN) {      
      // do stuff
   }
   Serial.println((String)"   function1 terminated"); 
}

//--------------------------------------------------------------------

void function0() {
   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio 
   Serial.println((String)"   function0 started"); 
   while(THREADRUN) {      
      // do stuff
   }
   Serial.println((String)"   function0 terminated"); 
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
void setup() {
   Serial.begin(115200);

   std::thread thread_0 (function0);
   std::thread thread_1 (function1);
   std::thread thread_2 (function2);
   
   Serial.println((String)"\n all threads being started, now waiting for all being terminated and to join main thread \n");  // <<<<<<  
   
   thread_0.join();
   thread_1.join();
   thread_2.join();
   
   Serial.println((String)"\n all threads joined, program terminated\n");  // <<<<<<  
}


//--------------------------------------------------------------------
void loop() { // empty placeholder for Arduino compatibility
}
