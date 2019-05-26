# Arduino



(C) 2018, 2019 by dsyleixa



MenuClass tMenu






// Graphic libraries: currently supported Adafruit-GFX-compatible API syntax




//instanciate 3 buttons for moving up, down, and enter:
// 3 buttons for menu control

   tButton btnUp;
   tButton btnDns;
   tButton btnEnter;

// init buttons in setup:

   btnUp.init(2, INPUT_PULLUP);     // <<< adjust gpio pin numbers!
   btnDn.init(3, INPUT_PULLUP);     // <<< adjust gpio pin numbers!
   btnEnter.init(4, INPUT_PULLUP);  // <<< adjust gpio pin numbers! 



// for either menu list, create a list of menu entries, e.g.:
   char * mlist1[6] = {"Line0","Line1"," menu02 >","Line3","Line4","Line5"}; 
// ~~~~~~~~~~~~~~^
   
// perhaps a 2nd one, called by 3rd line above:
   char * mlist2[4] = {"Line0"," Back <","Line2","Line3"}; 
// ~~~~~~~~~~~~~~^   


// pass either list to the menu object to construct it:

// 6 entries of the 1st list:
   
   tMenu menu1(6,   11,   (char**)mlist1,   &menu1,   1 ); // (if prev menu N/A, choose the same menu name)
// ~~~~~~~~~~~~^   ~~^   
// numEntries, lineLength: required 


// 4 entries of the 2nd list:
   
   tMenu menu2(4,   11,   (char**)mlist2,   &menu1,   2 ); // (prev menu name: in that case also menu1)
// ~~~~~~~~~~~~^   ~~^
// numEntries, lineLength: required



// (char**)listname: the list of the related menu entries

// preMenu: the predesessor menu by which this menu is called from 

//    (if no prev menu exists, choose the same menu name)

// menu-ID: arbitrary numbers for convenient access (recommended)


// create a pointer to the current active Menu for convenient access:
   tMenu * actMenu = &menu1;




// in loop():

// button maneuvers :
//===================

// button up code 1  short click:  move 1 line up
// button up code 2  double click: move 1 PgUp
// button up code 3  long press:   move to  top line

// button down code 1  short press:  move 1 line down
// button down code 2  double click: move 1 PgDn
// button down code 3  long press:   move to bottom line


// Enter button double click (code 2) returns the line number;
// calling checkbtn() retrieves this line number.

// Enter button long press (code 3) can tag/untag a line
// for further processing




// To call the menu function and see if lines have been selected and returned:
// pass all current button states to the menu object and read the line number if one is returned:

int16_t  ln = actMenu->checkbtn(btnUp.click(), btnDn.click(), btnEnter.click()); 


// read current menu ID:
    int32_t  ID = actMenu->ID;




// code what you have to do for either selected line
//---------------
// OPTION 1:
//--------------- 


 
   if(ln!=-1) {      // do when a line was selected and returned:
       
       sprintf(buf, "select: line=%d ID=%d  contents=%s", ln, ID, actMenu->list[ln]); 
       Serial.println(buf);
       
       if( ID==1 && ln==2) {  // goto next menu
         actMenu = &menu12;
       }
       else
       if( ID==1 && ln==0) {   
         // foo               // do anything
       }
       else
       if( ID==1 && ln==1) {   
         // bas               // do anything dfferent
       }
       
       
       else
       if( ID==2  && ln==0) {  // goto pre menu
         actMenu = actMenu->preMenu;
       }    
       else
       if( ID==2 && ln==1) {  
         // fab               // do anything else
       }

       actMenu->mdisplay();
    }   
    


//---------------
// OPTION 2:
//--------------- 

create for each menu a related Execute function ("e.g., "ExecList_n") :

char * ExecList1(int menuline){
  switch (menuline) {
     case  0:    /*...*/; break;  // insert cmd to do anything, e.g. digitalWrite(pin, HIGH/LOW);
     case  1:    /*...*/; break;  // insert cmd to do anything...
     case  2:    actMenu = &menu12; actMenu->mdisplay(); break;   // actMenu refresh for new menu ID
     case  3:    /*...*/; break;  // insert cmd to do anything... 
     case  4:    /*...*/; break;   
     case  5:    /*...*/; break;
  return "";              
}; 

char * ExecList12(int menuline){
  switch (menuline) {
     case  0:     actMenu = actMenu->preMenu; actMenu->mdisplay();   // actMenu refresh for new menu ID 
     case  1:    /*...*/; break;  // insert cmd to do anything...
     case  2:    /*...*/; break;
     case  3:    /*...*/; break;   
  }     
  return "";              
}; 


// then in loop():

   int line = actMenu->checkbtn(btnUp.click(), btnDown.click(), btnEnter.click() ); 
   int ID = actMenu->ID;
   if(line!=-1) {              // do when a line was selected and returned:
       // execute menu lines
       if( ID==1 )  { ExecList1(line);  }  // re menu line from mlist1      
       else
       if( ID==12)  { ExecList12(line); }  // re menu line from mlist12   
   } 




