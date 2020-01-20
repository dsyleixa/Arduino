// Arduino-Game-Of-Life

// Game of life
// http://en.wikipedia.org/wiki/Conway's_Game_of_Life
// Code adapted from
// http://cboard.cprogramming.com/c-programming/128982-simple-life-game-code-near-end.html 
// ported to Arduino by dsyleixa


// MCU: ESP8266 (probably also Zero M0, Due M3, Adafruit/Teensy M4)

// supported displays:
// OLED 128x64 SSD1306 (Adafruit_SSD1306) 
// and different Afadruit-GFX-compatible ones 
// (adjust libs+resolutions!)



#include <Arduino.h>
// i2c
#include <Wire.h>         // Incl I2C comm, but needed for not getting compile error

//-----------------------------------------------------------------------
// display driver
//-----------------------------------------------------------------------
#include <Adafruit_GFX.h>   // https://github.com/adafruit/Adafruit-GFX-Library 
// Adafruit Arduino OLED driver
#include <Adafruit_SSD1306.h> // https://github.com/esp8266/arduino 

 Adafruit_SSD1306 display(128, 64, &Wire); // new Adafruit lib
 
// fonts
#include <Fonts/FreeSans9pt7b.h>             // optional
#include <Fonts/FreeMono12pt7b.h>            // optional  
#include <Fonts/FreeMono9pt7b.h>       // used here by default
//#include <Fonts/FreeMonoBold7pt7b.h>    // optional, custom font


//---------------------------------------------------------------------------
// preferences and settings
//---------------------------------------------------------------------------
// The blocks are blockSize * blockSize big
// 2...6 seems to be a good value for this

int blockSize = 3;



// The size of the OLED screen

const int screenWidth = 128;   // <~~~~~~~~~~~~ adjust screen dimensions !
const int screenHeight = 64;
const int frame = 10;




// Make the board larger on either side to ensure that there's an invisible border of dead cells
int yrows = (screenHeight / blockSize) + 2*frame;
int xcols = (screenWidth / blockSize) + 2*frame;

#define centeryrow (yrows/2)-1 
#define centerxcol (xcols/2)-1 

// two boards, one for the current generation and one for calculating the next one
char board[screenHeight + 2*frame][screenWidth + 2*frame];
char tmpboard[screenHeight + 2*frame][screenWidth + 2*frame];


//---------------------------------------------------------------------------
// GoL functions
//---------------------------------------------------------------------------
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
  display.clearDisplay();
  for (int yrow=frame; yrow <(yrows-frame); yrow++)
  { 
    for (int xcol=frame; xcol<(xcols-frame); xcol++)
    {
      // Draw all the "live" cells.
      if (board[yrow][xcol])
        display.fillRect((xcol-frame)*blockSize, (yrow-frame)*blockSize, 
                          blockSize, blockSize, WHITE);
    }
  }
  display.display();
}


//---------------------------------------------------------------------------
// patterns
//---------------------------------------------------------------------------

void put_Blinker3x1(int starty, int startx) {       //  

  board[starty][startx]   = 1;
  board[starty][startx+1] = 1;
  board[starty][startx+2] = 1;
  
}

//---------------------------------------------------------------------------
void put_Block2x2(int starty, int startx) {       //  

  board[starty][startx]     = 1;
  board[starty][startx+1]   = 1;
  board[starty+1][startx]   = 1;
  board[starty+1][startx+1] = 1;
}


//---------------------------------------------------------------------------
void put_Bar5x1(int starty, int startx) {       //  

  board[starty][startx]   = 1;
  board[starty][startx+1] = 1;
  board[starty][startx+2] = 1;
  board[starty][startx+3] = 1;
  board[starty][startx+4] = 1;
}


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
      board[starty+y][startx+x]=sprite[y][x] ;
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
      board[starty+y][startx+x]=sprite[y][x] ;
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
      board[starty+y][startx+x]=sprite[y][x] ;
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
      board[starty+y][startx+x]=sprite[y][x] ;
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
      board[starty+y][startx+x]=sprite[y][x] ;
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
      board[starty+y][startx+x]=sprite[y][x] ;
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
      board[starty+y][startx+x]=sprite[y][x] ;
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
      board[starty+y][startx+x]=sprite[y][x] ;
    }
  }
}





//---------------------------------------------------------------------------
void put_GliderGun(int starty, int startx) {

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
    
      board[starty+y][startx+x] = sprite[y][x] ;
    }
  }
}



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
// setup
//---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(3000); // wait for Serial()
  Serial.println("Serial started");

  /*
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !LOW);  // inverted LED_BUILTIN signal logic
  
  btnUp.init( D6, INput_PULLUP);
  btnDown.init( D3, INput_PULLUP);
  btnEnter.init( D4, INput_PULLUP);    
  */
  
  // Start Wire (SDA, SCL)
  Wire.begin();

  // SSD1306 Init  
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // old Adafruit lib
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, false); // new Adafruit lib
  
  display.setRotation(2);  
  display.clearDisplay();  // Clear the buffer.

  // text display tests
  display.setTextSize(1);
  display.setFont();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("This is ");  
  display.println("Conway's Game of Live");  
  display.setCursor(0,30);
  display.println("yrows, xcols =");
  display.println(yrows);
  display.println(xcols);
  display.display();
  delay(3000);

  srand(analogRead(A0)+millis() );

    
   put_Blinker3x1( 10, 10 );                      // blockSize = 3-4 
   put_Block2x2( 10, centerxcol );
   put_Bar5x1( centeryrow, xcols-20 ); 
   put_Clock( yrows-20, centerxcol-10);
    
  
  //put_F_Pentomino( centeryrow+1, centerxcol );   // blockSize = 1 !!
  //put_Pi_Heptomino( centeryrow+2, centerxcol );  // blockSize = 1 !!             
  //put_23334M( centeryrow-1, centerxcol-1 );      // blockSize = 1 !!

  //put_Glider(30, 30 );  
  
/*
    // unit of 3 //
  put_GliderUp(centeryrow+5, 20 );     // rec. blockSize = 2; crash with LW_Spaceship
  put_LWSpaceship( (yrows/4)+5, 12 );  // rec. blockSize = 2; crash with Glider 
  put_HWSpaceship( yrows-19, 10 );     // rec. blockSize = 2;
*/   
  
  //put_GliderGun( 10, 10 );
  
  //put_randomBoard(3);
  
  drawBoard();
  delay(500);  
}


//---------------------------------------------------------------------------
// loop
//---------------------------------------------------------------------------
void loop()
{
    calculateGeneration();
    drawBoard();
    delay(200);
}



