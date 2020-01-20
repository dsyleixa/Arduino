// Arduino-Game-Of-Life

// Game of life
// http://en.wikipedia.org/wiki/Conway's_Game_of_Life
// Code adapted from
// http://cboard.cprogramming.com/c-programming/128982-simple-life-game-code-near-end.html 
// ported to Arduino by dsyleixa


// MCU: Adafruit Feather M4

// supported display:
// Adafruit Featherwing TFT35
// and different Afadruit-GFX-compatible ones 
// (adjust libs+resolutions!)


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
#else
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
#endif


#define TFT_RST -1

// display instance
Adafruit_HX8357    display = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610  ts = Adafruit_STMPE610(STMPE_CS);


// color defs

#define BLACK       HX8357_BLACK
#define DARKGREY    HX8357_DARKGREY 
#define WHITE       HX8357_WHITE
#define RED         HX8357_RED
#define YELLOW      HX8357_YELLOW
#define CYAN        HX8357_CYAN
#define DARKCYAN    HX8357_DARKCYAN
#define BLUE        HX8357_BLUE
#define GREENYELLOW HX8357_GREENYELLOW

#define COLOR_BKGR  BLACK

int tftheight, 
    tftwidth;

//---------------------------------------------------------------------------
// preferences and settings
//---------------------------------------------------------------------------
// The blocks are blockSize * blockSize big
// 2...6 seems to be a good value for this





// The size of the GoL screen window

const int screenWidth = 240;   // <~~~~~~~~~~~~ adjust screen dimensions !
const int screenHeight= 240;
const int frame = 10;




// Make the board larger on either side to ensure that there's an invisible border of dead cells

int yvisrows = (screenHeight / blockSize);
int xviscols = (screenWidth / blockSize);

int yrows = yvisrows + 2*frame;
int xcols = xviscols + 2*frame;

#define centeryrow (yrows/2)-1 
#define centerxcol (xcols/2)-1 

// two boards, one for the current generation and one for calculating the next one
char board[screenHeight + 2*frame][screenWidth + 2*frame];
char tmpboard[screenHeight + 2*frame][screenWidth + 2*frame];


//---------------------------------------------------------------------------
// GoL functions
//---------------------------------------------------------------------------

uint32_t GenerationCnt=1;

    
// Count thy neighbours
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

      // Any live cell with fewer than two live neighbours dies, as if caused by under-population.
      if(aliveNeighbours < 2)
        tmpboard[yrow][xcol] = 0;

      // Any live cell with two or three live neighbours lives on to the next calculateGeneration
      if (aliveNeighbours >= 2  &&  aliveNeighbours <= 3 )
        tmpboard[yrow][xcol] = board[yrow][xcol];

      // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction
      if(aliveNeighbours == 3 && board[yrow][xcol]==0)
        tmpboard[yrow][xcol] = 1;

      // Any live cell with more than three live neighbours dies, as if by overcyrowding
      if(aliveNeighbours > 3)
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
  // Wipe the screen
  display.fillRect(1, 1, screenWidth+1, screenHeight+1, COLOR_BKGR);
    
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

int Eater1x, Eater1y, Eater2x, Eater2y;


//---------------------------------------------------------------------------
// This adds some random live cells to the board
void put_randomBoard(int seedChance)
{
  for (int yrow = 1; yrow < (yrows - 1); yrow++)
  {
    for (int xcol = 1; xcol < (xcols - 1); xcol++)
    {
      board[yrow][xcol] = !(rand() % seedChance);
    }
  }
}


//---------------------------------------------------------------------------

void put_Blinker3x1(int starty, int startx) {       //  
 char sprite[1][5] = {  //  
    {1,1,1}
  } ;

  for(int x=0; x<3; ++x) {
       board[starty+frame][startx+frame+x]=sprite[0][x] ;
  }

  
}

//---------------------------------------------------------------------------
void put_Block2x2(int starty, int startx) {       //  

  char sprite[2][2] = {  //  
    {1,1},
    {1,1}
  } ; 

  for(int x=0; x<2; ++x) {
    for(int y=0; y<2; ++y) {
      board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  } 
}



//---------------------------------------------------------------------------
void put_Bar5x1(int starty, int startx) {       //  

  char sprite[1][5] = {  //  
    {1,1,1,1,1}
  } ;

  for(int x=0; x<5; ++x) {
       board[starty+frame][startx+frame+x]=sprite[0][x] ;
  }

  
}


//---------------------------------------------------------------------------
void put_Clock(int starty, int startx) {    //  
  int x,y;

  char sprite[4][4] = {  //  
    {0,0,1,0},
    {1,1,0,0},
    {0,0,1,1},
    {0,1,0,0},
  } ;

  for(x=0; x<4; ++x) {
    for(y=0; y<4; ++y) {
      board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }
}


//---------------------------------------------------------------------------

void put_F_Pentomino(int starty, int startx) {    //  == R-Pentomino
  int x,y;

  char sprite[3][3] = {  //  
  {0,1,1},
  {1,1,0},
  {0,1,0},
  } ;

  for(x=0; x<3; ++x) {
    for(y=0; y<3; ++y) {
      board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }
}


//---------------------------------------------------------------------------
void put_Pi_Heptomino(int starty, int startx) {    //  

  int x,y;

  char sprite[3][5] = {  //  
  {0,0,1,0,0},
  {0,1,0,1,0},
  {1,1,0,1,1}
  } ;

  for(x=0; x<5; ++x) {
    for(y=0; y<3; ++y) {
      board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }
}


//---------------------------------------------------------------------------
void put_23334M(int starty, int startx) {    //  
  int x,y;

  char sprite[8][5] = {  //  
  {0,0,1,0,0},
  {1,1,0,0,0},
  {0,1,0,0,0},
  {1,0,0,1,0},
  {0,0,0,0,1},
  {0,1,0,0,1},
  {0,0,1,0,1},
  {0,1,0,0,0},
  } ;

  for(x=0; x<5; ++x) {
    for(y=0; y<8; ++y) {
      board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }
}



//---------------------------------------------------------------------------
void put_Glider(int starty, int startx) {    //  

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
void put_GliderUp(int starty, int startx) {    //  

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
void put_LWSpaceship(int starty, int startx) {    //  

  int x,y;

  char sprite[4][5] = {  //  
  {1,0,0,1,0},
  {0,0,0,0,1},
  {1,0,0,0,1},
  {0,1,1,1,1},

  } ;

  for(x=0; x<5; ++x) {
    for(y=0; y<4; ++y) {
      board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }
}


//---------------------------------------------------------------------------
void put_HWSpaceship(int starty, int startx) {    // 

  int x,y;

  char sprite[5][7] = {  //  
  {0,0,1,1,0,0,0},
  {0,0,0,0,0,1,0},
  {0,0,0,0,0,0,1},
  {1,0,0,0,0,0,1},
  {0,1,1,1,1,1,1},

  } ;

  for(x=0; x<7; ++x) {
    for(y=0; y<5; ++y) {
      board[starty+frame+y][startx+frame+x]=sprite[y][x] ;
    }
  }
}


//---------------------------------------------------------------------------
void put_GliderGun(int starty, int startx) {  // Gosper Glider Gun, period=30

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
  
  for(x=0; x<37; ++x) {   // NXT screen (0,0) is bottom left, not top left  !
    for(y=0; y<9; ++y) {
    
      board[starty+frame+y][startx+frame+x] = sprite[y][x] ;
    }
  }
}


//---------------------------------------------------------------------------
void put_GliderGunRev(int starty, int startx) {  // Gosper Glider Gun, period=30

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
  
  for(x=0; x<37; ++x) {   // NXT screen (0,0) is bottom left, not top left  !
    for(y=0; y<9; ++y) {
    
      board[starty+frame+y][startx+frame+x] = sprite[y][37-x] ;
    }
  }
}



//---------------------------------------------------------------------------
void put_GliderEater(int starty, int startx, int V) {
  int x,y;

  char sprite[6][6] = {  //  
  {0 ,0 ,0 ,0 ,0 ,0 },    
  {0 ,1 ,1 ,0 ,0 ,0 },  
  {0 ,1 ,0 ,0 ,0 ,0 },
  {0 ,V ,1 ,1 ,1 ,0 },   // y+3, x+1
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
bool get_GliderEater(int starty, int startx, int ID) {
   
   bool isdestroyed= !board[starty+frame+0][startx+frame+0]                
                  &&  board[starty+frame+0][startx+frame+2]                  
                  && !board[starty+frame+1][startx+frame+0]
                  &&  board[starty+frame+1][startx+frame+2]
                  &&  board[starty+frame+2][startx+frame+1]
                  && !board[starty+frame+2][startx+frame+2];   
                   
   if(isdestroyed) {
      Serial.println( "TRUE:  Eater " +(String)ID 
                   + " generation=" +(String)GenerationCnt);
   }
   return isdestroyed;
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
  display.setRotation(1);
  tftheight=display.height(); 
  tftwidth =display.width();
  display.fillScreen(COLOR_BKGR);  // Clear Screen

  // text display tests
  display.setTextSize(1);
  display.setFont();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("This is ");  
  display.println("Conway's Game of Live");  
  display.setCursor(0,30);
  display.print("xcols="); display.println(xviscols);
  display.print("yrows="); display.println(yvisrows);
  
  //display.display();
  delay(1000);
  srand(analogRead(A0)+millis() );
  
  display.drawRect(0, 0, screenWidth+3, screenHeight+3, WHITE);

  // test
  // put_Glider(yvisrows-20, centerxcol);
  // put_GliderUp(10, centerxcol);
  
  int y1=10, x1=0, y2=10, x2=37+10;
  
  put_GliderGun( y1, x1 );
  bool vanish;
  // Eater 1
  int deltaXY=15;
  Eater1y = 9 +y1 +deltaXY;
  Eater1x =23 +x1 +deltaXY;
  vanish=0;   // 0: GliderEater solid (active) - 1: GliderEater vanishes (inactive)
  put_GliderEater( Eater1y, Eater1x, vanish);


  put_GliderGun( y2, x2 );
  // Eater 2
  deltaXY=15;
  Eater2y = 9 +y2 +deltaXY;
  Eater2x =23 +x2 +deltaXY;
  vanish=0;   // 0: GliderEater solid (active) - 1: GliderEater vanishes (inactive)
  put_GliderEater( Eater2y, Eater2x, vanish);
  
  drawBoard();
  delay(50);  
}


//---------------------------------------------------------------------------
// loop
//---------------------------------------------------------------------------
void loop()
{   volatile int ID;
    calculateGeneration();
    drawBoard();   

    // simulation board
     
    if(GenerationCnt>1200) {
      GenerationCnt=1;
    }
    
    if(GenerationCnt==1 ) {
        put_GliderEater( Eater1y, Eater1x, 0);
        put_GliderEater( Eater2y, Eater2x, 0);
    }
    else
    if(GenerationCnt==301) {
        put_GliderEater( Eater1y, Eater1x, 0);
        put_GliderEater( Eater2y, Eater2x, 1);
    }
    else
    if(GenerationCnt==601) {
        put_GliderEater( Eater1y, Eater1x, 1);
        put_GliderEater( Eater2y, Eater2x, 0);
    }
    else
    if(GenerationCnt==901) {
        put_GliderEater( Eater1y, Eater1x, 0);
        put_GliderEater( Eater2y, Eater2x, 0);
    }    
     
      
    // glider (bit) monitoring    
    ID=1;
    get_GliderEater(Eater1y, Eater1x, ID); // coordinates of Eater no.1
    delay(1);
    ID=2;
    get_GliderEater(Eater2y, Eater2x, ID); // coordinates of Eater no.2

    // generation monitor
    display.setCursor(0, display.height()-10);
    display.fillRect(0, tftheight-10, tftwidth-1, 10, COLOR_BKGR);
    GenerationCnt++;
    display.println((String)"Generation "+(String)GenerationCnt);  

    

    delay(100);
}



