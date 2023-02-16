// std::thread for ESP32, Arduino IDE

// ver 0.0.7 fibonacci, GPIO, blink, sort und main counter
// backup from github


#include <Arduino.h>
#include <thread>
#include <freertos/task.h>
#include <esp_task.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

volatile static bool THREADRUN = true;

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
#define tpin1  11  // GPIO test pins digitalWrite
#define tpin2  12  // GPIO test pins digitalWrite
#define tpin3  10  // GPIO test pins digitalRead

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
   Serial.println((String)"blinker_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");

   while(THREADRUN) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println((String)"\nblinker_loop (HIGH) counter: "+ counter+"\n");
      //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      delay(1000);

      digitalWrite(LED_BUILTIN, LOW);
      Serial.println((String)"\nblinker_loop (LOW) counter: "+ counter+"\n");
      //std::this_thread::sleep_for(std::chrono::milliseconds(500));
      delay(500);

      counter++;
      if(!THREADRUN) {
         Serial.println((String)"blink_loop terminated by THREADRUN");
      }
   }
}

//--------------------------------------------------------------------
void fibonacci_loop() {
   thread_local uint32_t counter = 0, i=0;

   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio
   Serial.println((String)"fibonacci_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");

   while(THREADRUN) {
      for(i=25; i<41; i++) {    // limits: test, debug
         uint32_t f = fibonacci(i);
         Serial.println( (String)"\nfibonacci of "+i+"="+ f +"\n");
         if(!THREADRUN) {
            Serial.println((String)"fibonacci_loop terminated by THREADRUN");
            break;
         }
      }
      counter++;
      Serial.println((String)"\nfibonacci_loop counter: "+counter+"\n");
   }
}

//--------------------------------------------------------------------
void sort_loop() {
   static int array[1000];
   thread_local uint32_t counter = 0, i=0;

   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio
   Serial.println((String)"sort_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");

   while(THREADRUN) {
      Serial.println((String)"\nsort_loop counter: "+counter+"\n");

      for(i=0; i<1000; i++) {
         array[i]= random(0,65000);
      }
      shellsort(1000, array);
      for(i=0; i<1000; i+=50) { // abgek�rzte Ausgabe  da Serial sehr langsam
         Serial.println((String)i + "="+array[i]+"\n");
      }
      counter++;
      if(!THREADRUN) {
         Serial.println((String)"sort_loop terminated by THREADRUN");
      }
   }
}

//--------------------------------------------------------------------
void GPIO_loop() {

   thread_local uint32_t counter = 0, timerms;
   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio

   Serial.println((String)"GPIO_loop Current priority :" + uxTaskPriorityGet(NULL)+"\n");
   while(THREADRUN) {
      Serial.println((String)"\nGPIO_loop counter: "+counter);
      timerms = millis();
      test_GPIO();
      timerms = millis() - timerms;
      Serial.println((String)"\nGPIO test timerms=" + (String)timerms + "\n");
      counter++;
      if(!THREADRUN) {
         Serial.println((String)"GPIO_loop terminated by THREADRUN");
      }
   }
}

//--------------------------------------------------------------------

void control_loop() {
   thread_local  uint32_t loop_counter = 0;

   vTaskPrioritySet( NULL, ESP_TASK_MAIN_PRIO ); // set Priority = main prio

   while(THREADRUN) {
      Serial.println((String)"\nloop counter: " + loop_counter);
      delay(500);
      loop_counter++;
      if(loop_counter>40) {
         Serial.println("loop counter limit reached!  all threads terminate!");
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


   std::thread thread_0 (control_loop);
   std::thread thread_1 (blinker_loop);
   std::thread thread_2 (fibonacci_loop);
   std::thread thread_3 (sort_loop);
   std::thread thread_4 (GPIO_loop);



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
