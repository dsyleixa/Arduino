// Button Press Object
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose only by personal written permission 
// by the author.

// history
// 0.0.9 private + to do
// 0.0.8 new level 31 & debounce premature 2nd click
// 0.0.7 PCF + MCP pin-read (outline) 
// 0.0.6 adjustable long press
// 0.0.5 adjustable dbl click speed, block press limit
// 0.0.4 constant long press intervall
// 0.0.3 long press= 6*short press

// ButtonClass.h
// ver 0.0.9


#ifndef __BTNCLASS__
#define __BTNCLASS__ 

#include <Arduino.h>
                                                // to do...
#define PCF_INPUT          8574
#define PCF_INPUT_PULLUP  -8574
#define MCP_INPUT         23017
#define MCP_INPUT_PULLUP -23017
#define ANALOG_HIGH         900
#define ANALOG_LOW          100
#define ANALOG_50            50
#define ANALOG_100          100
#define ANALOG_200          200
#define ANALOG_300          300
#define ANALOG_400          400
#define ANALOG_500          500
#define ANALOG_600          600
#define ANALOG_700          700
#define ANALOG_800          800
#define ANALOG_900          900
#define ANALOG_1000        1000



class tButton {
  
  //---------------------------------------------------------- 
  private:     
    
    int16_t  pin;
    int32_t  mode;
    uint32_t aktMillis, aktMillis2;
    int8_t   level, 
             dnstate, dnstate2, upstate, 
             btnstate, oldbtnstate;
    uint32_t MINPRESSms, DBLCLICKms, LONGPRESSms, BLOCKPRESSms, DEBOUNCEms;


    //------------------------------------- 
    int8_t dtimer() {
      if (millis()-aktMillis >= LONGPRESSms) { // long press 400ms
          return 3;
      }
      else
      if (millis()-aktMillis >= MINPRESSms) {   
          return -1;
      }
      else
      return 0;
    }

    
    //-------------------------------------
    int8_t dtimer2() {
      if (millis()-aktMillis2 >= BLOCKPRESSms) { // block press limit
          return 2;
      }
      if (millis()-aktMillis2 >= DBLCLICKms) { // double click limit
          return 1;
      }
      //else
      return 0;
    }


    //-------------------------------------
    int8_t readButton(int16_t _pin, int32_t _mode) {
      if(_mode==INPUT) return digitalRead(_pin);
      else
      if(_mode==INPUT_PULLUP) return !digitalRead(_pin);
      else                                                 // to do...
      if(_mode==ANALOG_HIGH) return analogRead(_pin)>900;
      else
      if(_mode==ANALOG_LOW)  return analogRead(_pin)<100;
      else
      if(_mode==ANALOG_50)  return inRange(analogRead(_pin),50);
      else
      if(_mode==ANALOG_100) return inRange(analogRead(_pin),100);
      else
      if(_mode==ANALOG_200) return inRange(analogRead(_pin),200);
      else
      if(_mode==ANALOG_400) return inRange(analogRead(_pin),400);
      else
      if(_mode==ANALOG_800) return inRange(analogRead(_pin),800);


      
      /*
      else                                                // to do...
      if(_mode==PCF_INPUT) return !pcfRead(_pin);
      else
      if(_mode==PCF_INPUT_PULLUP) return !pcfRead(_pin);
      else
      if(_mode==MCP_INPUT) return !mcpRead(_pin);
      else
      if(_mode==MCP_INPUT_PULLUP) return !mcpRead(_pin);
      */

    } 


 //----------------------------------------------------------
 public:       

    tButton () : 
      pin(0xFF), aktMillis(0), aktMillis2(0), 
      MINPRESSms(40),     // min duration for either button press
      DBLCLICKms(150),    // max priod between double click actions
      LONGPRESSms(300),   // min duration for long press 
      BLOCKPRESSms(200),  // min wait duration after completed actions
      DEBOUNCEms(20),     // debounce time after btn press (idle, ignore) 
      mode(INPUT_PULLUP),
      level(0), dnstate(0), dnstate2(0), upstate(0), 
      oldbtnstate(0), btnstate(0)  
      { }
   
    ~tButton () { }
    

    //-------------------------------------
    void init(int16_t _pin, int32_t _mode, uint32_t _minpressms=40) {      
      pin  = _pin;    
      mode = _mode;  
      MINPRESSms = _minpressms;
      
      if(mode==INPUT || mode==INPUT_PULLUP)         // Button at dig GPIO
      {  
         pinMode(pin, mode);
      }
      else 
      if(mode==PCF_INPUT || mode==PCF_INPUT_PULLUP) // Button at PCF8574
      { 
         // dummy     
      }
      else 
      if(mode==MCP_INPUT || mode==MCP_INPUT_PULLUP) // Button at MCP23017
      { 
         // dummy     
      }

    }


    //-------------------------------------
    bool inRange(int val, int ref, int range=10 ) {
      return( (val>ref-range)&&(val<ref+range) );
    }


    //-------------------------------------
    void setclickdurations( uint32_t _minpressms,
                            uint32_t _dblclickms,
                            uint32_t _longpressms,
                            uint32_t _blockpressms ) 
    {            
      MINPRESSms =   _minpressms;
      DBLCLICKms =   _dblclickms; 
      LONGPRESSms =  _longpressms;
      BLOCKPRESSms = _blockpressms;
    }


    //-------------------------------------
    int8_t click() { // returns 1(single), 2(double), 3(long), or 0(no press)

      btnstate=readButton(pin, mode); 

      if(level==0) {     
         dnstate=0;       
         if(!oldbtnstate && btnstate) { //  1st new btn down:
           aktMillis=millis();          //  restart btn down timer 
           level=1;           
           return 0;
         }  
      }

      if(level==1) {                           // either btn stroke happened        
         //Serial.println("level1");    Serial.println(dnstate);

         if(millis()-aktMillis <= DEBOUNCEms) { // debounce
             return 0;
         }

         dnstate= dtimer();             // -1=short, 3=long pess 
         
         if(!btnstate){                 // 1st btn up                          
           if(dnstate){                 // -1=short, 3=long pess

             aktMillis2=millis();             
             if(dnstate==3) {           // long press: finished !!    
                btnstate=0;
                oldbtnstate=0;
                dnstate=0;
                aktMillis=millis();
                aktMillis2=millis();
                upstate=0;
                level=4;            // extra wait after long press
                return 3;
            }
            else level=2;               // short press: next level
           }  
         }  
      }

      if(level==2) {                           // short press happened         
         //Serial.println("level2");  Serial.println(dnstate);

         upstate=dtimer2();             // check btn up pause 

         btnstate=readButton(pin, mode);          
         
         //Serial.print("upstate="); Serial.println(upstate);     
             
         if(btnstate) {           // second btn click during pause: double click! 
            dnstate2=1;
            //Serial.print(" dnstate2="); Serial.println(dnstate2); 
         }
         
         
         if(upstate==0 && dnstate2)   { // if double click: next level
            level=3;
         }
         else         
         if(upstate>=1) {                           // dbl click time passed:             
            //Serial.println(millis()-aktMillis2);   // single press finished !!
            dnstate=0; 
            dnstate2=0;
            //Serial.println(dnstate);
            btnstate=0;
            oldbtnstate=0;
            level=4;
            aktMillis=millis();   
            aktMillis2=millis(); 
            upstate=0;
            level=4;                  // extra wait after single press
            return 1;  
          }
      }

      if(level==3) {                       // double click
         if (btnstate) {     
            btnstate=readButton(pin, mode); 
            if (btnstate) level=31;    // non-blocking while still pressed 
            return 0;                                 
         }
         if (!btnstate) {              
            //Serial.println("level3");   
            dnstate=0;                   // double click finished !!
            dnstate2=0;
            upstate=0;
            //Serial.println(dnstate);
            oldbtnstate=0;              
            aktMillis=millis();  
            aktMillis2=millis(); 
            level=4;                // extra wait after double click
            return 2;    
         }         
      }

      if(level==4) {                // BlockPress  wait routine
         upstate=dtimer2();
         aktMillis=millis();  
         if(upstate>=2) {
             level=0;
             dnstate=0;     
             upstate=0;
             aktMillis=millis();   
             aktMillis2=millis();
             //Serial.println("level4 extra wait finished");
         }
      }


      if(level==31) {    // double click, still pressed
         btnstate=readButton(pin, mode);
         if (!btnstate) {            
            level=3;
            return 0; 
         } 
         oldbtnstate=btnstate;
         return 0; 
      } 

      oldbtnstate=btnstate;
      return 0;                
    };    

    //------------------------------------- 
    int8_t state() { // returns 1(single), 2(double), 3(long), or 0(no press)
         return  click();   // alias       
    }

};



//----------------------------------------------------------------

#endif
