// std::thread for ESP32, Arduino IDE

// ver 0.0.7 fibonacci, GPIO, blink, sort und main counter
// ver 0.0.8 dto, using atomic plus mutex



#include <Arduino.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <freertos/task.h>
#include <esp_task.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif


using namespace std;

mutex         serial_mutex;
atomic<bool>  THREADRUN(true); 

uint32_t fibonacci(int n) {
   if(n == 0) {
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

//--------------------------------------------------------------------
#define tpin1  11  // GPIO test pin digitalWrite
#define tpin2  12  // GPIO test pin digitalWrite
#define tpin3  10  // GPIO test pin digitalRead

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


//--------------------------------------------------------------------
void blinker_loop() {
   thread_local uint32_t counter = 0;

   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio   
   serial_mutex.lock();
   Serial.println((String)"blinker_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");
   serial_mutex.unlock();
   while(THREADRUN) {
      digitalWrite(LED_BUILTIN, HIGH);
      serial_mutex.lock();
      Serial.println((String)"\nblinker_loop (HIGH) counter: "+ counter+"\n");
      serial_mutex.unlock();
      //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      delay(1000);

      digitalWrite(LED_BUILTIN, LOW);
      serial_mutex.lock();
      Serial.println((String)"\nblinker_loop (LOW) counter: "+ counter+"\n");
      serial_mutex.unlock();
      //std::this_thread::sleep_for(std::chrono::milliseconds(500));
      delay(500);

      counter++;
      if(!THREADRUN) {
         serial_mutex.lock();
         Serial.println((String)"blink_loop terminated by THREADRUN");
         serial_mutex.unlock();
      }
   }
}

//--------------------------------------------------------------------
void fibonacci_loop() {
   thread_local uint32_t counter = 0, i=0;

   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio   
   serial_mutex.lock();
   Serial.println((String)"fibonacci_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");
   serial_mutex.unlock();
   while(THREADRUN) {
      for(i=25; i<41; i++) {    // limits: test, debug
         if(!THREADRUN) {
            serial_mutex.lock();
            Serial.println((String)"fibonacci_loop terminated by THREADRUN");
            serial_mutex.unlock();
            break;
         }
         serial_mutex.lock();
         Serial.println( (String)"\nfibonacci of "+i+"="+fibonacci(i)+"\n");
         serial_mutex.unlock();
      }
      counter++;
      serial_mutex.lock();
      Serial.println((String)"\nfibonacci_loop counter: "+counter+"\n");
      serial_mutex.unlock();
   }
}

//--------------------------------------------------------------------
void sort_loop() {
   static int array[1000];
   thread_local uint32_t counter = 0, i=0;

   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio 
   serial_mutex.lock();  
   Serial.println((String)"sort_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");
   serial_mutex.unlock();
   while(THREADRUN) {
      serial_mutex.lock();
      Serial.println((String)"\nsort_loop counter: "+counter+"\n");
      serial_mutex.unlock();
      for(i=0; i<1000; i++) {
         array[i]= random(0,65000);
      }
      shellsort(1000, array);
      for(i=0; i<1000; i+=20) { // abgek�rzte Ausgabe  da Serial sehr langsam
         serial_mutex.lock();
         Serial.println((String)i + "="+array[i]+"\n");
         serial_mutex.unlock();
      }
      counter++;
      if(!THREADRUN) {
         serial_mutex.lock();
         Serial.println((String)"sort_loop terminated by THREADRUN");
         serial_mutex.unlock();
      }
   }
}

//--------------------------------------------------------------------
void GPIO_loop() {

   thread_local uint32_t counter = 0, timerms;
   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio

   serial_mutex.lock();
   Serial.println((String)"GPIO_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");
   serial_mutex.unlock();
   while(THREADRUN) {
      serial_mutex.lock();
      Serial.println((String)"\nGPIO_loop counter: "+counter);
      serial_mutex.unlock();
      timerms = millis();
      test_GPIO();
      timerms = millis() - timerms;
      serial_mutex.lock();
      Serial.println((String)"\nGPIO test timerms=" + (String)timerms + "\n");
      serial_mutex.unlock();
      counter++;
      if(!THREADRUN) {
         serial_mutex.lock();
         Serial.println((String)"GPIO_loop terminated by THREADRUN");
         serial_mutex.unlock();
      }
   }
}

//--------------------------------------------------------------------

void control_loop() {
    thread_local  uint32_t loop_counter = 0;

   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio
   
   while(THREADRUN) {      
      serial_mutex.lock();
      Serial.println((String)"\nloop counter: " + loop_counter);
      serial_mutex.unlock();
      delay(500);
      loop_counter++;
      if(loop_counter>40) {
         serial_mutex.lock();
         Serial.println("loop counter limit reached!  all threads terminate!");
         serial_mutex.unlock();
         THREADRUN=false; // after 20sec
      }
   }
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
void setup() {
   Serial.begin(115200);
   delay(1000);

   pinMode(LED_BUILTIN, OUTPUT);
   pinMode(tpin1, OUTPUT);
   pinMode(tpin2, OUTPUT);
   pinMode(tpin3, INPUT_PULLUP);


   thread thread_0 (control_loop);
   thread thread_1 (blinker_loop);
   thread thread_2 (fibonacci_loop);
   thread thread_3 (sort_loop);
   thread thread_4 (GPIO_loop);



   thread_0.join();
   thread_1.join();
   thread_2.join();
   thread_3.join();
   thread_4.join();

   Serial.println((String)"\nall threads joined, program terminated\n");  // <<<<<<  

}


//--------------------------------------------------------------------
void loop() { // empty placeholder for Arduino compatibility
}
