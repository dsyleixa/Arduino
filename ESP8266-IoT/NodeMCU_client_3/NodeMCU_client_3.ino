/*  ESP8266 NodeMCU
    ESP8266WiFi Client für Remote Sensor Werte
    Client 3, Port 8081, wifi server .200

    Quelle website:
    http://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/

    Arduino IDE 1.8.9

*/

/*
   =======  -----------------------------------------------------------v
   digital:        default        myStd         bRk               ESP_motorShield
   D0     16       WAKE           ---           ---               ---
   D1      5       I2C SCL        I2C SCL       I2C SCL           >> in D1 (DHT2)
   D2      4       I2C SDA        I2C SDA       I2C SDA           out D2 motorB
   D3      0       FLASH/LED      out D3     built-in btn+LED     out D3 motorA
   D4      2       TX1            In DHT        ---               out D4 motorB
   D5     14       SPI SCK        In B/S        out D5            SDA
   D6     12       SPI MISO       Out D6        ---               SCL
   D7     13       SPI MOSI       Out D7        out D7            in  D7 (DHT1)
   D8     15       MTD0 PWM       Out D8        ---               out D8
   D9      3       UART RX0       USB, Serial   USB, Serial       USB, Serial
   D10     1       UART TX0       USB, Serial   USB, Serial       USB, Serial
   (D12)   10       GPIO intern    TFT reset     ---               TFT reset

*/

char * clientname = "Client 3"; // Gew.-Haus
char * ver = "0.017b";

// change log
// 3.017b reboot after svr timeout > 30min
// 3.017a PIN_O3=>c3out3 svr msg
// 3.016  auto switch PIN_O3=D8 by DHT2, not by website
// 3.015  client3 forked from client0 0.015

#include <stringEx.h>  // cstringarg()

//----------------------------------------------------------------------------
#include <Wire.h>

/*
   #define ESPSDA   D5 // esp-L293 Board
   #define ESPSCL   D6
*/

//----------------------------------------------------------------------------
// OLED SSD1306 126x64 0x3C

#include <ESP_SSD1306.h>    // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>   // Needs a little change in original Adafruit library (See README.txt file)
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>  // 

#define D12         10      // GPIOintern (N/A)
#define OLED_RESET  D12     // GPIO10=D12 Pin RESET signal (N/A)

ESP_SSD1306    display(OLED_RESET);

//----------------------------------------------------------------------------
#include <LiquidCrystal_I2C.h> // Library for LCD    
//LiquidCrystal_I2C  lcd = LiquidCrystal_I2C(0x27, 20, 4); // Change to (0x27,16,2) for 16x2 LCD.
LiquidCrystal_I2C  lcd = LiquidCrystal_I2C(0x27, 16, 2);


//----------------------------------------------------------------------------
// char symbols

#define CHR_DEGREE (unsigned char)247                 // ° symbol for OLED font
//char    STR_DEGREE[] = {247, 0, 0};                 // ° OLED font specimen (°,°C,°F,K)


//----------------------------------------------------------------------------
// output pins (LED, motors)

//#define PIN_P1    D1     // pwm

#define PIN_O1      D3
#define PIN_P2    D2     // pwm
#define PIN_O2      D4
#define PIN_O3    D8     // pwm

//----------------------------------------------------------------------------
// I2C
#define ESPSDA      D5 // SDA esp-L293 Board
#define ESPSCL      D6 // SCL esp-L293 Board

//----------------------------------------------------------------------------
// digital 1-wire pins for DHT sensors
#define PIN_DHT1    D7       //  DHT Sensor 1 STANDARD 
#define PIN_DHT2    D1       //  DHT Sensor 2 VARIANT


volatile int8  c3out1=0, c3out2=0, c3out3=0; // Client3; actual output pin states; stop=0, fwd=1, rev=-1;

/*
   String OUT1name = "1-F.TÜR";     // output captions
   String OUT2name = "2-PUMPE";
   String OUT3name = "3-KLIMA";
*/


//----------------------------------------------------------------------------
// input / sensor pins
//----------------------------------------------------------------------------

// N.N barometric pressure adjust (250m)
const double  FNNcorr=1013.0-984.0;  // ca. 250m

const double  fINVAL = -99.9;
char  sNEXIST[10]= "--";
char  sINVAL[10] = "??";

String SENSOR1name = "G.Aussen";
String SENSOR2name = "G.GwHaus";



//----------------------------------------------------
// analog pins

String A0intname = "Sens.int";  // intern: ESP8266
String A0muxname = "Erde.A0 ";  // mux: ADS1015 (i2c)
String A1muxname = "Erde.A1 ";
String A2muxname = "Erde.A2 ";
String A3muxname = "Erde.A3 ";


int16_t espA0;
double  fespA0;
char    sfespA0[20];


//----------------------------------------------------
// ADS1015 (4* ADC) 0x48

#include <Adafruit_ADS1015.h>

// Adafruit_ADS1115 ads;     /* Use this for the 16-bit version */
// Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

Adafruit_ADS1115 ADSx48(0x48);  // AS1015 i2c dev addr

int16_t xadc0, xadc1, xadc2, xadc3;  // global ADS1115 ADC readings
double  fadc0, fadc1, fadc2, fadc3;

char    sfadc0[20];
char    sfadc1[20];
char    sfadc2[20];
char    sfadc3[20];


//----------------------------------------------------
#include <DHT.h>
// DHT sensor defs


DHT  DHT_1(PIN_DHT1, DHT22);  // 1.DHT Typ
DHT  DHT_2(PIN_DHT2, DHT22);  // 2.DHT Typ

const float TEMP_AC_ON=24.0;



//----------------------------------------------------
// logval vlog struct

typedef struct {
   double    vact=fINVAL, vmin=fINVAL, vmax=fINVAL, vmean=fINVAL ; // min,act,max,mean
   uint32_t  tact=0, tmin=0, tmax=0, tmean=0, talert=0 ; // time (millis)
   char      sact[20]="--", smin[20]="--", smax[20]="--",smean[20]="--";
} vlog;


static vlog c3t1, c3h1, c3t2, c3h2,  // local temperature, humidity
       c3p1, c3q1;              // local barometr.air pressure, quality


//----------------------------------------------------------------------------
// MCP9808 Temperature Sensor

#include <Adafruit_MCP9808.h>

Adafruit_MCP9808 MCP9808_T = Adafruit_MCP9808();


//----------------------------------------------------------------------------
// BME280 Temperature+barometric pressure Sensor 0x77

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme_x77; // I2C



//----------------------------------------------------------------------------
#include <ESP8266WiFi.h>
WiFiClient client;
const int httpPort = 8081;

#include "data\settings.h"  // sets + initializes passwords

// WiFi Router
extern char* website_pwd;     //  website password log in  "xyz"
extern char* website_title;   //  website caption          "MySyte"
extern char* website_url;     //  website url              "http:\\mysite.com"
extern char* ssid;            //  local WiFi ssid          "myLocalWifiName"
extern char* password;        //  local wifi password      "1234567890"


const char* host = "192.168.2.200"; // Server der die temperatur empfangen soll

const char* script = "/client/client3/"; // URL/Verzeichnis das wir gewaehlt haben

const char* idnr = "0"; //Hier kann man dem Sensor eine beliebe ID zuteilen (derzeit ungenutzt)

volatile long timeoutcnt=0;  // timeouts to server


//----------------------------------------------------------------------------
// Tools
//----------------------------------------------------------------------------
double calADS1015 (int ADC) {
   int RES = 16383;
   if(ADC<0) ADC=0;
   if(ADC>RES) ADC=RES;
   return ( (double)ADC*100.0 * 1.0) /(double)RES;
}

//----------------------------------------------------------------------------
double calADC10k (int ADC) {
   int RES = 1023;
   if(ADC<0) ADC=0;
   if(ADC>RES) ADC=RES;
   return ( (double)ADC*100.0 * 1.0) /(double)RES;
}



//----------------------------------------------------------------------------
// dashboard
//----------------------------------------------------------------------------

int MaxDisplayModes = 6;

char * dashboard(int mode) {

   display.setRotation(0);
   display.clearDisplay();
   lcd.clear();
   // <<<<<<<<< variant >>>>>>>>>>>
   if (mode == 1) {
      mode=2;
   }

   if(mode==0) {
      display.setFont();

      String ip_oled = "IP: " + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1]
                       + "."  + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3]
                       + "-"  + (String)WiFi.RSSI() ;
      display.setCursor( 0, 0);  display.print(ip_oled  );
      display.setCursor( 0, 8);  display.print((String)clientname+" ver="+ver);

      display.setCursor( 0,16);  display.print("1." + (String)c3t1.sact + "*C");
      display.setCursor(75,16);  display.print("F:"+(String)c3h1.sact );
      display.setCursor( 0,24);  display.print("2." + (String)c3t2.sact + "*C" );
      display.setCursor(75,24);  display.print("F:"+(String)c3h2.sact );

      display.setCursor( 0,32);  display.print("3.iA0="+(String)sfespA0);
      display.setCursor(72,32);  display.print("hPa="+(String)(int)c3p1.vact);
      display.setCursor( 0,40);  display.print("4.xA0="+(String)sfadc0+" xA1="+(String)sfadc1);
      display.setCursor( 0,48);  display.print("5.xA2="+(String)sfadc2+" xA3="+(String)sfadc3);
      display.display();         display.print("timeout svr="+String(timeoutcnt) );
      delay(1);

      lcd.setCursor(0,0); lcd.print(WiFi.localIP());
      lcd.setCursor(0,1); lcd.print((String)clientname);
      lcd.setCursor(0,2); lcd.print((String)"ver= " + ver);
      lcd.setCursor(0,3); lcd.print("timeout svr="+String(timeoutcnt));

   }

   else if (mode == 1) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("Luftdruck:");
      display.setCursor( 0, 40);  display.print( (String)c3p1.sact +  " hPa");
      display.display();
      lcd.setCursor(0,0); lcd.print("Luftdruck:");
      lcd.setCursor(0,1); lcd.print((String)c3p1.sact +  " hPa");
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");
   }

   else if (mode == 2) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("T+H Aussen");
      display.setCursor( 0, 40);  display.print( (String)c3t1.sact + "*C " );
      display.print( (String)c3h1.sact + "%" );
      display.display();
      lcd.setCursor(0,0); lcd.print("T+H Aussen");
      lcd.setCursor(0,1); lcd.print( (String)c3t1.sact + "'C ");
      lcd.print( (String)c3h1.sact + "%" );
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");
   }

   else if (mode == 3) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("T+H Innen");
      display.setCursor( 0, 40);  display.print( (String)c3t2.sact + "*C " );
      display.print( (String)c3h2.sact + "%" );
      display.display();
      lcd.setCursor(0,0); lcd.print("T+H Innen");
      lcd.setCursor(0,1); lcd.print( (String)c3t2.sact + "'C ");
      lcd.print( (String)c3h2.sact + "%" );
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");
   }

   else if (mode == 4) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("AI: ");  display.print(sfespA0);
      display.setCursor( 0, 40);
      display.display();
      lcd.setCursor(0,0); lcd.print("Ai Rauch: ");  lcd.print(sfespA0);
      lcd.setCursor(0,1); lcd.print("");
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");

   }
   else if (mode == 5) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("A0: ");  display.print(sfadc0);
      display.setCursor( 0, 40);  display.print("A1: ");  display.print(sfadc1);
      display.display();
      lcd.setCursor(0,0); lcd.print("A0: "); lcd.print(sfadc0);
      lcd.setCursor(0,1); lcd.print("A1: "); lcd.print(sfadc1);
   }
   else if (mode == 6) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("A2: ");  display.print(sfadc2);
      display.setCursor( 0, 40);  display.print("A3: ");  display.print(sfadc3);
      display.display();
      lcd.setCursor(0,0); lcd.print("A2: "); lcd.print(sfadc2);
      lcd.setCursor(0,1); lcd.print("A3: "); lcd.print(sfadc3);
   }

   display.setFont();

}





//----------------------------------------------------------------------------
// SETUP
//----------------------------------------------------------------------------

void setup() {
   int IORes;

   Serial.begin(115200);
   delay(1000);

   //----------------------------------------
   //pinMode(PIN_P1, OUTPUT);
   //digitalWrite(PIN_P1, LOW);
   pinMode(PIN_O1, OUTPUT);
   digitalWrite(PIN_O1, LOW);

   pinMode(PIN_P2, OUTPUT);
   digitalWrite(PIN_P2, LOW);
   pinMode(PIN_O2, OUTPUT);
   digitalWrite(PIN_O2, LOW);

   pinMode(PIN_O3, OUTPUT);
   digitalWrite(PIN_O3, LOW);

   delay(10);

   //----------------------------------------
   // i2c + OLED/LCD
   Wire.pins(D5, D6); // SDA, SCL
   Wire.begin();
   Wire.setClock(400000);
   delay(1);

   //----------------------------------------
   // OLED 128x64
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)


   display.setRotation(0);

   display.setFont();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.clearDisplay();
   display.setCursor( 0, 0);  display.print("OLED TEST OK");
   Serial.print("OLED TEST OK");
   display.display();
   //----------------------------------------
   //  LCD
   lcd.init();
   lcd.backlight();
   lcd.noBlink();
   Serial.println("LCD display init OK");
   lcd.setCursor(0, 0);  lcd.print("LCD display init OK");


   //----------------------------------------
   // i2c:  ADS1115

   ADSx48.begin(); // ADS1115 I2C 4x 12/16bit ADC
   delay(1);
   Serial.println("ADS1115 sensor init... ");


   //----------------------------------------
   // i2c: BME280

   IORes=bme_x77.begin();
   if (!IORes) {
      Serial.println("Couldn't find BME280 sensor!");
   }
   else {
      Serial.println("BME280 sensor init: OK");
   }
   delay(1);

   //----------------------------------------
   // Connect to WiFi network
   Serial.println();
   Serial.println();
   Serial.print("Verbinde mich mit Netz: ");
   Serial.println(ssid);

   WiFi.begin(ssid, password);

   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
   }

   Serial.println("");
   Serial.println("WiFi Verbindung aufgebaut");
   Serial.print("Eigene IP des ESP-Moduls: ");
   Serial.println(WiFi.localIP() );

   display.setCursor(0,15);  display.print("WiFi Verbindung aufgebaut");
   display.display();

   lcd.setCursor(0,1);  lcd.print("WiFi aufgebaut");
   lcd.setCursor(0,2);  lcd.print( "C 0=" ); lcd.print( WiFi.localIP() );
   lcd.setCursor(0,3);  lcd.print( "Svr=" ); lcd.print( WiFi.gatewayIP() );

   delay(2000);

}


//----------------------------------------------------------------------------
// LOG ARRAY
//----------------------------------------------------------------------------
void logval( double f, vlog &v) {

   if(f<=fINVAL ) {
      if(millis()-v.tact>1000ul*60) {  // store invalid if outdated>1min
         v.vact=fINVAL;
         strcpy(v.sact,sINVAL);
      }
      delay(1);
      return;
   }

   if(v.vact<= fINVAL)
   {
      v.vact = f;
   }
   else
   {
      v.vact = (v.vact +f )/2;
   }
   v.tact = millis();

   // inval min, max
   if( v.vmin<=fINVAL )  {
      v.vmin=v.vact;
      v.tmin=v.tact;
   }
   if( v.vmax<=fINVAL )  {
      v.vmax=v.vact;
      v.tmax=v.tact;
   }
   if( v.vmean<=fINVAL )  {
      v.vmean=v.vact;
      v.tmean=v.tact;
   }

   // new min, max
   if (v.vact < v.vmin-10.0 ) {   // error? => small adjust min
      v.vmin-=0.5;
      v.tmin= v.tact;
   }
   else if( v.vact<=v.vmin )  {
      v.vmin= 0.9*v.vmin + 0.1*v.vact;
      v.tmin= v.tact;
   }

   if (v.vact > v.vmax+10.0 ) {   // error? => small adjust max
      v.vmax+=0.5;
      v.tmax = v.tact;
   }
   else if( v.vact>=v.vmax ) {
      v.vmax = 0.9*v.vmax + 0.1*v.vact;
      v.tmax = v.tact;
   }

   // time-out min, max
   if( millis()-v.tmin>1000*60*60*30) {  // 30h min/max time-out:
      v.vmin=0.8*v.vmin+0.2*v.vact;  // 0.2 re-adjust
      v.tmin=v.tact-1000ul*60*60*29;   // 29h clock reset
   }
   if( millis()-v.tmax>1000*60*60*30) {
      v.vmax=0.8*v.vmax+0.2*v.vact;
      v.tmax=v.tact-1000ul*60*60*29;
   }

   // mean
   v.vmean=0.9999*v.vmean + 0.0001*v.vact;

   // to String
   dtostrf(v.vact, 2, 1, v.sact);
   dtostrf(v.vmin, 2, 1, v.smin);
   dtostrf(v.vmax, 2, 1, v.smax);
   dtostrf(v.vmean, 2, 1, v.smean);
}



//----------------------------------------------------------------------------
// LOOP
//----------------------------------------------------------------------------

// Bei deepSleep wird die loop Schleife eigentlich nur einmal durchlaufen

void loop() {
   static double ftmp;
   unsigned long timeout;
   static int8_t  DisplayMode=0;

   display.setRotation(0);

   if(timeoutcnt>60*30) { // > 30min timeout
      display.clearDisplay();
      lcd.clear();
      Serial.println("\n timeout => restart \n");
      display.print("timeout => restart");
      lcd.print("timeout => restart");
      ESP.restart();
   }


   //---------------------------------------
   // read DHT+bme280 Sensors

   ftmp = DHT_1.readTemperature();          // 1. Temperatur auslesen (Celsius)
   delay(10);
   if (isnan(ftmp)) ftmp=fINVAL;
   logval(ftmp, c3t1);
   delay(10);

   ftmp = DHT_1.readHumidity();             // 1. Feuchtigkeit auslesen (Prozent)
   delay(1);
   if (isnan(ftmp)) ftmp=fINVAL;
   logval(ftmp, c3h1);
   delay(10);



   ftmp = DHT_2.readTemperature();          // 1. Temperatur auslesen (Celsius)
   delay(10);
   if (isnan(ftmp)) ftmp=fINVAL;
   logval(ftmp, c3t2);
   delay(10);

   ftmp = DHT_2.readHumidity();             // 1. Feuchtigkeit auslesen (Prozent)
   delay(1);
   if (isnan(ftmp)) ftmp=fINVAL;
   logval(ftmp, c3h2);
   delay(10);

   /*
      ftmp=fINVAL;
      ftmp = bme_x77.readTemperature();
      if (isnan(ftmp)) ftmp=fINVAL;
      logval(ftmp, c3t2);
      delay(1);

      ftmp=fINVAL;
      ftmp = bme_x77.readHumidity();       // 2. Feuchtigkeit auslesen (Prozent)
      if (isnan(ftmp)) ftmp=fINVAL;
      logval(ftmp, c3h2);
      delay(1);

      ftmp=fINVAL;
      ftmp = FNNcorr + bme_x77.readPressure()/100.0 ;
      if (isnan(ftmp)) ftmp=fINVAL;
      logval(ftmp, c3p1);
      delay(1);
   */
   Serial.print("DHT 1: ");
   Serial.print(c3t1.sact);   Serial.print("   ");  Serial.print(c3h1.sact); Serial.println(" rH");
   Serial.print(" HT 2: ");
   Serial.print(c3t2.sact);   Serial.print("   ");  Serial.print(c3h2.sact); Serial.println(" rH");
   Serial.print("hPa="); Serial.println(c3p1.vact);
   delay(1);


   // internal ADC A0
   espA0=0;
   espA0=analogRead(A0);
   fespA0=calADC10k(espA0);
   sprintDouble(sfespA0,fespA0,2,1,false);


   Serial.print("espA0="); Serial.print(sfespA0); Serial.println("%");

   // ADS1115 analog (ADC) sensors
   xadc0 = ADSx48.readADC_SingleEnded(0);  fadc0=calADS1015(xadc0); //delay(3);
   xadc1 = ADSx48.readADC_SingleEnded(1);  fadc1=calADS1015(xadc1); //delay(3);
   xadc2 = ADSx48.readADC_SingleEnded(2);  fadc2=calADS1015(xadc2); //delay(3);
   xadc3 = ADSx48.readADC_SingleEnded(3);  fadc3=calADS1015(xadc3);

   sprintDouble(sfadc0,fadc0,2,1,false); Serial.println(fadc0);
   sprintDouble(sfadc1,fadc1,2,1,false); Serial.println(fadc1);
   sprintDouble(sfadc2,fadc2,2,1,false); Serial.println(fadc2);
   sprintDouble(sfadc3,fadc3,2,1,false); Serial.println(fadc3);
   delay(1);

   Serial.println(" "); Serial.println(" ");

   delay(1);


   //---------------------------------------
   // switch output D8 (c3out3) by c3t2
   //---------------------------------------
   if(c3t2.vact >=TEMP_AC_ON) {
      digitalWrite(PIN_O3, HIGH);
      c3out3=HIGH;
   }
   else if(c3t2.vact<=TEMP_AC_ON-2.0) {
      digitalWrite(PIN_O3, LOW);
      c3out3=LOW;
   }
   


   //---------------------------------------
   // display on OLED
   dashboard(DisplayMode);
   delay(1000);

   //---------------------------------------
   // msg string

   String vals = "";
   // DHT t+h
   vals += "&c3t1=";   vals += c3t1.sact;
   vals += "&c3h1=";   vals += c3h1.sact;

   // BME t+h
   vals += "&c3t2=";   vals += c3t2.sact;
   vals += "&c3h2=";   vals += c3h2.sact;

   // BME hPa
   //vals += "&c3p1=";   vals += (int)c3p1.vact;

   // esp ADC
   vals += "&c3espA0="; vals += sfespA0;

   // ads ADC
   vals += "&c3adc0=";  vals += sfadc0;
   vals += "&c3adc1=";  vals += sfadc1;
   vals += "&c3adc2=";  vals += sfadc2;
   vals += "&c3adc3=";  vals += sfadc3;

   // output
   vals += "&c3out1=";  vals += c3out1;
   vals += "&c3out2=";  vals += c3out2;
   vals += "&c3out3=";  vals += c3out3;


   Serial.print("Sensorwerte: ");
   Serial.println( vals+"<<<" );
   delay(1);

   //---------------------------------------
   // connect to ESP8266 webserver

   int versuche=1;
   int erg=0;

   do
   {
      Serial.print("Verbindungsaufbau zu Server ");
      Serial.println(host);

      erg =client.connect(host, httpPort);
      if (!erg) {
         versuche++;
         timeoutcnt++;
         dashboard(DisplayMode);

         Serial.println("Verbindungsaufbau nicht moeglich!!!");
         if (versuche>10) {
            Serial.println("versuche es spaeter noch mal!");
            client.stop();
            //WiFi.disconnect();
            //ESP.deepSleep( 10*60000000); //Etwas später neu probieren, solange schlafen
            delay(2000);
         }
      }
      delay(2000);
   } while (erg!=1);

   //---------------------------------------
   // msg string to server

   String url = script; //Url wird zusammengebaut: script = "/client/client3/";
   url += "?pw=";
   url += password;
   url += "&idnr=";
   url += idnr;

   url += vals;

   Serial.print("Folgende URL wird aufgerufen: ");
   Serial.println((String)host + "?pw="+"*****"+"&idnr="+idnr+vals);

   client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Connection: close\r\n\r\n");


   timeout= millis();
   while ( !client.available() )  {
      timeoutcnt+=1;
      if( (millis()-timeout < 20000)) {
         Serial.print("Timeout svr count "); Serial.println((String)timeoutcnt +(" !") );
         Serial.println("Rueckmeldung von Server verzoegert, ich warte noch...");
         //client.stop();
         //WiFi.disconnect();
         //ESP.deepSleep( 60*1000000); //Etwas später neu probieren,solange schlafen
         dashboard(DisplayMode);
         delay(2000);
      }
      else {
         Serial.println("\nAbbruch, keine Rueckantwort vom Server!\n");
         break;
      }
   }

   timeout=millis();
   String msgline;
    
   Serial.println("Rueckgabe vom Server:\n");
   if(client.available()) {
      timeoutcnt=0;
      while(client.available()) {
         msgline = client.readStringUntil('\r');
         //Serial.print("msgline="); Serial.println(msgline);
      }
      String Sargval="";
      char carg[TOKLEN], ctok[TOKLEN], cmsg[MAXLEN];
      Sargval="";
      msgline.toCharArray(cmsg, MAXLEN-1);
      Serial.println();
      Serial.print("cmsg="); Serial.println(cmsg);

      if(cmsg!="") {
         cstringarg(cmsg, "c3out1", carg);  // out1  switch on/off/rev
         Sargval=(String)carg;
         if(Sargval!="") {
            c3out1=Sargval.toInt();
         }

         cstringarg(cmsg, "c3out2", carg);  // out2  switch on/off
         Sargval=(String)carg;
         if(Sargval!="") {
            c3out2=Sargval.toInt();
         }
         /*
         cstringarg(cmsg, "c3out3", carg);  // out3  switch on/off
         Sargval=(String)carg;
         if(Sargval!="") {
            c3out3=Sargval.toInt();
         }
         */
      }

      // debug
      Serial.println( (String)"c3out1=" + (String)c3out1 + " <<<<<<<<<<<<<<<<<<<<<<<<<<" );
      Serial.println( (String)"c3out2=" + (String)c3out2 + " " );
      Serial.println( (String)"c3out3=" + (String)c3out3 + " " );

      if(c3out1==1) {                    //
         //analogWrite(PIN_P1, 1023);
         digitalWrite(PIN_O1, HIGH);
      }
      else if(c3out1==0) {              //
         //analogWrite(PIN_P1, 0);
         digitalWrite(PIN_O1, LOW);
      }


      if(c3out2==1) {                    //
         analogWrite(PIN_P2, 1023);
         digitalWrite(PIN_O2, LOW);
      }
      /*
      else if(c3out2==0) {              //
         analogWrite(PIN_P2, 0);
         digitalWrite(PIN_O2, LOW);
      }
      */

      // c3out3 auto-switch by internal variable, not by website
            if(c3out3==1)
               digitalWrite(PIN_O3, HIGH);
            else if(c3out3==0)
               digitalWrite(PIN_O3, LOW);
      
   }

   client.flush();

   //---------------------------------------
   client.stop();
   //WiFi.disconnect();
   //Serial.println("\nVerbindung beendet");

   //---------------------------------------
   dashboard(DisplayMode);
   DisplayMode++;
   if(DisplayMode>MaxDisplayModes) DisplayMode=0;

   Serial.println("\nSchlafe jetzt ... \n");
   //ESP.deepSleep( 15*60000000); //Angabe in Minuten - hier 15
   delay(2000);  // delay(1000);
}



//----------------------------------------------------------------------------
// References
//----------------------------------------------------------------------------

/*
    https://github.com/ThingPulse/esp8266-oled-ssd1306/issues ;
*/
