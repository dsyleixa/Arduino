#pragma once

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
#define  OLIVE       0x7BE0  ///< 123, 125,   0 
#define  GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define  LIME        0x07E0  ///<   0, 255,   0
#define  GREEN       0x03E0  ///<   0, 125,   0  
#define  DARKGREEN   0x01E0  ///<   0,  63,   0 
#define  CYAN        0x07FF  ///<   0, 255, 255
#define  DARKCYAN    0x03EF  ///<   0, 125, 123
#define  BLUE        0x001F  ///<   0,   0, 255
#define  NAVY        0x000F  ///<   0,   0, 123
#define  MAGENTA     0xF81F  ///< 255,   0, 255
#define  PURPLE      0x780F  ///< 123,   0, 123
#define  PINK        0xFC18  ///< 255, 130, 198

//------------------------------------------------------------
//------------------------------------------------------------


uint16_t ColorRGB2Color16bit(uint8_t R, uint8_t G, uint8_t B) {  // (R,G,B) => 0xABCD

   return  ( ((R & 248) << 8) | ((G & 252) << 3) | ((B & 248) >> 3) );

}


//------------------------------------------------------------

void Color16bit2ColorRGB(uint16_t color16, uint8_t &R, uint8_t &G, uint8_t &B) {

   R = (uint8_t) (color16 >> 8) & 248 ;
   G = (uint8_t) (color16 >> 3) & 252 ;
   B = (uint8_t) (color16 << 3) & 248 ;
   
}


