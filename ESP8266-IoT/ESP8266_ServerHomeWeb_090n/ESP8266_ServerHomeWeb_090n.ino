//----------------------------------------------------------------------------
//  ESP8266 NodeMCU
//  wifiserver,  webserver + udpclient
//  ESP8266 WiFiServer für Client-comm (remote client sensor + motor comm)
//  ESP8266 WebServer für html-Website dashboard
//
//  nodeMCU 1.0 board ver 2.6.3 OK (test: 2.7.4)
//
// History:
// 0.9.0n:  fremde Zugriffe (keine Geisterschaltungen)
// 0.9.0m:  htmlButtons ok für PC+Android, logout noch immer für alle websites
// 0.9.0l:  cleanup aus 0.9.0j
// 0.9.0k:  NTP-UDP timezone time_t aktualisiert
// 0.9.0j:  große logIn Felder; c3 buttons auch als c0 Buttons
// 0.9.0i:  ? noch kleine logIn Felder
// 0.9.0h:  statt request.indexOf() jetzt path==
// 0.9.0g:  neues authorized Handling
// 0.9.0f:  3.+4. Zeile Tabelle c0, Sensor c3a1 gefixt
// 0.9.0e:  3.+4. Zeile Tabelle c3
// 0.9.0d:  ?
// 0.9.0c:  handlenotRoot() neu
// 0.9.0b:  handlenotAuthorized() neu
// 0.9.0b:  handleWebsite() neu
// 0.8.9c: send Client init-Werte
// 0.8.9:  min/maxreset buttons
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
// 0.7.2 core 2.4.0, ADC reworked
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

#define TARGET 'Z'  // Server-Zielpattform (Z,T,Q => versch. IPs, Ports, urls)
String  ver = (String)TARGET +  ".090n" ;

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
   D3      0       FLASH/LED      in  D3     built-in btn+LED     out D3 motorA
   D4      2       TX1            out D4        ---               out D4 motorB
   D5     14       SPI SCK        in  D5        in D5 (DHT)       SDA
   D6     12       SPI MISO       out D6        out D6            SCL
   D7     13       SPI MOSI       in D7         in D7 (DHT)       in  D7
   D8     15       MTD0 PWM       out D8        ---               out D8
   D9      3       UART RX0       USB, Serial   USB, Serial       USB, Serial
   D10     1       UART TX0       USB, Serial   USB, Serial       USB, Serial
   ---    10       intern ?       ---           ---               ---

   A0     IR Feuersensor, Alarm <10% // alt: MQ-2 LPG Rauch+Gas , Alarm>70%

*/

/*
   

digital  gpio         myStd Server         
   D0     16            out/LED         
   D1      5            I2C SCL        
   D2      4            I2C SDA        
   D3      0            in  D3      
   D4      2            out D4       
   D5     14            in  D5        
   D6     12            out D6        
   D7     13            in D7         
   D8     15            out D8        
   D9      3            USB, Serial    
   D10     1            USB, Serial   
   ---    10             ---            

   A0     IR Feuersensor, Alarm <10% // alt: MQ-2 LPG Rauch+Gas , Alarm>70%

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

#define PIN_RESETAlarm  D3      //  Btn 0=D3 reset Alarm

#define PIN_OUT0     LED_BUILTIN      //  out Alert LED_BUILTIN
#define PIN_OUT1     D6               //  out Alarmanlage ext
#define PIN_OUT2     D8               //  out Alarmanlage int
#define PIN_OUT3     D4               //  Alarm-Sirene pwm/tone



//----------------------------------------------------------------------------
// Display OLED SSD1306 + LiquidCrystal_I2C
//----------------------------------------------------------------------------

#include <ESP_SSD1306.h>        // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>       // Needs a little change in original Adafruit library (See README.txt file)
#include <Fonts/FreeSansBold12pt7b.h>      // 
#include <Fonts/FreeSans9pt7b.h>           //
#include <Fonts/FreeMono12pt7b.h>          //
ESP_SSD1306   display(-1);

//----------------------------------------------------------------------------
#include <LiquidCrystal_I2C.h> // Library for LCD    
LiquidCrystal_I2C  lcd = LiquidCrystal_I2C(0x27, 20, 4); // Change to (0x27,16,2) for 16x2 LCD.


//----------------------------------------------------------------------------
// GPIO outputs
//----------------------------------------------------------------------------
volatile int8_t  OUT0 = 0, OUT1 = 0, OUT2 = 0, OUT3 = 0; // Server; actual output pin states; stop=0, fwd=1, rev=-1;
volatile int8_t  c0out0=0, c0out1=0, c0out2=0, c0out3=0; // Client0; stop=0, fwd=1, rev=-1;
volatile int8_t  c0outMon0=0, c0outMon1=0, c0outMon2=0, c0outMon3=0;

volatile int8_t  c1out0=0, c1out1=0, c1out2=0, c1out3=0; // Client1; stop=0, fwd=1, rev=-1;

volatile int8_t  c2out0=0, c2out1=0, c2out2=0, c2out3=0; // Client2; stop=0, fwd=1, rev=-1;

volatile int8_t  c3out0=0, c3out1=0, c3out2=0, c3out3=0; // Client3; stop=0, fwd=1, rev=-1;
volatile int8_t  c3outMon0=0, c3outMon1=0, c3outMon2=0, c3outMon3=0;
volatile int16_t c0tx3=21; // Thermostat-Sollwert
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

unsigned long lastI2CDataMillis = 0;
unsigned long lastSensorDataMillis = 0;

// N.N barometric pressure adjust (250m)

const double  FNNcorr = 1013.0 - 984.0; // ca. 250m

const double fINVAL = -999.0;
float    fFireLIMIT =  6.0;   // fSmokeLIMIT Gas-S.= 82.0;

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
// i2c 
//----------------------------------------------------

void i2cBusReset() {

   pinMode(SCL, OUTPUT);
   pinMode(SDA, INPUT_PULLUP);

   for (int i = 0; i < 9; i++) {
      digitalWrite(SCL, LOW);
      delayMicroseconds(5);
      digitalWrite(SCL, HIGH);
      delayMicroseconds(5);
   }

   Wire.begin();
} // Ende i2cBusReset


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
   Pin No.   Pin Name Pin Description
   1     VCC      Power source of 3.3VDC
   2     GND      Ground
   3     SCL      Serial Clock
   4     SDA      Serial Data
   5     CSB      CSB pin to GND to have SPI and to VCC(3.3V) for I2C.
   6     SDO      Serial Data Out / Master In Slave Out pin, for data sent
   f r Adafruit: default Adresse 0x77 => SDO-pin HIGH! (CS nicht verbunden!)
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



//  Colors

#define  SIGNYellow   255,209,22
#define  ROSE         255,0,204
#define  SPRINGGREEN3 0,205,102
#define  SkyBlue               #6698FF
#define  LightCyan             #58FAD0

//----------------------------------------------------------------------------
// WiFi Libs + dependencies
//----------------------------------------------------------------------------


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//----------------------------------------------------------------------------
// 090n -> 091a
//----------------------------------------------------------------------------

// COMPILER-BRÜCKE: Sichert Variablen und Typen vor Tab-Konflikten ab für 091a
#include <WiFiClient.h>

extern int auth_realm_counter;

String basicAuthString(String username, String password);

void handleWebsite(WiFiClient client, String HTTP_req);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


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




//----------------------------------------------------------------------------
// MADAM = Maintainance, display, and alert management
//----------------------------------------------------------------------------
void systemWatchdog() {

   unsigned long now = millis();

   // I2C Daten stehen?
   if (now-lastI2CDataMillis>10000 || millis()-lastSensorDataMillis>15000) {   // z.B. 10s keine neuen Werte
      Serial.println("WATCHDOG: I2C stalled");

      i2cBusReset();
      delay(50);

      ESP.restart();   // Eskalation
   }
} // Ende systemWatchdog



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

int      auth_realm_counter = 1;

//----------------------------------------------------------------------------
// Internet Udp Time
//----------------------------------------------------------------------------

//#include <Time.h>       // (alt) Arduino Time lib https://github.com/PaulStoffregen/Time
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
TimeChangeRule CEST = { "CEST", Last, Sun, Mar, 2, 120 };   //Central European Summer Time
TimeChangeRule CET = { "CET ", Last, Sun, Oct, 3, 60 };     //Central European Standard Time
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
//char    STR_DEGREE[] = {247, 0, 0};                 // ° OLED font specimen (°,°C,°F,K)



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

#include <stdarg.h>

// ssprintf: Aufruf wie String ssprintf("%d %f", x, y);
String ssprintf(const char* format, ...) {
  size_t bufferSize = 64;  // Startpuffer
  char* buffer = new char[bufferSize];
  int n;

  while (true) {
    va_list args;
    va_start(args, format);
    n = vsnprintf(buffer, bufferSize, format, args);
    va_end(args);

    if (n < 0) {
      delete[] buffer;
      return String("");  // Formatfehler
    }

    if ((size_t)n < bufferSize) {
      // Passt → String erzeugen
      String result(buffer);
      delete[] buffer;
      return result;
    }

    // Puffer war zu klein → vergrößern
    delete[] buffer;
    bufferSize = n + 1;   // exakt benötigte Größe
    buffer = new char[bufferSize];
  }
} // Ende:   ssprintf




// Tendenz-Symbol

char dsymbol[4] = "=\0";

char * tendencysymbol(float dpromille) {
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
      yield();
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
      tone(PIN_OUT3, 440, 250);
      delay(500);
   }
   else digitalWrite(PIN_OUT3, 0);


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
      display.setCursor( 0, 12);  display.print(datestr+" "+timestr);
      display.setCursor( 0, 28);  display.print(ssprintf("Server OUT  %d %d %d", OUT1, OUT2, OUT3 ));
      display.setCursor( 0, 44);  display.print("Svr Innen T = " + (String)svt1.sact + "'C");
      display.setCursor( 0, 60);  display.print("    AIR-Q   = " + (String)svespA0.sact + "%");
      if(!refreshcntr%10) {                  // <<<<<<<<<<<<<<<<<<<  ????
         lcd.setCursor(0,0); lcd.print((datestr+" "+timestr));
         lcd.setCursor(0,1); lcd.print(ssprintf("Server OUT  %d %d %d", OUT1, OUT2, OUT3 ));
         lcd.setCursor(0,2); lcd.print("Svr Innen T = " + (String)svt1.sact + "'C");
         lcd.setCursor(0,3); lcd.print("AIR-Q       = " + (String)svespA0.sact + "%");
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
         lcd.setCursor(0,0); lcd.print(ssprintf("C0 GwH OUT  %d %d %d", c0out1, c0out2, c0out3 ));
         lcd.setCursor(0,1); lcd.print("Aussen T= " + (String)c0t1.sact + "'C");
         lcd.setCursor(0,2); lcd.print("Innen  T= " + (String)c0t2.sact + "'C");
         lcd.setCursor(0,3); lcd.print("AIR-Q   = " + (String)c0espA0.sact + " %");
      }
   }

   else if (mode == 4) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c0 GwH A0= " + (String)c0adc0.sact);
         lcd.setCursor(0,1); lcd.print("c0 GwH A1= " + (String)c0adc1.sact);
         lcd.setCursor(0,2); lcd.print("c0 GwH A2= " + (String)c0adc2.sact);
         lcd.setCursor(0,3); lcd.print("c0 GwH A3= " + (String)c0adc3.sact);
      }
   }
   else if (mode == 5) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print(ssprintf("C1 Kel OUT  %d %d %d", c1out1, c1out2, c1out3 ));
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
         lcd.setCursor(0,0); lcd.print(ssprintf("C2 Kue OUT  %d %d %d", c2out1, c2out2, c2out3 ));
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
         lcd.setCursor(0,0); lcd.print(ssprintf("C3 GwH OUT  %d %d %d", c3out1, c3out2, c3out3 ));
         lcd.setCursor(0,1); lcd.print("Aussen T= " + (String)c3t1.sact + "'C");
         lcd.setCursor(0,2); lcd.print("Innen  T= " + (String)c3t2.sact + "'C");
         lcd.setCursor(0,3); lcd.print("AIR-Q   = " + (String)c3espA0.sact + " %");
      }
   }

   else if (mode == 8) { // c0adc
      display.setFont(&FreeSans9pt7b);
      display.setCursor( 0, 12);  display.print("");
      display.setCursor( 0, 28);  display.print("");
      display.setCursor( 0, 44);  display.print("");
      display.setCursor( 0, 60);  display.print("");
      if(!refreshcntr%10) {
         lcd.setCursor(0,0); lcd.print("c3 GwH A0= " + (String)c3adc0.sact);
         lcd.setCursor(0,1); lcd.print("c3 GwH A1= " + (String)c3adc1.sact);
         lcd.setCursor(0,2); lcd.print("c3 GwH A2= " + (String)c3adc2.sact);
         lcd.setCursor(0,3); lcd.print("c3 GwH A3= " + (String)c3adc3.sact);
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
      display.setCursor( 0, 12);  display.print(datestr+" "+timestr);
      display.setCursor( 0, 28);  display.print(ssprintf("Server OUT  %d %d %d", OUT1, OUT2, OUT3 ));
      display.setCursor( 0, 44);  display.print("Svr Innen T = " + (String)svt1.sact + "'C");
      display.setCursor( 0, 60);  display.print("    AIR-Q   = " + (String)svespA0.sact + "%");
      if(!refreshcntr%10) {                  // <<<<<<<<<<<<<<<<<<<  ????
         lcd.setCursor(0,0); lcd.print((datestr+" "+timestr));
         lcd.setCursor(0,1); lcd.print(ssprintf("Server OUT  %d %d %d", OUT1, OUT2, OUT3 ));
         lcd.setCursor(0,2); lcd.print("Svr Innen T = " + (String)svt1.sact + "'C");
         lcd.setCursor(0,3); lcd.print("AIR-Q       = " + (String)svespA0.sact + "%");
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


void wifiConnect() {
   int progress=0;

   unsigned long wifiStartMillis = millis();          // <<< NEU: Startzeit
   const unsigned long WIFI_TIMEOUT = 20000;          // <<< NEU: Timeout 20s

   Serial.println();
   Serial.println("Connecting to WiFi... ");
   lcd.setCursor(0, 1);  lcd.print("Connecting to WiFi: ");

   WiFi.mode(WIFI_STA);
   WiFi.config(this_ip, gateway, subnet, gateway, gateway);   // dns = gateway
   WiFi.begin(ssid, password);

   while (WiFi.status() != WL_CONNECTED) {

      if (millis() - wifiStartMillis > WIFI_TIMEOUT) {   // <<< NEU: Timeout-Check
         Serial.println();
         Serial.println("WiFi connect TIMEOUT -> restart");
         ESP.restart();
      }

      delay(500);
      yield();                                           // <<< NEU: WDT / WiFi Stack
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

   delay(100);  
   display.clearDisplay();
   progress = 100;

   display.setCursor( 0, 20);  display.print("WiFi connecting - ");
   drawHorizontalBargraph( 0, 30, (int16_t) display.width(), 9, 1, progress);
   display.setCursor( 0, 40);  display.print((String)progress + "%");
 
   display.setCursor( 0, 50);  display.print(WiFi.gatewayIP());
   display.display();
   lcd.setCursor(0, 2);  lcd.print("Gateway ID: ");  lcd.print(WiFi.gatewayIP());   
   Serial.println();
   Serial.print("Gateway ID: "); Serial.println(WiFi.gatewayIP());

} // Ende wifiConnect


//----------------------------------------------------------------------------
// SETUP
//----------------------------------------------------------------------------
void setup() {

   int IORes;
   //int progress = 0;


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

   Wire.pins(SDA, SCL);        // SDA, SCL
   Wire.begin();
   Wire.setClock(100000ul);
   Wire.setTimeout(50);
   
   yield();

   //----------------------------------------
   // i2c: PCF8574

   PCFx20.write8(0xFF); // (all pins INPUT HIGH)
   yield();

   //----------------------------------------
   // OLED
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
   display.setFont();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.clearDisplay();
   display.setCursor( 0, 0);  display.print("OLED display init OK");
   display.display();
   yield();
   Serial.println("OLED display init OK");

   //----------------------------------------
   //  LCD
   lcd.init();
   lcd.backlight();
   lcd.noBlink();
   Serial.println("LCD display init OK");
   lcd.setCursor(0, 0);  lcd.print("LCD display init OK");
   Serial.println();


   /**
      //----------------------------------------
      // i2c:  ADS1115

      ADSx48.begin(0x48); // ADS1115 I2C 4x 12/16bit ADC
      yield();
      Serial.println("ADS1115 sensor init... ");
   */



   //----------------------------------------
   // Init MCP3421: I2C-Adresse, 18 Bit Modus, keine Verstärkung
   Serial.println("ADCmcp3421 sensor init... ");
   ADCmcp3421.init(0x68, 3, 0);
   Serial.println("ADCmcp3421 sensor done. ");
   Serial.println();

   /*
      //----------------------------------------
      // i2c:  MCP9808

      IORes=MCP9808_T.begin();
      yield();
      if (!IORes) {
      Serial.println("Couldn't find MCP9808!");
      }
      else
      Serial.println("MCP9808 sensor init: OK");

   */


   //----------------------------------------
   // i2c: BMP280
   Serial.println("bmp_x77 sensor init... ");
   IORes = bmp_x77.begin();
   if (!IORes) {
      Serial.println("Couldn't find BMP280 sensor!");
   }
   else {
      Serial.println("BMP280 sensor init: OK.");
   }
   Serial.println();
   yield();



   //----------------------------------------
   // Connect to WiFi network
   
   Serial.println();
   wifiConnect();
   Serial.println("WiFi connected ");
   delay(500);

   //----------------------------------------
   // Start the WiFi server (-> www)
   wifiserver.begin();
   Serial.println("ESP wifiserver started");

   //----------------------------------------
   // Start the ESP LAN server (-> ESP client)
   webserver.on("/", handleRoot) ;
   webserver.on("/client/client0/", handleClients);
   delay(10);
   webserver.on("/client/client1/", handleClients);
   delay(10);
   webserver.on("/client/client2/", handleClients);
   delay(10);
   webserver.on("/client/client3/", handleClients);
   delay(10);
   webserver.begin();
   Serial.println("ESP webserver started");

   // Print the IP address
   Serial.print("Use this URL to connect: ");
   Serial.print("http://");
   Serial.print(WiFi.localIP());
   Serial.print(":");
   Serial.print(http_port);
   Serial.println("/");
   Serial.print((String)website_url + ":" + http_port + "/");


   //----------------------------------------
   // Start NTP-UDP
   Serial.println("\nStarting UdpTime");
   UdpTime.begin(8888);
   //setSyncProvider(getNtpTime);
   yield();
   Serial.println("waiting for sync");
   updateTimeNow();
   buildDateTimeString();

   //----------------------------------------
   // reset watchdog confirm
   resetConfirmTime();


   //---------------------------------------- 
   // PRNG Initialisierung 
   randomSeed(micros() ^ analogRead(A0) ^ ESP.getChipId());


   // setup done
   LCDmode = 0;
   dashboard(LCDmode);
   Serial.println("setup done \n");

}


void  resetAllOutputs() {
   //reset Client init-Werte (auch bei Url-Aufruf zurückgeben)
   Serial.println("sending initial client vars");
   String message = "*** ";

   // re SERVER
   OUT0 = 0; message += "OUT0=" + (String)OUT0;
   OUT1 = 0; message += "OUT1=" + (String)OUT1;
   OUT2 = 0; message += "OUT2=" + (String)OUT2;
   OUT3 = 0; message += "OUT3=" + (String)OUT3;

   // re CLIENT 0
   c0out0=0; message += "&c1out0=" + (String)c1out0;
   c0out1=0; message += "&c0out1=" + (String)c0out1;
   c0out2=0; message += "&c0out2=" + (String)c0out2;
   c0out3=0; message += "&c0out3=" + (String)c0out3;
   // re CLIENT 1
   c1out0=0; message += "&c1out0=" + (String)c1out0;
   c1out1=0; message += "&c1out1=" + (String)c1out1;
   c1out2=0; message += "&c1out2=" + (String)c1out2;
   c1out3=0; message += "&c1out3=" + (String)c1out3;
   // re CLIENT 2
   c2out0=0; message += "&c2out0=" + (String)c2out0;
   c2out1=0; message += "&c2out1=" + (String)c2out1;
   c2out2=0; message += "&c2out2=" + (String)c2out2;
   c1out3=0; message += "&c1out3=" + (String)c1out0;
   // re CLIENT 3
   c3out0=0; message += "&c3out0=" + (String)c3out0;
   c3out1=0; message += "&c3out1=" + (String)c3out1;
   c3out2=0; message += "&c3out2=" + (String)c3out2 ;
   //c3out3=0; message += "&c3out3=" + (String)c3out2 ;   // c3out3=intern auto ctrl
   c3out3=0; message += "&c3tx3=" + (String)c3tx3;        // Thermostat für c3out3

   // all clients
   message += "&remindcnt=" + (String)RemindCnt;
   message += "&emergencycnt=" + (String)EmergencyCnt;

   message += " ###";
   Serial.println(message);
   webserver.send(200, "text/plain", message);

   Serial.println("delay(2000)... ");
   delay(2000);
   Serial.println("client msg sent.");

} // Ende resetAllOutputs


//---------------------------------------------
// Wrapper für html-Button, ver 090n (to do: actionID, neues Modell)
//---------------------------------------------

String htmlButton(String Label, String Shref, int h, int w, int fontSize = 16) {
   String buf;

   buf.reserve(120);
   buf  = "<a href=\"/" + Shref + "\">";
   buf += "<button style=\"height:" + String(h) + "px;width:" + String(w);
   buf += "px;font-size:" + String(fontSize) + "px\">";
   buf += Label;
   buf += "</button></a>";

   return buf;
} // Ende htmlButton

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////
// bis hierhin Original 090n
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////

// es folgen 091a Änderungen
//////////////////////////////////////////////////////////


//============================================================================
// ANFANG: ABSCHNITT A
// Firmware-Basis: 090n / 091a (Lückenloser Anschluss nach resetAllOutputs)
//============================================================================

void handleRoot() {
   handleClients();
} // Ende: handleRoot


void loop() {

   static double ftmp;
   static unsigned long tsec = millis(), tms = millis(), tms2 = millis() ;

   static unsigned long tWatchdog = 0;


   //---------------------------------------
   // Read incoming Web connections
   //---------------------------------------

   WiFiClient client = wifiserver.available();
   if (client) {
      while (client.connected() && !client.available()) {
         delay(1);
      }

      String req = client.readStringUntil('\r');

      //---------------------------------------
      // URL-Aktionsparser fuer Relais und Thermostate

      if (req.indexOf("CONFIRM=ON") != -1)  { millisLastConfirm = millis(); RemindCnt = 0; }

      if (req.indexOf("OUT1=ON") != -1)     OUT1 = 1;
      if (req.indexOf("OUT1=OFF") != -1)    OUT1 = 0;
      if (req.indexOf("OUT2=ON") != -1)     OUT2 = 1;
      if (req.indexOf("OUT2=OFF") != -1)    OUT2 = 0;

      if (req.indexOf("c0out1=ON") != -1)   c0out1 = 1;
      if (req.indexOf("c0out1=OFF") != -1)  c0out1 = 0;
      if (req.indexOf("c0out2=ON") != -1)   c0out2 = 1;
      if (req.indexOf("c0out2=OFF") != -1)  c0out2 = 0;

      if (req.indexOf("c0tx3=UP") != -1)    c0tx3++;
      if (req.indexOf("c0tx3=DN") != -1)    c0tx3--;

      if (req.indexOf("c1out1=ON") != -1)   c1out1 = 1;
      if (req.indexOf("c1out1=OFF") != -1)  c1out1 = 0;
      if (req.indexOf("c1out2=ON") != -1)   c1out2 = 1;
      if (req.indexOf("c1out2=OFF") != -1)  c1out2 = 0;

      if (req.indexOf("c2out1=ON") != -1)   c2out1 = 1;
      if (req.indexOf("c2out1=OFF") != -1)  c2out1 = 0;
      if (req.indexOf("c2out2=ON") != -1)   c2out2 = 1;
      if (req.indexOf("c2out2=OFF") != -1)  c2out2 = 0;

      if (req.indexOf("svreset") != -1) {
         svt1.vmin = 99.9; svt1.vmax = -99.9; svt1.vmean = svt1.vact;
         svh1.vmin = 99.9; svh1.vmax = -99.9; svh1.vmean = svh1.vact;
      }
      if (req.indexOf("c0reset") != -1) {
         c0t1.vmin = 99.9; c0t1.vmax = -99.9; c0t1.vmean = c0t1.vact;
         c0h1.vmin = 99.9; c0h1.vmax = -99.9; c0h1.vmean = c0h1.vact;
         c0t2.vmin = 99.9; c0t2.vmax = -99.9; c0t2.vmean = c0t2.vact;
         c0h2.vmin = 99.9; c0h2.vmax = -99.9; c0h2.vmean = c0h2.vact;
         c0adc0.vmin = 1023; c0adc0.vmax = 0; c0adc1.vmin = 1023; c0adc1.vmax = 0;
         c0adc2.vmin = 1023; c0adc2.vmax = 0; c0adc3.vmin = 1023; c0adc3.vmax = 0;
      }

      // ERWEITERUNG: Liest alle restlichen Header-Zeilen (inkl. Authorization) lückenlos ein
      while (client.available()) {
         String line = client.readStringUntil('\n');
         req += line + "\n";
         if (line == "\r" || line.length() == 0) break;
      }

      handleWebsite(client, req);

      delay(1);
      client.stop();
   }

   webserver.handleClient();
   delay(10);

   //---------------------------------------
   // Read local + Udp data

   EmergencyCnt = checkAlarms();
   if(EmergencyCnt==0) digitalWrite(PIN_OUT0, 1);
   else digitalWrite(PIN_OUT0, 0);

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
      //Serial.println(timestr+"   "+datestr);

      ftmp = fINVAL;
      ftmp = bmp_x77.readTemperature();
      if (isnan(ftmp)) ftmp = fINVAL;
      logval(ftmp, svt1);
      yield();
      lastI2CDataMillis = millis();

      ftmp = fINVAL;
      ftmp = FNNcorr + bmp_x77.readPressure() / 100.0 ;
      if (isnan(ftmp)) ftmp = fINVAL;
      logval(ftmp, svp1);
      double dhPa = svp1.vact - svp1.vmean;
      yield();
      lastI2CDataMillis = millis();


      ftmp = fINVAL;
      ftmp = (float)analogRead(A0);
      ftmp = calADC1023(ftmp);
      if (isnan(ftmp)) ftmp = 0;
      logval(ftmp, svespA0);
      yield();
      lastSensorDataMillis = millis();

      if ( millis() - tms2 >= 1000 ) {    // refresh Serial.println(message)rate
         tms2 = millis();

         Serial.println("Svr+Clients sensors+motors:");

         Serial.println((String)"Svr ");
         Serial.println((String)"  Svr OUT1  OUT2  OUT3  " + (String)OUT1 +"  " + OUT2 +"  " + OUT3);
         Serial.print(" c0_t1="); Serial.print(c0t1.sact);
         Serial.print(" c0_h1="); Serial.print(c0h1.sact);
         Serial.print(" c0_t2="); Serial.print(c0t2.sact);
         Serial.print(" c0_h2="); Serial.println(c0h2.sact);
         Serial.println((String)"  c0  out1  out2  out3  " + c0out1 +"  " + c0out2 +"  " + c0out3);

         Serial.print(" c1_t1="); Serial.print(c1t1.sact);
         Serial.print(" c1_h1="); Serial.print(c1h1.sact);
         Serial.print(" c1_t2="); Serial.print(c1t2.sact);
         Serial.print(" c1_h2="); Serial.println(c1h2.sact);
         Serial.println((String)"  c1  out1  out2  out3  " + c1out1 +"  " + c1out2 +"  " + c1out3);

         Serial.print(" c2_t1="); Serial.print(c2t1.sact);
         Serial.print(" c2_h1="); Serial.print(c2h1.sact);
         Serial.print(" c2_t2="); Serial.print(c2t2.sact);
         Serial.print(" c2_h2="); Serial.println(c2h2.sact);
         Serial.println((String)"  c1  out1  out2  out3  " + c2out1 +"  " + c2out2 +"  " + c2out3);

         Serial.println(" "); Serial.println(" ");
      }

      //---------------------------------------
      // display on OLED
      if ( millis() - tsec >= 4000 ) {
         tsec = millis();
         LCDmode++;
      }
      dashboard(LCDmode);
      yield();
      lastI2CDataMillis = millis();
   }

} // loop end

//============================================================================
// ENDE: ABSCHNITT A
//============================================================================

//============================================================================
// ANFANG: ABSCHNITT B
// Firmware-Basis: 090n / 091a (Multi-User-Login & HTML-Verkettung)
//============================================================================

void handleWebsite(WiFiClient client, String HTTP_req) {

   String script = "";
   char   istr[32]; // Geändert von nacktem 'char' zu funktionierendem Puffer
   char   dsymbol;

   //-------------------------------------------------------------------------
   // Browser-autonomer Multi-User Regelkreis (Ersatz für 'bool authorized')
   //-------------------------------------------------------------------------
   String expectedAuth = "Authorization: Basic " + basicAuthString(website_uname, website_upwd);

   if (HTTP_req.indexOf("logout") != -1) {
      auth_realm_counter++;

      script += "HTTP/1.1 401 Unauthorized\r\n";
      script += "WWW-Authenticate: Basic realm=\"" + String(website_title) + "_Abgemeldet_" + String(auth_realm_counter) + "\"\r\n";
      script += "Content-Type: text/html; charset=utf-8\r\n";
      script += "Connection: close\r\n\r\n";
      script += "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Abgemeldet</title></head>";
      script += "<body style='font-family:sans-serif;text-align:center;margin-top:100px;background-color:#d0d0d0;'>";
      script += "<h2>Erfolgreich abgemeldet!</h2><p><a href='/'>Erneut anmelden</a></p></body></html>";
      client.print(script);
      return;
   }

   if (HTTP_req.indexOf(expectedAuth) == -1) {
      script += "HTTP/1.1 401 Unauthorized\r\n";
      script += "WWW-Authenticate: Basic realm=\"" + String(website_title) + "_Sitzung_" + String(auth_realm_counter) + "\"\r\n";
      script += "Content-Type: text/html\r\n";
      script += "Connection: close\r\n\r\n";
      script += "<!DOCTYPE html><html><body><h1>401 Unauthorized</h1></body></html>";
      client.print(script);
      return;
   }

   //-------------------------------------------------------------------------
   // HTML-Seitenaufbau Dashboard (100% Ihr originales Layout)
   //-------------------------------------------------------------------------

   // init website - HTTP-Header
   script += "HTTP/1.1 200 OK\r\n";
   script += "Content-Type: text/html; charset=utf-8\r\n";
   script += "Connection: close\r\n";
   script += "\r\n";

   // HTML head
   script += "<!DOCTYPE html>\n";
   script += "<html>\n";
   script += "<head>\n";
   // automatische Aktualisierung alle 20 sec.
   script += "<meta http-equiv=\"refresh\" content=\"20; URL=";
   script += String(website_url) + ":" + String(http_port);
   script += "\">\n";
   script += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
   script += "<title>" + String(website_title) + "</title>\n";
   script += "</head>\n";
   script += "<body>\n";

   // Header & Status
   script += "<h1><p><font style=\"color:rgb(255,0,204);\"> DON'T PANIC ! </font>";
   script += "&nbsp; <wbr> <wbr> ";
   script += "<font style=\"color:rgb(0,205,102);\"> Welcome to " + String(website_url) + "! </font></p></h1>\n";

   script += "<h2><p style=\"color:rgb(0,205,102);\">" + datestr + " &nbsp; <wbr> <wbr> " + timestr + " &nbsp; <wbr> <wbr> <wbr>";
   script += "<font style=\"color:rgb(0,0,0);\">";
   script += "<br>";
   script += "<font style=\"color:rgb(255,0,0);\"> Notfall Alarms: " + String(EmergencyCnt) + "</font>";
   script += "<font style=\"color:rgb(255,209,22);\"> &nbsp; <wbr> <wbr> <wbr> Erinnerungen: " + String(RemindCnt) + "</font>";
   script += "</p></h2>\n";

   script += "<p> </p>\n";

   script += "<h3>letztes confirm (Std): &nbsp;" + String(dhrsLastConfirm) + " &nbsp;&nbsp;&nbsp;";
   script += htmlButton(" Confirm ", "CONFIRM=ON", 50, 100);
   script += "</h3>\n";

   script += "<p><font face=\"courier\"><font style=\"color:rgb(0,0,0);\"></font></p>\n";

   // SERVER block - Buttons
   script += "<h1><br>" + SERVERname + "<br></h1>\n";

   script += OUT1name + " ist: ";
   if (OUT1 == 1) script += "SCHARF &nbsp; <wbr> <wbr> ";
   else if (OUT1 == -1) script += "REV &nbsp;&nbsp;&nbsp;&nbsp; <wbr> <wbr> ";
   else script += "AUS &nbsp;&nbsp;&nbsp;&nbsp; <wbr> <wbr> ";

   script += htmlButton(" SCHARF ", "OUT1=ON", 70, 140) + " ";
   script += htmlButton(" AUS ", "OUT1=OFF", 70, 140) + "<br>\n\n";

   script += OUT2name + " ist: ";
   if (OUT2 == 1) script += "SCHARF &nbsp; <wbr> <wbr> ";
   else script += "AUS &nbsp;&nbsp;&nbsp;&nbsp; <wbr> <wbr> ";

   script += htmlButton(" SCHARF ", "OUT2=ON", 70, 140) + " ";
   script += htmlButton(" AUS ", "OUT2=OFF", 70, 140) + "<br>\n\n";

   // Server sensors table (Block)
   script += "<p><font face=\"courier\"></font></p>\n";
   script += "<h2>\n";
   script += "<table border=4 cellpadding=4>";
   script += "<caption> Messwerte " + SERVERname + " </caption>";
   script += htmlButton(" reset ", "svreset", 35, 70);
   script += "<thead><tr>";
   script += "<td bgcolor='Peru'>" + svSECT1name + "</td>";
   script += "<td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "<td bgcolor='Orange'>" + String(A0intname) + " % </td>";
   script += "<td bgcolor='Orange'>&nbsp;∅&nbsp;</td>";
   script += "<td bgcolor='Fuchsia'>" + String(A0muxname) + "</td>";
   script += "<td bgcolor='Fuchsia'>" + String(A1muxname) + "</td>";
   script += "</tr></thead><tbody>";
   script += "<tr>";
   script += "<th>" + String(svt1.sact) + " °C</th>";
   script += "<th>" + String(svt1.smin) + "</th>";
   script += "<th>" + String(svt1.smax) + "</th>";
   script += "<th>" + String(svh1.sact) + "</th>";

   if (svespA0.vact < fFireLIMIT) script += "<td bgcolor='red'>";
   else script += "<th>";
   script += String(svespA0.sact) + "</th>";
   script += "<th>" + String(svespA0.smean) + "</th>";
   script += "<th>-</th><th>-</th>";
   script += "</tr>";

   script += "</tbody></table></h2>\n<br><br>\n";

   // SEND server block
   client.print(script);

//============================================================================
// ENDE: ABSCHNITT B
//============================================================================

//============================================================================
// ANFANG: ABSCHNITT C
// Firmware-Basis: 090n
//============================================================================

   // ----------------------
   // Client 0 - Buttons + Sensors (Block)
   // ----------------------
   script = "";

   script += "<h1><br>" + CLIENT0name + "<br></h1>\n";
   script += c0OUT1name + " ist: ";
   if (c0out1 == 1) script += "EIN&nbsp;&nbsp;&nbsp;";
   else script += "AUS&nbsp;&nbsp;&nbsp;";
   script += htmlButton(" EIN ", "c0out1=ON", 70, 140) + "&nbsp;";
   script += htmlButton(" AUS ", "c0out1=OFF", 70, 140) + "<br>\n\n";

   script += c0OUT2name + " ist: ";
   if (c0out2 == 1) script += "EIN&nbsp;&nbsp;&nbsp;";
   else script += "AUS&nbsp;&nbsp;&nbsp;";
   script += htmlButton(" EIN ", "c0out2=ON", 70, 140) + " ";
   script += htmlButton(" AUS ", "c0out2=OFF", 70, 140) + "<br>\n\n";

   // Thermostat controls
   sprintf(istr, "%+3d", c0tx3);
   script += "c0-Thermost= " + String(istr) + "&nbsp;&nbsp;&nbsp;";
   script += htmlButton("Therm +1", "c0tx3=UP", 70, 140) + " ";
   script += htmlButton("Therm -1", "c0tx3=DN", 70, 140) + "<br>\n\n";

   script += "<h2>\n<table border=4 cellpadding=4>";
   script += "<caption> Messwerte " + CLIENT0name + " (+ min)<caption>";
   
   script += htmlButton(" reset ", "c0reset", 35, 70);
   
   script += "<thead><tr>";
   
   // --------- Zeile 1 ---------
   script += "<td bgcolor='Peru'>" + c0SECT1name + "</td>";
   script += "<td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "<td bgcolor='Avocado'>" + c0SECT2name + "</td><td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "<td bgcolor='Orange'>" + (String)c0A0intname + "</td>";
   script += "<td bgcolor='Orange'>&nbsp;∅&nbsp;</td>";
   script += "</tr>";
   
   script += "<tr>";
   script += "<th>" + String(c0t1.sact) + " °C</th>";
   script += "<th>" + String(c0t1.smin) + "</th>";
   script += "<th>" + String(c0t1.smax) + "</th>";
   script += "<th>" + String(c0h1.sact) + "</th>";
   
   script += "<th>" + String(c0t2.sact) + " °C</th>";
   script += "<th>" + String(c0t2.smin) + "</th>";
   script += "<th>" + String(c0t2.smax) + "</th>";
   script += "<th>" + String(c0h2.sact) + "</th>";
   
   script += "<th>" + String(c0espA0.sact) + "</th>";
   script += "<th>" + String(c0espA0.smean) + "</th>";
   script += "</tr>";
   
   // --------- Zeile 2 (Erde) ---------
   script += "<tr>";
   script += "<td bgcolor='Lime'>" + String(c0A0muxname) + "</td><td bgcolor='Lime'>" + String(c0A1muxname) + "</td>";
   script += "<td bgcolor='Lime'>" + String(c0A2muxname) + "</td><td bgcolor='Lime'>" + String(c0A3muxname) + "</td>";
   script += "</tr>";
   
   script += "<tr>";
   if (c0adc0.vmean < adcSoilMin && c0adc0.tFail < 60) script += "<td bgcolor='red'>"; else script += "<th>";
   script += String(c0adc0.sact) + "</th>";
   if (c0adc1.vmean < adcSoilMin && c0adc1.tFail < 60) script += "<td bgcolor='red'>"; else script += "<th>";
   script += String(c0adc1.sact) + "</th>";
   if (c0adc2.vmean < adcSoilMin && c0adc2.tFail < 60) script += "<td bgcolor='red'>"; else script += "<th>";
   script += String(c0adc2.sact) + "</th>";
   if (c0adc3.vmean < adcSoilMin && c0adc3.tFail < 60) script += "<td bgcolor='red'>"; else script += "<th>";
   script += String(c0adc3.sact) + "</th>";
   script += "</tr>";
   
   script += "</tbody></table></h2>\n<br><br>\n";

   // SEND client 0 block
   client.print(script);

//============================================================================
// ENDE: ABSCHNITT C
//============================================================================

//============================================================================
// ANFANG: ABSCHNITT D
// Firmware-Basis: 090n
//============================================================================

   script = "";

   // ----------------------
   // Client 1 - Buttons + Sensors (Block)
   // ----------------------
   script += "<h1><br>" + CLIENT1name + "<br></h1>\n";
   script += c1OUT1name + " ist: ";
   if (c1out1 == 1) script += "EIN&nbsp;&nbsp;&nbsp;";
   else script += "AUS&nbsp;&nbsp;&nbsp;";
   script += htmlButton(" EIN ", "c1out1=ON", 70, 140) + "&nbsp;";
   script += htmlButton(" AUS ", "c1out1=OFF", 70, 140) + "<br>\n\n";

   script += c1OUT2name + " ist: ";
   if (c1out2 == 1) script += "EIN&nbsp;&nbsp;&nbsp;";
   else script += "AUS&nbsp;&nbsp;&nbsp;";
   script += htmlButton(" EIN ", "c1out2=ON", 70, 140) + " ";
   script += htmlButton(" AUS ", "c1out2=OFF", 70, 140) + "<br>\n\n";

   script += "<h2>\n<table border=4 cellpadding=4>";
   script += "<caption> Messwerte " + CLIENT1name + " (+ min)</caption>";
   script += htmlButton(" reset ", "c1reset", 35, 70);
   script += "<thead><tr>";
   script += "<td bgcolor='Peru'>" + c1SECT1name + "</td>";
   script += "<td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "<td bgcolor='Avocado'>" + c1SECT2name + "</td><td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "</tr></thead><tbody>";
   script += "<tr>";
   script += "<th>" + String(c1t1.sact) + " °C</th>";
   script += "<th>" + String(c1t1.smin) + "</th>";
   script += "<th>" + String(c1t1.smax) + "</th>";
   script += "<th>" + String(c1h1.sact) + "</th>";
   script += "<th>" + String(c1t2.sact) + " °C</th>";
   script += "<th>" + String(c1t2.smin) + "</th>";
   script += "<th>" + String(c1t2.smax) + "</th>";
   script += "<th>" + String(c1h2.sact) + "</th>";
   script += "</tr></tbody></table></h2>\n<br><br>\n";

   // SEND client 1 block
   client.print(script);
   script = "";

   // ----------------------
   // Client 2 - Buttons + Sensors (Block)
   // ----------------------
   script += "<h1><br>" + CLIENT2name + "<br></h1>\n";
   script += c2OUT1name + " ist: ";
   if (c2out1 == 1) script += "EIN&nbsp;&nbsp;&nbsp;";
   else script += "AUS&nbsp;&nbsp;&nbsp;";
   script += htmlButton(" EIN ", "c2out1=ON", 70, 140) + "&nbsp;";
   script += htmlButton(" AUS ", "c2out1=OFF", 70, 140) + "<br>\n\n";

   script += c2OUT2name + " ist: ";
   if (c2out2 == 1) script += "EIN&nbsp;&nbsp;&nbsp;";
   else if (c2out2 == -1) script += "REV&nbsp;&nbsp;&nbsp;";
   else script += "AUS&nbsp;&nbsp;&nbsp;";
   script += htmlButton(" EIN ", "c2out2=ON", 70, 140) + " ";
   script += htmlButton(" AUS ", "c2out2=OFF", 70, 140) + " ";
   script += htmlButton(" REV ", "c2out2=REV", 70, 140) + "<br>\n\n";

   script += "<h2>\n<table border=4 cellpadding=4>";
   script += "<caption> Messwerte " + CLIENT2name + " (+ min)</caption>";
   script += htmlButton(" reset ", "c2reset", 35, 70);
   script += "<thead><tr>";
   script += "<td bgcolor='Peru'>" + c2SECT1name + "</td>";
   script += "<td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "<td bgcolor='Avocado'>" + c2SECT2name + "</td><td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "</tr></thead><tbody>";
   script += "<tr>";
   script += "<th>" + String(c2t1.sact) + " °C</th>";
   script += "<th>" + String(c2t1.smin) + "</th>";
   script += "<th>" + String(c2t1.smax) + "</th>";
   script += "<th>" + String(c2h1.sact) + "</th>";
   script += "<th>" + String(c2t2.sact) + " °C</th>";
   script += "<th>" + String(c2t2.smin) + "</th>";
   script += "<th>" + String(c2t2.smax) + "</th>";
   script += "<th>" + String(c2h2.sact) + "</th>";
   script += "</tr></tbody></table></h2>\n<br><br>\n";

   // SEND client 2 block
   client.print(script);

//============================================================================
// ENDE: ABSCHNITT D
//============================================================================

//============================================================================
// ANFANG: ABSCHNITT E
// Firmware-Basis: 090n / 091a (Lückenloser Seitenabschluss & Logout)
//============================================================================

   script = "";

   // ----------------------
   // Client 3 - Buttons + Sensors (Block)
   // ----------------------
   script += "<h1><br>" + CLIENT3name + "<br></h1>\n";
   script += c3OUT1name + " ist: ";
   if (c3out1 == 1) script += "EIN&nbsp;&nbsp;&nbsp;";
   else script += "AUS&nbsp;&nbsp;&nbsp;";
   script += htmlButton(" EIN ", "c3out1=ON", 70, 140) + "&nbsp;";
   script += htmlButton(" AUS ", "c3out1=OFF", 70, 140) + "<br>\n\n";

   script += c3OUT2name + " ist: ";
   if (c3out2 == 1) script += "EIN&nbsp;&nbsp;&nbsp;";
   else script += "AUS&nbsp;&nbsp;&nbsp;";
   script += htmlButton(" EIN ", "c3out2=ON", 70, 140) + " ";
   script += htmlButton(" AUS ", "c3out2=OFF", 70, 140) + "<br>\n\n";

   // Thermostat controls
   sprintf(istr, "%+3d", c3tx3);
   script += "c3-Thermost= " + String(istr) + "&nbsp;&nbsp;&nbsp;";
   script += htmlButton("Therm +1", "c3tx3=UP", 70, 140) + " ";
   script += htmlButton("Therm -1", "c3tx3=DN", 70, 140) + "<br>\n\n";

   script += "<h2>\n<table border=4 cellpadding=4>";
   script += "<caption> Messwerte " + CLIENT3name + " (+ min)</caption>";
   script += htmlButton(" reset ", "c3reset", 35, 70);
   script += "<thead><tr>";
   script += "<td bgcolor='Peru'>" + c3SECT1name + "</td>";
   script += "<td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "<td bgcolor='Avocado'>" + c3SECT2name + "</td><td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "</tr></thead><tbody>";
   script += "<tr>";
   script += "<th>" + String(c3t1.sact) + " °C</th>";
   script += "<th>" + String(c3t1.smin) + "</th>";
   script += "<th>" + String(c3t1.smax) + "</th>";
   script += "<th>" + String(c3h1.sact) + "</th>";
   script += "<th>" + String(c3t2.sact) + " °C</th>";
   script += "<th>" + String(c3t2.smin) + "</th>";
   script += "<th>" + String(c3t2.smax) + "</th>";
   script += "<th>" + String(c3h2.sact) + "</th>";
   script += "</tr></tbody></table></h2>\n<br><br>\n";

   // ----------------------
   // Analog-Sensoren Tabelle
   // ----------------------
   script += "<table>";
   script += "<tr><th colspan='5'><b> Analog-Sensoren </b></th></tr>";
   script += "<tr><td> " + A0intname + " </td>";
   script += "<td colspan='4'> " + (String)svespA0.vact + " </td></tr>";
   script += "<tr><td> " + c0A0intname + " </td>";
   script += "<td colspan='4'> " + (String)c0espA0.vact + " </td></tr>";
   script += "</table>";

   // HTML-Abschluss
   script += "</body>\n";
   script += "</html>\n";

   // ERWEITERUNG: Neuer Multi-User Logout-Button (Browser-autonom)
   script += "<p><br><hr><br></p>\n";
   script += "<p><a href=\"/?logout=1\"><button style=\"background-color:#707070;color:white;width:100%;max-width:600px;height:45px;font-size:16px;font-weight:bold;\">LOGOUT (Abmelden)</button></a></p>\n";

   client.print(script);

} // handleWebsite end


//============================================================================
// ANFANG: ABSCHNITT F (Linker-Fix für basicAuthString)
// Firmware-Basis: 090n / 091a
//============================================================================

//----------------------------------------------------------------------------
String basicAuthString(String username, String password) {
   String toEncode = username + ":" + password;
   
   const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   String encoded = "";
   int i = 0, j = 0;
   unsigned char char_array_3[3]; // REPARIERT: Array-Größe hinzugefügt
   unsigned char char_array_4[4]; // REPARIERT: Array-Größe hinzugefügt

   for (unsigned int n = 0; n < toEncode.length(); n++) {
      char_array_3[i++] = toEncode[n];
      if (i == 3) {
         char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
         char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
         char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
         char_array_4[3] = char_array_3[2] & 0x3f;

         for (i = 0; i < 4; i++) encoded += base64_chars[char_array_4[i]];
         i = 0;
      }
   }

   if (i) {
      for (j = i; j < 3; j++) char_array_3[j] = '\0';

      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (j = 0; j < (i + 1); j++) encoded += base64_chars[char_array_4[j]];
      while (i++ < 3) encoded += '=';
   }

   return encoded;
} // Ende basicAuthString

//============================================================================
// ENDE: ABSCHNITT F
//============================================================================


//============================================================================
// ENDE: ABSCHNITT E
//============================================================================

//============================================================================
// ABSCHNITT F: es folgt wieder unveränderter vorheriger 090n-Code
//============================================================================

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
   static unsigned long tsec = millis(), tms = millis();

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

   msgtok = webserver.arg("c0out1"); // <<<< c3h2
   if (msgtok != "") {
      c0outMon1 = msgtok.toInt();  // noch nicht auf c0 implem.
   }

   msgtok = webserver.arg("c0out2"); // <<<< c3h2
   if (msgtok != "") {
      c0outMon2 = msgtok.toInt();
   }

   msgtok = webserver.arg("c0out3"); // <<<< c3h2
   if (msgtok != "") {
      c0outMon3 = msgtok.toInt();
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
   message += "&c3out0=" + (String)c3out0;
   message += "&c3out1=" + (String)c3out1;
   message += "&c3out2=" + (String)c3out2;
   //message += "&c3out3=" + (String)c3out3 ;   // c3out3=intern auto ctrl
   message += "&c3tx3=" + (String)c3tx3;        // Thermostat für c3out3

   // all clients
   message += "&remindcnt=" + (String)RemindCnt;
   message += "&emergencycnt=" + (String)EmergencyCnt;

   message += " ###";
   webserver.send(200, "text/plain", message);
   yield();

   if ( millis() - tms >= 1000 ) {    // refresh Serial.println(message)rate
      tms = millis();
      Serial.println(message);
   }

} // Ende:   handleClients()




//----------------------------------------------------------------------------
// UDP Time
//----------------------------------------------------------------------------

/*-------- NTP code ----------*/

time_t getNtpTime() {
   const char* ntpServerName = "pool.ntp.org";
   const int timeZone = 1; // MEZ/CEST handled by Timezone lib
   const int localPort = 8888;

   byte packetBuffer[48];
   IPAddress ntpServerIP;

   WiFi.hostByName(ntpServerName, ntpServerIP);

   UdpTime.begin(localPort);
   memset(packetBuffer, 0, 48);
   packetBuffer[0] = 0b11100011;   // NTP request header
   UdpTime.beginPacket(ntpServerIP, 123);
   UdpTime.write(packetBuffer, 48);
   UdpTime.endPacket();

   delay(1000);

   if (UdpTime.parsePacket() > 0) {
      UdpTime.read(packetBuffer, 48);
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord  = word(packetBuffer[42], packetBuffer[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;

      const unsigned long seventyYears = 2208988800UL;
      time_t epoch = secsSince1900 - seventyYears;

      // Lokale Zeitzone anwenden
      time_t localTime = CE.toLocal(epoch);  // time_t wird autom. geupdated

      return localTime;
   }

   return 0; // Fehlerfall
}

// Convenience-Funktion: Zeit abrufen + Strings bauen
void updateTimeNow() {
   time_t t = getNtpTime();
   if (t != 0) {
      setTime(t);            // TimeLib globale Zeit setzen
      buildDateTimeString(); // Strings bauen
   }
}


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

/* Muster
String htmlButton(String caption, String path, int h, int w, String actionID = "") {
   String btn = "<a href=\"" + path;
   if (actionID != "") btn += "?actionID=" + actionID;
   btn += "\"><button style=\"height:" + String(h) + "px;width:" + String(w) + "px\">";
   btn += caption;
   btn += "</button></a>";
   return btn;
} // Ende: htmlButton()
  
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


/*
   Log:



















   /*
   //----------------------------------------
   to do:
   0.90h+







*/








