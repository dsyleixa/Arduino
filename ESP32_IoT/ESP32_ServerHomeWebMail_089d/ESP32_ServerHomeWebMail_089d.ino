//----------------------------------------------------------------------------
//  ESP32 Adafrit Feather
//  wifiserver,  webserver
//  ESP WiFi Server für website (remote display und remote control)
//
//  nodeMCU 1.0 board ver 2.6.3 OK (test: 2.7.4)
//  ESP32 Adafruit Feather
//
// History:
// 0.8.9b: ESP32 DateTime
// 0.8.9: min/maxreset buttons
// 0.8.8f: IR fire sensor
// 0.8.8c: fINVAL, html red BGnd c1(2)t1.vmean>-4.0
// 0.8.8a Buttons client 3
// 0.8.8  Smoke Alarm; html buttons reworked
// 0.8.7a null-client; c3out:auto+website; OUT0:Alertcnt; OUT1+OUT2:digital;
//      b smoke, colors,
//      c espA0 html color
// 0.8.5a lcd2004_i2c ; a: 'red'-- else in tables
// 0.8.4 alarms
// 0.8.3 confirm last activity; ==>> email dropped!
// 0.8.1 Webserver+dns
// 0.8.0 stringEx.h  // cstringarg() etc.
// 0.7.9 neu c1out0,c2out0
// 0.7.8 neu für core 2.5.2: handleNotAuthorized()
// 0.7.4 neu: handleNotAuthorized(), cstrinarg(), dashboard:RstAlarmBtn
// 0.7.3 usr name+pwd login + var names reworked
// 0.7.2 ESP8266 core 2.4.0, ADC reworked
// -------------^^
// 0.7.0 remake website login
// 0.6.9 new Sendmail management (still no ssl)
// 0.6.8 soil humidity alarm, no warn emails
// 0.6.6 html Button c2, no PIN_setrotation
// 0.6.4n new pins I/O new SSID (show bottom)
// 0.6.3 alert Email: + mean
// 0.6.2 handleRoot() = handleClients(),
//       loop:handleNotAuthorized => continue
// 0.6.0 weniger Website-Buttons
//

//----------------------------------------------------------------------------
// target server, version

#define TARGET 'T'  // Server-Zielpattform (Z,F,T => versch. IPs, Ports, urls)
String  ver = (String)TARGET +  ".089c" ;

//----------------------------------------------------------------------------
// Wifi data from  "data\settings.h"
#include "data\settings.h"  // sets + initializes passwords

extern const char* ssid;              // WIFI network name
extern const char* password;          // WIFI network password

// url data from  "data\settings.h"
extern char  website_uname[20]; //  website user name log in  "MyWebsiteLoginName"
extern char  website_upwd[20];  //  website user pwd log in   "MyWebsiteLoginPwd"
extern char* website_title;     //  website caption           "MySiteCaption"
extern char* website_url;       //  website url               "http:\\mysite.com"




//----------------------------------------------------------------------------

/*
    ESP8266WebServer (WiFi Server)
    conn. 1 WiFi client (C1)
    WiFiUDP client f. Internet DateTime (incl Timezone), Local Time Port 8888

    Sensors / Actors:
    PCF8574 digiMux, ADS1115 ADC, MCP9808, OLED

    Arduino IDE 1.8.9
*/

/*
   PIN-OUT:
   ========                         --v--
   digital:         default        myStd         bRk              ESP_motorShield      ESP32
   D0     16       WAKE/LED       out/LED        ---               ---                  ---
   D1      5       I2C SCL        I2C SCL       I2C SCL           out D1 motorA        default
   D2      4       I2C SDA        I2C SDA       I2C SDA           out D2 motorB        default
   D3      0       FLASH/LED      in  D3     built-in btn+LED     out D3 motorA         ---
   D4      2       TX1            out D4        ---               out D4 motorB        27 PIN_OUT1
   D5     14       SPI SCK        in  D5        in D5 (DHT)       SDA                  33 PIN_OUT2
   D6     12       SPI MISO       out D6        out D6            SCL                  12 PIN_OUT3
   D7     13       SPI MOSI       in D7         in D7 (DHT)       in  D7               13 PIN_OUT0
   D8     15       MTD0 PWM       out D8        ---               out D8               15 PIN_RESETAlarm
   D9      3       UART RX0       USB, Serial   USB, Serial       USB, Serial          default
   D10     1       UART TX0       USB, Serial   USB, Serial       USB, Serial          default
   ---    10       intern ?       ---           ---               ---

   A0     IR Feuersensor, Alarm <10% // alt: MQ-2 LPG Rauch+Gas , Alarm>70%

*/


#include <stringEx.h>  // cstringarg() etc.

//----------------------------------------------------------------------------
// i2c Wire
#include <Wire.h>
/*
   // esp8266
   #define SCL         D1      // SCL
   #define SDA         D2      // SDA
   #define OLED_RESET  10      // GPIO10
*/

//----------------------------------------------------------------------------
// IO pins
//----------------------------------------------------------------------------
/*
   // ESP32 Adafruit Feather
    13 - This is GPIO #13 and also an analog input A12 on ADC #2.
         It's also connected to the red LED next to the USB port
    12 - This is GPIO #12 and also an analog input A11 on ADC #2.
         This pin has a pull-down resistor built into it, we recommend using it as an output only, or making sure that the pull-down is not affected during boot.
    27 - This is GPIO #27 and also an analog input A10 on ADC #2
    33 - This is GPIO #33 and also an analog input A9 on ADC #1.
         It can also be used to connect a 32 KHz crystal.
    15 - This is GPIO #15 and also an analog input A8 on ADC #2
    32 - This is GPIO #32 and also an analog input A7 on ADC #1.
         It can also be used to connect a 32 KHz crystal.
    14 - This is GPIO #14 and also an analog input A6 on ADC #2

*/

#define PIN_RESETAlarm  15 /*D3*/            //  Btn D3 reset Alarm

#define PIN_OUT0     LED_BUILTIN /*13*/      //  GPIO 13 out Alert LED_BUILTIN
#define PIN_OUT1     27 /*D6*/               //  out Alarmanlage ext
#define PIN_OUT2     33 /*D8*/               //  out Alarmanlage int
#define PIN_OUT3     12 /*D4*/               //  Alarm-Sirene pwm/tone


//----------------------------------------------------------------------------
// Display OLED SSD1306 + LiquidCrystal_I2C

#include <ESP_SSD1306.h>        // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>       // Needs a little change in original Adafruit library (See README.txt file)
#include <Fonts/FreeSansBold12pt7b.h>      // 
#include <Fonts/FreeSans9pt7b.h>           //
#include <Fonts/FreeMono12pt7b.h>          //
ESP_SSD1306   display(-1);

//----------------------------------------------------------------------------
// LCD 2004

#include <LiquidCrystal_I2C.h> // Library for LCD    
LiquidCrystal_I2C  lcd = LiquidCrystal_I2C(0x27, 20, 4); // Change to (0x27,16,2) for 16x2 LCD.

//----------------------------------------------------------------------------

#define  SkyBlue      #6698FF
#define  LightCyan    #58FAD0

#define  SIGNYellow   255,209,22
#define  ROSE         255,0,204
#define  SPRINGGREEN3 0,205,102



//----------------------------------------------------------------------------
// GPIO outputs
//----------------------------------------------------------------------------
volatile int8_t  OUT0 = 0, OUT1 = 0, OUT2 = 0, OUT3 = 0; // Server; actual output pin states; stop=0, fwd=1, rev=-1;
volatile int8_t  c0out0=0, c0out1=0, c0out2=0, c0out3=0; // Client0; stop=0, fwd=1, rev=-1;
volatile int8_t  c1out0=0, c1out1=0, c1out2=0, c1out3=0; // Client1; stop=0, fwd=1, rev=-1;
volatile int8_t  c2out0=0, c2out1=0, c2out2=0, c2out3=0; // Client2; stop=0, fwd=1, rev=-1;
volatile int8_t  c3out0=0, c3out1=0, c3out2=0, c3out3=0; // Client3; stop=0, fwd=1, rev=-1;
volatile int8_t  c3outMon0=0, c3outMon1=0, c3outMon2=0, c3outMon3=0;
volatile int16_t c3tx3=21; // Thermostat-Sollwert

String OUT1name = "Alarmanlage-ext";     // output captions
String OUT2name = "Alarmanlage-int";
String OUT3name = "3-KLIMA";

String c0OUT1name = "1-F.TÜR";   // c0 output names
String c0OUT2name = "2-PUMPE";
String c0OUT3name = "3-KLIMA";

String c1OUT1name = "1-LICHT";   // c1 output name
String c1OUT2name = "2-JALOU";

String c2OUT1name = "1-LICHT";   // c2 output name
String c2OUT2name = "2-JALOU";

String c3OUT1name = "1-LICHT";   // c3 output names
String c3OUT2name = "2-PUMPE";
String c3OUT3name = "3-KLIMA";



//----------------------------------------------------------------------------
// sensors
//----------------------------------------------------------------------------

// N.N barometric pressure adjust (250m)

const double  FNNcorr = 1013.0 - 984.0; // ca. 250m

const double fINVAL = -999.0;
float   fFireLIMIT =  6.0;   // fSmokeLIMIT Gas-S.= 82.0;

char    sNEXIST[10] = "--";
char    sINVAL[10]  = "??";


String SERVERname   = "SERVER";
String svSECT1name  = "Haus.1";
String svSECT2name  = "Haus.2";

String CLIENT0name  = "c0 GEW.-HAUS";
String c0SECT1name  = "Aussen";
String c0SECT2name  = "Innen °C";

String CLIENT1name  = "c1 KELLER";
String c1SECT1name  = "Gefr";
String c1SECT2name  = "Kühl";

String CLIENT2name  = "c2 KÜCHE";
String c2SECT1name  = "Gefr";
String c2SECT2name  = "Kühl";

String CLIENT3name  = "c3 GEW.-HAUS";
String c3SECT1name  = "T1 °C";
String c3SECT2name  = "T2 °C";




typedef struct {
   double    vact = fINVAL, vmin = fINVAL, vmax = fINVAL, vmean = fINVAL ; // min,act,max,mean
   uint32_t  tact = 0, tmin = 0, tmax = 0, tmean = 0, tFail = 0 ; // time (millis)
   char      sact[20] = "--", smin[20] = "--", smax[20] = "--", smean[20] = "--";
} vlog;


static vlog svt1, svh1, svt2, svh2; // Server: temperature, humidity
static vlog svp1, svq1;             // Server: barometr.air pressure, quality
static vlog svespA0;                // Server: built-in ADC A0

static vlog c0t1, c0h1, c0t2, c0h2; // Client 0: temperature, humidity
static vlog c0p1, c0q1;             // Client 0: barometr.air pressure, quality
static vlog c0espA0, c0adc0, c0adc1, c0adc2, c0adc3;  // client 0 analog readings

static vlog c1t1, c1h1, c1t2, c1h2; // Client 1: temperature, humidity
static vlog c1espA0;  // client 1 analog readings

static vlog c2t1, c2h1, c2t2, c2h2; // Client 2: temperature, humidity
static vlog c2espA0;  // client 2 analog readings

static vlog c3t1, c3h1, c3t2, c3h2; // Client 3: temperature, humidity
static vlog c3espA0, c3adc0, c3adc1, c3adc2, c3adc3;  // client 3 analog readings





//----------------------------------------------------
// local analog pins
//----------------------------------------------------

String A0intname = "AIR-Q";   // intern: ESP8266
String A0muxname = "Alarm.0";  // mux: ADS1115 (i2c)
String A1muxname = "Alarm.1";
String A2muxname = "Sens.2";
String A3muxname = "Sens.3";

String c0A0intname = "AIR-Q";     // intern: ESP8266
String c0A0muxname = "Erde.A0 ";  // mux: ADS1115 (i2c)
String c0A1muxname = "Erde.A1 ";
String c0A2muxname = "Erde.A2 ";
String c0A3muxname = "Sens.A3 ";

String c1A0intname = "AIR-Q";     // intern: ESP8266

String c2A0intname = "AIR-Q";     // intern: ESP8266

String c3A0intname = "AIR-Q";     // intern: ESP8266
String c3A0muxname = "Erde.A0 ";  // mux: ADS1115 (i2c)
String c3A1muxname = "Erde.A1 ";
String c3A2muxname = "Erde.A2 ";
String c3A3muxname = "Sens.A3 ";



//----------------------------------------------------
// ADS1015/1115 (4* ADC)
//----------------------------------------------------
#include <Adafruit_ADS1X15.h>

// Adafruit_ADS1115 ads;     /* Use this for the 16-bit version */
// Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

Adafruit_ADS1X15 ads;  // AS1115 i2c dev addr


//----------------------------------------------------------------------------
// bRk ADC 18-bit
//----------------------------------------------------
#include <MCP3421.h>

MCP3421 ADCmcp3421 = MCP3421();


//----------------------------------------------------
// MCP9808 Temperature Sensor
//----------------------------------------------------
#include <Adafruit_MCP9808.h>
Adafruit_MCP9808 MCP9808_T = Adafruit_MCP9808();


//----------------------------------------------------
// BMP280 Temperature+barometric pressure Sensor
//----------------------------------------------------

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp_x77; // I2C
/* BMP280 Pin configuration
   Pin No.	Pin Name	Pin Description
   1		VCC		Power source of 3.3VDC
   2		GND		Ground
   3		SCL		Serial Clock
   4		SDA		Serial Data
   5		CSB		CSB pin to GND to have SPI and to VCC(3.3V) for I2C.
   6		SDO		Serial Data Out / Master In Slave Out pin, for data sent
   f�r Adafruit: default Adresse 0x77 => SDO-pin HIGH! (CS nicht verbunden!)
*/




//----------------------------------------------------
// PCF8574 (8* digital IO)
//----------------------------------------------------

#include <PCF8574.h>

PCF8574    PCFx20(0x20);
#define    PCF8574addr  0x20

int8_t     ppD0 = 0, ppD1 = 0, ppD2 = 0, ppD3 = 0, // PCF digital pin states; initial=0
           ppD4 = 0, ppD5 = 0, ppD6 = 0, ppD7 = 0;



//----------------------------------------------------
// esp client sensors
//----------------------------------------------------
char sdhPa[4] = "";  // ++, =+, ==, =-, ≤, --


//----------------------------------------------------------------------------
// WiFi Libs
//----------------------------------------------------------------------------
//#include <ESP8266WiFi.h>
#include <WiFi.h>

//#include <ESP8266WebServer.h>
#include <ESPWebServer.h>

// WiFi Router

#if TARGET=='Z'
#define     this_iph     200      // <<< local host ip (200:website=Z)
#define     http_port     80
#elif TARGET=='F'
#define     this_iph     201      // <<< local host ip (201:website=T)
#define     http_port   8080
#elif TARGET=='T'
#define     this_iph     202      // <<< local host ip (209:test)
#define     http_port   8008      //     test  
#else
#define     this_iph     200      // <<< local host ip (200: default)
#define     http_port     80
#endif

#define     webs_port   8081

IPAddress    this_ip(192, 168, 2, this_iph); // <<< Feste lokale IP dieses ESP8266-Servers
IPAddress    gateway(192, 168, 2, 1);       // <<< LAN Gateway IP
IPAddress    subnet(255, 255, 255, 0);      // <<< LAN Subnet Mask

WiFiServer   wifiserver(http_port);

// ESP8266WebServer webserver(webs_port);    // for #include <ESP8266WebServer.h>
WebServer    webserver(webs_port);           // for #include <ESPWebServer.h>

bool      authorized = false;  //false


//----------------------------------------------------------------------------
// Madam = Maintainance, display, and alert management
//----------------------------------------------------------------------------
int8_t   LocAlive = 0;

static int8_t  LCDmode = 0;

#define  LCDMAXM  10
#define  RSTMODE  LCDMAXM+1


int      RemindCnt    = 0;
int      EmergencyCnt = 0;
uint32_t millisLastConfirm = millis();
uint32_t dmillisLastConfirm = 0;
uint32_t dhrsLastConfirm = 0;
uint32_t hrsConfirmLimit = 72;

//----------------------------------------------------------------------------
// Internet Time
//----------------------------------------------------------------------------

#include "time.h"
time_t now;
struct tm  timeinfo;  // timeinfo

//const char* ntpServer = "pool.ntp.org";
//char* ntpServer = "at.pool.ntp.org";

char* ntpServer = "de.pool.ntp.org";
/*
   const long  gmtOffset_sec = 3600;
   const int   daylightOffset_sec = 0;
*/

//----------------------------------------------------------------------------
// strings and symbols for website, IO, display
//----------------------------------------------------------------------------
String timestr = "--:--:--", datestr = "--.--.----";
#define CHR_DEGREE (unsigned char)247              // ° symbol for OLED font
//char    STR_DEGREE[] = {247, 0, 0};              // ° OLED font specimen (°,°C,°F,K)



//------------------------------------
void getLocalTime(char * buffer) {
   time(&now); // read the current time
   localtime_r(&now, &timeinfo); // update the structure tm with the current time
   strftime (buffer,80,"%a %d %b %Y %H:%M:%S ", &timeinfo);
}


void buildDateTimeString() {
   char sbuf[80];
   time(&now); // read the current time
   localtime_r(&now, &timeinfo); // update the structure tm with the current time

   timestr = "";
   strftime (sbuf,80,"%H:%M:%S", &timeinfo);
   timestr = sbuf;
   //Serial.println(timestr);

   datestr = "";
   strftime (sbuf,80,"%a %d %b %Y", &timeinfo);
   datestr = sbuf;
   //Serial.println(datestr);
   //Serial.println();
}



//------------------------------------
void displayLocalTime()
{
   char buffer [80];
   Serial.println("Date & Time:  ");

   getLocalTime(buffer);
   Serial.println(buffer);

   display.setTextSize(3);
   display.fillRect(0,130, display.width(),30, BLACK);  // x1,y1, dx,dy
   display.setTextColor(WHITE);
   display.setCursor(0, 60);
   display.print("Date & Time:  ");
   display.setCursor(0, 130);
   display.print(buffer);
   display.setTextSize(2);
   display.setTextColor(WHITE);

}



//----------------------------------------------------------------------------
// Tools
//----------------------------------------------------------------------------


double calADS1115 (int ADC) {
   int RES = 16383;
   if (ADC < 0) ADC = 0;
   if (ADC > RES) ADC = RES;
   return ( (double)ADC * 100.0 * 1.0) / (double)RES;
}

//-----------------------------------------------------
double calADC1023 (int ADC) {
   int RES = 1023;
   if (ADC < 0) ADC = 0;
   if (ADC > RES) ADC = RES;
   return ( (double)ADC * 100.0 * 1.0) / (double)RES;
}

//-----------------------------------------------------
void drawHorizontalBargraph(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, uint16_t percent)
{
   uint16_t hsize;

   // Create rectangle
   display.drawRect(x, y, w, h, color)  ;
   // Do not do stupid job
   if ( h > 2 && w > 2 )  {
      // calculate pixel size of bargraph
      hsize = ( ( w - 2) * percent ) / 100  ;
      // Fill it from left (0%) to right (100%)
      display.fillRect(x + 1 , y + 1 , hsize, h - 2, color);
   }
}


//-----------------------------------------------------
// Tendenz-Symbol

char dsymbol[4] = "=\0";

String tendencysymbol(float dpromille) {
   char symbol[4] = "~";
   if ((dpromille >= 0) && (dpromille <= 0.5)) strcpy(symbol, "↔");
   else if ((dpromille < 0) && (dpromille >= -0.5)) strcpy(symbol, "↔");
   else if ((dpromille > 0.5) && (dpromille <= 3.0)) strcpy(symbol, "↖");
   else if ((dpromille > 3.0)) strcpy(symbol, "⇈");
   else if ((dpromille < -0.5) && (dpromille >= -3.0)) strcpy(symbol, "↙");
   else if ((dpromille < -3.0)) strcpy(symbol, "⇊");
   symbol[3] = '\0';
   return symbol;
}



//----------------------------------------------------------------------------
// LOG ARRAY
//----------------------------------------------------------------------------

void resetMinMaxValues( vlog &v ) {
   v.vmax=v.vact;
   v.vmin=v.vact;
   v.vmean=v.vact;
   // to String
   dtostrf(v.vact, 2, 1, v.sact);
   dtostrf(v.vmin, 2, 1, v.smin);
   dtostrf(v.vmax, 2, 1, v.smax);
   dtostrf(v.vmean, 2, 1, v.smean);
}


void logval( double f, vlog &v) {

   if (f <= fINVAL ) {
      if (millis() - v.tact > 1000ul * 60) { // store invalid if outdated > 1min
         v.vact = fINVAL;
         strcpy(v.sact, sINVAL); // <<< invalid: "??" or "--"
         v.tFail= (millis()-v.tact)/(1000ul*60);   // alert time in min
      }
      delay(1);
      return;
   }

   v.tact  = millis();
   v.tFail = 0;         // reset alert time in min

   if (v.vact <= fINVAL)
   {
      v.vact = f;
   }
   else
   {
      v.vact = (v.vact + f ) / 2;
   }




   // inval min, max
   if ( v.vmin <= fINVAL )  {
      v.vmin = v.vact;
      v.tmin = v.tact;
   }
   if ( v.vmax <= fINVAL )  {
      v.vmax = v.vact;
      v.tmax = v.tact;
   }
   if ( v.vmean <= fINVAL )  {
      v.vmean = v.vact;
      v.tmean = v.tact;
   }

   // new min, max
   if (v.vact < v.vmin - 10.0 ) { // error? => small adjust min
      v.vmin -= 0.5;
      v.tmin = v.tact;
   }
   else if ( v.vact <= v.vmin )  {
      v.vmin = 0.9 * v.vmin + 0.1 * v.vact;
      v.tmin = v.tact;
   }

   if (v.vact > v.vmax + 10.0 ) { // error? => small adjust max
      v.vmax += 0.5;
      v.tmax = v.tact;
   }
   else if ( v.vact >= v.vmax ) {
      v.vmax = 0.9 * v.vmax + 0.1 * v.vact;
      v.tmax = v.tact;
   }

   // time-out min, max
   if ( millis() - v.tmin > 30 * 60 * 60 * 1000ul) { // 30h min/max time-out:
      v.vmin = 0.8 * v.vmin + 0.2 * v.vact; // 0.2 re-adjust
      v.tmin = v.tact - 29 * 60 * 60 * 1000ul; // 29h clock reset
   }
   if ( millis() - v.tmax > 1000ul * 60 * 60 * 30) {
      v.vmax = 0.8 * v.vmax + 0.2 * v.vact;
      v.tmax = v.tact - 1000ul * 60 * 60 * 29;
   }


   // mean
   v.vmean = 0.999 * v.vmean + 0.001 * v.vact;

   // to String
   dtostrf(v.vact, 2, 1, v.sact);
   dtostrf(v.vmin, 2, 1, v.smin);
   dtostrf(v.vmax, 2, 1, v.smax);
   dtostrf(v.vmean, 2, 1, v.smean);
}


/*

   typedef struct {
   double    vact = fINVAL, vmin = fINVAL, vmax = fINVAL, vmean = fINVAL ; // min,act,max,mean
   uint32_t  tact = 0, tmin = 0, tmax = 0, tmean = 0, tFail = 0 ; // time (millis)
   char      sact[20] = "--", smin[20] = "--", smax[20] = "--", smean[20] = "--";
   } vlog;


   static vlog t1, h1, t2, h2;  // Server: temperature, humidity
   static vlog p1, q1;          // Server: barometr.air pressure, quality
   static vlog espA0;           // Server: built-in ADC A0

   static vlog c0t1, c0h1, c0t2, c0h2; // Client 0: temperature, humidity
   static vlog c0p1, c0q1;             // Client 0: barometr.air pressure, quality
   static vlog c0espA0, c0adc0, c0adc1, c0adc2, c0adc3;  // client 0 analog readings

   static vlog c1t1, c1h1, c1t2, c1h2; // Client 1: temperature, humidity

   static vlog c2t1, c2h1, c2t2, c2h2; // Client 2: temperature, humidity

   static vlog c3t1, c3h1, c3t2, c3h2; // Client 3: temperature, humidity
   static vlog c3espA0, c3adc0, c3adc1, c3adc2, c3adc3;  // client 3 analog readings


*/

//----------------------------------------------------------------------------
//  handle Alarms
//----------------------------------------------------------------------------
void resetConfirmTime() {
   millisLastConfirm = millis();
   dmillisLastConfirm = 0;
   dhrsLastConfirm = 0;
   RemindCnt = 0;
}


//----------------------------------------------------------------------------
int adcSoilMin = 30;

int checkAlarms() {
   int tempEmergencyCnt = 0;

   if(c1t1.vmean > -4.0) tempEmergencyCnt++;     // Temperature avrg Freezer  > -4°C
   if(c2t1.vmean > -4.0) tempEmergencyCnt++;

   if(c0t1.tFail > 2*60) tempEmergencyCnt++;    // Value Alarm timeout > 120min
   if(c0adc0.vmean< adcSoilMin || c0adc0.tFail>2*60) tempEmergencyCnt++;  // real ADC Value % < adcSoilMin
   if(c0adc1.vmean< adcSoilMin || c0adc1.tFail>2*60) tempEmergencyCnt++;
   if(c0adc2.vmean< adcSoilMin || c0adc2.tFail>2*60) tempEmergencyCnt++;
   if(c0adc3.vmean< adcSoilMin || c0adc3.tFail>2*60) tempEmergencyCnt++;

   if(c1t1.tFail > 2*60) tempEmergencyCnt++;  // Value Alarm timeout > 120min
   if(c2t1.tFail > 2*60) tempEmergencyCnt++;
   if(c3t1.tFail > 2*60) tempEmergencyCnt++;

   // Smoke Alarm > threshold
   if(svespA0.vact<fFireLIMIT) { // Gas: >=fSmokeLIMIT
      tempEmergencyCnt++;
   }

   c0espA0.vact=50;               //  N/A
   if(c0espA0.vact<fFireLIMIT) {  //  N/A
      // tempEmergencyCnt++;      //  N/A
   }
   if(c1espA0.vact<fFireLIMIT) {
      tempEmergencyCnt++;
   }
   if(c2espA0.vact<fFireLIMIT) {
      tempEmergencyCnt++;
   }
   if(c3espA0.vact<fFireLIMIT) {
      tempEmergencyCnt++;
   }
   digitalWrite(PIN_OUT3, 0);
   if(  (svespA0.vact<fFireLIMIT)
         || (c0espA0.vact<fFireLIMIT)
         || (c1espA0.vact<fFireLIMIT)
         || (c2espA0.vact<fFireLIMIT)
         || (c3espA0.vact<fFireLIMIT))
   {
//      tone(PIN_OUT3, 440, 250);
      delay(500);
   }
   else digitalWrite(PIN_OUT3, 0);

   /*
      if(c3adc0.vmean< adcSoilMin || c3adc0.tFail>2*60) tempEmergencyCnt++;  // real ADC Value % < adcSoilMin
      if(c3adc1.vmean< adcSoilMin || c3adc1.tFail>2*60) tempEmergencyCnt++;
      if(c3adc2.vmean< adcSoilMin || c3adc2.tFail>2*60) tempEmergencyCnt++;
      if(c3adc3.vmean< adcSoilMin || c3adc3.tFail>2*60) tempEmergencyCnt++;
   */

   return tempEmergencyCnt;
}





//----------------------------------------------------------------------------
// OLED dashboard
//----------------------------------------------------------------------------


void dashboard(int mode) {
   static uint16_t refreshcntr=0;

   if (mode > LCDMAXM) {
      LCDmode = 0;
      mode = 0;
   }
   if ( !digitalRead(PIN_RESETAlarm) ) mode = RSTMODE;

   display.setFont();
   display.clearDisplay();

   if(refreshcntr>=10) refreshcntr=0;
   if(refreshcntr==0)  {
      lcd.clear();
   }

   if (mode == 0)  {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print(timestr);
      display.setCursor( 0, 28);  display.print(datestr);
      display.setCursor( 0, 44);  display.print("Svr Innen T = " + (String)svt1.sact + "'C");
      display.setCursor( 0, 60);  display.print("    AIR-Q   = " + (String)svespA0.sact + "%");
      if(!refreshcntr%10) {                  // <<<<<<<<<<<<<<<<<<<  ????
         lcd.setCursor(0,0); lcd.print(timestr);
         lcd.setCursor(0,1); lcd.print(datestr);
         lcd.setCursor(0,2); lcd.print("Svr Innen T = " + (String)svt1.sact + "'C");
         lcd.setCursor(0,3); lcd.print("    AIR-Q   = " + (String)svespA0.sact + "%");
      }
   }

   else if (mode == 1) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("c0 Fail : "+(String)(c0t1.tFail ));
      display.setCursor( 0, 28);  display.print("c1 Fail : "+(String)(c1t1.tFail ));
      display.setCursor( 0, 44);  display.print("c2 Fail : "+(String)(c2t1.tFail ));
      display.setCursor( 0, 60);  display.print("c3 Fail : "+(String)(c3t1.tFail ));
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c0 Fail : "+(String)(c0t1.tFail ));
         lcd.setCursor(0,1); lcd.print("c1 Fail : "+(String)(c1t1.tFail ));
         lcd.setCursor(0,2); lcd.print("c2 Fail : "+(String)(c2t1.tFail ));
         lcd.setCursor(0,3); lcd.print("c3 Fail : "+(String)(c3t1.tFail ));
      }
   }

   else if (mode == 2) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c0 AIR-Q : "+(String)(c0espA0.vact ));
         lcd.setCursor(0,1); lcd.print("c1 AIR-Q : "+(String)(c1espA0.vact ));
         lcd.setCursor(0,2); lcd.print("c2 AIR-Q : "+(String)(c2espA0.vact ));
         lcd.setCursor(0,3); lcd.print("c3 AIR-Q : "+(String)(c3espA0.vact ));
      }
   }

   else if (mode == 3) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c0 GH ");
         lcd.setCursor(0,1); lcd.print("  Aussen T= " + (String)c0t1.sact + "'C");
         lcd.setCursor(0,2); lcd.print("  Innen  T= " + (String)c0t2.sact + "'C");
         lcd.setCursor(0,3); lcd.print("  AIR-Q   = " + (String)c0espA0.sact + " %");
      }
   }

   else if (mode == 4) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c0 GH A0= " + (String)c0adc0.sact);
         lcd.setCursor(0,1); lcd.print("c0 GH A1= " + (String)c0adc1.sact);
         lcd.setCursor(0,2); lcd.print("c0 GH A2= " + (String)c0adc2.sact);
         lcd.setCursor(0,3); lcd.print("c0 GH A3= " + (String)c0adc3.sact);
      }
   }
   else if (mode == 5) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c1 Keller:");
         lcd.setCursor(0,1); lcd.print("Freezer:  " + (String)c1t1.sact+ "'C" );
         lcd.setCursor(0,2); lcd.print("Kuehlsch: " + (String)c1t2.sact+ "'C" );
         lcd.setCursor(0,3); lcd.print("AIR-Q:    " + (String)c1espA0.sact+ "%" );
      }
   }


   else if (mode == 6) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c2 Kueche:");
         lcd.setCursor(0,1); lcd.print("Freezer:  " + (String)c2t1.sact+ "'C" );
         lcd.setCursor(0,2); lcd.print("Kuehlsch: " + (String)c2t2.sact+ "'C" );
         lcd.setCursor(0,3); lcd.print("AIR-Q:    " + (String)c2espA0.sact+ "%" );
      }
   }

   else if (mode == 7) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c3 GH ");
         lcd.setCursor(0,1); lcd.print("  Aussen T= " + (String)c3t1.sact + "'C");
         lcd.setCursor(0,2); lcd.print("  Innen  T= " + (String)c3t2.sact + "'C");
         lcd.setCursor(0,3); lcd.print("  AIR-Q   = " + (String)c3espA0.sact + " %");
      }
   }

   else if (mode == 8) { // c0adc
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c3 GH A0= " + (String)c3adc0.sact);
         lcd.setCursor(0,1); lcd.print("c3 GH A1= " + (String)c3adc1.sact);
         lcd.setCursor(0,2); lcd.print("c3 GH A2= " + (String)c3adc2.sact);
         lcd.setCursor(0,3); lcd.print("c3 GH A3= " + (String)c3adc3.sact);
      }
   }

   else if (mode == 9) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("Alarms: " + (String)EmergencyCnt);
         lcd.setCursor(0,1); lcd.print("Erinn.: " + (String)RemindCnt);
         lcd.setCursor(0,2); lcd.print("");
         lcd.setCursor(0,3); lcd.print("");
      }
   }
   else if (mode ==10) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print(timestr);
      display.setCursor( 0, 28);  display.print(datestr);
      display.setCursor( 0, 44);  display.print("Svr Innen T = " + (String)svt1.sact + "'C");
      display.setCursor( 0, 60);  display.print("    AIR-Q   = " + (String)svespA0.sact + "%");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print(timestr);
         lcd.setCursor(0,1); lcd.print(datestr);
         lcd.setCursor(0,2); lcd.print("Svr Innen T = " + (String)svt1.sact + "'C");
         lcd.setCursor(0,3); lcd.print("    AIR-Q   = " + (String)svespA0.sact + "%");
      }
   }
   else  if (mode == RSTMODE)  {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print( "reset warnings" );
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("reset warnings");
         lcd.setCursor(0,1); lcd.print("");
         lcd.setCursor(0,2); lcd.print("");
         lcd.setCursor(0,3); lcd.print("");
      }
   }
   refreshcntr++;
   display.display();
   display.setFont();
   delay(10);                // <<<<<<<<<<<<<<<<<<<<<<<<<< Test, neu
}





//----------------------------------------------------------------------------
// SETUP
//----------------------------------------------------------------------------
void setup() {

   int IORes;
   int progress = 0;


   //----------------------------------------
   Serial.begin(115200);
   delay(1000);

   //----------------------------------------
   pinMode(PIN_OUT0, OUTPUT);
   digitalWrite(PIN_OUT0, LOW);

   pinMode(PIN_OUT1, OUTPUT);
   digitalWrite(PIN_OUT1, LOW);

   pinMode(PIN_OUT2, OUTPUT);
   digitalWrite(PIN_OUT2, LOW);

   pinMode(PIN_OUT3, OUTPUT);
   digitalWrite(PIN_OUT3, LOW);


   pinMode(PIN_RESETAlarm, INPUT_PULLUP);  // reset Alarm



   //----------------------------------------
   // i2c: init

   //Wire.pins(SDA, SCL);        // SDA, SCL
   Wire.begin();
   Wire.setClock(100000ul);
   delay(1);

   //----------------------------------------
   // i2c: PCF8574

   PCFx20.write8(0xFF); // (all pins INPUT HIGH)
   delay(1);

   //----------------------------------------
   // OLED
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
   display.setFont();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.clearDisplay();
   display.setCursor( 0, 0);  display.print("OLED display init OK");
   display.display();
   delay(1);
   Serial.println("OLED display init OK");

   //----------------------------------------
   //  LCD
   lcd.init();
   lcd.backlight();
   lcd.noBlink();
   Serial.println("LCD display init OK");
   lcd.setCursor(0, 0);  lcd.print("LCD display init OK");


   /*
      //----------------------------------------
      // i2c:  ADS1115
      Serial.println("ADS1115 sensor init... ");
      ADSx48.begin(0x48); // ADS1115 I2C 4x 12/16bit ADC
      delay(1);

   */



   //----------------------------------------
   // Init MCP3421: I2C-Adresse, 18 Bit Modus, keine Verstärkung
   Serial.println("ADCmcp3421 init...");
   ADCmcp3421.init(0x68, 3, 0);


   /*
      //----------------------------------------
      // i2c:  MCP9808
      Serial.println("MCP9808 init...");
      IORes=MCP9808_T.begin();
      delay(1);
      if (!IORes) {
      Serial.println("Couldn't find MCP9808!");
      }
      else
      Serial.println("MCP9808 sensor init: OK");

   */


   //----------------------------------------
   // i2c: BMP280
   Serial.println("BMP280 init...");
   IORes = bmp_x77.begin();
   if (!IORes) {
      Serial.println("Couldn't find BMP280 sensor!");
   }
   else {
      Serial.println("BMP280 sensor init: OK");
   }
   delay(1);



   //----------------------------------------
   // Connect to WiFi network
   Serial.println();
   Serial.println();
   Serial.println("Connecting to WiFi: ");
   Serial.println( WiFi.gatewayIP().toString() );
   lcd.setCursor(0, 1);  lcd.print("Connecting to WiFi: ");

   WiFi.mode(WIFI_STA);
   // WiFi.config(local_this_ip, gateway, subnet);
   WiFi.config(this_ip, gateway, subnet, gateway, gateway);   // dns = gateway
   WiFi.begin(ssid, password);

   while (WiFi.status() != WL_CONNECTED) {

      delay(500);
      Serial.print(".");

      display.clearDisplay();
      display.setCursor( 0, 20);  display.print("WiFi connecting...");
      drawHorizontalBargraph( 0, 30, (int16_t) display.width(), 9, 1, progress);
      display.setCursor( 0, 40);  display.print((String)progress + "%");
      if (progress >= 98) {
         progress = 80;
         Serial.println();
      }
      lcd.setCursor(0, 2);  lcd.print((String)progress + "%");
      display.display();

      if (progress < 10) progress += 5;
      else if (progress < 50) progress += 2;
      else if (progress < 90) progress += 1;
   }
   display.clearDisplay();
   progress = 100;
   display.setCursor( 0, 20);  display.print("WiFi connecting...");
   drawHorizontalBargraph( 0, 30, (int16_t) display.width(), 9, 1, progress);
   display.setCursor( 0, 40);  display.print((String)progress + "%");
   display.setCursor( 0, 50);  display.print(WiFi.gatewayIP());
   display.display();
   lcd.setCursor(0, 2);  lcd.print(WiFi.gatewayIP());
   delay(300);

   Serial.println("");
   Serial.print("WiFi connected: ");
   Serial.println(WiFi.gatewayIP());
   Serial.println(this_ip);
   Serial.println(http_port);


   //----------------------------------------
   // Start the WiFi server (-> www)
   wifiserver.begin();
   Serial.println("WiFi Server started");

   //----------------------------------------
   // Start the ESP web server (connect -> ESP clients)
   webserver.on("/", handleRoot) ;
   webserver.on("/client/client0/", handleClients);
   delay(10);
   webserver.on("/client/client1/", handleClients);
   delay(10);
   webserver.on("/client/client2/", handleClients);
   delay(10);
   webserver.on("/client/client3/", handleClients);
   delay(10);
   webserver.begin(webs_port);
   Serial.println("Web Server started");

   // Print the IP address
   Serial.print("Use this URL to connect: ");
   Serial.print("http://");
   Serial.print(WiFi.localIP());
   Serial.print(":");
   Serial.print(http_port);
   Serial.println("/");
   Serial.print((String)website_url + ":" + http_port + "/");

   //---------------------------------------------------------
   // NTP Timeserver

#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03" // Europe/Berlin  CT-1CEST,M3.5.0,M10.5.0/3

   configTime(0, 0, ntpServer); // 0, 0 because we will use TZ in the next line
   setenv("TZ", MY_TZ, 1);      // Set environment variable with your time zone
   tzset();
   Serial.println();
   Serial.println("  NTP TimeServer started!");

   //------------------------------------
   displayLocalTime();

   //----------------------------------------
   // setup done
   LCDmode = 0;
   dashboard(LCDmode);
   Serial.println("setup done \n");

   authorized=false;

}






//----------------------------------------------------------------------------
// LOOP
//----------------------------------------------------------------------------

void loop() {

   static double ftmp;
   static unsigned long tsec = millis(), tms = millis();


   //---------------------------------------
   // Check log-in


   if (!authorized) {
      handleNotAuthorized();
      delay(100);
   }

   if (authorized) {
      handleWebsite();
      delay(10);
   }

   // test, debug
   //handleLEDsite();
   //handleNotAuthorized();
   //handleWebsite();




   webserver.handleClient();
   delay(10);

   //---------------------------------------
   // Read local + Udp data

   EmergencyCnt = checkAlarms();
   if(EmergencyCnt==0) digitalWrite(PIN_OUT0, 1); // reverse led_builtin pin switch
   else digitalWrite(PIN_OUT0, 0);

   //---------------------------------------------------------------------
   //test, debug
   if(OUT1==1) digitalWrite(PIN_OUT0, 1); //  led_builtin pin switch
   else digitalWrite(PIN_OUT0, 0);
   //---------------------------------------------------------------------


   dmillisLastConfirm = millis() - millisLastConfirm;
   dhrsLastConfirm = dmillisLastConfirm/(1000ul*60*60);
   if(dhrsLastConfirm > hrsConfirmLimit) {
      RemindCnt = dhrsLastConfirm-hrsConfirmLimit;
   }

   if ( millis() - tms >= 100 ) {    // refresh data rate
      tms = millis();

      //---------------------------------------
      // build date + time strings
      buildDateTimeString();
      Serial.println(timestr+"   "+datestr);

      //---------------------------------------
      // read DHT Sensor
      /*
         ftmp = DHT_1.readTemperature();          // 1. Temperatur auslesen (Celsius)
         delay(1);
         if (isnan(ftmp)) ftmp=fINVAL;
         logval(ftmp, svt1);
         delay(10);

         ftmp = DHT_1.readHumidity();             // 1. Feuchtigkeit auslesen (Prozent)
         delay(1);
         if (isnan(ftmp)) ftmp=fINVAL;
         logval(ftmp, svh1);
         delay(10);
      */

      ftmp = fINVAL;
      ftmp = bmp_x77.readTemperature();
      if (isnan(ftmp)) ftmp = fINVAL;
      logval(ftmp, svt1);
      delay(1);
      /*
         ftmp=fINVAL;
         ftmp = bme_x77.readHumidity();       // 2. Feuchtigkeit auslesen (Prozent)
         if (isnan(ftmp)) ftmp=fINVAL;
         logval(ftmp, svh2);
         delay(1);
      */

      ftmp = fINVAL;
      ftmp = FNNcorr + bmp_x77.readPressure() / 100.0 ;
      if (isnan(ftmp)) ftmp = fINVAL;
      logval(ftmp, svp1);
      double dhPa = svp1.vact - svp1.vmean;
      delay(1);


      ftmp = fINVAL;
      ftmp = (float)analogRead(A0);
      ftmp = calADC1023(ftmp);
      if (isnan(ftmp)) ftmp = 0;
      logval(ftmp, svespA0);
      delay(1);


      Serial.println("Client sensors:");
      Serial.print(" c0_t1="); Serial.print(c0t1.sact);
      Serial.print(" c0_h1="); Serial.print(c0h1.sact);
      Serial.print(" c0_t2="); Serial.print(c0t2.sact);
      Serial.print(" c0_h2="); Serial.print(c0h2.sact);
      Serial.println(" ");
      Serial.print(" c1_t1="); Serial.print(c1t1.sact);
      Serial.print(" c1_h1="); Serial.print(c1h1.sact);
      Serial.print(" c1_t2="); Serial.print(c1t2.sact);
      Serial.print(" c1_h2="); Serial.print(c1h2.sact);
      Serial.println(" ");
      Serial.print(" c2_t1="); Serial.print(c2t1.sact);
      Serial.print(" c2_h1="); Serial.print(c2h1.sact);
      Serial.print(" c2_t2="); Serial.print(c2t2.sact);
      Serial.print(" c2_h2="); Serial.print(c2h2.sact);
      Serial.println(" ");
      Serial.print(" c3_t1="); Serial.print(c3t1.sact);
      Serial.print(" c3_h1="); Serial.print(c3h1.sact);
      Serial.print(" c3_t2="); Serial.print(c3t2.sact);
      Serial.print(" c3_h2="); Serial.print(c3h2.sact);
      Serial.println(" "); Serial.println(" ");

      //---------------------------------------
      // display on OLED
      if ( millis() - tsec >= 4000 ) {
         tsec = millis();
         LCDmode++;
      }
      dashboard(LCDmode);
      delay(1);
   }

}





//----------------------------------------------------------------------------
//  handle Websites
//----------------------------------------------------------------------------



void OLDhandleNotAuthorized() {
   String readString = "";
   String script = "";

   char   strinput[MAXLEN], strupwd[TOKLEN], struname[TOKLEN] ;
   strcpy(strinput, "");
   strcpy(strupwd, "");
   strcpy(struname, "");

   WiFiClient client = wifiserver.available();

   while ( client.connected() ) {

      if (authorized) return;

      if ( client.available() ) {

         char c = client.read();

         //read char by request
         readString = "";
         while ( (readString.length() < TOKLEN) && (c != '\n') ) {
            readString += c;
            c = client.read();
         }

         if (strstr(website_upwd, strupwd) != NULL & strstr(website_uname, struname) != NULL)

            readString.toCharArray(strinput, MAXLEN);
         // cstringarg( char* haystack, char* vname, char* sarg )
         // haystack pattern: &varname=1234abc,  delimiters &, \n, \0, SPACE, EOF
         cstringarg(strinput, "uname", struname);  // uname
         cstringarg(strinput, "upwd", strupwd);   // upwd

         // debug
         Serial.print("strupwd     >>>"); Serial.print(strupwd); Serial.println("<<<");
         Serial.print("website_upwd>>>"); Serial.print(website_upwd); Serial.println("<<<");
         Serial.print("readString>>>"); Serial.println(readString);

         if ( (strlen(strupwd) == strlen(website_upwd)) && (strcmp(website_upwd, strupwd) == 0)
               && (strlen(struname) == strlen(website_uname)) && (strcmp(website_uname, struname) == 0)
            )
         {
            authorized = true;
            readString = "";
            return;
         }

         //if HTTP request has ended
         if (c == '\n') {
            client.flush();

            //now output html data header
            script = "";
            script += ("<!DOCTYPE html> \n");
            script += ("<html> \n");

            script += ("<head> \n");

            // utf-8 für "°" Zeichen
            script +=  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> \n" ;
            script +=  "<title>" ;
            script +=  website_title ;
            script +=  "</title> \n" ;
            script +=  "</head> \n" ;
            script +=  "<body> \n" ;

            script += ("<h2><p style=\"color:rgb(255,0,191);\"> log in to proceed: </p> </h2> \n");

            script += ("<FORM ACTION='/' method=GET > \n");
            script += ("<h2>user name:  <INPUT TYPE=text NAME='uname' VALUE=''  MAXLENGTH='50'> </h2> \n");
            script += ("<h2>password :  <INPUT TYPE=PASSWORD NAME='upwd' VALUE='' MAXLENGTH='50'> </h2> \n");

            script += ("<h2><INPUT TYPE=SUBMIT></h2> \n");

            script += ("</FORM> \n");
            script += ("<BR> \n");
            script += ("</body> \n");
            script += ("</html> \n");

            //----------------------------------

            String script1 = "";
            script1 += "HTTP/1.1 200 OK \n";
            script1 += "Content-Type: text/html \n";
            //script1 += "Content-Length: " + (String)script.length() + "\n";
            script1 += "\n";  //  do not forget this one //????

            script = script1 + script;

            client.print(script);

            delay(100);

            //stopping client
            client.stop();
         }
      } // if client.available

      delay(1);
   }
}









// new Authenticate site

void handleNotAuthorized() {
   String readString = "";
   String script = "";

   char   strinput[MAXLEN], strupwd[TOKLEN], struname[TOKLEN] ;
   strcpy(strinput, "");
   strcpy(strupwd, "");
   strcpy(struname, "");


   WiFiClient client = wifiserver.available();   // listen for incoming clients

   //if(!client) return;  // <<<<<<<< needed?

   if (client) {                               // if you get a client,
      Serial.println("New Client.");           // print a message out the serial port
      String currentLine = "";                 // make a String to hold incoming data from the client

      while (client.connected()) {             // loop while the client's connected

         if (authorized) return;

         if (client.available()) {             // if there's bytes to read from the client,

            char c = client.read();            // read a byte, then
            Serial.write(c);                   // print it out the serial monitor




//read char by request
            readString = "";
            while ( (readString.length() < TOKLEN) && (c != '\n') ) {
               readString += c;
               c = client.read();
            }

            if (strstr(website_upwd, strupwd) != NULL & strstr(website_uname, struname) != NULL)

               readString.toCharArray(strinput, MAXLEN);
            // cstringarg( char* haystack, char* vname, char* sarg )
            // haystack pattern: &varname=1234abc,  delimiters &, \n, \0, SPACE, EOF

            cstringarg(strinput, "uname", struname);  // uname
            cstringarg(strinput, "upwd", strupwd);   // upwd

            // debug
            Serial.print("strupwd     >>>"); Serial.print(strupwd); Serial.println("<<<");
            Serial.print("website_upwd>>>"); Serial.print(website_upwd); Serial.println("<<<");
            Serial.print("readString>>>"); Serial.println(readString);

            if ( (strlen(strupwd) == strlen(website_upwd)) && (strcmp(website_upwd, strupwd) == 0)
                  && (strlen(struname) == strlen(website_uname)) && (strcmp(website_uname, struname) == 0)
               )
            {
               authorized = true;
               readString = "";
               return;
            }








            if (c == '\n') {                   // if the byte is a newline character

               // if the current line is blank, you got two newline characters in a row.
               // that's the end of the client HTTP request, so send a response:
               if (currentLine.length() == 0) {
                  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                  // and a content-type so the client knows what's coming, then a blank line:

                  script += "HTTP/1.1 200 OK \n";
                  script += "Content-type:text/html \n";
                  script += "\n";

                  script += "<head> \n";

                  // utf-8 für "°" Zeichen
                  script +=  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> \n" ;
                  script +=  "<title>" ;
                  script +=  website_title ;
                  script +=  "</title> \n" ;
                  script +=  "</head> \n" ;
                  script +=  "<body> \n" ;

                  script += ("<h2><p style=\"color:rgb(255,0,191);\"> log in to proceed: </p> </h2> \n");

                  script += ("<FORM ACTION='/' method=GET > \n");
                  script += ("<h2>user name:  <INPUT TYPE=text NAME='uname' VALUE=''  MAXLENGTH='50'> </h2> \n");
                  script += ("<h2>password :  <INPUT TYPE=PASSWORD NAME='upwd' VALUE='' MAXLENGTH='50'> </h2> \n");

                  script += ("<h2><INPUT TYPE=SUBMIT></h2> \n");

                  script += ("</FORM> \n");
                  script += ("<BR> \n");
                  script += ("</body> \n");
                  //script += ("</html> \n");

                  script += "\n";
                  // The HTTP response ends with another blank line:
                  script += "\n";


                  client.print(script);


                  /*
                                     script += "<head> \n";
                                     // autom. Aktualisierung alle 10 sec.
                                     script += "<meta http-equiv=\"refresh\" content=\"10\" > \n" ;

                                     // utf-8 für "°" Zeichen
                                     script += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> \n" ;
                                     script += "<title>";
                                     script += website_title;
                                     script += "</title> ";

                                     script += "</head> \n";


                                     script += "<h1>My First WiFiServer with ESP32 - Station Mode &#128522;</h1> \n";

                                     script += "<h2>";
                                     script += (String)timestr +"<br>\n";
                                     script += "</h2> \n";

                                     //---------------------------------------
                                     // text font Courier, color black
                                     script += "<p> <font face=\"courier\">"; // <<<<<<<<<<<<<<<< Courier
                                     script += "<font style=\"color:rgb(0,0,0);\" > </p>";
                                     script += "<br> \n";

                                     script += "<h3> ";

                                     script += "Output 1 ist: ";
                                     if (OUT1 == 1)
                                     {
                                        script += ("EIN &nbsp; <wbr> <wbr> <br>\n");
                                     }
                                     else
                                     {
                                        script += ("AUS &nbsp; <wbr> <wbr> <br>\n");
                                     }

                                     script += "<a href=\" /OUT1H\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ";
                                     script += "<a href=\" /OUT1L\"\"> <button style=\"height:70px;width:140px\" > AUS </button></a>  ";
                                     script += "<br> \n\n";

                                     script += "</h3> ";
                                     script += "\n";
                                     // The HTTP response ends with another blank line:
                                     script += "\n";

                                     client.print(script);
                  */

                  delay(1);

                  // break out of the while loop:
                  break;

               }
               else {    // if you got a newline, then clear currentLine:
                  currentLine = "";
               }
            }
            else if (c != '\r') {  // if you got anything else but a carriage return character,
               currentLine += c;      // add it to the end of the currentLine
            }
            delay(1);



            /*
                        // Check if the client request was "GET /OUT1H" or "GET /OUT1L":
                        if (currentLine.endsWith("GET /OUT1H")) {
                           OUT1=HIGH;              // GET /OUT1H turns the LED on
                        }
                        if (currentLine.endsWith("GET /OUT1L")) {
                           OUT1=LOW;               // GET /OUT1L turns the LED off
                        }
            */



         } // if client.available
      } // while client.connected


      // close the connection:
      client.stop();
      delay(1);
      Serial.println("Client Disconnected.");
   }
   delay(5);

}



//======================================
// test,debug
void handleLEDsite() {
   char timestr[80];
   getLocalTime(timestr);
   Serial.println(timestr);

   String script="";
   String website_title = "myESP32WifiServer";

   WiFiClient client = wifiserver.available();   // listen for incoming clients

   //if(!client) return;  // <<<<<<<< needed?

   if (client) {                               // if you get a client,
      Serial.println("New Client.");           // print a message out the serial port
      String currentLine = "";                 // make a String to hold incoming data from the client

      while (client.connected()) {             // loop while the client's connected

         if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();            // read a byte, then
            Serial.write(c);                   // print it out the serial monitor

            if (c == '\n') {                   // if the byte is a newline character

               // if the current line is blank, you got two newline characters in a row.
               // that's the end of the client HTTP request, so send a response:
               if (currentLine.length() == 0) {
                  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                  // and a content-type so the client knows what's coming, then a blank line:

                  script += "HTTP/1.1 200 OK \n";
                  script += "Content-type:text/html \n";
                  script += "\n";

                  script += "<head> \n";
                  // autom. Aktualisierung alle 10 sec.
                  script += "<meta http-equiv=\"refresh\" content=\"10\" > \n" ;

                  // utf-8 für "°" Zeichen
                  script += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> \n" ;
                  script += "<title>";
                  script += website_title;
                  script += "</title> ";

                  script += "</head> \n";


                  script += "<h1>My First WiFiServer with ESP32 - Station Mode &#128522;</h1> \n";

                  script += "<h2>";
                  script += (String)timestr +"<br>\n";
                  script += "</h2> \n";

                  //---------------------------------------
                  // text font Courier, color black
                  script += "<p> <font face=\"courier\">"; // <<<<<<<<<<<<<<<< Courier
                  script += "<font style=\"color:rgb(0,0,0);\" > </p>";
                  script += "<br> \n";

                  script += "<h3> ";

                  script += "Output 1 ist: ";
                  if (OUT1 == 1)
                  {
                     script += ("EIN &nbsp; <wbr> <wbr> <br>\n");
                  }
                  else
                  {
                     script += ("AUS &nbsp; <wbr> <wbr> <br>\n");
                  }

                  script += "<a href=\" /OUT1H\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ";
                  script += "<a href=\" /OUT1L\"\"> <button style=\"height:70px;width:140px\" > AUS </button></a>  ";
                  script += "<br> \n\n";

                  script += "</h3> ";
                  script += "\n";

                  // The HTTP response ends with another blank line:
                  script += "\n";

                  client.print(script);

                  delay(1);

                  // break out of the while loop:
                  break;

               }
               else {    // if you got a newline, then clear currentLine:
                  currentLine = "";
               }
            }
            else if (c != '\r') {  // if you got anything else but a carriage return character,
               currentLine += c;      // add it to the end of the currentLine
            }
            delay(1);

            // Check if the client request was "GET /OUT1H" or "GET /OUT1L":
            if (currentLine.endsWith("GET /OUT1H")) {
               OUT1=HIGH;              // GET /OUT1H turns the LED on
            }
            if (currentLine.endsWith("GET /OUT1L")) {
               OUT1=LOW;               // GET /OUT1L turns the LED off
            }

         }
      }
      // close the connection:
      client.stop();
      delay(1);
      Serial.println("Client Disconnected.");
   }
   delay(5);

}




//----------------------------------------------------------------------------
// site works!

void handleWebsite() {
   String script="";

   char   istr[10]; // formatted var buffer
   String Str;      // buffer
   script.reserve(12000);

   //String website_title = "myESP32WifiServer";

   WiFiClient client = wifiserver.available();   // listen for incoming clients

   //if(!client) return;  // <<<<<<<< needed?

   if (client) {                               // if you get a client,
      Serial.println("New Client.");           // print a message out the serial port
      String currentLine = "";                 // make a String to hold incoming data from the client

      while (client.connected()) {             // loop while the client's connected
         if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();            // read a byte, then
            Serial.write(c);                   // print it out the serial monitor
            if (c == '\n') {                   // if the byte is a newline character

               // if the current line is blank, you got two newline characters in a row.
               // that's the end of the client HTTP request, so send a response:
               if (currentLine.length() == 0) {
                  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                  // and a content-type so the client knows what's coming, then a blank line:

                  // init website

                  script += ("HTTP/1.1 200 OK \n");
                  script += ("Content-Type: text/html \n");
                  script += ("\n"); //  <<<<<<<<<<<<<<<<<<
                  script += ("<!DOCTYPE html> \n");
                  script += ("<html> \n");

                  // head + title
                  script += ("<head> \n");
                  // autom. Aktualisierung alle 20 sec.

                  script += "<meta http-equiv=\"refresh\" content=\"20; URL=";
                  script += (String)website_url + ":" + (String)http_port + "\"> \n" ;


                  // utf-8 für "°" Zeichen
                  script += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> \n" ;
                  script += ("<title>");
                  script += (website_title);
                  script += ("</title> \n");

                  script += ("</head> \n");

                  // body + caption
                  script += ("<body> \n");
                  script += ("<h1> <p> ");
                  script += ("<font style=\"color:rgb(255,0,204);\"> DON'T PANIC ! ");
                  script += ("&nbsp; <wbr> <wbr> ");
                  script += ("<font style=\"color:rgb(0,205,102);\"> SmartHome by " + (String)website_url );
                  script += ("! </p> </h1>  ");

                  // date + time
                  script += "<h2><p style=\"color:rgb(0,205,102);\"> " ;
                  script += datestr + " &nbsp; <wbr> <wbr> " + timestr + " &nbsp; <wbr> <wbr> <wbr> ";
                  script += "<font style=\"color:rgb(0,0,0);\" >  ";

                  script += ("<br>");
                  script += "<font style=\"color:rgb(255,0,0);\"> " ;
                  script += " Notfall Alarms: " + String(EmergencyCnt);
                  script += "<font style=\"color:rgb(255,209,22);\"> " ;
                  script += " &nbsp; <wbr> <wbr> <wbr> Erinnerungen: " + String(RemindCnt) + " ";
                  script += "</p> </h2> ";
                  script += "</br> ";

                  //---------------------------------------
                  // text font Courier, color black
                  //script += ("<p> <font face=\"courier\">"); // <<<<<<<<<<<<<<<< Courier
                  script += "<p> "; // <<<<<<<<<<<<<<<<
                  script += "<font style=\"color:rgb(0,0,0);\" > </p>";

                  script += "<h3> " + (String)("letztes confirm (Std): &nbsp;") + (String)dhrsLastConfirm + " &nbsp;&nbsp;&nbsp; ";

                  script += "<a href=\" /CONFIRM=ON\"\"> <button style=\"height:50px;width:100px\" > Confirm </button></a>";
                  script += "</h3> ";
                  script += "</br> ";

                  //---------------------------------------
                  // text font Courier, color black
                  script += "<p> <font face=\"courier\">"; // <<<<<<<<<<<<<<<< Courier
                  script += "<font style=\"color:rgb(0,0,0);\" > </p>";
                  //---------------------------------------

                  delay(1);

                  //---------------------------------------

                  script += OUT1name + " ist: ";
                  if (OUT1 == 1)
                  {
                     script += ("SCHARF &nbsp; <wbr> <wbr> ");
                  }
                  else if (OUT1 == -1)
                  {  // test, debug
                     script += ("REV &nbsp;&nbsp;&nbsp;&nbsp; <wbr> <wbr> ");
                  }
                  else
                  {
                     script += ("AUS &nbsp;&nbsp;&nbsp;&nbsp; <wbr> <wbr> ");
                  }

                  script += "<a href=\" /OUT1=ON\"\"> <button style=\"height:70px;width:140px\" > SCHARF </button></a>  ";
                  script += "<a href=\" /OUT1=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a>  ";
                  script += "<br> \n\n";

                  script += (OUT2name + " ist: ");
                  if (OUT2 == 1)
                  {
                     script += ("SCHARF &nbsp; <wbr> <wbr> ");
                  }
                  else
                  {
                     script += ("AUS &nbsp;&nbsp;&nbsp;&nbsp; <wbr> <wbr> ");
                  }

                  script += ("<a href=\" /OUT2=ON\"\"> <button style=\"height:70px;width:140px\" > SCHARF </button></a>  ");
                  script += ("<a href=\" /OUT2=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
                  script += ("<br> \n\n");


                  script += "</h3> ";
                  script += "</br> ";

                  script += "\n";
                  // The HTTP response ends with another blank line:
                  script += "\n";
                  /*
                                    client.print(script);
                                    script="";
                  */
                  //---------------------------------------
                  // sensors  Server
                  // chart table
                  //---------------------------------------
                  // text font Courier, color black
                  script += ("<p> <font face=\"courier\"> "); // <<< Courier
                  script += "<h2> ";
                  script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;
                  script += ("<br> \n");
                  script += "<table border=4 cellpadding=4>"; // "<table>";

                  script +=    "<caption>  Messwerte " + SERVERname + "  </caption> ";

                  // reset
                  script += ("<a href=\" /svreset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");


                  script +=     "<thead> ";
                  // Zeile 1 Server
                  script +=      "<tr> ";
                  script +=         "<td bgcolor='Peru'> " + svSECT1name + " </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
                  script +=         "<td bgcolor='White'> rF%  </td>";
                  script +=         "<td bgcolor='Orange'>" + (String)A0intname + " % </td>";
                  script +=         "<td bgcolor='Orange'>" + (String)"&nbsp;∅&nbsp;" + "</td>";
                  script +=         "<td bgcolor='Fuchsia'>" + (String)A0muxname + " </td>";
                  script +=         "<td bgcolor='Fuchsia'>" + (String)A1muxname + " </td>";
                  script +=     "</tr> ";
                  script +=     "</thead> ";

                  script +=     "<tbody> ";

                  // Zeile 2 Server
                  script +=       "<tr> ";
                  script +=         (String)"<th>" + svt1.sact  + " °C  </th> ";
                  script +=         (String)"<th>" + svt1.smin   + "</th> ";
                  script +=         (String)"<th>" + svt1.smax   + "</th> ";
                  script +=         (String)"<th>" + svh1.sact + "</th> ";

                  if (svespA0.vact<fFireLIMIT) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=                  (String)svespA0.sact + "</th> ";
                  script +=           "<th>"+(String)(int)round(svespA0.vmean) + "</th> ";

                  script +=         "<th>" + (String)(" - ")  + "</th> ";
                  script +=         "<th>" + (String)(" - ")  + "</th> ";
                  "<th>" + (String)("")  + "</th> ";
                  script +=       "</tr> ";


                  script +=     "</tbody> ";

                  script +=   "</table>  ";
                  script += "</h2>";

                  script += ("<br> \n");
                  script += ("<br> \n");
                  /*
                                    client.print(script);
                                    script = "";
                  */


                  //---------------------------------------
                  // Client 0
                  //---------------------------------------

                  //---------------------------------------
                  //script +=  "<h1> <br> \n  C0: GEW.-HAUS  <br> \n </h1>";
                  script +=  "<p><font style=\"color:rgb(0,205,102);\" > </p>";
                  script +=  "<h1> <br> \n" + CLIENT0name + "<br> \n </h1>";
                  script +=  "<p><font style=\"color:rgb(0,0,0);\" > </p>";
                  //---------------------------------------
                  // remote buttons Client 0
                  //---------------------------------------

                  // <input type="button" value="submit" style="height: 100px; width: 100px; left: 250; top: 250;">
                  // <button style=\"height:200px;width:200px\"> </button>

                  script += (c0OUT1name + " ist: "); // script+= (" <h2>" + OUT1name + " ist: ");
                  if (c0out1 == 1)
                  {
                     script += ("VOR &nbsp; <wbr> <wbr> ");
                  }
                  else if (c0out1 == -1)
                  {
                     script += ("REV &nbsp; <wbr> <wbr> ");
                  }
                  else
                  {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                  }
                  script += ("<a href=\" /c0out1=ON\"\"> <button style=\"height:70px;width:140px\" > ÖFFNEN </button></a>  ");
                  script += ("<a href=\" /c0out1=OFF\"\"> <button style=\"height:70px;width:140px\" >  STOP  </button></a>  ");
                  script += ("<a href=\" /c0out1=REV\"\"> <button style=\"height:70px;width:140px\" > SCHLIESSEN </button></a> "); // <br/>
                  script += ("<br> \n\n");


                  script += (c0OUT2name + " ist: ");
                  if (c0out2 == 1)
                  {
                     script += ("EIN &nbsp; <wbr> <wbr> ");
                  }
                  else
                  {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                  }
                  script += ("<a href=\" /c0out2=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
                  script += ("<a href=\" /c0out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
                  script += ("<br> \n\n");


                  script += (c0OUT3name + " ist: ");
                  if (c0out3 == 1)
                  {
                     script += ("EIN &nbsp; <wbr> <wbr> ");
                  }
                  else
                  {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                  }
                  script += ("<a href=\" /c0out3=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
                  script += ("<a href=\" /c0out3=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");

                  /*
                                    client.print(script);
                                    script = "";
                  */


                  //---------------------------------------
                  // sensors  Client 0
                  // chart table
                  //---------------------------------------

                  // text font Courier, color black
                  script += ("<p> <font face=\"courier\"> "); // <<< Courier
                  script += "<h2> ";
                  //script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;  // <<<<<<<<<<<<<<<<<
                  script += "<style=\"color:rgb(0,0,0);\" > </p>  " ;  // <<<<<<<<<<<<<<<<<
                  script += ("<br> \n");
                  script += "<table border=4 cellpadding=4>"; // "<table>";

                  script +=  "<caption>  Messwerte " + CLIENT0name;
                  script +=  "(Verb.-Fehler: " + (String)( c0t1.tFail )  + " min)" ;
                  script +=  "</caption>" ;

                  // reset
                  script += ("<a href=\" /c0reset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");

                  script +=     "<thead> ";
                  // Zeile 1 Client 0
                  script +=      "<tr> ";
                  script +=         "<td bgcolor='Peru'> " + c0SECT1name + " </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
                  script +=         "<td bgcolor='White'> rF%  </td>";
                  script +=         "<td bgcolor='Avocado'>  " + c0SECT2name + " </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
                  script +=         "<td bgcolor='White'> rF%  </td>";
                  script +=         "<td bgcolor='White'> &nbsp;&nbsp; </td>";
                  script +=         "<td bgcolor='White'> &nbsp;&nbsp; </td>";
                  script +=     "</tr> ";
                  script +=     "</thead> ";

                  script +=     "<tbody> ";

                  // Zeile 2 Client 0
                  script +=       "<tr> ";
                  script +=         (String)"<th>" + c0t1.sact  + " °C  </th> ";
                  script +=         (String)"<th>" + c0t1.smin   + "</th> ";
                  script +=         (String)"<th>" + c0t1.smax   + "</th> ";
                  script +=         (String)"<th>" + c0h1.sact + "</th> ";
                  script +=         (String)"<th>" + c0t2.sact  + " °C  </th> ";
                  script +=         (String)"<th>" + c0t2.smin   + "</th> ";
                  script +=         (String)"<th>" + c0t2.smax  + "</th> ";
                  script +=         (String)"<th>" + c0h2.sact + "</th> ";
                  script +=         "<th>   &nbsp;   </th> ";
                  script +=         "<th>   &nbsp;   </th> ";
                  script +=       "</tr> ";

                  /*
                                    client.print(script);
                                    script = "";
                  */


                  // Zeile 3 Client 0
                  script +=       "<tr> ";
                  script +=         "<td bgcolor='Yellow'> hPa </td>";
                  script +=         "<td bgcolor='Yellow'> &nbsp; ±  </td>";
                  script +=         "<td bgcolor='Yellow'> hPa ∅ </td>";
                  script +=         "<td bgcolor='White'>  &nbsp;  </td>";

                  script +=         "<td bgcolor='Avocado'>" + (String)c0A0muxname + "</td>";
                  script +=         "<td bgcolor='Avocado'>" + (String)c0A1muxname + "</td>";
                  script +=         "<td bgcolor='Avocado'>" + (String)c0A2muxname + "</td>";
                  script +=         "<td bgcolor='Avocado'>" + (String)c0A3muxname + "</td>";
                  script +=         "<td bgcolor='Orange'>" + (String)c0A0intname + "</td>";
                  script +=         "<td bgcolor='Orange'>" + (String)"&nbsp;∅&nbsp;" + "</td>";
                  script +=       "</tr> ";

                  // Zeile 4 Client 0

                  strcpy(dsymbol, tendencysymbol(c0p1.vact - c0p1.vmean).c_str() );
                  script +=  "<tr> ";
                  script +=      "<th>" + (String)(int)round(c0p1.vact) + "</th> ";
                  script +=      "<th> " + (String)dsymbol  + " </th> ";
                  script +=      "<th>" + (String)(int)round(c0p1.vmean) + " </th> ";
                  script +=      "<th>" + (String)(" &nbsp; ")  + "</th> ";

                  if (c0adc0.tFail>60 || c0adc0.vmean<adcSoilMin) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=               (String)c0adc0.sact + "</th> ";

                  if (c0adc1.tFail>60 || c0adc1.vmean<adcSoilMin) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=               (String)c0adc1.sact + "</th> ";

                  if (c0adc2.tFail>60 || c0adc2.vmean<adcSoilMin) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=               (String)c0adc2.sact + "</th> ";

                  if (c0adc3.tFail>60 || c0adc3.vmean<adcSoilMin) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=               (String)c0adc3.sact + "</th> ";

                  if (c0espA0.vact<fFireLIMIT) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=                  (String)c0espA0.sact + "</th> ";
                  script +=           "<th>"+(String)(int)round(c0espA0.vmean) + "</th> ";

                  script +=    "</tr> ";

                  script +=  "</tbody> ";

                  script +=   "</table> ";
                  script += "</h2>";

                  script += ("<br> \n");
                  script += ("<br> \n");

                  /*
                     client.print(script);
                     script = "";
                  */


                  //---------------------------------------
                  // Client 1
                  //---------------------------------------

                  //---------------------------------------
                  // text font Courier, color black
                  script += ("<p> <font face=\"courier\"> "); // <<< Courier
                  //script += ("<font style=\"color:rgb(0,0,0);\" > ");
                  script += ("<font style=\"color:rgb(0,0,0);\" > </p> ");
                  //---------------------------------------
                  //script +=  "<h1> <br> \n  C1: KELLER <br> \n </h1>";
                  script +=  "<p><font style=\"color:rgb(0,205,102);\" > </p>";
                  script +=  "<h1> <br> \n" + CLIENT1name + "<br> \n </h1>";
                  script +=  "<p><font style=\"color:rgb(0,0,0);\" > </p>";

                  //---------------------------------------
                  // remote buttons Client C1
                  //---------------------------------------

                  /*
                     script += (c1OUT1name + " ist: "); // script+= (" <h2>" + c0OUT1name + " ist: ");
                     if (c1out1 == 1)
                     {
                     script += ("VOR &nbsp; <wbr> <wbr> ");
                     }
                     else if (c1out1 == -1)
                     {
                     script += ("REV &nbsp; <wbr> <wbr> ");
                     }
                     else
                     {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                     }
                     script += ("<a href=\" /c1out1=ON\"\"> <button style=\"height:70px;width:140px\" > ÖFFNEN </button></a>  ");
                     script += ("<a href=\" /c1out1=OFF\"\"> <button style=\"height:70px;width:140px\" >  STOP  </button></a>  ");
                     script += ("<a href=\" /c1out1=REV\"\"> <button style=\"height:70px;width:140px\" > SCHLIESSEN </button></a> "); // <br/>
                     script += ("<br> \n\n");
                  */


                  script += (c1OUT2name + " ist: ");
                  if (c1out2 == 1)
                  {
                     script += ("EIN &nbsp; <wbr> <wbr> ");
                  }
                  else if (c1out2 == -1)
                  {
                     script += ("Rev &nbsp; <wbr> <wbr> ");
                  }
                  else if (c1out2 == 0)
                  {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                  }
                  script += ("<a href=\" /c1out2=ON\"\"> <button style=\"height:70px;width:140px\" > ÖFFNEN </button></a>  ");
                  script += ("<a href=\" /c1out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  STOP  </button></a>  ");
                  script += ("<a href=\" /c1out2=REV\"\"> <button style=\"height:70px;width:140px\" > SCHLIESSEN </button></a> "); // <br/>
                  script += ("<br> \n\n");

                  /*
                     script += (c0OUT3name + " ist: ");
                     if (c1out3 == 1)
                     {
                     script += ("EIN &nbsp; <wbr> <wbr> ");
                     }

                     else if (OUT3 == -1)
                     {
                     script += ("Rev &nbsp; <wbr> <wbr> ");
                     }
                     else
                     {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                     }
                     script += ("<a href=\" /c1out3=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
                     script += ("<a href=\" /c1out3=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
                     script += ("<br> \n");

                  */
                  /*
                     client.print(script);
                     script = "";
                  */

                  // remote client 1 sensors
                  // chart table
                  //---------------------------------------
                  // text font Courier, color black
                  script += ("<p> <font face=\"courier\"> "); // <<< Courier
                  script += "<h2> ";
                  //script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;
                  script += "<style=\"color:rgb(0,0,0);\" > </p>  " ;
                  script += ("<br> \n");
                  script += "<table border=4 cellpadding=4>"; // "<table>";

                  script +=  "<caption>  Messwerte " + CLIENT1name ;
                  script +=  "(Verb.-Fehler: " + (String)( c1t1.tFail )  + " min)" ;
                  script +=  "</caption>" ;

                  // reset
                  script += ("<a href=\" /c1reset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");


                  // client 1 Zeile 1
                  script +=     "<thead> ";
                  script +=       "<tr> ";
                  script +=       "<td bgcolor='Teal'> " + c1SECT1name + "  </td>";
                  script +=       "<td bgcolor='Yellow'> °Cmin  </td>";
                  script +=       "<td bgcolor='Yellow'> °Cmax  </td>";
                  script +=       "<td bgcolor='Yellow'> °C ∅  </td>";
                  script +=       "<td bgcolor='White'> rF%   </td>";
                  script +=       "<td bgcolor='LightCyan'>  " + c1SECT2name + " </td>";
                  script +=       "<td bgcolor='Yellow'> °Cmin </td>";
                  script +=       "<td bgcolor='Yellow'> °Cmax </td>";
                  script +=       "<td bgcolor='White'> rF%  </td>";
                  script +=       "<td bgcolor='Orange'>" + (String)c1A0intname + "</td>";
                  script +=       "<td bgcolor='Orange'>" + (String)"&nbsp;∅&nbsp;" + "</td>";
                  script +=       "</tr> ";
                  script +=     "</thead> ";

                  // client 1 Zeile 2
                  script +=     "<tbody> ";
                  script +=       "<tr> ";
                  if (c1t1.tFail > 60 || c1t1.vmean > -4.0 ) {
                     script += (String)"<td bgcolor='red'>";
                  } else
                     script += (String)"<th>";
                  script +=         (String)c1t1.sact  + " °C </th> ";
                  script +=         (String)"<th>" + c1t1.smin   + "</th> ";
                  script +=         (String)"<th>" + c1t1.smax   + "</th> ";
                  script +=         (String)"<th>" + c1t1.smean  + "</th> ";
                  script +=         (String)"<th>" + c1h1.sact + "</th> ";
                  script +=         (String)"<th>" + c1t2.sact  + " °C </th> ";
                  script +=         (String)"<th>" + c1t2.smin   + "</th> ";
                  script +=         (String)"<th>" + c1t2.smax   + "</th> ";
                  script +=         (String)"<th>" + c1h2.sact + "</th> ";

                  if (c1espA0.vact<fFireLIMIT) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=                  (String)c1espA0.sact + "</th> ";
                  script +=           "<th>"+(String)(int)round(c1espA0.vmean) + "</th> ";

                  script +=       "</tr> ";



                  script +=     "</tbody> ";
                  script +=   "</table>  ";
                  script += "</h2> ";


                  script += ("<br> \n");
                  script += ("<br> \n");

                  /*
                     client.print(script);
                     script = "";
                  */


                  //---------------------------------------
                  // Client 2
                  //---------------------------------------

                  //---------------------------------------
                  // text font Courier, color black
                  script += ("<p> <font face=\"courier\"> "); // <<< Courier
                  //script += ("<font style=\"color:rgb(0,0,0);\" > ");
                  script += ("<font style=\"color:rgb(0,0,0);\" > </p> ");
                  //---------------------------------------
                  //script +=  "<h1> <br> \n  C2: KÜCHE <br> \n </h1>";
                  script +=  "<p><font style=\"color:rgb(0,205,102);\" > </p>";
                  script +=  "<h1> <br> \n" + CLIENT2name + "<br> \n </h1>";
                  script +=  "<p><font style=\"color:rgb(0,0,0);\" > </p>";
                  //---------------------------------------


                  //---------------------------------------
                  // remote buttons Client C2
                  //---------------------------------------
                  /*
                     script += (c2OUT2name + " ist: "); // script+= (" <h2>" + "Check" + " ist: ");
                     if (c2out2 == 1)
                     {
                     script += ("EIN &nbsp; <wbr> <wbr> ");
                     }
                     else
                     {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                     }
                     script += ("<a href=\" /c2out2=ON\"\"> <button style=\"height:70px;width:140px\" >  EIN  </button></a>  ");
                     script += ("<a href=\" /c2out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a>  ");
                     script += ("<br> \n\n");
                  */

                  script += (c2OUT2name + " ist: ");
                  if (c2out2 == 1)
                  {
                     script += ("EIN &nbsp; <wbr> <wbr> ");
                  }
                  else if (c2out2 == -1)
                  {
                     script += ("Rev &nbsp; <wbr> <wbr> ");
                  }
                  else if (c2out2 == 0)
                  {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                  }

                  script += ("<a href=\" /c2out2=ON\"\"> <button style=\"height:70px;width:140px\" > ÖFFNEN </button></a>  ");
                  script += ("<a href=\" /c2out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  STOP  </button></a>  ");
                  script += ("<a href=\" /c2out2=REV\"\"> <button style=\"height:70px;width:140px\" > SCHLIESSEN </button></a> "); // <br/>
                  script += ("<br> \n\n");

                  //---------------------------------------
                  // remote client 2 sensors
                  // chart table
                  //---------------------------------------
                  // text font Courier, color black
                  script += ("<p> <font face=\"courier\"> "); // <<< Courier
                  script += "<h2> ";
                  //script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;
                  script += "<style=\"color:rgb(0,0,0);\" > </p>  " ;
                  script += ("<br> \n");
                  script += "<table border=4 cellpadding=4>"; // "<table>";

                  script +=  "<caption>  Messwerte " + CLIENT2name ;
                  script +=  "(Verb.-Fehler: " + (String)( c2t1.tFail )  + " min)"  ;
                  script +=  "</caption>" ;

                  // reset
                  script += ("<a href=\" /c2reset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");

                  // client 2 Zeile 1
                  script +=  "<thead> ";
                  script +=    "<tr> ";
                  script +=    "<td bgcolor='Teal'> " + c2SECT1name + "  </td>";
                  script +=    "<td bgcolor='Yellow'> °Cmin  </td>";
                  script +=    "<td bgcolor='Yellow'> °Cmax  </td>";
                  script +=    "<td bgcolor='Yellow'> °C ∅  </td>";
                  script +=    "<td bgcolor='White'> rF%   </td>";
                  script +=    "<td bgcolor='LightCyan'>  " + c2SECT2name + " </td>";
                  script +=    "<td bgcolor='Yellow'> °Cmin </td>";
                  script +=    "<td bgcolor='Yellow'> °Cmax </td>";
                  script +=    "<td bgcolor='White'> rF%  </td>";
                  script +=    "<td bgcolor='Orange'>" + (String)c2A0intname + "</td>";
                  script +=    "<td bgcolor='Orange'>" + (String)"&nbsp;∅&nbsp;" + "</td>";
                  script +=    "</tr> ";
                  script +=  "</thead> ";

                  // client 2 Zeile 2
                  script +=  "<tbody> ";
                  script +=    "<tr> ";
                  if (c2t1.tFail > 60 || c2t1.vmean > -4.0 ) {
                     script += (String)"<td bgcolor='red'>";
                  } else
                     script += (String)"<th>";
                  script +=      (String)c2t1.sact  + " °C </th> ";
                  script +=      (String)"<th>" + c2t1.smin   + "</th> ";
                  script +=      (String)"<th>" + c2t1.smax   + "</th> ";
                  script +=      (String)"<th>" + c2t1.smean  + "</th> ";
                  script +=      (String)"<th>" + c2h1.sact   + "</th> ";
                  script +=      (String)"<th>" + c2t2.sact  + " °C </th> ";
                  script +=      (String)"<th>" + c2t2.smin   + "</th> ";
                  script +=      (String)"<th>" + c2t2.smax   + "</th> ";
                  script +=      (String)"<th>" + c2h2.sact + "</th> ";

                  if (c2espA0.vact<fFireLIMIT) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=                  (String)c2espA0.sact + "</th> ";
                  script +=           "<th>"+(String)(int)round(c2espA0.vmean) + "</th> ";

                  script +=    "</tr> ";


                  script +=     "</tbody> ";
                  script +=   "</table>  ";
                  script += "</h2> ";

                  script += ("<br> \n");
                  script += ("<br> \n");

                  /*
                     client.print(script);
                     script = "";
                  */


                  //---------------------------------------
                  //  Client 3
                  //---------------------------------------

                  //---------------------------------------
                  // text font Courier, color black
                  script += ("<p> <font face=\"courier\"> "); // <<< Courier
                  //script += ("<font style=\"color:rgb(0,0,0);\" > ");
                  script += ("<font style=\"color:rgb(0,0,0);\" > </p> ");
                  //---------------------------------------
                  //script +=  "<h1> <br> \n  C3: GEW.-HAUS  <br> \n </h1>";
                  script +=  "<p><font style=\"color:rgb(0,205,102);\" > </p>";
                  script +=  "<h1> <br> \n" + CLIENT3name + "<br> \n </h1>";
                  script +=  "<p><font style=\"color:rgb(0,0,0);\" > </p>";
                  //---------------------------------------



                  //---------------------------------------
                  // remote buttons Client 3
                  //---------------------------------------

                  // <input type="button" value="submit" style="height: 100px; width: 100px; left: 250; top: 250;">
                  // <button style=\"height:200px;width:200px\"> </button>

                  script += (c3OUT1name + " ist: "); // script+= (" <h2>" + OUT1name + " ist: ");
                  if (c3out1 == 1)
                  {
                     script += ("EIN &nbsp; <wbr> <wbr> ");
                  }
                  else
                  {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                  }
                  script += ("<a href=\" /c3out1=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
                  script += ("<a href=\" /c3out1=OFF\"\"> <button style=\"height:70px;width:140px\" > AUS </button></a> "); // <br/>
                  script += ("<br> \n\n");


                  script += (c3OUT2name + " ist: ");
                  if (c3out2 == 1)
                  {
                     script += ("EIN &nbsp; <wbr> <wbr> ");
                  }
                  else
                  {
                     script += ("AUS &nbsp; <wbr> <wbr> ");
                  }
                  script += ("<a href=\" /c3out2=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
                  script += ("<a href=\" /c3out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
                  script += ("<br> \n\n");

                  //----------------------
                  // Thermostat
                  //----------------------

                  script += ("c3-Thermost= ");
                  sprintf(istr, "%+3d", c3tx3);
                  script += (String)( istr ) ;
                  script += (" &nbsp &nbsp ");
                  script += ("<a href=\" /c3tx3=UP\"\"> <button style=\"height:70px;width:140px\" >Therm +1</button></a>  ");
                  script += ("<a href=\" /c3tx3=DN\"\"> <button style=\"height:70px;width:140px\" >Therm -1</button></a> ");
                  script += ("<br> \n\n");



                  //---------------------------------------
                  // remote client 3 sensors
                  // chart table
                  //---------------------------------------

                  // text font Courier, color black
                  script += ("<p> <font face=\"courier\"> "); // <<< Courier
                  script += "<h2> ";
                  //script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;  // <<<<<<<<<<<<<<<<<
                  script += "<style=\"color:rgb(0,0,0);\" > </p>  " ;  // <<<<<<<<<<<<<<<<<
                  script += ("<br> \n");
                  script += "<table border=4 cellpadding=4>"; // "<table>";

                  script +=  "<caption>  Messwerte " + CLIENT3name ;
                  script +=  "(Verb.-Fehler: " + (String)( c3t1.tFail )  + " min)" ;
                  script +=  "</caption>" ;

                  // reset
                  script += ("<a href=\" /c3reset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");


                  script +=     "<thead> ";
                  // Zeile 1 Client 3
                  script +=      "<tr> ";
                  script +=         "<td bgcolor='Peru'> " + c3SECT1name + " </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
                  script +=         "<td bgcolor='White'> rF%  </td>";
                  script +=         "<td bgcolor='Avocado'>  " + c3SECT2name + " </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
                  script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
                  script +=         "<td bgcolor='White'> rF%  </td>";
                  script +=         "<td bgcolor='White'> &nbsp;&nbsp; </td>";
                  script +=         "<td bgcolor='White'> &nbsp;&nbsp; </td>";
                  script +=     "</tr> ";
                  script +=     "</thead> ";

                  script +=     "<tbody> ";

                  // Zeile 2 Client 3
                  script +=       "<tr> ";
                  script +=         (String)"<th>" + c3t1.sact  + " °C  </th> ";
                  script +=         (String)"<th>" + c3t1.smin   + "</th> ";
                  script +=         (String)"<th>" + c3t1.smax   + "</th> ";
                  script +=         (String)"<th>" + c3h1.sact + "</th> ";
                  script +=         (String)"<th>" + c3t2.sact  + " °C  </th> ";
                  script +=         (String)"<th>" + c3t2.smin   + "</th> ";
                  script +=         (String)"<th>" + c3t2.smax  + "</th> ";
                  script +=         (String)"<th>" + c3h2.sact + "</th> ";
                  script +=         "<th>   &nbsp;   </th> ";
                  script +=         "<th>   &nbsp;   </th> ";
                  script +=       "</tr> ";

                  // Zeile 3 Client 3
                  script +=       "<tr> ";
                  script +=         "<td bgcolor='White'>   - </td>";
                  script +=         "<td bgcolor='LightGray'> c3out1 </td>";
                  script +=         "<td bgcolor='LightGray'> c3out2 </td>";
                  script +=         "<td bgcolor='LightGray'> c3out3 </td>";

                  script +=         "<td bgcolor='Avocado'>" + (String)c3A0muxname + "</td>";
                  script +=         "<td bgcolor='Avocado'>" + (String)c3A1muxname + "</td>";
                  script +=         "<td bgcolor='Avocado'>" + (String)c3A2muxname + "</td>";
                  script +=         "<td bgcolor='Avocado'>" + (String)c3A3muxname + "</td>";
                  script +=         "<td bgcolor='Orange'>"  + (String)c3A0intname + "</td>";
                  script +=         "<td bgcolor='Orange'>"  + (String)"&nbsp;∅&nbsp;" + "</td>";
                  script +=       "</tr> ";

                  // Zeile 4 Client 3
                  //strcpy(dsymbol, tendencysymbol(c3p1.vact - c3p1.vmean).c_str() );
                  script +=  "<tr> ";
                  script +=      "<th>" + (String)"  -  " + "</th> ";
                  script +=      "<th>" + (String)(c3outMon1)  + "</th> ";
                  script +=      "<th>" + (String)(c3outMon2)  + "</th> ";
                  script +=      "<th>" + (String)(c3outMon3)  + "</th> ";

                  if (c3adc0.tFail>60 || c3adc0.vmean<adcSoilMin) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=               (String)c3adc0.sact + "</th> ";

                  if (c3adc1.tFail>60 || c3adc1.vmean<adcSoilMin) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=               (String)c3adc1.sact + "</th> ";

                  if (c3adc2.tFail>60 || c3adc2.vmean<adcSoilMin) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=               (String)c3adc2.sact + "</th> ";

                  if (c3adc3.tFail>60 || c3adc3.vmean<adcSoilMin) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=               (String)c3adc3.sact + "</th> ";


                  if (c3espA0.vact<fFireLIMIT) {
                     script += (String)"<td bgcolor='red'>";
                  } else script += (String)"<th>";
                  script +=                  (String)c3espA0.sact + "</th> ";
                  script +=           "<th>"+(String)(int)round(c3espA0.vmean) + "</th> ";

                  script +=    "</tr> ";

                  script +=  "</tbody> ";

                  script +=   "</table> ";
                  script += "</h2>";

                  script += ("<br> \n");
                  script += ("<br> \n");

                  /*
                     client.print(script);
                     script = "";
                  */


                  //---------------------------------------
                  // log out
                  //---------------------------------------
                  script += ("<h3>Log Out: ");
                  script += ("<a href=\" /logout\"\"> <button style=\"height:70px;width:140px\" > Log Out </button></a> </h3> ");

                  script += ver + " " + WiFi.localIP().toString() + " " + (String)ssid + " <br>" ;
                  //script += "</font> </p> \n";
                  script += "</font>   \n";

                  script += "</body> \n";
                  script += "</html> \n";

                  script += ("<br> \n");
                  script += ("<br> \n");

                  Serial.println((String)"\n script.length()="+script.length());
                  client.print(script);
                  script = "";






                  delay(1);

                  // break out of the while loop:
                  break;

               }
               else {    // if you got a newline, then clear currentLine:
                  currentLine = "";
               }
            }
            else if (c != '\r') {  // if you got anything else but a carriage return character,
               currentLine += c;      // add it to the end of the currentLine
            }
            delay(1);


            //---------------------------------------
            //-----  react on website widgets  ------
            //---------------------------------------

            //---------------------------------------
            // Match the request on Server
            // if (currentLine.endsWith("GET ... ")
            //---------------------------------------

            // confirm user active
            if (currentLine.endsWith("GET /CONFIRM=ON") )  {
               resetConfirmTime();
            }

            if (currentLine.endsWith("GET /OUT1=ON")  ) {
               digitalWrite(PIN_OUT1, 1);
               OUT1 = +1;
            }
            if (currentLine.endsWith("GET /OUT1=OFF") ) {
               digitalWrite(PIN_OUT1, 0);
               OUT1 = 0;
            }


            if (currentLine.endsWith("GET /OUT2=ON")  ) {
               digitalWrite(PIN_OUT2, 1);
               OUT2 = +1;
            }
            if (currentLine.endsWith("GET /OUT2=OFF") ) {
               digitalWrite(PIN_OUT2, 0);
               OUT2 = 0;
            }

            // Reset
            if (currentLine.endsWith("GET /svreset") )  {
               resetMinMaxValues(svt1);
               resetMinMaxValues(svt2);
            }


            //---------------------------------------
            // Match the request on Client 0
            //---------------------------------------

            if (currentLine.endsWith("GET /c0out1=ON") ) {
               c0out1 = +1;
            }
            if (currentLine.endsWith("GET /c0out1=OFF") ) {
               c0out1 = 0;
            }
            if (currentLine.endsWith("GET /c0out1=REV") ) {
               c0out1 = -1;
            }


            if (currentLine.endsWith("GET /c0out2=ON") ) {
               c0out2 = +1;
            }
            if (currentLine.endsWith("GET /c0out2=OFF") ) {
               c0out2 = 0;
            }


            if (currentLine.endsWith("GET /c0out3=ON") ) {
               c0out3 = +1;
            }
            if (currentLine.endsWith("GET /c0out3=OFF") ) {
               c0out3 = 0;
            }

            // Reset
            if (currentLine.endsWith("GET /c0reset") )  {
               resetMinMaxValues(c0t1);
               resetMinMaxValues(c0t2);
            }


            //---------------------------------------
            // Match the request for Client 1
            //---------------------------------------

            if (currentLine.endsWith("GET /c1out1=ON") ) {
               c1out1 = +1;
            }
            if (currentLine.endsWith("GET /c1out1=OFF") ) {
               c1out1 = 0;
            }


            if (currentLine.endsWith("GET /c1out2=ON") ) {
               c1out2 = +1;
            }
            if (currentLine.endsWith("GET /c1out2=OFF") ) {
               c1out2 = 0;
            }
            if (currentLine.endsWith("GET /c1out2=REV") ) {
               c1out2 = -1;
            }


            if (currentLine.endsWith("GET /c1out3=ON") ) {
               c1out3 = +1;
            }
            if (currentLine.endsWith("GET /c1out3=OFF") ) {
               c1out3 = 0;
            }


            // Reset
            if (currentLine.endsWith("GET /c1reset") )  {
               resetMinMaxValues(c1t1);
               resetMinMaxValues(c1t2);
            }


            //---------------------------------------
            // Match the request for Client 2
            //---------------------------------------

            if (currentLine.endsWith("GET /c2out1=ON") ) {
               c2out1 = +1;
            }
            if (currentLine.endsWith("GET /c2out1=OFF") ) {
               c2out1 = 0;
            }


            if (currentLine.endsWith("GET /c2out2=ON") ) {
               c2out2 = +1;
            }
            if (currentLine.endsWith("GET /c2out2=OFF") ) {
               c2out2 = 0;
            }
            if (currentLine.endsWith("GET /c2out2=REV") ) {
               c2out2 = -1;
            }

            // Reset
            if (currentLine.endsWith("GET /c2reset") )  {
               resetMinMaxValues(c2t1);
               resetMinMaxValues(c2t2);
            }


            //---------------------------------------
            // Match the request for Client 3
            //---------------------------------------

            if (currentLine.endsWith("GET /c3out1=ON") ) {
               c3out1 = +1;
            }
            if (currentLine.endsWith("GET /c3out1=OFF") ) {
               c3out1 = 0;
            }


            if (currentLine.endsWith("GET /c3out2=ON") ) {
               c3out2 = +1;
            }
            if (currentLine.endsWith("GET /c3out2=OFF") ) {
               c3out2 = 0;
            }


            if (currentLine.endsWith("GET /c3tx3=UP") ) {
               c3tx3 = c3tx3+1;
            }
            if (currentLine.endsWith("GET /c3tx3=DN") ) {
               c3tx3 = c3tx3-1;
            }

            // Reset
            if (currentLine.endsWith("GET /c3reset") )  {
               resetMinMaxValues(c3t1);
               resetMinMaxValues(c3t2);
            }


            //---------------------------------------
            // LogOut
            //---------------------------------------
            if (currentLine.endsWith("GET /logout") )  {
               authorized = false;
               break;
            }

         }  //  if client.available
      }  //  while client.connected
      

      // close the connection:
      client.stop();
      delay(1);
      Serial.println("Client Disconnected.");
   }
   delay(5);

}








//----------------------------------------------------------------------------

void handleEsp8266site() {
   char istr[5];
   String Str;
   WiFiClient client = wifiserver.available();


   //---------------------------------------
   // Check if a client has connected
   if (!client) {
      return;
   }

   // Read the first line of the request
   String request = client.readStringUntil('\r');
   Serial.println(request);
   client.flush();

   //---------------------------------------
   // Match the request on Server
   //---------------------------------------

   if ((request.indexOf("/OUT1=ON") != -1)  ) {
      digitalWrite(PIN_OUT1, 1);
      OUT1 = +1;
   }
   if ((request.indexOf("/OUT1=OFF") != -1) ) {
      digitalWrite(PIN_OUT1, 0);
      OUT1 = 0;
   }


   if ((request.indexOf("/OUT2=ON") != -1)  ) {
      digitalWrite(PIN_OUT2, 1);
      OUT2 = +1;
   }
   if ((request.indexOf("/OUT2=OFF") != -1) ) {
      digitalWrite(PIN_OUT2, 0);
      OUT2 = 0;
   }

   // Reset
   if (request.indexOf("/svreset") != -1)  {
      resetMinMaxValues(svt1);
      resetMinMaxValues(svt2);
   }


   //---------------------------------------
   // Match the request on Client 0
   //---------------------------------------

   if ((request.indexOf("/c0out1=ON") != -1)  ) {
      c0out1 = +1;
   }
   if ((request.indexOf("/c0out1=OFF") != -1) ) {
      c0out1 = 0;
   }
   if ((request.indexOf("/c0out1=REV") != -1) ) {
      c0out1 = -1;
   }


   if ((request.indexOf("/c0out2=ON") != -1)  ) {
      c0out2 = +1;
   }
   if ((request.indexOf("/c0out2=OFF") != -1) ) {
      c0out2 = 0;
   }


   if ((request.indexOf("/c0out3=ON") != -1)  ) {
      c0out3 = +1;
   }
   if ((request.indexOf("/c0out3=OFF") != -1) ) {
      c0out3 = 0;
   }

   // Reset
   if (request.indexOf("/c0reset") != -1)  {
      resetMinMaxValues(c0t1);
      resetMinMaxValues(c0t2);
   }


   //---------------------------------------
   // Match the request for Client 1
   //---------------------------------------

   if ((request.indexOf("/c1out1=ON") != -1)  ) {
      c1out1 = +1;
   }
   if ((request.indexOf("/c1out1=OFF") != -1) ) {
      c1out1 = 0;
   }


   if ((request.indexOf("/c1out2=ON") != -1)  ) {
      c1out2 = +1;
   }
   if ((request.indexOf("/c1out2=OFF") != -1) ) {
      c1out2 = 0;
   }
   if ((request.indexOf("/c1out2=REV") != -1) ) {
      c1out2 = -1;
   }


   if ((request.indexOf("/c1out3=ON") != -1)  ) {
      c1out3 = +1;
   }
   if ((request.indexOf("/c1out3=OFF") != -1) ) {
      c1out3 = 0;
   }


   // Reset
   if (request.indexOf("/c1reset") != -1)  {
      resetMinMaxValues(c1t1);
      resetMinMaxValues(c1t2);
   }


   //---------------------------------------
   // Match the request for Client 2
   //---------------------------------------

   if ((request.indexOf("/c2out1=ON") != -1)  ) {
      c2out1 = +1;
   }
   if ((request.indexOf("/c2out1=OFF") != -1) ) {
      c2out1 = 0;
   }


   if ((request.indexOf("/c2out2=ON") != -1)  ) {
      c2out2 = +1;
   }
   if ((request.indexOf("/c2out2=OFF") != -1) ) {
      c2out2 = 0;
   }
   if ((request.indexOf("/c2out2=REV") != -1) ) {
      c2out2 = -1;
   }

   // Reset
   if (request.indexOf("/c2reset") != -1)  {
      resetMinMaxValues(c2t1);
      resetMinMaxValues(c2t2);
   }


   //---------------------------------------
   // Match the request for Client 3
   //---------------------------------------

   if ((request.indexOf("/c3out1=ON") != -1)  ) {
      c3out1 = +1;
   }
   if ((request.indexOf("/c3out1=OFF") != -1) ) {
      c3out1 = 0;
   }


   if ((request.indexOf("/c3out2=ON") != -1)  ) {
      c3out2 = +1;
   }
   if ((request.indexOf("/c3out2=OFF") != -1) ) {
      c3out2 = 0;
   }


   if ((request.indexOf("/c3tx3=UP") != -1)  ) {
      c3tx3 = c3tx3+1;
   }
   if ((request.indexOf("/c3tx3=DN") != -1) ) {
      c3tx3 = c3tx3-1;
   }

   // Reset
   if (request.indexOf("/c3reset") != -1)  {
      resetMinMaxValues(c3t1);
      resetMinMaxValues(c3t2);
   }


   //---------------------------------------
   // LogOut
   //---------------------------------------
   if (request.indexOf("/logout") != -1)  {
      authorized = false;
      return;
   }

   // confirm

   if ((request.indexOf("/CONFIRM=ON") != -1)   )  {
      resetConfirmTime();
   }

   delay(1);


   //---------------------------------------
   // Return the response
   //---------------------------------------

   String script = "";

   // init website

   script += ("HTTP/1.1 200 OK \n");
   script += ("Content-Type: text/html \n");
   script += ("\n"); //  <<<<<<<<<<<<<<<<<<
   script += ("<!DOCTYPE html> \n");
   script += ("<html> \n");

   // head + title
   script += ("<head> \n");
   // autom. Aktualisierung alle 20 sec.

   script += "<meta http-equiv=\"refresh\" content=\"20; URL=";
   script += (String)website_url + ":" + (String)http_port + "\"> \n" ;


   // utf-8 für "°" Zeichen
   script += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> \n" ;
   script += ("<title>");
   script += (website_title);
   script += ("</title> \n");

   script += ("</head> \n");

   // body + caption
   script += ("<body> \n");
   script += ("<h1> <p> ");
   script += ("<font style=\"color:rgb(255,0,204);\"> DON'T PANIC ! ");
   script += ("&nbsp; <wbr> <wbr> ");
   script += ("<font style=\"color:rgb(0,205,102);\"> Welcome to " + (String)website_url );
   script += ("! </p> </h1>  ");

   // date + time
   script += "<h2><p style=\"color:rgb(0,205,102);\"> " ;
   script += datestr + " &nbsp; <wbr> <wbr> " + timestr + " &nbsp; <wbr> <wbr> <wbr> ";
   script += "<font style=\"color:rgb(0,0,0);\" >  ";

   script += ("<br>");
   script += "<font style=\"color:rgb(255,0,0);\"> " ;
   script += " Notfall Alarms: " + String(EmergencyCnt);
   script += "<font style=\"color:rgb(255,209,22);\"> " ;
   script += " &nbsp; <wbr> <wbr> <wbr> Erinnerungen: " + String(RemindCnt) + " ";
   script += "</p> </h2> ";

   delay(1);


   //---------------------------------------
   // text font Courier, color black
   //script += ("<p> <font face=\"courier\">"); // <<<<<<<<<<<<<<<< Courier
   script += "<p> "; // <<<<<<<<<<<<<<<<
   script += "<font style=\"color:rgb(0,0,0);\" > </p>";

   script += "<h3> " + (String)("letztes confirm (Std): &nbsp;") + (String)dhrsLastConfirm + " &nbsp;&nbsp;&nbsp; ";

   script += "<a href=\" /CONFIRM=ON\"\"> <button style=\"height:50px;width:100px\" > Confirm </button></a>";
   script += "</h3> ";

   //---------------------------------------
   // text font Courier, color black
   script += "<p> <font face=\"courier\">"; // <<<<<<<<<<<<<<<< Courier
   script += "<font style=\"color:rgb(0,0,0);\" > </p>";

   //---------------------------------------
   script +=  "<p><font style=\"color:rgb(0,205,102);\" > </p>";
   script +=  "<h1> <br> \n " + SERVERname + " <br> \n </h1>";
   script +=  "<p><font style=\"color:rgb(0,0,0);\" > </p>";

   //---------------------------------------
   // remote buttons Server
   // button style color https://de.wikihow.com/Die-Farbe-einer-Schaltfl%C3%A4che-in-HTML-%C3%A4ndern
   // <button style="background-color:red; border-color:blue; color:white">
   // https://www.w3schools.com/colors/colors_shades.asp
   // HTML LightGray    #D3D3D3  rgb(211,211,211)
   //---------------------------------------

   script += OUT1name + " ist: ";
   if (OUT1 == 1)
   {
      script += ("SCHARF &nbsp; <wbr> <wbr> ");
   }
   else if (OUT1 == -1)
   {  // test, debug
      script += ("REV &nbsp;&nbsp;&nbsp;&nbsp; <wbr> <wbr> ");
   }
   else
   {
      script += ("AUS &nbsp;&nbsp;&nbsp;&nbsp; <wbr> <wbr> ");
   }

   script += "<a href=\" /OUT1=ON\"\"> <button style=\"height:70px;width:140px\" > SCHARF </button></a>  ";
   script += "<a href=\" /OUT1=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a>  ";
   script += "<br> \n\n";


   script += (OUT2name + " ist: ");
   if (OUT2 == 1)
   {
      script += ("SCHARF &nbsp; <wbr> <wbr> ");
   }
   else
   {
      script += ("AUS &nbsp;&nbsp;&nbsp;&nbsp; <wbr> <wbr> ");
   }

   script += ("<a href=\" /OUT2=ON\"\"> <button style=\"height:70px;width:140px\" > SCHARF </button></a>  ");
   script += ("<a href=\" /OUT2=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
   script += ("<br> \n\n");

   /*
      script += (OUT3name + " ist: ");
      if (OUT3 == 1)
      {
      script += ("EIN &nbsp; <wbr> <wbr> ");
      }
      else
      if(OUT3 == -1)
      { script+= ("Rev &nbsp; <wbr> <wbr> ");  }

      else
      {
      script += ("AUS &nbsp; <wbr> <wbr> ");
      }
      script += ("<a href=\" /OUT3=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
      script += ("<a href=\" /OUT3=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
      script += ("<br> \n");
   */

   client.print(script);
   script = "";


   //---------------------------------------
   // sensors  Server
   // chart table
   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += "<h2> ";
   script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;
   script += ("<br> \n");
   script += "<table border=4 cellpadding=4>"; // "<table>";

   script +=    "<caption>  Messwerte " + SERVERname + "  </caption> ";

   // reset
   script += ("<a href=\" /svreset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");


   script +=     "<thead> ";
   // Zeile 1 Server
   script +=      "<tr> ";
   script +=         "<td bgcolor='Peru'> " + svSECT1name + " </td>";
   script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
   script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
   script +=         "<td bgcolor='White'> rF%  </td>";
   script +=         "<td bgcolor='Orange'>" + (String)A0intname + " % </td>";
   script +=         "<td bgcolor='Orange'>" + (String)"&nbsp;∅&nbsp;" + "</td>";
   script +=         "<td bgcolor='Fuchsia'>" + (String)A0muxname + " </td>";
   script +=         "<td bgcolor='Fuchsia'>" + (String)A1muxname + " </td>";
   script +=     "</tr> ";
   script +=     "</thead> ";

   script +=     "<tbody> ";

   // Zeile 2 Server
   script +=       "<tr> ";
   script +=         (String)"<th>" + svt1.sact  + " °C  </th> ";
   script +=         (String)"<th>" + svt1.smin   + "</th> ";
   script +=         (String)"<th>" + svt1.smax   + "</th> ";
   script +=         (String)"<th>" + svh1.sact + "</th> ";

   if (svespA0.vact<fFireLIMIT) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=                  (String)svespA0.sact + "</th> ";
   script +=           "<th>"+(String)(int)round(svespA0.vmean) + "</th> ";

   script +=         "<th>" + (String)(" - ")  + "</th> ";
   script +=         "<th>" + (String)(" - ")  + "</th> ";
   "<th>" + (String)("")  + "</th> ";
   script +=       "</tr> ";


   script +=     "</tbody> ";

   script +=   "</table>  ";
   script += "</h2>";

   script += ("<br> \n");
   script += ("<br> \n");

   client.print(script);

   script = "";





   //---------------------------------------
   // Client 0
   //---------------------------------------

   //---------------------------------------
   //script +=  "<h1> <br> \n  C0: GEW.-HAUS  <br> \n </h1>";
   script +=  "<p><font style=\"color:rgb(0,205,102);\" > </p>";
   script +=  "<h1> <br> \n" + CLIENT0name + "<br> \n </h1>";
   script +=  "<p><font style=\"color:rgb(0,0,0);\" > </p>";
   //---------------------------------------
   // remote buttons Client 0
   //---------------------------------------

   // <input type="button" value="submit" style="height: 100px; width: 100px; left: 250; top: 250;">
   // <button style=\"height:200px;width:200px\"> </button>

   script += (c0OUT1name + " ist: "); // script+= (" <h2>" + OUT1name + " ist: ");
   if (c0out1 == 1)
   {
      script += ("VOR &nbsp; <wbr> <wbr> ");
   }
   else if (c0out1 == -1)
   {
      script += ("REV &nbsp; <wbr> <wbr> ");
   }
   else
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
   }
   script += ("<a href=\" /c0out1=ON\"\"> <button style=\"height:70px;width:140px\" > ÖFFNEN </button></a>  ");
   script += ("<a href=\" /c0out1=OFF\"\"> <button style=\"height:70px;width:140px\" >  STOP  </button></a>  ");
   script += ("<a href=\" /c0out1=REV\"\"> <button style=\"height:70px;width:140px\" > SCHLIESSEN </button></a> "); // <br/>
   script += ("<br> \n\n");


   script += (c0OUT2name + " ist: ");
   if (c0out2 == 1)
   {
      script += ("EIN &nbsp; <wbr> <wbr> ");
   }
   else
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
   }
   script += ("<a href=\" /c0out2=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
   script += ("<a href=\" /c0out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
   script += ("<br> \n\n");


   script += (c0OUT3name + " ist: ");
   if (c0out3 == 1)
   {
      script += ("EIN &nbsp; <wbr> <wbr> ");
   }
   else
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
   }
   script += ("<a href=\" /c0out3=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
   script += ("<a href=\" /c0out3=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");

   client.print(script);
   script = "";


   //---------------------------------------
   // sensors  Client 0
   // chart table
   //---------------------------------------

   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += "<h2> ";
   //script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;  // <<<<<<<<<<<<<<<<<
   script += "<style=\"color:rgb(0,0,0);\" > </p>  " ;  // <<<<<<<<<<<<<<<<<
   script += ("<br> \n");
   script += "<table border=4 cellpadding=4>"; // "<table>";

   script +=  "<caption>  Messwerte " + CLIENT0name;
   script +=  "(Verb.-Fehler: " + (String)( c0t1.tFail )  + " min)" ;
   script +=  "</caption>" ;

   // reset
   script += ("<a href=\" /c0reset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");

   script +=     "<thead> ";
   // Zeile 1 Client 0
   script +=      "<tr> ";
   script +=         "<td bgcolor='Peru'> " + c0SECT1name + " </td>";
   script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
   script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
   script +=         "<td bgcolor='White'> rF%  </td>";
   script +=         "<td bgcolor='Avocado'>  " + c0SECT2name + " </td>";
   script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
   script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
   script +=         "<td bgcolor='White'> rF%  </td>";
   script +=         "<td bgcolor='White'> &nbsp;&nbsp; </td>";
   script +=         "<td bgcolor='White'> &nbsp;&nbsp; </td>";
   script +=     "</tr> ";
   script +=     "</thead> ";

   script +=     "<tbody> ";

   // Zeile 2 Client 0
   script +=       "<tr> ";
   script +=         (String)"<th>" + c0t1.sact  + " °C  </th> ";
   script +=         (String)"<th>" + c0t1.smin   + "</th> ";
   script +=         (String)"<th>" + c0t1.smax   + "</th> ";
   script +=         (String)"<th>" + c0h1.sact + "</th> ";
   script +=         (String)"<th>" + c0t2.sact  + " °C  </th> ";
   script +=         (String)"<th>" + c0t2.smin   + "</th> ";
   script +=         (String)"<th>" + c0t2.smax  + "</th> ";
   script +=         (String)"<th>" + c0h2.sact + "</th> ";
   script +=         "<th>   &nbsp;   </th> ";
   script +=         "<th>   &nbsp;   </th> ";
   script +=       "</tr> ";

   // Zeile 3 Client 0
   script +=       "<tr> ";
   script +=         "<td bgcolor='Yellow'> hPa </td>";
   script +=         "<td bgcolor='Yellow'> &nbsp; ±  </td>";
   script +=         "<td bgcolor='Yellow'> hPa ∅ </td>";
   script +=         "<td bgcolor='White'>  &nbsp;  </td>";

   script +=         "<td bgcolor='Avocado'>" + (String)c0A0muxname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c0A1muxname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c0A2muxname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c0A3muxname + "</td>";
   script +=         "<td bgcolor='Orange'>" + (String)c0A0intname + "</td>";
   script +=         "<td bgcolor='Orange'>" + (String)"&nbsp;∅&nbsp;" + "</td>";
   script +=       "</tr> ";

   // Zeile 4 Client 0

   strcpy(dsymbol, tendencysymbol(c0p1.vact - c0p1.vmean).c_str() );
   script +=  "<tr> ";
   script +=      "<th>" + (String)(int)round(c0p1.vact) + "</th> ";
   script +=      "<th> " + (String)dsymbol  + " </th> ";
   script +=      "<th>" + (String)(int)round(c0p1.vmean) + " </th> ";
   script +=      "<th>" + (String)(" &nbsp; ")  + "</th> ";

   if (c0adc0.tFail>60 || c0adc0.vmean<adcSoilMin) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c0adc0.sact + "</th> ";

   if (c0adc1.tFail>60 || c0adc1.vmean<adcSoilMin) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c0adc1.sact + "</th> ";

   if (c0adc2.tFail>60 || c0adc2.vmean<adcSoilMin) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c0adc2.sact + "</th> ";

   if (c0adc3.tFail>60 || c0adc3.vmean<adcSoilMin) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c0adc3.sact + "</th> ";

   if (c0espA0.vact<fFireLIMIT) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=                  (String)c0espA0.sact + "</th> ";
   script +=           "<th>"+(String)(int)round(c0espA0.vmean) + "</th> ";

   script +=    "</tr> ";

   script +=  "</tbody> ";

   script +=   "</table> ";
   script += "</h2>";

   script += ("<br> \n");
   script += ("<br> \n");

   client.print(script);

   script = "";

   //---------------------------------------
   // Client 1
   //---------------------------------------

   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   //script += ("<font style=\"color:rgb(0,0,0);\" > ");
   script += ("<font style=\"color:rgb(0,0,0);\" > </p> ");
   //---------------------------------------
   //script +=  "<h1> <br> \n  C1: KELLER <br> \n </h1>";
   script +=  "<p><font style=\"color:rgb(0,205,102);\" > </p>";
   script +=  "<h1> <br> \n" + CLIENT1name + "<br> \n </h1>";
   script +=  "<p><font style=\"color:rgb(0,0,0);\" > </p>";

   //---------------------------------------
   // remote buttons Client C1
   //---------------------------------------

   /*
      script += (c1OUT1name + " ist: "); // script+= (" <h2>" + c0OUT1name + " ist: ");
      if (c1out1 == 1)
      {
      script += ("VOR &nbsp; <wbr> <wbr> ");
      }
      else if (c1out1 == -1)
      {
      script += ("REV &nbsp; <wbr> <wbr> ");
      }
      else
      {
      script += ("AUS &nbsp; <wbr> <wbr> ");
      }
      script += ("<a href=\" /c1out1=ON\"\"> <button style=\"height:70px;width:140px\" > ÖFFNEN </button></a>  ");
      script += ("<a href=\" /c1out1=OFF\"\"> <button style=\"height:70px;width:140px\" >  STOP  </button></a>  ");
      script += ("<a href=\" /c1out1=REV\"\"> <button style=\"height:70px;width:140px\" > SCHLIESSEN </button></a> "); // <br/>
      script += ("<br> \n\n");
   */


   script += (c1OUT2name + " ist: ");
   if (c1out2 == 1)
   {
      script += ("EIN &nbsp; <wbr> <wbr> ");
   }
   else if (c1out2 == -1)
   {
      script += ("Rev &nbsp; <wbr> <wbr> ");
   }
   else if (c1out2 == 0)
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
   }
   script += ("<a href=\" /c1out2=ON\"\"> <button style=\"height:70px;width:140px\" > ÖFFNEN </button></a>  ");
   script += ("<a href=\" /c1out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  STOP  </button></a>  ");
   script += ("<a href=\" /c1out2=REV\"\"> <button style=\"height:70px;width:140px\" > SCHLIESSEN </button></a> "); // <br/>
   script += ("<br> \n\n");

   /*
      script += (c0OUT3name + " ist: ");
      if (c1out3 == 1)
      {
      script += ("EIN &nbsp; <wbr> <wbr> ");
      }

      else if (OUT3 == -1)
      {
      script += ("Rev &nbsp; <wbr> <wbr> ");
      }
      else
      {
      script += ("AUS &nbsp; <wbr> <wbr> ");
      }
      script += ("<a href=\" /c1out3=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
      script += ("<a href=\" /c1out3=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
      script += ("<br> \n");

   */

   client.print(script);
   script = "";

   // remote client 1 sensors
   // chart table
   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += "<h2> ";
   //script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;
   script += "<style=\"color:rgb(0,0,0);\" > </p>  " ;
   script += ("<br> \n");
   script += "<table border=4 cellpadding=4>"; // "<table>";

   script +=  "<caption>  Messwerte " + CLIENT1name ;
   script +=  "(Verb.-Fehler: " + (String)( c1t1.tFail )  + " min)" ;
   script +=  "</caption>" ;

   // reset
   script += ("<a href=\" /c1reset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");


   // client 1 Zeile 1
   script +=     "<thead> ";
   script +=       "<tr> ";
   script +=       "<td bgcolor='Teal'> " + c1SECT1name + "  </td>";
   script +=       "<td bgcolor='Yellow'> °Cmin  </td>";
   script +=       "<td bgcolor='Yellow'> °Cmax  </td>";
   script +=       "<td bgcolor='Yellow'> °C ∅  </td>";
   script +=       "<td bgcolor='White'> rF%   </td>";
   script +=       "<td bgcolor='LightCyan'>  " + c1SECT2name + " </td>";
   script +=       "<td bgcolor='Yellow'> °Cmin </td>";
   script +=       "<td bgcolor='Yellow'> °Cmax </td>";
   script +=       "<td bgcolor='White'> rF%  </td>";
   script +=       "<td bgcolor='Orange'>" + (String)c1A0intname + "</td>";
   script +=       "<td bgcolor='Orange'>" + (String)"&nbsp;∅&nbsp;" + "</td>";
   script +=       "</tr> ";
   script +=     "</thead> ";

   // client 1 Zeile 2
   script +=     "<tbody> ";
   script +=       "<tr> ";
   if (c1t1.tFail > 60 || c1t1.vmean > -4.0 ) {
      script += (String)"<td bgcolor='red'>";
   } else
      script += (String)"<th>";
   script +=         (String)c1t1.sact  + " °C </th> ";
   script +=         (String)"<th>" + c1t1.smin   + "</th> ";
   script +=         (String)"<th>" + c1t1.smax   + "</th> ";
   script +=         (String)"<th>" + c1t1.smean  + "</th> ";
   script +=         (String)"<th>" + c1h1.sact + "</th> ";
   script +=         (String)"<th>" + c1t2.sact  + " °C </th> ";
   script +=         (String)"<th>" + c1t2.smin   + "</th> ";
   script +=         (String)"<th>" + c1t2.smax   + "</th> ";
   script +=         (String)"<th>" + c1h2.sact + "</th> ";

   if (c1espA0.vact<fFireLIMIT) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=                  (String)c1espA0.sact + "</th> ";
   script +=           "<th>"+(String)(int)round(c1espA0.vmean) + "</th> ";

   script +=       "</tr> ";



   script +=     "</tbody> ";
   script +=   "</table>  ";
   script += "</h2> ";


   script += ("<br> \n");
   script += ("<br> \n");

   client.print(script);

   script = "";


   //---------------------------------------
   // Client 2
   //---------------------------------------

   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   //script += ("<font style=\"color:rgb(0,0,0);\" > ");
   script += ("<font style=\"color:rgb(0,0,0);\" > </p> ");
   //---------------------------------------
   //script +=  "<h1> <br> \n  C2: KÜCHE <br> \n </h1>";
   script +=  "<p><font style=\"color:rgb(0,205,102);\" > </p>";
   script +=  "<h1> <br> \n" + CLIENT2name + "<br> \n </h1>";
   script +=  "<p><font style=\"color:rgb(0,0,0);\" > </p>";
   //---------------------------------------


   //---------------------------------------
   // remote buttons Client C2
   //---------------------------------------
   /*
      script += (c2OUT2name + " ist: "); // script+= (" <h2>" + "Check" + " ist: ");
      if (c2out2 == 1)
      {
      script += ("EIN &nbsp; <wbr> <wbr> ");
      }
      else
      {
      script += ("AUS &nbsp; <wbr> <wbr> ");
      }
      script += ("<a href=\" /c2out2=ON\"\"> <button style=\"height:70px;width:140px\" >  EIN  </button></a>  ");
      script += ("<a href=\" /c2out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a>  ");
      script += ("<br> \n\n");
   */

   script += (c2OUT2name + " ist: ");
   if (c2out2 == 1)
   {
      script += ("EIN &nbsp; <wbr> <wbr> ");
   }
   else if (c2out2 == -1)
   {
      script += ("Rev &nbsp; <wbr> <wbr> ");
   }
   else if (c2out2 == 0)
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
   }

   script += ("<a href=\" /c2out2=ON\"\"> <button style=\"height:70px;width:140px\" > ÖFFNEN </button></a>  ");
   script += ("<a href=\" /c2out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  STOP  </button></a>  ");
   script += ("<a href=\" /c2out2=REV\"\"> <button style=\"height:70px;width:140px\" > SCHLIESSEN </button></a> "); // <br/>
   script += ("<br> \n\n");

   //---------------------------------------
   // remote client 2 sensors
   // chart table
   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += "<h2> ";
   //script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;
   script += "<style=\"color:rgb(0,0,0);\" > </p>  " ;
   script += ("<br> \n");
   script += "<table border=4 cellpadding=4>"; // "<table>";

   script +=  "<caption>  Messwerte " + CLIENT2name ;
   script +=  "(Verb.-Fehler: " + (String)( c2t1.tFail )  + " min)"  ;
   script +=  "</caption>" ;

   // reset
   script += ("<a href=\" /c2reset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");

   // client 2 Zeile 1
   script +=  "<thead> ";
   script +=    "<tr> ";
   script +=    "<td bgcolor='Teal'> " + c2SECT1name + "  </td>";
   script +=    "<td bgcolor='Yellow'> °Cmin  </td>";
   script +=    "<td bgcolor='Yellow'> °Cmax  </td>";
   script +=    "<td bgcolor='Yellow'> °C ∅  </td>";
   script +=    "<td bgcolor='White'> rF%   </td>";
   script +=    "<td bgcolor='LightCyan'>  " + c2SECT2name + " </td>";
   script +=    "<td bgcolor='Yellow'> °Cmin </td>";
   script +=    "<td bgcolor='Yellow'> °Cmax </td>";
   script +=    "<td bgcolor='White'> rF%  </td>";
   script +=    "<td bgcolor='Orange'>" + (String)c2A0intname + "</td>";
   script +=    "<td bgcolor='Orange'>" + (String)"&nbsp;∅&nbsp;" + "</td>";
   script +=    "</tr> ";
   script +=  "</thead> ";

   // client 2 Zeile 2
   script +=  "<tbody> ";
   script +=    "<tr> ";
   if (c2t1.tFail > 60 || c2t1.vmean > -4.0 ) {
      script += (String)"<td bgcolor='red'>";
   } else
      script += (String)"<th>";
   script +=      (String)c2t1.sact  + " °C </th> ";
   script +=      (String)"<th>" + c2t1.smin   + "</th> ";
   script +=      (String)"<th>" + c2t1.smax   + "</th> ";
   script +=      (String)"<th>" + c2t1.smean  + "</th> ";
   script +=      (String)"<th>" + c2h1.sact   + "</th> ";
   script +=      (String)"<th>" + c2t2.sact  + " °C </th> ";
   script +=      (String)"<th>" + c2t2.smin   + "</th> ";
   script +=      (String)"<th>" + c2t2.smax   + "</th> ";
   script +=      (String)"<th>" + c2h2.sact + "</th> ";

   if (c2espA0.vact<fFireLIMIT) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=                  (String)c2espA0.sact + "</th> ";
   script +=           "<th>"+(String)(int)round(c2espA0.vmean) + "</th> ";

   script +=    "</tr> ";


   script +=     "</tbody> ";
   script +=   "</table>  ";
   script += "</h2> ";

   script += ("<br> \n");
   script += ("<br> \n");

   client.print(script);

   script = "";


   //---------------------------------------
   //  Client 3
   //---------------------------------------

   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   //script += ("<font style=\"color:rgb(0,0,0);\" > ");
   script += ("<font style=\"color:rgb(0,0,0);\" > </p> ");
   //---------------------------------------
   //script +=  "<h1> <br> \n  C3: GEW.-HAUS  <br> \n </h1>";
   script +=  "<p><font style=\"color:rgb(0,205,102);\" > </p>";
   script +=  "<h1> <br> \n" + CLIENT3name + "<br> \n </h1>";
   script +=  "<p><font style=\"color:rgb(0,0,0);\" > </p>";
   //---------------------------------------



   //---------------------------------------
   // remote buttons Client 3
   //---------------------------------------

   // <input type="button" value="submit" style="height: 100px; width: 100px; left: 250; top: 250;">
   // <button style=\"height:200px;width:200px\"> </button>

   script += (c3OUT1name + " ist: "); // script+= (" <h2>" + OUT1name + " ist: ");
   if (c3out1 == 1)
   {
      script += ("EIN &nbsp; <wbr> <wbr> ");
   }
   else
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
   }
   script += ("<a href=\" /c3out1=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
   script += ("<a href=\" /c3out1=OFF\"\"> <button style=\"height:70px;width:140px\" > AUS </button></a> "); // <br/>
   script += ("<br> \n\n");


   script += (c3OUT2name + " ist: ");
   if (c3out2 == 1)
   {
      script += ("EIN &nbsp; <wbr> <wbr> ");
   }
   else
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
   }
   script += ("<a href=\" /c3out2=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
   script += ("<a href=\" /c3out2=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
   script += ("<br> \n\n");

//----------------------
// Thermostat
//----------------------

   script += ("c3-Thermost= ");
   sprintf(istr, "%+3d", c3tx3);
   script += (String)( istr ) ;
   script += (" &nbsp &nbsp ");
   script += ("<a href=\" /c3tx3=UP\"\"> <button style=\"height:70px;width:140px\" >Therm +1</button></a>  ");
   script += ("<a href=\" /c3tx3=DN\"\"> <button style=\"height:70px;width:140px\" >Therm -1</button></a> ");
   script += ("<br> \n\n");

//----------------------


   client.print(script);
   script = "";




   //---------------------------------------
   // remote client 3 sensors
   // chart table
   //---------------------------------------

   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += "<h2> ";
   //script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;  // <<<<<<<<<<<<<<<<<
   script += "<style=\"color:rgb(0,0,0);\" > </p>  " ;  // <<<<<<<<<<<<<<<<<
   script += ("<br> \n");
   script += "<table border=4 cellpadding=4>"; // "<table>";

   script +=  "<caption>  Messwerte " + CLIENT3name ;
   script +=  "(Verb.-Fehler: " + (String)( c3t1.tFail )  + " min)" ;
   script +=  "</caption>" ;

   // reset
   script += ("<a href=\" /c3reset\"\"> <button style=\"height:35px;width:70px\" > reset </button></a> </h3> ");


   script +=     "<thead> ";
   // Zeile 1 Client 3
   script +=      "<tr> ";
   script +=         "<td bgcolor='Peru'> " + c3SECT1name + " </td>";
   script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
   script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
   script +=         "<td bgcolor='White'> rF%  </td>";
   script +=         "<td bgcolor='Avocado'>  " + c3SECT2name + " </td>";
   script +=         "<td bgcolor='Yellow'> °Cmin  </td>";
   script +=         "<td bgcolor='Yellow'> °Cmax  </td>";
   script +=         "<td bgcolor='White'> rF%  </td>";
   script +=         "<td bgcolor='White'> &nbsp;&nbsp; </td>";
   script +=         "<td bgcolor='White'> &nbsp;&nbsp; </td>";
   script +=     "</tr> ";
   script +=     "</thead> ";

   script +=     "<tbody> ";

   // Zeile 2 Client 3
   script +=       "<tr> ";
   script +=         (String)"<th>" + c3t1.sact  + " °C  </th> ";
   script +=         (String)"<th>" + c3t1.smin   + "</th> ";
   script +=         (String)"<th>" + c3t1.smax   + "</th> ";
   script +=         (String)"<th>" + c3h1.sact + "</th> ";
   script +=         (String)"<th>" + c3t2.sact  + " °C  </th> ";
   script +=         (String)"<th>" + c3t2.smin   + "</th> ";
   script +=         (String)"<th>" + c3t2.smax  + "</th> ";
   script +=         (String)"<th>" + c3h2.sact + "</th> ";
   script +=         "<th>   &nbsp;   </th> ";
   script +=         "<th>   &nbsp;   </th> ";
   script +=       "</tr> ";

   // Zeile 3 Client 3
   script +=       "<tr> ";
   script +=         "<td bgcolor='White'>   - </td>";
   script +=         "<td bgcolor='LightGray'> c3out1 </td>";
   script +=         "<td bgcolor='LightGray'> c3out2 </td>";
   script +=         "<td bgcolor='LightGray'> c3out3 </td>";

   script +=         "<td bgcolor='Avocado'>" + (String)c3A0muxname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c3A1muxname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c3A2muxname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c3A3muxname + "</td>";
   script +=         "<td bgcolor='Orange'>"  + (String)c3A0intname + "</td>";
   script +=         "<td bgcolor='Orange'>"  + (String)"&nbsp;∅&nbsp;" + "</td>";
   script +=       "</tr> ";

   // Zeile 4 Client 3
   //strcpy(dsymbol, tendencysymbol(c3p1.vact - c3p1.vmean).c_str() );
   script +=  "<tr> ";
   script +=      "<th>" + (String)"  -  " + "</th> ";
   script +=      "<th>" + (String)(c3outMon1)  + "</th> ";
   script +=      "<th>" + (String)(c3outMon2)  + "</th> ";
   script +=      "<th>" + (String)(c3outMon3)  + "</th> ";

   if (c3adc0.tFail>60 || c3adc0.vmean<adcSoilMin) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c3adc0.sact + "</th> ";

   if (c3adc1.tFail>60 || c3adc1.vmean<adcSoilMin) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c3adc1.sact + "</th> ";

   if (c3adc2.tFail>60 || c3adc2.vmean<adcSoilMin) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c3adc2.sact + "</th> ";

   if (c3adc3.tFail>60 || c3adc3.vmean<adcSoilMin) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c3adc3.sact + "</th> ";


   if (c3espA0.vact<fFireLIMIT) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=                  (String)c3espA0.sact + "</th> ";
   script +=           "<th>"+(String)(int)round(c3espA0.vmean) + "</th> ";

   script +=    "</tr> ";

   script +=  "</tbody> ";

   script +=   "</table> ";
   script += "</h2>";

   script += ("<br> \n");
   script += ("<br> \n");

   client.print(script);
   script = "";


   //---------------------------------------
   // log out
   //---------------------------------------
   script += ("<h3>Log Out: ");
   script += ("<a href=\" /logout\"\"> <button style=\"height:70px;width:140px\" > Log Out </button></a> </h3> ");

   script += ver + " " + WiFi.localIP().toString() + " " + (String)ssid + " <br>" ;
   //script += "</font> </p> \n";
   script += "</font>   \n";

   script += "</body> \n";
   script += "</html> \n";

   script += ("<br> \n");
   script += ("<br> \n");

   client.print(script);
   script = "";

   delay(1);

}

//----------------------------------------------------------------------------
// handle root +clients
//----------------------------------------------------------------------------

void handleRoot() {
   handleClients();
}


//----------------------------------------------------------------------------

void printUrlArg() {
   //Alle Parameter auch seriell ausgeben
   String message = "";
   for (uint8_t i = 0; i < webserver.args(); i++) {
      message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
   }
   webserver.send(200, "text/plain", message);
   Serial.println("*** printUrlArg(): msg Client C1 => WiFi Server ***");
   Serial.println(message);
   Serial.println("### ----------------- end msg ----------------- ###");
   Serial.println();
}



//----------------------------------------------------------------------------

void handleClients() {

   //printUrlArg(); //fuer Debug Zwecke

   double ftmp;
   String msgtok;

   //------------------------------------------
   // c0

   msgtok = webserver.arg("c0t1"); // <<<< c0t1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0t1);

   msgtok = webserver.arg("c0h1"); // <<<< c0h1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0h1);

   msgtok = webserver.arg("c0t2"); // <<<< c0t2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0t2);

   msgtok = webserver.arg("c0h2"); // <<<< c0h2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0h2);

   msgtok = webserver.arg("c0p1"); // <<<< c0p1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0p1);


   msgtok = webserver.arg("c0espA0"); // <<<< c0espA0
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0espA0);

   msgtok = webserver.arg("c0adc0"); // <<<< c0adc0
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0adc0);

   msgtok = webserver.arg("c0adc1"); // <<<< c0adc1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0adc1);

   msgtok = webserver.arg("c0adc2"); // <<<< c0adc2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0adc2);

   msgtok = webserver.arg("c0adc3"); // <<<< c0adc3
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c0adc3);


   //------------------------------------------

   msgtok = webserver.arg("LocalAlive"); // <<<< client alive btn pressed?
   if (msgtok != "") {
      int itemp = msgtok.toInt();
      if (itemp == 1) {
         resetConfirmTime();
      }
   }

   //------------------------------------------
   // c1

   msgtok = webserver.arg("c1t1"); // <<<< c1t1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c1t1);

   msgtok = webserver.arg("c1h1"); // <<<< c1h1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c1h1);

   msgtok = webserver.arg("c1t2"); // <<<< c1t2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c1t2);


   msgtok = webserver.arg("c1h2"); // <<<< c1h2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c1h2);


   msgtok = webserver.arg("c1espA0"); // <<<< c1espA0
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c1espA0);

   //------------------------------------------
   // c2

   msgtok = webserver.arg("c2t1"); // <<<< c2t1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c2t1);

   msgtok = webserver.arg("c2h1"); // <<<< c2h1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c2h1);

   msgtok = webserver.arg("c2t2"); // <<<< c2t2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c2t2);


   msgtok = webserver.arg("c2h2"); // <<<< c2h2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c2h2);


   msgtok = webserver.arg("c2espA0"); // <<<< c2espA0
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c2espA0);

   //------------------------------------------
   // c3

   msgtok = webserver.arg("c3t1"); // <<<< c3t1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c3t1);

   msgtok = webserver.arg("c3h1"); // <<<< c3h1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c3h1);

   msgtok = webserver.arg("c3t2"); // <<<< c3t2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c3t2);

   msgtok = webserver.arg("c3h2"); // <<<< c3h2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c3h2);

   msgtok = webserver.arg("c3espA0"); // <<<< c3espA0
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c3espA0);

   msgtok = webserver.arg("c3adc0"); // <<<< c3adc0
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c3adc0);

   msgtok = webserver.arg("c3adc1"); // <<<< c3adc1
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c3adc1);

   msgtok = webserver.arg("c3adc2"); // <<<< c3adc2
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c3adc2);

   msgtok = webserver.arg("c3adc3"); // <<<< c3adc3
   ftmp = fINVAL;
   if (msgtok != "") {
      ftmp = msgtok.toFloat();
   }
   logval(ftmp, c3adc3);


   msgtok = webserver.arg("c3out1"); // <<<< c3h2
   if (msgtok != "") {
      c3outMon1 = msgtok.toInt();
   }
   msgtok = webserver.arg("c3out2"); // <<<< c3h2
   if (msgtok != "") {
      c3outMon2 = msgtok.toInt();
   }
   msgtok = webserver.arg("c3out3"); // <<<< c3h2
   if (msgtok != "") {
      c3outMon3 = msgtok.toInt();
   }


   //------------------------------------------

   msgtok = webserver.arg("LocalAlive"); // <<<< client alive btn pressed?
   if (msgtok != "") {
      int itemp = msgtok.toInt();
      if (itemp == 1) {
         resetConfirmTime();
      }
   }

   //------------------------------------------

   //Werte auch bei Url-Aufruf zurückgeben

   String message = "*** ";

   // re CLIENT 0
   message += "&c0out1=" + (String)c0out1;
   message += "&c0out2=" + (String)c0out2;
   message += "&c0out3=" + (String)c0out3;
   // re CLIENT 1
   message += "&c1out0=" + (String)c1out0;
   message += "&c1out1=" + (String)c1out1;
   message += "&c1out2=" + (String)c1out2;
   message += "&c1out3=" + (String)c1out3;
   // re CLIENT 2
   message += "&c2out0=" + (String)c2out0;
   message += "&c2out1=" + (String)c2out1;
   message += "&c2out2=" + (String)c2out2;
   // re CLIENT 3
   message += "&c3out1=" + (String)c3out1;
   message += "&c3out2=" + (String)c3out2;
   //message += "&c3out3=" + (String)c3out3 ;   // c3out3=intern auto ctrl
   message += "&c3tx3=" + (String)c3tx3;        // Thermostat für c3out3

   // all clients
   message += "&remindcnt=" + (String)RemindCnt;
   message += "&emergencycnt=" + (String)EmergencyCnt;

   message += " ###";
   Serial.println(message);
   webserver.send(200, "text/plain", message);

}




//----------------------------------------------------------------------------
// UDP Time
//----------------------------------------------------------------------------

/*-------- NTP code ----------*/
/*
   const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
   byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets


   time_t getNtpTime()
   {
   time_t timebuf;
   while (UdpTime.parsePacket() > 0) ; // discard any previously received packets
   Serial.println("Transmit NTP Request");
   sendNTPpacket(timeServer);
   uint32_t beginWait = millis();

   while (millis() - beginWait < 1500) {
      int size = UdpTime.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
         Serial.println("Receive NTP Response");
         UdpTime.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
         unsigned long secsSince1900;
         // convert four bytes starting at location 40 to a long integer
         secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
         secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
         secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
         secsSince1900 |= (unsigned long)packetBuffer[43];
         timebuf = secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;  // timezone=0 for auto sync (CEST)
         timebuf = CE.toLocal(timebuf, &tcr);
         return timebuf;
      }
   }
   Serial.println("No NTP Response :-(");
   return 0; // return 0 if unable to get the time
   }




   // send an NTP request to the time server at the given address
   void sendNTPpacket(IPAddress &address)
   {
   // set all bytes in the buffer to 0
   memset(packetBuffer, 0, NTP_PACKET_SIZE);
   // Initialize values needed to form NTP request
   // (see URL above for details on the packets)
   packetBuffer[0] = 0b11100011;   // LI, Version, Mode
   packetBuffer[1] = 0;     // Stratum, or type of clock
   packetBuffer[2] = 6;     // Polling Interval
   packetBuffer[3] = 0xEC;  // Peer Clock Precision
   // 8 bytes of zero for Root Delay & Root Dispersion
   packetBuffer[12]  = 49;
   packetBuffer[13]  = 0x4E;
   packetBuffer[14]  = 49;
   packetBuffer[15]  = 52;
   // all NTP fields have been given values, now
   // you can send a packet requesting a timestamp:
   UdpTime.beginPacket(address, 123); //NTP requests are to port 123
   UdpTime.write(packetBuffer, NTP_PACKET_SIZE);
   UdpTime.endPacket();
   }

*/



//----------------------------------------------------------------------------
// REFERENCES
//----------------------------------------------------------------------------
/*
    Lit.:
    http://www.instructables.com/id/Quick-Start-to-Nodemcu-ESP8266-on-Arduino-IDE/ ,
    https://iotdesignpro.com/projects/esp8266-based-webserver-to-control-led-from-webpage ,
    http://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/ ,
    https://tttapa.github.io/ESP8266/Chap10%20-%20Simple%20Web%20Server.html

    https://github.com/digitalloggers/PLDuino/blob/master/Arduino/libraries/Time-master/examples/TimeNTP_ESP8266WiFi/TimeNTP_ESP8266WiFi.ino ,
    https://github.com/JChristensen/Timezone ,
    https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/udp-examples.html ,
    https://github.com/montotof123/esp8266-12/tree/master/050_Mail_Sender

    https://github.com/adafruit/DHT-sensor-library ,
    https://playground.arduino.cc/Main/PCF8574Class ,
    https://github.com/adafruit/Adafruit_ADS1X15/blob/master/examples/singleended/singleended.pde ,
    https://learn.adafruit.com/adafruit-mcp9808-precision-i2c-temperature-sensor-guide/wiring ,
    http://arduino-projekte.webnode.at/projekte/portexpander-pcf8574/ ;

    http://wiki.selfhtml.org/wiki/HTML/Tabellen/Aufbau_einer_Tabelle
    http://wiki.selfhtml.org/wiki/Caption
    http://wiki.selfhtml.org/wiki/CSS/Eigenschaften/Tabellenformatierung
    http://www.aip.de/groups/soe/local/handbuch/html/tceb.htm

    // html colors: https://www.w3schools.com/tags/ref_colornames.asp
    // button style color https://de.wikihow.com/Die-Farbe-einer-Schaltfl%C3%A4che-in-HTML-%C3%A4ndern
    // <button style="background-color:red; border-color:blue; color:white">
    // https://www.w3schools.com/colors/colors_shades.asp
    // HTML LightGray    #D3D3D3  rgb(211,211,211)
*/

//----------------------------------------------------------------------------
// END OF FILE
//----------------------------------------------------------------------------
