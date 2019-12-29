//----------------------------------------------------------------------------
//  ESP8266 NodeMCU
//  webserver,  webserver und udpclient
//  ESP8266WiFi Server für website (remote display und remote control)
//
// History:
// 0.8.1 WiFi dns, no login, core 2.6.3
// 0.8.0 stringEx.h (cstringarg())
// 0.7.9 neu c1out1=c2out1
// 0.7.8 neu für core 2.5.2: handleNotAuthorized()
// 0.7.4 neu: handleNotAuthorized(), cstrinarg(), dashboard:RstAlarmBtn
// 0.7.3 usr name+pwd login + var names reworked
// 0.7.2 core 2.4.0, ADC reworked
// -------------^^
// 0.7.0 remake website login
// 0.6.9 new Sendmail management (still no ssl)
// 0.6.8 soil humidity alarm, no warn emails
// 0.6.6 html Button c2, no PIN_setrotation
// 0.6.4n new pins I/O new SSID (show bottom)
// 0.6.3 alert Email: + mean
// 0.6.2 handleRoot() = commClients(),
//       loop:handleNotAuthorized => continue
// 0.6.0 weniger Website-Buttons
//

//----------------------------------------------------------------------------
#define TARGET 'Z'  // Server-Zielpattform (Z,T,Q => versch. IPs, Ports, urls)
String  ver = "0.8.1." + (String)TARGET;
//----------------------------------------------------------------------------
// Wifi data from  "data\settings.h"
#include "data\settings.h"  // sets + initializes passwords

extern const char* ssid;              // WIFI network name
extern const char* password;          // WIFI network password

// url data from  "data\settings.h"

const char* www_username = "fp";
const char* www_password = "tftf42";

extern char  website_uname[20]; //  website user name log in  "MyWebsiteLoginName"
extern char  website_upwd[20];  //  website user pwd log in   "MyWebsiteLoginPwd"
extern char* website_title;     //  website caption           "MySiteCaption"
extern char* website_url;       //  website url               "http:\\mysite.com"
extern char* mail_account;      //  regular email address     "myaccount@myprovider.com"
extern char* mailto_emerg;      //  emergency email address   "contact@hisprovider.com"



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
   digital:         default        myStd         bRk               ESP_motorShield
   D0     16       WAKE/LED       out/LED        ---               ---
   D1      5       I2C SCL        I2C SCL       I2C SCL           out D1 motorA
   D2      4       I2C SDA        I2C SDA       I2C SDA           out D2 motorB
   D3      0       FLASH/LED      in D3      built-in btn+LED     out D3 motorA
   D4      2       TX1            in/out        ---               out D4 motorB
   D5     14       SPI SCK        in D5         in D5 (DHT)       SDA
   D6     12       SPI MISO       Out D6        out D6            SCL
   D7     13       SPI MOSI       in D7         in D7 (DHT)       in  D7
   D8     15       MTD0 PWM       in/out        ---               out D8
   D9      3       UART RX0       USB, Serial   USB, Serial       USB, Serial
   D10     1       UART TX0       USB, Serial   USB, Serial       USB, Serial
   ---    10       intern ?       ---           ---               ---

*/


#include <stringEx.h>  // cstringarg() etc.

//----------------------------------------------------------------------------
// i2c Wire
#include <Wire.h>

#define SCL         D1      // SCL
#define SDA         D2      // SDA    

#define OLED_RESET  10      // GPIO10 


//----------------------------------------------------------------------------
// IO pins
//----------------------------------------------------------------------------

#define PIN_RESETAlarm  D3      // Btn 0=D3 reset Alarm
#define PIN_OUT1        D6      //  out 
#define PIN_OUT2        D8      //  out 
#define PIN_OUT3        D0      //  out 


//----------------------------------------------------------------------------
// Display OLED SSD1306
//----------------------------------------------------------------------------

#include <ESP_SSD1306.h>        // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>       // Needs a little change in original Adafruit library (See README.txt file)
#include <Fonts/FreeSansBold12pt7b.h>      // 
#include <Fonts/FreeSans9pt7b.h>           //
#include <Fonts/FreeMono12pt7b.h>          //

ESP_SSD1306   display(-1);


int trestalive = 0;

String OUT1name = "Alarmanlage-int";     // output captions
String OUT2name = "Alarmanlage-ext";
String OUT3name = "3-KLIMA";

String c0OUT1name = "1-F.TÜR";   // output names
String c0OUT2name = "2-PUMPE";
String c0OUT3name = "3-KLIMA";

String c1OUT1name = "1-F.TÜR";   // c1out2 output name (c1out0=intern)
String c1OUT2name = "2-F.TÜR";

String c2OUT1name = "1-F.TÜR";   // c1out2 output name (c2out0=intern)
String c2OUT2name = "2-F.TÜR";

volatile int8_t  OUT1 = 0, OUT2 = 0, OUT3 = 0; // Server; actual output pin states; stop=0, fwd=1, rev=-1;
volatile int8_t  c0out0=0, c0out1=0, c0out2=0, c0out3=0; // Client0; stop=0, fwd=1, rev=-1;
volatile int8_t  c1out0=0, c1out1=0, c1out2=0, c1out3=0; // Client1; stop=0, fwd=1, rev=-1;
volatile int8_t  c2out0=0, c2out1=0, c2out2=0, c2out3=0; // Client2; stop=0, fwd=1, rev=-1;


//----------------------------------------------------------------------------
// sensors
//----------------------------------------------------------------------------

// N.N barometric pressure adjust (250m)

const double  FNNcorr = 1013.0 - 984.0; // ca. 250m

const double  fINVAL = -99.9;
char  sNEXIST[10] = "--";
char  sINVAL[10] = "??";

String STATION1name = "Heim.1";
String STATION2name = "Heim.2";

String c0STATION1name = "G.Aussen";
String c0STATION2name = "G.GwHaus";




typedef struct {
   double    vact = fINVAL, vmin = fINVAL, vmax = fINVAL, vmean = fINVAL ; // min,act,max,mean
   uint32_t  tact = 0, tmin = 0, tmax = 0, tmean = 0, talert = 0 ; // time (millis)
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





//----------------------------------------------------
// analog pins
//----------------------------------------------------

String A0intname = "Alarm.int";  // intern: ESP8266
String A0muxname = "Alarm.ext";  // mux: ADS1115 (i2c)
String A1muxname = "Sens.1";
String A2muxname = "Sens.2";
String A3muxname = "Sens.3";


String c0A0intname = "Sens.A0";  // intern: ESP8266
String c0A0muxname = "Erde.Li";  // mux: ADS1115 (i2c)
String c0A1muxname = "Erde.Re";
String c0A2muxname = "Erde.A2";
String c0A3muxname = "Wasser ";



//----------------------------------------------------
// ADS1015/1115 (4* ADC)
//----------------------------------------------------
#include <Adafruit_ADS1015.h>

// Adafruit_ADS1115 ads;     /* Use this for the 16-bit version */
// Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

Adafruit_ADS1115 ADSx48(0x48);  // AS1115 i2c dev addr


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
// client 1

char sdhPa[4] = "";  // ++, =+, ==, =-, ≤, --

String C1SENSORname1 = "Keller.Gefr";
String C1SENSORname2 = "Keller.Kühl";
String C2SENSORname1 = "Küche.Gefr";
String C2SENSORname2 = "Küche.Kühl";


//----------------------------------------------------------------------------
// WiFi Libs
//----------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

// allows you to set the realm of authentication Default:"Login Required"
const char* www_realm = "Custom Auth Realm";

// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";


// WiFi Router

#if TARGET=='Z'
#define     this_iph     200      // <<< local host ip (200:website=Z)
#define     http_port     80
#elif TARGET=='T'
#define     this_iph     201      // <<< local host ip (201:website=T)
#define     http_port   8080
#elif TARGET=='Q'
#define     this_iph     209      // <<< local host ip (209:test)
#define     http_port     80      //     test  
#else
#define     this_iph     200      // <<< local host ip (200: default)
#define     http_port     80
#endif

IPAddress    this_ip(192, 168, 2, this_iph); // <<< Feste lokale IP dieses ESP8266-Servers
IPAddress    gateway(192, 168, 2, 1);       // <<< LAN Gateway IP
IPAddress    subnet(255, 255, 255, 0);      // <<< LAN Subnet Mask

WiFiServer   wifiserver(http_port);

ESP8266WebServer webserver(8081);

bool      authorized = false;
//uint32_t  tstop;









#define  SkyBlue      #6698FF
#define  ORANGE       255,102,0
#define  SIGNYELLOW   255,209,22
#define  ROSE         255,0,204
#define  SPRINGGREEN3 0,205,102






//----------------------------------------------------------------------------
// Email
//----------------------------------------------------------------------------
#include "Gsender.h"
Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance

bool     ALERT = false;
int      AlertCnt = 0,
         RemindCnt = 0;
bool     EMERGENCY = false,
         EMERGENCYRESET = false;
int      EmergencyCnt = 0;
uint32_t temergency_millis = 0;        // timestamp (millis)
#define  MAX_ALIVEPERIOD_STD      40   // hours until emergency alarm
#define  FINAL_ALIVEREMIND_STD    11
int      tminalert = FINAL_ALIVEREMIND_STD;

int8_t   LocAlive = 0;
static int8_t  LEDmode = 0;
#define  RSTMODE  8
#define  MAXMODE  9


//----------------------------------------------------------------------------
// Internet Udp Time
//----------------------------------------------------------------------------

#include <Time.h>       // Arduino Time lib https://github.com/PaulStoffregen/Time 
#include <TimeLib.h>    // Arduino Time lib https://github.com/PaulStoffregen/Time 
#include <WiFiUdp.h>    // esp8266 WiFiUdp.h https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFiUdp.h 
#include <Timezone.h>   // Timezone Lib  https://github.com/JChristensen/Timezone

WiFiUDP UdpTime;
unsigned int localTime_port = 8888;  // local port to listen for UDP packets


// manual time zone settings
const int timeZone = 0;     // GMT, auto mode (CEST)
//const int timeZone =  1;  // Central European Time (Berlin, Paris)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -6;  // Central Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)

// automatic Timezone setting
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = { "CEST", Last, Sun, Mar, 2, 120 };     //Central European Summer Time
TimeChangeRule CET = { "CET ", Last, Sun, Oct, 3, 60 };       //Central European Standard Time
Timezone CE(CEST, CET);
TimeChangeRule *tcr;        //pointer to the time change rule, use to get the TZ abbrev


//----------------------------------------------------------------------------
// NTP Servers
//----------------------------------------------------------------------------
// NIST Internet Time Servers: http://tf.nist.gov/tf-cgi/servers.cgi
IPAddress timeServer(129, 6, 15, 28); // 129.6.15.28 NIST, Gaithersburg, Maryland
//IPAddress timeServer(129,6,15,29);
//IPAddress timeServer(129,6,15,30);
//IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov



//----------------------------------------------------------------------------
// strings and symbols for website, IO, display
//----------------------------------------------------------------------------

String timestr = "--:--:--", datestr = "--.--.----";
#define CHR_DEGREE (unsigned char)247               // ° symbol for OLED font
char    STR_DEGREE[] = {247, 0, 0};                 // ° OLED font specimen (°,°C,°F,K)



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
// build String timestr, datestr

void buildDateTimeString() {
   char sbuf[20];

   // digital clock display of the time
   timestr = "";
   sprintf(sbuf, "%02d:%02d:%02d", (int)hour(), (int)minute(), (int)second());
   timestr = sbuf;
   //Serial.println(timestr);

   datestr = "";
   sprintf(sbuf, "%02d.%02d.%4d", (int)day(), (int)month(), (int)year());
   datestr = sbuf;
   //Serial.println(datestr);
   //Serial.println();
}


//-----------------------------------------------------
// Tendenz-Symbol

char dsymbol[4] = "=\0";

char * tendencysymbol(float dpromille) {
   char symbol[4] = "~";
   if ((dpromille >= 0) && (dpromille <= 0.5)) strcpy(symbol, "↔");
   else if ((dpromille < 0) && (dpromille >= -0.5)) strcpy(symbol, "↔");
   else if ((dpromille > 0.5) && (dpromille <= 3.0)) strcpy(symbol, "↗");
   else if ((dpromille > 3.0)) strcpy(symbol, "⇈");
   else if ((dpromille < -0.5) && (dpromille >= -3.0)) strcpy(symbol, "↘");
   else if ((dpromille < -3.0)) strcpy(symbol, "⇊");
   symbol[3] = '\0';
   return symbol;
}



//----------------------------------------------------------------------------
// OLED dashboard
//----------------------------------------------------------------------------
void dashboard(int mode) {

   if (LEDmode > MAXMODE) {
      LEDmode = 0;
      mode = 0;
   }
   if ( !digitalRead(PIN_RESETAlarm) ) mode = RSTMODE;

   display.clearDisplay();
   display.setFont();

   if (mode == 0) {
      display.setCursor( 0, 0);  // display.print( timestr+"   "+datestr );

      display.setCursor( 0, 8);  display.print("c0t1=" + (String)c0t1.sact + STR_DEGREE + "C");
      display.setCursor(80, 8);  display.print("F:" + (String)c0h1.sact );
      display.setCursor( 0, 16);  display.print("c0t2=" + (String)c0t2.sact + STR_DEGREE + "C" );

      display.setCursor(80, 16);  display.print("F:" + (String)c0h2.sact );
      display.setCursor( 0, 24);  display.print("c0Ai=" + (String)c0espA0.sact);
      display.setCursor(80, 24);  display.print( (String)(int)c0p1.vact + " hPa");

      display.setCursor( 0, 32);  display.print("c0A0=" + (String)c0adc0.sact);
      display.setCursor(66, 32);  display.print("c0A1=" + (String)c0adc1.sact);
      display.setCursor( 0, 40);  display.print("c0A2=" + (String)c0adc2.sact);
      display.setCursor(66, 40);  display.print("c0A3=" + (String)c0adc3.sact);

      display.setCursor( 0, 49);  display.print("c1t1=" + (String)c1t1.sact + STR_DEGREE + "C" );
      display.setCursor(80, 49);  display.print("F:" + (String)c1h1.sact );
      display.setCursor( 0, 57);  display.print("c1t2=" + (String)c1t2.sact + STR_DEGREE + "C" );
      display.setCursor(80, 57);  display.print("F:" + (String)c1h2.sact );
   }
   else if (mode == 1) {
      display.setFont(&FreeSansBold12pt7b);                    // opt.: FreeSerif9pt7b
      display.setCursor( 0, 17);  display.print( datestr );
      display.setCursor( 0, 37);  display.print( timestr);
   }
   else if (mode == 2) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 17);  display.print("T Innen:");
      display.setCursor( 0, 37);  display.print( (String)t1.sact + STR_DEGREE + "*C");
   }
   else if (mode == 3) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 17);  display.print("T Aussen:");
      display.setCursor( 0, 37);  display.print( (String)c0t1.sact + STR_DEGREE + "*C");
   }
   else if (mode == 4) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 17);  display.print("Luftdruck:");
      display.setCursor( 0, 37);  display.print( (String)c0p1.sact +  " hPa");
   }
   else if (mode == 5) {
      display.setFont(&FreeSans9pt7b);                             // opt.: FreeSerif9pt7b
      display.setCursor( 0, 12);  display.print("Keller:");
      display.setCursor( 0, 28);  display.print("F: " + (String)c1t1.sact);
      display.setCursor( 0, 44);  display.print("K: " + (String)c1t2.sact);
   }
   else if (mode == 6) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("Kueche:");
      display.setCursor( 0, 28);  display.print("F: " + (String)c2t1.sact);
      display.setCursor( 0, 44);  display.print("K: " + (String)c2t2.sact);
   }
   else if (mode == 7) { // c0adc
      display.setCursor( 0, 10);  display.print((String)"C0 Garten: ");
      display.setCursor( 0, 20);  display.print((String)"ESPA0: " + c0espA0.sact);
      display.setCursor( 0, 30);  display.print((String)"adcA0: " + c0adc0.sact);
      display.setCursor( 0, 40);  display.print((String)"adcA1: " + c0adc1.sact);
      display.setCursor( 0, 50);  display.print((String)"adcA2: " + c0adc2.sact);
      display.setCursor( 0, 60);  display.print((String)"adcA3: " + c0adc3.sact);
   }
   else if (mode == 8) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("Alarms: " + (String)EmergencyCnt);
      display.setCursor( 0, 28);  display.print("Warng.: " + (String)AlertCnt);
      display.setCursor( 0, 44);  if ( !digitalRead(PIN_RESETAlarm) ) display.print("Reset Alarm ");
      else display.print( (String)("(") + trestalive + " h)" );
   }
   else if (mode == 9) {
      display.setCursor( 0, 10); display.print("Alarms (min):");
      display.setCursor( 0, 20); display.print("c1t1: ");
      display.setCursor(60, 20); display.print((String)(c1t1.talert / (1000ul * 60)));
      display.setCursor( 0, 30); display.print("c2t1: ");
      display.setCursor(60, 30); display.print((String)(c2t1.talert / (1000ul * 60)));
      display.setCursor( 0, 40); display.print("c0adc0: ");
      display.setCursor(60, 40); display.print((String)(c0adc0.talert / (1000ul * 60)));
      display.setCursor( 0, 50); display.print("c0adc1: ");
      display.setCursor(60, 50); display.print((String)(c0adc1.talert / (1000ul * 60)));
   }
   display.display();
   display.setFont();
}



//----------------------------------------------------------------------------
// LOG ARRAY
//----------------------------------------------------------------------------
void logval( double f, vlog &v) {

   if (f <= fINVAL ) {
      if (millis() - v.tact > 1000ul * 60) { // store invalid if outdated>1min
         v.vact = fINVAL;
         strcpy(v.sact, sINVAL);
      }
      delay(1);
      return;
   }

   if (v.vact <= fINVAL)
   {
      v.vact = f;
   }
   else
   {
      v.vact = (v.vact + f ) / 2;
   }
   v.tact = millis();

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


//----------------------------------------------------------------------------
// resetEmergencyAlarm()
//----------------------------------------------------------------------------
void resetEmergencyAlarm() {
   temergency_millis = millis();
   EMERGENCY = false;
   EmergencyCnt = 0;
   EMERGENCYRESET = true;
   RemindCnt = 0;
   tminalert = FINAL_ALIVEREMIND_STD;
   c1out1 = 0; c2out1 = 0; // report to Client 1+2: now switch OFF  ALARM LED
   if (!ALERT && !EMERGENCY
         && (millis() - c1t1.talert > 2 * 60 * 60 * 1000ul)
         && (millis() - c2t1.talert > 2 * 60 * 60 * 1000ul)
         && (millis() - c0adc0.talert > 2 * 60 * 60 * 1000ul)
         && (millis() - c0adc1.talert > 2 * 60 * 60 * 1000ul) )
   {
      AlertCnt = 0;
      c0adc0.talert = 0;
      c0adc1.talert = 0;
      c1t1.talert = 0;
      c2t1.talert = 0;
   }
   delay(1);
}

//----------------------------------------------------------------------------
// CheckForAlert Email
//----------------------------------------------------------------------------
void CheckForAlert() {
   String alertmsg = "";
   int  err = 0;

   ALERT = false;
   EMERGENCY = false;
   EMERGENCYRESET = false;

   trestalive = (MAX_ALIVEPERIOD_STD - (millis() - temergency_millis) / (1000ul * 60 * 60));

   alertmsg = "";

   if ( EMERGENCY || (trestalive < tminalert) ) { //  <<<<<<<<  ALERT
      ALERT = true;
      AlertCnt += 1;
      RemindCnt += 1;
      alertmsg += "\n\r Alarm! Reset Alarm, Confirm Alive!\n\r Restzeit (h) : " + (String)trestalive + "\n\r";
      c1out1 = c2out1 = RemindCnt; // report to Client 1+2: switch ON ALARM LED <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   }


   if ( ((c1t1.vmean > -8.0) || (c1t1.vact > -1.0)
         || (c1t1.vact <= fINVAL && millis() - c1t1.tact > 2 * 60 * 60 * 1000ul))
         &&  (millis() - c1t1.talert > 2 * 60 * 60 * 1000ul)  ) { //  <<<<<<<<   sensor ALERT
      ALERT = true;
      AlertCnt += 1;
      alertmsg += "\n\r Sensor c1t1= " + (String)c1t1.vact;
      alertmsg += "\n\r Durchschnitt=" + (String)c1t1.vmean + "\n\r";
      c1t1.talert = millis();
   }


   if ( ((c2t1.vmean > -8.0) || (c2t1.vact > -1.0)
         || (c2t1.vact <= fINVAL && millis() - c2t1.tact > 2 * 60 * 60 * 1000ul))
         &&  (millis() - c2t1.talert > 2 * 60 * 60 * 1000ul)  ) { //  <<<<<<<<   sensor ALERT
      ALERT = true;
      AlertCnt += 1;
      alertmsg += "\n\r Sensor c2t1=" + (String)c2t1.vact;
      alertmsg += "\n\r Durchschnitt=" + (String)c2t1.vmean + "\n\r";
      c2t1.talert = millis();
   }


   // soil humidity 0:
   if ( c0adc0.vmean < 20.0 &&
         (millis() - c0adc0.talert > 2 * 60 * 60 * 1000ul)  )    { //  <<<<<<<<   sensor ALERT
      ALERT = true;
      AlertCnt += 1;
      alertmsg += "\n\r Sensor c0adc0=" + (String)c0adc0.vact + "\n\r";
      c0adc0.talert = millis();
   }


   // soil humidity 1:
   if ( c0adc1.vmean < 20.0 &&
         (millis() - c0adc1.talert > 2 * 60 * 60 * 1000ul)  )    { //  <<<<<<<<   sensor ALERT
      ALERT = true;
      AlertCnt += 1;
      alertmsg += "\n\r Sensor c0adc0=" + (String)c0adc0.vact + "\n\r";
      c0adc1.talert = millis();
   }


   // send mails

   if (alertmsg != "") {
      Serial.println("Warnungen: " + alertmsg);
      err = SendEmail(mail_account, "Sensor-Warnung", alertmsg);  delay(100);
      err = 0; // debug
      if (!err) {
         Serial.println("Success: Sensor-Warnung Email gesendet!");
         Serial.println();
         ALERT = false;
         tminalert -= 2;
      }
      delay(1);
   }

   if (trestalive < 1) { // <<<< EMERGENCY EMAIL CALL: Not-Alive Alarm
      EMERGENCY = true;
      EmergencyCnt += 1;
      AlertCnt     += 1;
      Serial.println("ACHTUNG! NOTFALL EMAIL WIRD GESENDET \n\r");
   }


   if (EMERGENCY) {
      alertmsg = "Helmut OK? bitte Helmut anrufen!";
      err = SendEmail(mail_account, "Notfall?", alertmsg);    delay(10);
      err = SendEmail(mailto_emerg, "Notfall?", alertmsg);    delay(1);
      err = 0; // debug;
      if (!err) {
         temergency_millis = min( (uint32_t)(millis() - (MAX_ALIVEPERIOD_STD - 4) * 60 * 60 * 1000ul), (uint32_t)millis() );
         EMERGENCY = false;
         tminalert = 2 ;
         Serial.println("Success: 2 EMERGENCY Emails gesendet!");
         Serial.println();
      }
      else  {
         temergency_millis = millis() - (MAX_ALIVEPERIOD_STD - 1) * 60 * 60 * 1000ul;
      }
      delay(10);
   }


   if (!ALERT && !EMERGENCY
         && (trestalive >= FINAL_ALIVEREMIND_STD)
         && (millis() - c1t1.talert > 2 * 60 * 60 * 1000ul)
         && (millis() - c2t1.talert > 2 * 60 * 60 * 1000ul)
         && (millis() - c0adc0.talert > 2 * 60 * 60 * 1000ul)
         && (millis() - c0adc1.talert > 2 * 60 * 60 * 1000ul) )
   {
      AlertCnt = 0;
      c1t1.talert = 0;
      c2t1.talert = 0;
      c0adc0.talert = 0;
      c0adc1.talert = 0;
   }


}





//----------------------------------------------------------------------------
// checkRESETBtn
//----------------------------------------------------------------------------
void checkRESETBtn() {
   if (!digitalRead(PIN_RESETAlarm )) { // reset Alarm
      LEDmode = RSTMODE;
      dashboard(LEDmode);
      resetEmergencyAlarm();
      while ( !digitalRead(PIN_RESETAlarm) ) {
         delay(100);
      }
      delay(100);       // debounce
   }
   trestalive = (MAX_ALIVEPERIOD_STD - (millis() - temergency_millis) / (60 * 60 * 1000ul));
   delay(1);
}
















//----------------------------------------------------------------------------
// SETUP
//----------------------------------------------------------------------------


void setup() {

   int IORes;

   //STR_DEGREE[0] = CHR_DEGREE; // ° symbol as ANSI C string

   int progress = 0;


   //----------------------------------------
   Serial.begin(115200);
   delay(1000);

   //----------------------------------------
   pinMode(PIN_OUT1, OUTPUT);
   digitalWrite(PIN_OUT1, LOW);

   pinMode(PIN_OUT2, OUTPUT);
   digitalWrite(PIN_OUT2, LOW);

   pinMode(PIN_OUT3, OUTPUT);
   digitalWrite(PIN_OUT3, LOW);

   pinMode(PIN_RESETAlarm, INPUT_PULLUP);  // reset Alarm



   //----------------------------------------
   // i2c: init

   Wire.pins(SDA, SCL);        // SDA, SCL
   Wire.begin();
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
   display.setCursor( 0, 0);  display.print("OLED TEST OK");
   display.display();
   delay(1);
   Serial.println("OLED sensor init...");

   /**
      //----------------------------------------
      // i2c:  ADS1115

      ADSx48.begin(); // ADS1115 I2C 4x 12/16bit ADC
      delay(1);
      Serial.println("ADS1115 sensor init... ");
   */


   //----------------------------------------
   // Init MCP3421: I2C-Adresse, 18 Bit Modus, keine Verstärkung
   ADCmcp3421.init(0x68, 3, 0);


   /*
      //----------------------------------------
      // i2c:  MCP9808

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
   Serial.println("Connecting to Router: ");
   Serial.println( WiFi.gatewayIP().toString() );

   WiFi.mode(WIFI_STA);
   // WiFi.config(local_this_ip, gateway, subnet);
   WiFi.config(this_ip, gateway, gateway, subnet);   // feste IP, dns server
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
   display.display();
   delay(300);




   Serial.println("");
   Serial.print("WiFi connected: ");
   Serial.println(WiFi.gatewayIP());


   //----------------------------------------
   // Start the WiFi server (-> www)
   wifiserver.begin();
   Serial.println("WiFi Server started");








   ArduinoOTA.begin();
   webserver.on("/", []() {
      if (!webserver.authenticate(www_username, www_password))
         //Basic Auth Method with Custom realm and Failure Response
         //return webserver.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
         //Digest Auth Method with realm="Login Required" and empty Failure Response
         //return webserver.requestAuthentication(DIGEST_AUTH);
         //Digest Auth Method with Custom realm and empty Failure Response
         //return webserver.requestAuthentication(DIGEST_AUTH, www_realm);
         //Digest Auth Method with Custom realm and Failure Response
      {
         return webserver.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
      }
      webserver.send(200, "text/plain", "Login OK");
   });





   //----------------------------------------
   // Start the ESP webserver (-> website)

   webserver.on("/", commClients) ;
   delay(1);
   webserver.on("/client/client0/", commClients);
   delay(1);
   webserver.on("/client/client1/", commClients);
   delay(1);
   webserver.on("/client/client2/", commClients);
   delay(1);
   webserver.on("/client/client3/", commClients);
   delay(1);
   webserver.begin();
   delay(1);

   

   Serial.println("ESP Server started");

   // Print the IP address
   Serial.print("Use this URL to connect: ");
   Serial.print("http://");
   Serial.print(WiFi.localIP());
   Serial.print(":");
   Serial.print(http_port);
   Serial.println("/");
   Serial.print((String)website_url + ":" + http_port + "/");


   //----------------------------------------
   // Start UDP
   Serial.println("Starting UdpTime");
   UdpTime.begin(localTime_port);
   Serial.print("Local Time port: ");
   Serial.println(UdpTime.localPort());
   Serial.println("waiting for sync");
   delay(250);
   setSyncProvider(getNtpTime);
   delay(100);

   String emailmsg      = (String)website_url + " setup done, all systems go";  //  <<<<<<<<<<<<<
   SendEmail(mail_account, "Info", emailmsg);

   delay(100);

   //----------------------------------------
   // setup done

   dashboard(1);
   Serial.println("setup done \n");

}



//----------------------------------------------------------------------------
// LOOP
//----------------------------------------------------------------------------

void loop() {

   static double ftmp;
   static unsigned long tsec = millis(), tms = millis();

   trestalive = (MAX_ALIVEPERIOD_STD - (millis() - temergency_millis) / (60 * 60 * 1000ul));
   checkRESETBtn();

   //---------------------------------------
   // Check log-in

   ArduinoOTA.handle();
   //htmlWebsite();

   webserver.handleClient();
   delay(10);

   checkRESETBtn();

   //---------------------------------------
   // Read local + Udp data
   if ( millis() - tms >= 100 ) {    // refresh data rate
      tms = millis();

      //---------------------------------------
      // build date + time strings
      buildDateTimeString();
      //Serial.println(timestr+"   "+datestr);

      //---------------------------------------
      // read DHT Sensor
      /*
         ftmp = DHT_1.readTemperature();          // 1. Temperatur auslesen (Celsius)
         delay(1);
         if (isnan(ftmp)) ftmp=fINVAL;
         logval(ftmp, t1);
         delay(10);

         ftmp = DHT_1.readHumidity();             // 1. Feuchtigkeit auslesen (Prozent)
         delay(1);
         if (isnan(ftmp)) ftmp=fINVAL;
         logval(ftmp, h1);
         delay(10);
      */

      ftmp = fINVAL;
      ftmp = bmp_x77.readTemperature();
      if (isnan(ftmp)) ftmp = fINVAL;
      logval(ftmp, t1);
      delay(1);
      /*
         ftmp=fINVAL;
         ftmp = bme_x77.readHumidity();       // 2. Feuchtigkeit auslesen (Prozent)
         if (isnan(ftmp)) ftmp=fINVAL;
         logval(ftmp, h2);
         delay(1);
      */

      ftmp = fINVAL;
      ftmp = FNNcorr + bmp_x77.readPressure() / 100.0 ;
      if (isnan(ftmp)) ftmp = fINVAL;
      logval(ftmp, p1);
      double dhPa = p1.vact - p1.vmean;
      delay(1);


      ftmp = fINVAL;
      ftmp = (float)analogRead(A0);
      ftmp = calADC1023(ftmp);
      if (isnan(ftmp)) ftmp = fINVAL;
      logval(ftmp, espA0);
      delay(1);

      /*

         //---------------------------------------
         // bRk internal ADC A0
         espA0 = 0;
         espA0=analogRead(A0);
         fespA0 = espA0 * 10/1024.0;            // *10 wegen 10:1 Teiler am ADC (Vin max. 9V)
         Serial.print("espA0="); Serial.print(sfespA0); Serial.println("%");
         delay(1);

         //---------------------------------------
         // bRk 2nd  ADC: ADCmcp3421 18bit
         double    fadc0 = ADCmcp3421.getDouble();    // Spannung als Double aus I2C ADC, Vin: 0 bis 2,048V
         fadc0 *= 10.0;                               // *10 wegen 10:1 Teiler am ADC (Vin max. 20V)
         Serial.print("fadc0="); Serial.print(sfadc0); Serial.println("%");
         delay(1);
      */

      /*
         //---------------------------------------
         // ADS1115 analog (ADC) sensors
         xadc0 = ADSx48.readADC_SingleEnded(0);  fadc0=calADS1115(xadc0);
         xadc1 = ADSx48.readADC_SingleEnded(1);  fadc1=calADS1115(xadc1);
         xadc2 = ADSx48.readADC_SingleEnded(2);  fadc2=calADS1115(xadc2);
         xadc3 = ADSx48.readADC_SingleEnded(3);  fadc3=calADS1115(xadc3);
         delay(1);

         Serial.println(" "); Serial.println(" ");
      */



      Serial.println("Client sensors:");
      Serial.print(" c1_t1="); Serial.print(c1t1.sact);
      Serial.print(" c1_h1="); Serial.print(c1h1.sact);
      Serial.print(" c1_t2="); Serial.print(c1t2.sact);
      Serial.print(" c1_h2="); Serial.print(c1h2.sact);
      Serial.println(" ");
      Serial.print(" c2_t1="); Serial.print(c2t1.sact);
      Serial.print(" c2_h1="); Serial.print(c2h1.sact);
      Serial.print(" c2_t2="); Serial.print(c2t2.sact);
      Serial.print(" c2_h2="); Serial.print(c2h2.sact);

      Serial.println(" "); Serial.println(" ");

      CheckForAlert();

      //---------------------------------------
      // display on OLED
      if ( millis() - tsec >= 4000 ) {
         tsec = millis();
         LEDmode++;
      }
      dashboard(LEDmode);
      delay(1);
   }

   checkRESETBtn();

}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

void htmlWebsite() {

   WiFiClient client = wifiserver.available();

   //---------------------------------------
   // Check if a client has connected


   // Read the first line of the request
   String request = client.readStringUntil('\r');
   Serial.println(request);
   client.flush();


   //---------------------------------------
   // Match the request on Server
   if ((request.indexOf("/OUT1=ON") != -1)  ) {
      analogWrite(PIN_OUT1, 1023);
      OUT1 = +1;
   }
   if ((request.indexOf("/OUT1=OFF") != -1) ) {
      analogWrite(PIN_OUT1, 0);
      OUT1 = 0;
   }
   /*
      if ((request.indexOf("/OUT1=REV") != -1) ) {
      analogWrite(PIN_OUT1, 1023);
      OUT1 = -1;
      }
   */


   if ((request.indexOf("/OUT2=ON") != -1)  ) {
      analogWrite(PIN_OUT2, 1023);
      OUT2 = +1;
   }
   if ((request.indexOf("/OUT2=OFF") != -1) ) {
      analogWrite(PIN_OUT2, 0);
      OUT2 = 0;
   }
   /*
      if ((request.indexOf("/OUT2=REV") != -1) ) {
      analogWrite(PIN_OUT2, 1023);
      digitalWrite(PIN_O2, HIGH);
      OUT2 = -1;
      }
   */

   /*
      if ((request.indexOf("/OUT3=ON") != -1)  ) {
      digitalWrite(PIN_O3, HIGH);
      OUT3 = +1;
      }
      if ((request.indexOf("/OUT3=OFF") != -1) ) {
      digitalWrite(PIN_O3, LOW);
      OUT3 = 0;
      }
   */

   //---------------------------------------
   // Match the request on Client 0

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



   //---------------------------------------
   // Match the request for Client 1

   if ((request.indexOf("/c1out1=ON") != -1)  ) {
      c1out1 = +1;
   }
   if ((request.indexOf("/c1out1=OFF") != -1) ) {
      c1out1 = 0;
   }
   if ((request.indexOf("/c1out1=REV") != -1) ) {
      c1out1 = -1;
   }


   if ((request.indexOf("/c1out2=ON") != -1)  ) {
      c1out2 = +1;
   }
   if ((request.indexOf("/c1out2=OFF") != -1) ) {
      c1out2 = 0;
   }


   if ((request.indexOf("/c1out3=ON") != -1)  ) {
      c1out3 = +1;
   }
   if ((request.indexOf("/c1out3=OFF") != -1) ) {
      c1out3 = 0;
   }


   //---------------------------------------
   // Match the request for Client 2

   if ((request.indexOf("/c2out2=ON") != -1)  ) {
      c2out2 = +1;
   }
   if ((request.indexOf("/c1out1=OFF") != -1) ) {
      c2out2 = 0;
   }

   //---------------------------------------
   // LogOut
   if (request.indexOf("/logout") != -1)  {
      authorized = false;
      return;
   }

   // confirm

   if ((request.indexOf("/CONFIRM=ON") != -1) && EMERGENCYRESET == false )  {
      resetEmergencyAlarm();
   }

   delay(1);

   //---------------------------------------
   // Return the response

   String script = "";

   // init website

   script += ("HTTP/1.1 200 OK \n");
   script += ("Content-Type: text/html \n");
   script += ("\n"); //  do not forget this one
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
   script += ("! </p> </h1>  "); // script+= ("! </p> </h1> \n");

   // date + time
   script += "<h2><p style=\"color:rgb(0,205,102);\"> " ;
   script += datestr + " &nbsp; <wbr> <wbr> " + timestr + " &nbsp; <wbr> <wbr> <wbr> ";
   script += "<font style=\"color:rgb(0,0,0);\" >  ";

   script += ("<br>");
   script += "<font style=\"color:rgb(255,0,0);\"> " ;
   script += " Notfall Alarms: " + String(EmergencyCnt);
   script += "<font style=\"color:rgb(255,209,22);\"> " ;
   script += " &nbsp; <wbr> <wbr> <wbr> Warnungen: " + String(AlertCnt);
   script += "</p> </h2> ";

   delay(1);

   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += ("<font style=\"color:rgb(0,0,0);\" > ");

   //script+= ("<br>"); //("<br> \n");

   script += "<h3> " + (String)(trestalive) + " ";
   script += ("<a href=\" /CONFIRM=ON\"\"> <button style=\"height:35px;width:70px\" > Confirm </button></a> </h3> ");

   //---------------------------------------
   script +=  "<h2> <br> \n  HEIMSERVER  <br> \n </h2>";
   //---------------------------------------
   // remote buttons Server
   // <input type="button" value="submit" style="height: 100px; width: 100px; left: 250; top: 250;">
   // <button style=\"height:200px;width:200px\"> </button>

   script += (OUT1name + " ist: "); // script+= (" <h2>" + OUT1name + " ist: ");
   if (OUT1 == 1)
   {
      script += ("SCHARF &nbsp; <wbr> <wbr> ");
   }
   else if (OUT1 == -1)
   {
      script += ("REV &nbsp; <wbr> <wbr> ");
   }
   else
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
   }
   script += ("<a href=\" /OUT1=ON\"\"> <button style=\"height:70px;width:140px\" > SCHARF </button></a>  ");
   script += ("<a href=\" /OUT1=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a>  ");
   /*
      script += ("<a href=\" /OUT1=REV\"\"> <button style=\"height:70px;width:140px\" > SCHLIESSEN </button></a> "); // <br/>
   */
   script += ("<br> \n\n");


   script += (OUT2name + " ist: ");
   if (OUT2 == 1)
   {
      script += ("SCHARF &nbsp; <wbr> <wbr> ");
   }

   /*
      else
      if(OUT2 == -1)
      { script+= ("Rev &nbsp; <wbr> <wbr> ");  }
   */
   else
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
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

   script +=    "<caption>  Messwerte HEIMSERVER </caption> ";

   script +=     "<thead> ";
   // Zeile 1 Server
   script +=      "<tr> ";
   script +=         "<td bgcolor='orange'> " + STATION1name + " </td>";
   script +=         "<td bgcolor='yellow'> °Cmin  </td>";
   script +=         "<td bgcolor='yellow'> °Cmax  </td>";
   script +=         "<td bgcolor='orange'> rF%  </td>";
   script +=         "<td bgcolor='orange'>" + (String)A0intname + " % </td>";
   script +=         "<td bgcolor='orange'>" + (String)A0muxname + " % </td>";
   script +=     "</tr> ";
   script +=     "</thead> ";

   script +=     "<tbody> ";

   // Zeile 2 Server
   script +=       "<tr> ";
   script +=         (String)"<th>" + t1.sact  + " °C  </th> ";
   script +=         (String)"<th>" + t1.smin   + "</th> ";
   script +=         (String)"<th>" + t1.smax   + "</th> ";
   script +=         (String)"<th>" + h1.sact + "</th> ";
   script +=         "<th>" + (String)espA0.sact + "</th> ";
   script +=         "<th>" + (String)("")  + "</th> ";
   script +=       "</tr> ";

   /*
      // Zeile 3 Server
      script +=       "<tr> ";
      script +=         "<td bgcolor='SkyBlue'>  - </td>";
      script +=         "<td bgcolor='SkyBlue'>  - </td>";
      script +=         "<td bgcolor='SkyBlue'>  -  </td>";
      script +=         "<td bgcolor='SkyBlue'>  -  </td>";
      script +=         "<td bgcolor='Avocado'>" + (String)A0intname + " V </td>";
      script +=         "<td bgcolor='Avocado'>" + (String)A0muxname + " V </td>";
      script +=       "</tr> ";

      // Zeile 4 Server
      script +=       "<tr> ";
      script +=         "<th>" + (String)(" - ") + "</th> ";
      script +=         "<th>" + (String)(" - ") + "</th> ";
      script +=         "<th>" + (String)(" - ") + "</th> ";
      script +=         "<th>" + (String)(" - ") + "</th> ";
      script +=         "<th>" + (String)sfespA0 + "</th> ";
      script +=         "<th>" + (String)sfadc0  + "</th> ";
      script +=       "</tr> ";
   */

   script +=     "</tbody> ";

   script +=   "</table>  ";
   script += "</h2>";

   client.print(script);
   script = "";



   //---------------------------------------
   script +=  "<h2> <br> \n  C0: GARTEN  <br> \n </h2>";
   //---------------------------------------
   // remote buttons Client 0

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
   /*
      else
      if(c0out2 == -1)
      { script+= ("Rev &nbsp; <wbr> <wbr> ");  }
   */
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
   /*
      else
      if(c0out3 == -1)
      { script+= ("Rev &nbsp; <wbr> <wbr> ");  }
   */
   else
   {
      script += ("AUS &nbsp; <wbr> <wbr> ");
   }
   script += ("<a href=\" /c0out3=ON\"\"> <button style=\"height:70px;width:140px\" > EIN </button></a>  ");
   script += ("<a href=\" /c0out3=OFF\"\"> <button style=\"height:70px;width:140px\" >  AUS  </button></a> ");
   script += ("<br> \n");



   client.print(script);
   script = "";


   //---------------------------------------
   // sensors  Client 0
   // chart table
   //---------------------------------------

   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += "<h2> ";
   script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;
   script += ("<br> \n");
   script += "<table border=4 cellpadding=4>"; // "<table>";

   script +=  "<caption>  Messwerte Garten ";
   script +=  "(Fehlerdauer: " + (String)( (millis() - c0t1.tact) / (1000ul * 60) ) + " min)" ;
   script +=  "</caption>" ;

   script +=     "<thead> ";
   // Zeile 1 Client 0
   script +=      "<tr> ";
   script +=         "<td bgcolor='SkyBlue'> " + c0STATION1name + " </td>";
   script +=         "<td bgcolor='yellow'> °Cmin  </td>";
   script +=         "<td bgcolor='yellow'> °Cmax  </td>";
   script +=         "<td bgcolor='SkyBlue'> rF%  </td>";
   script +=         "<td bgcolor='Avocado'>  " + c0STATION2name + " </td>";
   script +=         "<td bgcolor='yellow'> °Cmin  </td>";
   script +=         "<td bgcolor='yellow'> °Cmax  </td>";
   script +=         "<td bgcolor='Avocado'> rF%  </td>";
   script +=         "<td bgcolor='Avocado'>   -   </td>";
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
   script +=         "<th>   -   </th> ";
   script +=       "</tr> ";

   // Zeile 3 Client 0
   script +=       "<tr> ";
   script +=         "<td bgcolor='SkyBlue'> hPa </td>";
   script +=         "<td bgcolor='SkyBlue'> hPa ∅ </td>";
   script +=         "<td bgcolor='SkyBlue'>  -  </td>";
   script +=         "<td bgcolor='SkyBlue'>  -  </td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c0A0intname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c0A0muxname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c0A1muxname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c0A2muxname + "</td>";
   script +=         "<td bgcolor='Avocado'>" + (String)c0A3muxname + "</td>";
   script +=       "</tr> ";

   // Zeile 4 Client 0
   strcpy(dsymbol, tendencysymbol(c0p1.vact - c0p1.vmean) );
   script +=  "<tr> ";
   script +=      "<th>" + (String)(int)round(c0p1.vact)  + " " + (String)dsymbol + "</th> ";
   script +=      "<th>" + (String)(int)round(c0p1.vmean) + " </th> ";
   script +=      "<th>" + (String)(" - ")  + "</th> ";
   script +=      "<th>" + (String)(" - ")  + "</th> ";
   script +=      "<th>" + (String)c0espA0.sact + "</th> ";
   if (c0adc0.talert > 0) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c0adc0.sact + "</th> ";
   if (c0adc1.talert > 0) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=               (String)c0adc1.sact + "</th> ";
   script +=      "<th>" + (String)c0adc2.sact + "</th> ";
   script +=      "<th>" + (String)c0adc3.sact + "</th> ";
   script +=    "</tr> ";

   script +=  "</tbody> ";

   script +=   "</table> ";
   script += "</h2>";

   client.print(script);
   script = "";






   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += ("<font style=\"color:rgb(0,0,0);\" > ");
   //---------------------------------------
   script +=  "<h2> <br> \n  C1: KELLER <br> \n </h2>";
   //---------------------------------------

   // remote buttons Client C1

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

   else
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
   script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;
   script += ("<br> \n");
   script += "<table border=4 cellpadding=4>"; // "<table>";

   script +=  "<caption>  Messwerte Keller ";
   script +=  "(Fehlerdauer: " + (String)( (millis() - c1t1.tact) / (1000ul * 60) ) + " min)" ;
   script +=  "</caption>" ;

   // client 1 Zeile 1
   script +=     "<thead> ";
   script +=       "<tr> ";
   script +=       "<td bgcolor='LightCyan'> " + C1SENSORname1 + "  </td>";
   script +=       "<td bgcolor='yellow'> °Cmin  </td>";
   script +=       "<td bgcolor='yellow'> °Cmax  </td>";
   script +=       "<td bgcolor='yellow'> °C ∅  </td>";
   script +=       "<td bgcolor='LightCyan'> rF%   </td>";
   script +=       "<td bgcolor='orange'>  " + C1SENSORname2 + " </td>";
   script +=       "<td bgcolor='yellow'> °Cmin </td>";
   script +=       "<td bgcolor='yellow'> °Cmax </td>";
   script +=       "<td bgcolor='orange'> rF%  </td>";
   script +=       "</tr> ";
   script +=     "</thead> ";

   // client 1 Zeile 2
   script +=     "<tbody> ";
   script +=       "<tr> ";
   if (c1t1.talert > 0) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=         (String)c1t1.sact  + " °C </th> ";
   script +=         (String)"<th>" + c1t1.smin   + "</th> ";
   script +=         (String)"<th>" + c1t1.smax   + "</th> ";
   script +=         (String)"<th>" + c1t1.smean  + "</th> ";
   script +=         (String)"<th>" + c1h1.sact + "</th> ";
   script +=         (String)"<th>" + c1t2.sact  + " °C </th> ";
   script +=         (String)"<th>" + c1t2.smin   + "</th> ";
   script +=         (String)"<th>" + c1t2.smax   + "</th> ";
   script +=         (String)"<th>" + c1h2.sact + "</th> ";
   script +=       "</tr> ";


   /*
      // client 1 Zeile 3
      script+=       "<tr> ";
      script+=         "<td bgcolor='orange'>" + (String)A0intname + "</td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=       "</tr> ";

      // client 1 Zeile 4
      script+=       "<tr> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th>   ";
      script+=       "</tr> ";
   */

   script +=     "</tbody> ";
   script +=   "</table>  ";
   script += "</h2> ";


   script += ("<br> \n");


   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += ("<font style=\"color:rgb(0,0,0);\" > ");
   //---------------------------------------
   script +=  "<h2> <br> \n  C2: KÜCHE <br> \n </h2>";
   //---------------------------------------


   //---------------------------------------
   // remote buttons Client C2

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


   // remote client 2 sensors
   // chart table
   //---------------------------------------
   // text font Courier, color black
   script += ("<p> <font face=\"courier\"> "); // <<< Courier
   script += "<h2> ";
   script += "<p style=\"color:rgb(0,0,0);\" > </p>  " ;
   script += ("<br> \n");
   script += "<table border=4 cellpadding=4>"; // "<table>";

   script +=  "<caption>  Messwerte Küche ";
   script +=  "(Fehlerdauer: " + (String)( (millis() - c2t1.tact) / (1000ul * 60) ) + " min)" ;
   script +=  "</caption>" ;

   // client 2 Zeile 1
   script +=  "<thead> ";
   script +=    "<tr> ";
   script +=    "<td bgcolor='LightCyan'> " + C2SENSORname1 + "  </td>";
   script +=    "<td bgcolor='yellow'> °Cmin  </td>";
   script +=    "<td bgcolor='yellow'> °Cmax  </td>";
   script +=    "<td bgcolor='yellow'> °C ∅  </td>";
   script +=    "<td bgcolor='LightCyan'> rF%   </td>";
   script +=    "<td bgcolor='orange'>  " + C2SENSORname2 + " </td>";
   script +=    "<td bgcolor='yellow'> °Cmin </td>";
   script +=    "<td bgcolor='yellow'> °Cmax </td>";
   script +=    "<td bgcolor='orange'> rF%  </td>";
   script +=    "</tr> ";
   script +=  "</thead> ";

   // client 2 Zeile 2
   script +=  "<tbody> ";
   script +=    "<tr> ";
   if (c2t1.talert > 0) {
      script += (String)"<td bgcolor='red'>";
   } else script += (String)"<th>";
   script +=      (String)c2t1.sact  + " °C </th> ";
   script +=      (String)"<th>" + c2t1.smin   + "</th> ";
   script +=      (String)"<th>" + c2t1.smax   + "</th> ";
   script +=      (String)"<th>" + c2t1.smean  + "</th> ";
   script +=      (String)"<th>" + c2h1.sact   + "</th> ";
   script +=      (String)"<th>" + c2t2.sact  + " °C </th> ";
   script +=      (String)"<th>" + c2t2.smin   + "</th> ";
   script +=      (String)"<th>" + c2t2.smax   + "</th> ";
   script +=      (String)"<th>" + c2h2.sact + "</th> ";
   script +=    "</tr> ";


   /*
      // client 2 Zeile 3
      script+=       "<tr> ";
      script+=         "<td bgcolor='orange'>" + (String)A0intname + "</td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=         "<td bgcolor='orange'>  -  </td>";
      script+=       "</tr> ";

      // client 2 Zeile 4
      script+=       "<tr> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th> ";
      script+=         "<th>" +(String)" - " + "</th>   ";
      script+=       "</tr> ";
   */

   script +=     "</tbody> ";
   script +=   "</table>  ";
   script += "</h2> ";


   script += ("<br> \n");


   // log out
   script += ("<h3>Log Out: ");
   script += ("<a href=\" /logout\"\"> <button style=\"height:70px;width:140px\" > Log Out </button></a> </h3> ");

   script += ver + " " + WiFi.localIP().toString() + " " + (String)ssid + " <br>" ;
   script += "</font> </p> \n";

   script += "</body> \n";
   script += "</html> \n";

   client.print(script);

   delay(1);

}

//----------------------------------------------------------------------------
// handle root +clients
//----------------------------------------------------------------------------

void handleRoot() {
   commClients();
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

void commClients() {

   //printUrlArg(); //fuer Debug Zwecke

   double ftmp;
   String msgtok;

   //------------------------------------------

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

   //------------------------------------------

   msgtok = webserver.arg("c2LocAlive"); // <<<< c2 alive btn pressed?
   if (msgtok != "") {
      int itemp = msgtok.toInt();
      if (itemp == 1) {
         resetEmergencyAlarm();
      }
   }

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

   //------------------------------------------

   //Werte auch bei Url-Aufruf zurückgeben

   String message = "*** ";
   message += "&trestalive=" + (String)trestalive;
   // re CLIENT 0
   message += (String)"&c0t1=" + c0t1.sact + "&c0h1=" + c0h1.sact;
   message += (String)"&c0t2=" + c0t2.sact + "&c0h2=" + c0h2.sact;
   message += "&c0out1=" + (String)c0out1 + "&c0out2=" + (String)c0out2 + "&c0out3=" + (String)c0out3 ;
   // re CLIENT 1
   message += (String)"&c1t1=" + c1t1.sact + "&c1h1=" + c1h1.sact;
   message += (String)"&c1t2=" + c1t2.sact + "&c1h2=" + c1h2.sact;
   message += "&c1out1=" + (String)c1out1;
   message += "&c1out2=" + (String)c1out2;
   message += "&c1out3=" + (String)c1out3;
   // re CLIENT 2
   message += "&c2out1=" + (String)c2out1;
   message += "&c2out2=" + (String)c2out2;
   message += "&alertcnt=" + (String)AlertCnt;
   message += "&remindtcnt=" + (String)RemindCnt;
   message += " ###";
   Serial.println(message);
   webserver.send(200, "text/plain", message);

}




//----------------------------------------------------------------------------
// UDP Time
//----------------------------------------------------------------------------

/*-------- NTP code ----------*/

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

//----------------------------------------------------------------------------
// SendEmail
//----------------------------------------------------------------------------
int  SendEmail(String mail_to, String mail_subj, String mail_msg)
{
   int err = 0;

   String serror = "";
   display.clearDisplay();
   display.setFont(&FreeSans9pt7b);
   display.setCursor(0, 20);

   if (gsender->Subject(mail_subj)->Send(mail_to, mail_msg)) { // <<<< SEND EMAIL MESSAGE
      Serial.println("Email send.");
      display.setFont(&FreeSans9pt7b);
      display.print( "Email send." ); display.display();
      err = 0;
   }
   else   {
      serror = (String)gsender->getError();
      Serial.println("Error Email: " + serror);
      display.setFont(&FreeSans9pt7b);
      display.print("Error Email:\n" + serror);
      display.setFont();
      display.display();
      err = -1;
   }
   delay(1000);
   return err;
}





//----------------------------------------------------------------------------
// REFERENCES
//----------------------------------------------------------------------------
/*
    Lit.:
    http://www.instructables.com/id/Quick-Start-to-Nodemcu-ESP8266-on-Arduino-IDE/ ,
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

*/

//----------------------------------------------------------------------------
// END OF FILE
//----------------------------------------------------------------------------
