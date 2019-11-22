// MenuClass.h
// ver 0.0.5

// default TFT: Adafruit OLED SH1306 128x64
 
#ifndef _MENUCLASS_H
#define _MENUCLASS_H

// TFT Menu Object
// (C) 2018 by dsyleixa

// This example code is in the public domain for private use.
// Use for professional or business purpose only by personal written permission 
// by the author.

// 0.0.6  doubleClick(2): exec; longPress(3): tag
// 0.0.5  resetted to: tMenu menu entries as copies
// 0.0.4  tag + return selected line, parse lines
// 0.0.3  clean-up
// 0.0.2  tMenu menu entries as referencces
// 0.0.1  tMenu menu entries as copies


// MenuClass.h
// ver 0.0.6

// default TFT: OLED 128x64, compatible to Adafruit (R) Libs
 


#include <Arduino.h>


class tMenu {  
  
  private:     
       
     
  protected:  
     int16_t  MENULEN, LINELEN, VISLNUM, FONTHI, FONTWI;     
     int16_t  firstvln, lastvln, displn;  
     char     buf[20];      
     
  
  public:      
     char  ** list; 
     tMenu  * preMenu;
     int16_t  actln; 
     int32_t  ID;  
     int16_t  tagged; 
     
     

     
     tMenu (int16_t menulen, int16_t linelen,    // constructor
            char ** extlist,                     // entry lines
            tMenu* pMenu,                        // predecessor menu
            int id=0)                            // id optional for convenience!
     :            
     MENULEN(5), LINELEN(11), VISLNUM(5), 
     actln(0), ID(0), tagged(-1)  
     {
        firstvln=0; lastvln=0; displn=0;     
        actln=0;
        MENULEN = menulen;  // number of available menu options        
        LINELEN = linelen;  // line length of menu options       
        preMenu = pMenu;    // predesessor menu     
        ID      = id;       // optional: menu ID  

        FONTHI=display.height()/VISLNUM +1;
        FONTWI=(FONTHI*3/4) +1;
                
        //list = extlist;   
        list = new char*[MENULEN];       

        for (int i=0; i<MENULEN; i++) {   // adjust/parse           
           list[i] = new char[LINELEN];           
           strncpy(list[i], extlist[i], LINELEN-1);
           int len=strlen(extlist[i]);           
           if(len > LINELEN-1 ) {
              if (extlist[i][len-1] =='>') list[i][LINELEN-2] ='>';
              if (extlist[i][len-1] =='<') list[i][LINELEN-2] ='<';
              list[i][LINELEN-1]='\0';   
           }
        } 
        
     }       

     
     ~tMenu() { }
    

    
     void initscr(int16_t vislnum, uint8_t fonthi, uint8_t fontwi) 
     { 
        VISLNUM = vislnum;  // number of visible menu options
        FONTHI = fonthi; 
        FONTWI = fontwi;    
     }



     void IntToLine(int ival, int16_t line) 
     { 
        sprintf(list[line], "%d", ival);
        mdisplay();   
     }


     
     void FloatToLine(int fval, int16_t line, int prec=2) 
     { 
        dtostrf(fval, 2+prec, prec, list[line]);
        mdisplay();   
     }


     
     void mdisplay() {          
        if(actln>VISLNUM-1) firstvln=min((int)actln-1, (int)MENULEN-VISLNUM);
        else firstvln=0;       
        lastvln=min((int)firstvln+VISLNUM-1, (int)firstvln+MENULEN-1) ; 
        display.fillScreen(0x0000);
        
        for(int i=firstvln; i<=lastvln; i++) {   
            displn=(FONTHI-3) + (i-firstvln)*FONTHI;   
            display.setCursor(0, displn);           
            
            if(i!=actln && i==tagged) {   
              display.print('*');  Serial.print('*'); 
            }              
            else
            if(i==actln && i==tagged) {   
              display.print('#');  Serial.print('#'); 
            }              
            else                       
            if(i==actln && i!=tagged) {                
              display.print('>');  Serial.print('>');              
            }
            else {            
              display.print(' ');  Serial.print(' ');              
            }
            display.setCursor(FONTWI, displn);            
            display.print(list[i]);  Serial.println(list[i]);  
        }       
        if(firstvln>0) {
           display.setCursor(display.width()-FONTWI, 0+FONTHI-4); 
           display.print('|');  
           display.setCursor(display.width()-FONTWI, 0+FONTHI-4);  
           display.print('^');            
        }
        if(lastvln<MENULEN-1) {
           display.setCursor(display.width()-FONTWI, display.height()-1);           
           display.print('|');  
           display.setCursor(display.width()-FONTWI, display.height()+1);       
           display.print('v');  
          
        }

        // debug
        //
                          
        display.display();
        Serial.println();
     }
 
     int32_t checkbtn(int8_t btop, int8_t bbtm, int8_t bfunc) {
        int len=strlen(list[actln]);

        if(btop==1){ // dec
           if(actln>0) actln--;
           //Serial.print ("^"); Serial.println(actln);  // debug
           mdisplay();
        }
        if(btop==2){  // PgUp
           actln=max((int)actln-VISLNUM, 0);
           //Serial.print ("^"); Serial.println(actln);  // debug
           mdisplay();
        }
        if(btop==3){  // min
           actln=0;
           //Serial.print ("^"); Serial.println(actln);  // debug
           mdisplay();
        }
        if(bbtm==1){  // inc
           if(actln<MENULEN-1) actln++;
           //Serial.print ("v"); Serial.println(actln);  // debug
           mdisplay();
        }
        if(bbtm==2){  // PgDn
           actln=min((int)actln+VISLNUM, (int)MENULEN-1);
           //Serial.print ("v"); Serial.println(actln);  // debug
           mdisplay();
        }
        if(bbtm==3){  // max
           actln=MENULEN-1;
           //Serial.print ("v"); Serial.println(actln);  // debug  
           mdisplay();
        }
        if(bfunc==1){                
                // unused
        }
        
        if(bfunc==2){             // 2 = double clicked 0 => return actln  
           return actln; 
        }

        if(bfunc==3){             // 3 = long press = tag/untag 
           if(tagged==actln) {    // untag (if tagged currently) 
              tagged=-1;
           }           
           else tagged=actln;
           mdisplay();           
        }
        return -1;         
     }


}; 



//-----------------------------------


#endif 