another issue can be solved by std lib too: thread-safe r/w communication when using std::mutex.
e.g., if 1 thread is accessing i2c or Serial() and another parallel thread requires access too, or if different threads read or write to identical global variables or arrays, then simply the thread which came first may reserve access by mutexes, and when finished then simply frees it again: the timeslice scheduler will not muddle that up when switching to the other thread.
A simple ESP32 std::thread/mutex example how they did it:
```

// std::thread plus mutex for ESP32, Arduino IDE

#include <Arduino.h>
#include <thread>
#include <freertos/task.h>
#include <mutex>

const auto one_sec = std::chrono::seconds { 1 };

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

std::thread *thread_1;
std::thread *thread_2;

std::mutex print_mutex;


void mtxPrintln(String str) // String: Arduino API  // C++: std::string
{
    print_mutex.lock();
    Serial.println(str);
    print_mutex.unlock();
}

void loop1() {
    thread_local uint32_t counter = 0, i=0;  
    vTaskPrioritySet(NULL,0); //set Priority
    
    while(true) 
        mtxPrintln((String)"this is loop1, counter="+counter");
       counter++;
        // do stuff     
        delay(1300);
    }
}

void loop2() {
    thread_local uint32_t counter = 0, i=0;  
    vTaskPrioritySet(NULL,0); //set Priority
    
    while(true) {
        mtxPrintln((String)"this is loop2, counter="+counter);
        counter++;
        // do stuff     
        delay(700);     
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

```

