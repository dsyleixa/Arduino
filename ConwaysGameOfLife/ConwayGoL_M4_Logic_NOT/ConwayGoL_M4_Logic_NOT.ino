// Conway's Game-Of-Life
// as a Turing machine
// Platform: Arduino

// Game of life
// http://en.wikipedia.org/wiki/Conway's_Game_of_Life
// Code adapted from
// http://cboard.cprogramming.com/c-programming/128982-simple-life-game-code-near-end.html
// http://www.rennard.org/alife/english/logicellgb.html#LCellFcnmt 
// (modified)
// ported to Arduino by dsyleixa


// MCU: Adafruit Feather M4

// supported display:
// Adafruit Featherwing TFT35
// and different Afadruit-GFX-compatible ones 
// (adjust libs+resolutions!)

//-----------------------------------------------------------------------
// Simulation: NOT Gate
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// new: enhanced version to opt. also invert output   
//-----------------------------------------------------------------------

int blockSize = 2;



#include <Arduino.h>
// i2c, SPI
#include <Wire.h>         // Incl I2C comm, but needed for not getting compile error
#include <SPI.h>


//-----------------------------------------------------------------------
// display driver
//-----------------------------------------------------------------------
#include <Adafruit_GFX.h>   // https://github.com/adafruit/Adafruit-GFX-Library 
// Adafruit TFT35 LED driver
#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>
 
// fonts
#include <Fonts/FreeSans9pt7b.h>             // optional
#include <Fonts/FreeMono12pt7b.h>            // optional  
#include <Fonts/FreeMono9pt7b.h>       // used here by default
//#include <Fonts/FreeMonoBold7pt7b.h>    // optional, custom font

// TFT pins
#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
 
#elif defined ESP32
   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14
 
#elif defined TEENSYDUINO
   #define TFT_DC   10
   #define TFT_CS   4
   #define STMPE_CS 3
   #define SD_CS    8
  
#elif defined ARDUINO_STM32_FEATHER
   #define TFT_DC   PB4
   #define TFT_CS   PA15
   #define STMPE_CS PC7
   #define SD_CS    PC5
  
#elif defined ARDUINO_FEATHER52
   #define STMPE_CS 30
   #define TFT_CS   13
   #define TFT_DC   11
   #define SD_CS    27

#elif  defined(ARDUINO_MAX32620FTHR) || defined(ARDUINO_MAX32630FTHR)
   #define TFT_DC   P5_4
   #define TFT_CS   P5_3
   #define STMPE_CS P3_3
   #define SD_CS    P3_2

// Something else!
#elif  defined (__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined (__AVR_ATmega328P__) || defined(ARDUINO_SAMD_ZERO) || defined(__SAMD51__)   
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5

 // default 
#else // to be adjusted
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
#endif

#define TFT_RST -1

// display instance
Adafruit_HX8357    display = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610  ts = Adafruit_STMPE610(STMPE_CS);


// color defs 16bit

#define  WHITE       0xFFFF  ///< 255, 255, 255
#define  LIGHTGRAY   0xC618  ///< 198, 195, 198
#define  GRAY        0x7BEF  ///< 123, 125, 123
#define  DARKGRAY    0x39E7  ///<  63,  63,  63  
#define  BLACK       0x0000  ///<   0,   0,   0
#define  RED         0xF800  ///< 255,   0,   0
#define  MAROON      0x7800  ///< 123,   0,   0
#define  ORANGE      0xFD20  ///< 255, 165,   0
#define  YELLOW      0xFFE0  ///< 255, 255,   0
#define  GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define  LIME        0x07E0  ///<   0, 255,   0
#define  GREEN       0x03E0  ///<   0, 125,   0  
#define  DARKGREEN   0x01E0  ///<   0,  63,   0   
#define  CYAN        0x07FF  ///<   0, 255, 255
#define  DARKCYAN    0x03EF  ///<   0, 125, 123
#define  OLIVE       0x7BE0  ///< 123, 125,   0
#define  BLUE        0x001F  ///<   0,   0, 255
#define  NAVY        0x000F  ///<   0,   0, 123
#define  PINK        0xFC18  ///< 255, 130, 198
#define  MAGENTA     0xF81F  ///< 255,   0, 255
#define  PURPLE      0x780F  ///< 123,   0, 123


#define  COLOR_BKGR  BLACK

int tftheight, 
    tftwidth;

//---------------------------------------------------------------------------
// preferences and settings
//---------------------------------------------------------------------------

// The size of the GoL screen window

const int GOLwindowWidth = 420;   // <~~~~~~~~~~~~ adjust GOL window dimensions !
const int GOLwindowHeight= 190;
const int frame = 10;


//enlarge on either side to create an invisible border of dead cells

int yvisrows = (GOLwindowHeight / blockSize);
int xviscols = (GOLwindowWidth / blockSize);

int yrows = yvisrows + 2*frame;
int xcols = xviscols + 2*frame;

#define centeryrow (yrows/2)
#define centerxcol (xcols/2)

// two boards, one for the current generation and one for calculating the next one
char board[GOLwindowHeight + 2*frame][GOLwindowWidth + 2*frame];
char tmpboard[GOLwindowHeight + 2*frame][GOLwindowWidth + 2*frame];


//---------------------------------------------------------------------------
// GoL functions
//---------------------------------------------------------------------------

uint32_t GenerationCnt=1;
    
// Count neighbours
int countNeighbours(int yrow, int xcol)
{
  int count = 0;
  for (int x = -1; x <= +1; x++)  {
    for (int y = -1; y <= +1; y++) {
      if ((board[yrow + y][xcol + x] == 1) && (x != 0 || y != 0))
        count++;
    }
  }
  return count;
}


//---------------------------------------------------------------------------
// Calculate the cells that will live and die for the next generation
void calculateGeneration()
{
  int aliveNeighbours = 0;

  // Clear the board for the next generation
  memset(tmpboard, 0, sizeof(tmpboard));

  for (int yrow = 1; yrow < (yrows-1); yrow++)  {
    for (int xcol = 1; xcol < (xcols-1); xcol++)   {
      aliveNeighbours = countNeighbours(yrow, xcol);

      // a live cell with < 2 live neighbours dies, as if caused by under-population.
      if( board[yrow][xcol]==1 && aliveNeighbours < 2 )
        tmpboard[yrow][xcol] = 0;

      // a live cell with 2 or 3 live neighbours lives on to the next Generation
      if ( board[yrow][xcol]==1 && aliveNeighbours >= 2  &&  aliveNeighbours <= 3 )
        tmpboard[yrow][xcol] = board[yrow][xcol];

      // a dead cell with exactly 3 live neighbours becomes alive, as if by reproduction
      if( board[yrow][xcol]==0 && aliveNeighbours == 3 )
        tmpboard[yrow][xcol] = 1;

      // a live cell with > 3 live neighbours dies, as if by overcrowding
      if( board[yrow][xcol]==1 && aliveNeighbours > 3 )
        tmpboard[yrow][xcol] = 0;
    }
  }
  // Copy the new board to the old one
  memcpy(board, tmpboard, sizeof(tmpboard));
}


//---------------------------------------------------------------------------
// Draw all the cells
//---------------------------------------------------------------------------

void drawBoard()
{
  // Wipe the GOLwindow
  display.fillRect(1, 1, GOLwindowWidth+1, GOLwindowHeight+1, COLOR_BKGR);
    
  for (int yrow=frame; yrow <(yrows-frame); yrow++) { 
    for (int xcol=frame; xcol<(xcols-frame); xcol++)  {
      // Draw all the "live" cells.
      if (board[yrow][xcol])
        display.fillRect((xcol-frame+1)*blockSize, (yrow-frame+1)*blockSize, 
                          blockSize, blockSize, WHITE);
    }
  }
  //display.display();
}


//---------------------------------------------------------------------------
// patterns
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void poke_Glider(int starty, int startx) {    //  

  int x,y;

  char sprite[3][3] = {  //  
  {0,1,0,},
  {0,0,1,},
  {1,1,1,}
  } ;
  
  for(x=0; x<3; ++x) {
    for(y=0; y<3; ++y) {
      board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }
}

//---------------------------------------------------------------------------
void poke_GliderUp(int starty, int startx) {    //  

  int x,y;

  char sprite[3][3] = {  //  
  {0,1,1,},
  {1,0,1,},
  {0,0,1,}
  } ;
  
  for(x=0; x<3; ++x) {
    for(y=0; y<3; ++y) {
      board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }
}


//---------------------------------------------------------------------------
void poke_GliderGun(int starty, int startx) {  // Gosper Glider Gun, period=30

  int x,y;

  char sprite[9][37] = {  //  
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
  {0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
  } ;
  
  for(x=0; x<37; ++x) {    
    for(y=0; y<9; ++y) {
    
      board[starty+frame+y][startx+frame+x] = sprite[y][x] ;
    }
  }
}


//---------------------------------------------------------------------------
void poke_GliderGunRev(int starty, int startx) {  // Gosper Glider Gun, period=30

  int x,y;

  char sprite[9][37] = {  //  
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
  {0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
  } ;
  
  for(x=0; x<37; ++x) {    
    for(y=0; y<9; ++y) {
    
      board[starty+frame+y][startx+frame+x] = sprite[y][37-x] ;
    }
  }
}

//---------------------------------------------------------------------------
void poke_GliderEater(int starty, int startx, int V) { // V=input; 0=stable, 1=vanish
  int x,y;

  char sprite[6][6] = {  //  
  {0 ,0 ,0 ,0 ,0 ,0 },     
  {0 ,1 ,1 ,0 ,0 ,0 },  
  {0 ,1 ,0 ,0 ,0 ,0 },
  {0 ,V ,1 ,1 ,1 ,0 },   // <~~~ V=input
  {0 ,0 ,0 ,0 ,1 ,0 },
  {0 ,0 ,0 ,0 ,0 ,0 }, 
  } ;

  for(x=0; x<6; ++x) {
    for(y=0; y<6; ++y) {      
       board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }  
}

//---------------------------------------------------------------------------
void poke_GliderEaterRev(int starty, int startx, int V) {  // V=input; 0=stable, 1=vanish
  int x,y;

  char sprite[6][6] = {  //  
  {0 ,0 ,0 ,0 ,0 ,0 },     
  {0 ,0 ,0 ,1 ,1 ,0 },  
  {0 ,0 ,0 ,0 ,1 ,0 },
  {0 ,1 ,1 ,1 ,V ,0 },   // <~~~ V=input 
  {0 ,1 ,0 ,0 ,0 ,0 },
  {0 ,0 ,0 ,0 ,0 ,0 }, 
  } ;

  for(x=0; x<6; ++x) {
    for(y=0; y<6; ++y) {      
       board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }  
}

//---------------------------------------------------------------------------
bool peek_GliderEater(int starty, int startx, int ID) {
   
   bool isdestroyed= !board[starty+frame+0][startx+frame+0]                
                  &&  board[starty+frame+0][startx+frame+2]                  
                  && !board[starty+frame+1][startx+frame+0]
                  &&  board[starty+frame+1][startx+frame+2]
                  &&  board[starty+frame+2][startx+frame+1]
                  && !board[starty+frame+2][startx+frame+2];  

   //debug       
   //if(isdestroyed) { Serial.println( "ID " +(String)ID + " Input=FALSE =>  OUT=FALSE " + " gen=" +(String)GenerationCnt); }
   
   return isdestroyed;
}

//---------------------------------------------------------------------------
bool peek_GliderEaterRev(int starty, int startx, int ID) {
   
   bool isdestroyed= !board[starty+frame+0][startx+frame+5-0]                
                  &&  board[starty+frame+0][startx+frame+5-2]                  
                  && !board[starty+frame+1][startx+frame+5-0]
                  &&  board[starty+frame+1][startx+frame+5-2]
                  &&  board[starty+frame+2][startx+frame+5-1]
                  && !board[starty+frame+2][startx+frame+5-2];  
      
   return isdestroyed;
}

//---------------------------------------------------------------------------
bool InputA, InputB;

int  Eatercx, Eatercy,                    // CLOCK
     Eater0x, Eater0y,                    // Inverter
     Eater1x, Eater1y, Eater2x, Eater2y,  // A, B
     Eater3x, Eater3y ;                   // GATE

bool ClockTick=0;


//---------------------------------------------------------------------------
void labelBoard() {
     display.setFont(&FreeMono9pt7b);
     display.setTextColor(LIME);  display.setCursor(34,19*blockSize); 
     display.print("A"); //   
     display.setTextColor(RED); display.setCursor(34+2*blockSize*39,19*blockSize); 
     display.print("GUN");
          
     display.setTextColor(YELLOW); display.setCursor(28*blockSize, 80*blockSize ); 
     display.print("GATE");
     display.setTextColor(BLUE); display.setCursor(34+4*blockSize*39,19*blockSize); 
     display.print("CLK");
}


//---------------------------------------------------------------------------
void ResetGate() {
  
  int yc= 1, xc=(37+2)*4,    // CLOCK
      y1= 1, x1=0,           // A => => GATE
      y2= 1, x2=1+(37+2)*1,  //  N/A
      y3= 1, x3=1+(37+2)*2,  // GUN invert A
      y4= 1, x4=(37+2)*3 ;   //  N/A  
      
  int deltaXY;  

  memset(board, 0, sizeof(board));
  memset(tmpboard, 0, sizeof(tmpboard));
  display.fillRect(1, 1, GOLwindowWidth+1, GOLwindowHeight+1, COLOR_BKGR);

  
  // --------------------------------- 
  poke_GliderGun( y1, x1 );    // Input A 
  // Gun Eater 1
  deltaXY=4;
  Eater1y = 9 +y1 +deltaXY;
  Eater1x =23 +x1 +deltaXY;  
  // input 0: GliderEater active => output 0 (blocked)  
  InputA=0;  
  poke_GliderEater( Eater1y, Eater1x, InputA);


  // ---------------------------------
  poke_GliderGunRev( y3, x3 );    // GUN invert A => GATE
  // Gun Eater 3 = gate detector
  deltaXY=37+20;
  Eater3y =  9     +y3 +deltaXY;
  Eater3x = 23-14  +x3 -deltaXY;
  // 0: GliderEater always solid (active)  
  poke_GliderEaterRev( Eater3y, Eater3x, 0);      // 


  // ---------------------------------
  poke_GliderGunRev( yc, xc );      // CLOCK: duplicate of GATE, different x/y offsets
  // Gun Eater 0 = clock detector
  deltaXY=37+20;
  Eatercy =  9     +yc +deltaXY;
  Eatercx = 23-14  +xc -deltaXY;
  // 0: GliderEater solid (active)  
  poke_GliderEaterRev( Eatercy, Eatercx, 0);
  
     
  // ---------------------------------
  drawBoard();
  labelBoard();
  
  delay(1);  
}


//---------------------------------------------------------------------------
// setup
//---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(3000); // wait for Serial()
  Serial.println("Serial started");
  
  // Start Wire (SDA, SCL)
  Wire.begin();

  // TFT
  display.begin(HX8357D);    
  display.setRotation(3);
  tftheight=display.height(); 
  tftwidth =display.width();
  display.fillScreen(COLOR_BKGR);  // Clear Screen

  // text display tests
  display.setTextSize(1);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE);
  display.setCursor(0,20);
  display.println("This is ");  
  display.println("Conway's Game of Live");  
  display.setCursor(0,80);
  display.print("xcols="); display.println(xviscols);
  display.print("yrows="); display.println(yvisrows);  
  //display.display();
  delay(1000);
  
  srand(analogRead(A0)+millis() );  
  display.drawRect(0, 0, GOLwindowWidth+3, GOLwindowHeight+3, WHITE);

  ResetGate();   
  
}



//---------------------------------------------------------------------------
// loop
//---------------------------------------------------------------------------
void loop()
{   
    int ID;    
    static int8_t  GATE=-1;  // init as -1=undefined; 1=true, 0=false;
    static uint32_t ticker=0, period=0, counter=0;

    display.setFont(&FreeMono12pt7b);
    display.setCursor(0, display.height()-20);
    display.setTextColor(YELLOW);    
    display.print("NOT");   
        
    calculateGeneration();
    drawBoard();  
    labelBoard();    
    
     
    // Input Simulation

    if(counter>=800) counter=0;
    
    if(counter==0)   { 
      ResetGate();
      GATE=-1;
      ticker=0; 
      InputA=0;    
      poke_GliderEater( Eater1y, Eater1x, InputA); 
      Serial.println();
      Serial.println("processing...  A="+(String)InputA ); 
      display.fillRect(220, tftheight-34, tftwidth-220, 34, COLOR_BKGR); 
            display.setCursor(220, tftheight-20);
            display.print("  A="+(String)InputA + " processing...");             
    } 
    

    if(counter==400)   { 
      ResetGate();
      GATE=-1;
      ticker=0; 
      InputA=1;   
      poke_GliderEater( Eater1y, Eater1x, InputA); 
      Serial.println();
      Serial.println("processing...  A="+(String)InputA );          
      display.fillRect(220, tftheight-34, tftwidth-220, 34, COLOR_BKGR); 
            display.setCursor(220, tftheight-20);
            display.print("  A="+(String)InputA + " processing...");        
    } 
    
 
    display.setTextColor(WHITE);
    //---------------------------------
    // debug: only for screen messages, not for functionality    
    // outcomment optionally:    
       
        // get clock signal
        ID=-1;  ClockTick= peek_GliderEaterRev(Eatercy, Eatercx, ID); // Eater no.-1 (c=CLOCK)  

        // show A(1) + B(2) gun eater states      
        ID=1;   peek_GliderEater(Eater1y, Eater1x, ID); // Eater 1 (A)    

        if(ClockTick) {   
           GATE= peek_GliderEaterRev(Eater3y, Eater3x, ID); // Eater 0 (GATE)    
        }
       
        // show current gate states on screen
        display.setFont(&FreeMono12pt7b);
        display.setTextColor(WHITE); 
        display.setCursor(80, display.height()-20);
        display.fillRect(80, tftheight-34, 140, 34, COLOR_BKGR);  
        display.print(GenerationCnt); 
        
        if(ClockTick) {
            Serial.println("  A="+(String)InputA    
                + " => GATE=" + (String)GATE + "     count="+(String)counter);
                
            display.fillRect(220, tftheight-34, tftwidth-220, 34, COLOR_BKGR); 
            display.setCursor(220, tftheight-20);
            display.setTextColor(YELLOW);
            display.print("A="+(String)InputA    
                + " => GATE=" + (String)GATE);
        }
     
    // end debug
    //---------------------------------     
        
    counter++;
    ticker++;      
    GenerationCnt++; 
    
    delay(1);  // <~~~~~~~~~~ adjust for slow motion
}



//--------------------------------- 
// EOF
//---------------------------------
