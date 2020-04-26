/*  ESP8266 NodeMCU
    ESP8266WiFi Client für Remote Sensor Werte
    Client 1, Port 8081, wii server .200

    Quelle website:
    http://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/

    Temperatur wird über angeschlossenen Sensor DS18B20 ausgelesen und per
    Url an einen Server uebertragen.
    Als Server (also Empfaenger) kann ebenfalls ein NodeMcu-Board verwendet werden.
    Ein Beispiel-Empfaenger empfehlen wir das Script "NodeMCU-Server-TFT-Temperatur" auf unser
    Projektseite.

    Arduino IDE 1.8.9

*/

/*
   PINOUT:
   =======
   digital:         default        myStd         bRk               ESP_motorShield
   D0     16       WAKE           ---           ---               ---
   D1      5       I2C SCL        I2C SCL       I2C SCL           out D1 motorA
   D2      4       I2C SDA        I2C SDA       I2C SDA           out D2 motorB
   D3      0       FLASH/LED      out D3     built-in btn+LED     out D3 motorA
   D4      2       TX1            In DHT        ---               out D4 motorB
   D5     14       SPI SCK        In B/S        out D5            SDA
   D6     12       SPI MISO       Out D6        ---               SCL
   D7     13       SPI MOSI       Out D7        out D7            in  D7
   D8     15       MTD0 PWM       Out D8        ---               out D8
   D9      3       UART RX0       USB, Serial   USB, Serial       USB, Serial
   D10     1       UART TX0       USB, Serial   USB, Serial       USB, Serial
   (D12)   10       GPIO intern    TFT reset     ---               TFT reset

*/

char * clientname = "Client 0"; // Garten
char * ver = "0.015";

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
LiquidCrystal_I2C  lcd = LiquidCrystal_I2C(0x27, 20, 4); // Change to (0x27,16,2) for 16x2 LCD.



//----------------------------------------------------------------------------
// char symbols

#define CHR_DEGREE (unsigned char)247               // ° symbol for OLED font
//char    STR_DEGREE[] = {247, 0, 0};                 // ° OLED font specimen (°,°C,°F,K)


//----------------------------------------------------------------------------
// output pins (LED, motors)

#define PIN_P1    D1     // pwm
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
#define PIN_DHT1    D7       //  DHT Sensor 


volatile int8  c0out1=0, c0out2=0, c0out3=0; // Client0; actual output pin states; stop=0, fwd=1, rev=-1;

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

String A0intname = "Sens.A0";  // intern: ESP8266
String A0muxname = "Erde.Li";  // mux: ADS1015 (i2c)
String A1muxname = "Erde.Re";
String A2muxname = "Erde.A2";
String A3muxname = "Wasser ";


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


DHT  DHT_1(PIN_DHT1, DHT22);  // 1.DHT Typ definieren




//----------------------------------------------------
// logval vlog struct

typedef struct {
   double    vact=fINVAL, vmin=fINVAL, vmax=fINVAL, vmean=fINVAL ; // min,act,max,mean
   uint32_t  tact=0, tmin=0, tmax=0, tmean=0, talert=0 ; // time (millis)
   char      sact[20]="--", smin[20]="--", smax[20]="--",smean[20]="--";
} vlog;


static vlog c0t1, c0h1, c0t2, c0h2,  // local temperature, humidity
       c0p1, c0q1;              // local barometr.air pressure, quality


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

const char* script = "/client/client0/"; // URL/Verzeichnis das wir gewaehlt haben

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
double calA0ESP (int ADC) {
   int RES = 1023;
   if(ADC<0) ADC=0;
   if(ADC>RES) ADC=RES;
   return ( (double)ADC*100.0 * 1.0) /(double)RES;
}



//----------------------------------------------------------------------------

char * dashboard(int mode) {

   display.setRotation(0);
   display.clearDisplay();
   lcd.clear();

   if(mode==0) {
      display.setFont();

      String ip_oled = "IP: " + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] 
                            + "."  + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3] 
                            + "-"  + (String)WiFi.RSSI() ; 
      display.setCursor( 0, 0);  display.print(ip_oled  );
      display.setCursor( 0, 8);  display.print((String)clientname+" ver="+ver);

      display.setCursor( 0,16);  display.print("1." + (String)c0t1.sact + "*C");
      display.setCursor(75,16);  display.print("F:"+(String)c0h1.sact );
      display.setCursor( 0,24);  display.print("2." + (String)c0t2.sact + "*C" );
      display.setCursor(75,24);  display.print("F:"+(String)c0h2.sact );

      display.setCursor( 0,32);  display.print("3.iA0="+(String)sfespA0);
      display.setCursor(72,32);  display.print("hPa="+(String)(int)c0p1.vact);
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
      display.setCursor( 0, 40);  display.print( (String)c0p1.sact +  " hPa");
      display.display();
      lcd.setCursor(0,0); lcd.print("Luftdruck:");
      lcd.setCursor(0,1); lcd.print((String)c0p1.sact +  " hPa");
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");
   }
   else if (mode == 2) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("T Aussen:");
      display.setCursor( 0, 40);  display.print( (String)c0t1.sact + "*C");
      display.display();
      lcd.setCursor(0,0); lcd.print("T Aussen:");
      lcd.setCursor(0,1); lcd.print((String)c0t1.sact + "'C");
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");
   }
   else if (mode == 3) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("Hu Aussen");
      display.setCursor( 0, 40);  display.print( (String)c0h1.sact +  " %");
      display.display();
      lcd.setCursor(0,0); lcd.print("Hu Aussen");
      lcd.setCursor(0,1); lcd.print((String)c0h1.sact +  " %");
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");
   }
   else if (mode == 4) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("T Innen:");
      display.setCursor( 0, 40);  display.print( (String)c0t2.sact + "*C");
      display.display();
      lcd.setCursor(0,0); lcd.print("T Innen:");
      lcd.setCursor(0,1); lcd.print((String)c0t2.sact + "'C");
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");
   }
   else if (mode == 5) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("Hu Innen:");
      display.setCursor( 0, 40);  display.print( (String)c0h2.sact +  " %");
      display.display();
      lcd.setCursor(0,0); lcd.print("Hu Innen:");
      lcd.setCursor(0,1); lcd.print((String)c0h2.sact +  " %");
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");
   }
   else if (mode == 6) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("A S0:");  display.print(sfespA0);
      display.setCursor( 0, 40);
      display.display();
      lcd.setCursor(0,0); lcd.print("A S0: ");  lcd.print(sfespA0);
      lcd.setCursor(0,1); lcd.print("");
      lcd.setCursor(0,2); lcd.print("");
      lcd.setCursor(0,3); lcd.print("");
      
   }
   else if (mode == 7) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("0 Li:");  display.print(sfadc0);
      display.setCursor( 0, 40);  display.print("1 Re:");  display.print(sfadc1);
      display.display();
      lcd.setCursor(0,0); lcd.print("0 Li: "); lcd.print(sfadc0);
      lcd.setCursor(0,1); lcd.print("1 Re: "); lcd.print(sfadc1);
      lcd.setCursor(0,2); lcd.print("2 OL: "); lcd.print(sfadc2);
      lcd.setCursor(0,3); lcd.print("3 Wa: "); lcd.print(sfadc3);
   }
   else if (mode == 8) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("2 OL:");  display.print(sfadc2);
      display.setCursor( 0, 40);  display.print("3 Wa:");  display.print(sfadc3);
      display.display();
      lcd.setCursor(0,0); lcd.print("0 Li: "); lcd.print(sfadc0);
      lcd.setCursor(0,1); lcd.print("1 Re: "); lcd.print(sfadc1);
      lcd.setCursor(0,2); lcd.print("2 OL: "); lcd.print(sfadc2);
      lcd.setCursor(0,3); lcd.print("3 Wa: "); lcd.print(sfadc3);
   }
   /*
   else
      if (mode == 9) {
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor( 0, 20);  display.print("Alarms: "+(String)EmergencyCnt);
      display.setCursor( 0, 60);  display.print("Warng.: "+(String)AlertCnt);
      display.display();
   }
     
   */
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
   pinMode(PIN_P1, OUTPUT);
   digitalWrite(PIN_P1, LOW);
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
   static int8_t  LEDmode=0;

   display.setRotation(0);
 

   //---------------------------------------
   // read DHT+bme280 Sensors

   ftmp = DHT_1.readTemperature();          // 1. Temperatur auslesen (Celsius)
   delay(10);
   if (isnan(ftmp)) ftmp=fINVAL;
   logval(ftmp, c0t1);
   delay(10);

   ftmp = DHT_1.readHumidity();             // 1. Feuchtigkeit auslesen (Prozent)
   delay(1);
   if (isnan(ftmp)) ftmp=fINVAL;
   logval(ftmp, c0h1);
   delay(10);

   ftmp=fINVAL;
   ftmp = bme_x77.readTemperature();
   if (isnan(ftmp)) ftmp=fINVAL;
   logval(ftmp, c0t2);
   delay(1);

   ftmp=fINVAL;
   ftmp = bme_x77.readHumidity();       // 2. Feuchtigkeit auslesen (Prozent)
   if (isnan(ftmp)) ftmp=fINVAL;
   logval(ftmp, c0h2);
   delay(1);

   ftmp=fINVAL;
   ftmp = FNNcorr + bme_x77.readPressure()/100.0 ;
   if (isnan(ftmp)) ftmp=fINVAL;
   logval(ftmp, c0p1);

   delay(1);

   Serial.print("DHT 1: ");
   Serial.print(c0t1.sact);   Serial.print("   ");  Serial.print(c0h1.sact); Serial.println(" rH");
   Serial.print(" HT 2: ");
   Serial.print(c0t2.sact);   Serial.print("   ");  Serial.print(c0h2.sact); Serial.println(" rH");
   Serial.print("hPa="); Serial.println(c0p1.vact);
   delay(1);

   // internal ADC A0
   espA0=0;
   // espA0=analogRead(A0);
   fespA0=calA0ESP(espA0);
   sprintDouble(sfespA0,fespA0,2,1,false);


   Serial.print("espA0="); Serial.print(sfespA0); Serial.println("%");

   // ADS1115 analog (ADC) sensors
   xadc0 = ADSx48.readADC_SingleEnded(0);  fadc0=calADS1015(xadc0);
   xadc1 = ADSx48.readADC_SingleEnded(1);  fadc1=calADS1015(xadc1);
   xadc2 = ADSx48.readADC_SingleEnded(2);  fadc2=calADS1015(xadc2);
   xadc3 = ADSx48.readADC_SingleEnded(3);  fadc3=calADS1015(xadc3);

   sprintDouble(sfadc0,fadc0,2,1,false);
   sprintDouble(sfadc1,fadc1,2,1,false);
   sprintDouble(sfadc2,fadc2,2,1,false);
   sprintDouble(sfadc3,fadc3,2,1,false);
   delay(1);

   Serial.println(" "); Serial.println(" ");

   delay(1);

   //---------------------------------------
   // display on OLED
   dashboard(LEDmode);

   //---------------------------------------
   // msg string

   String vals = "";        
   // DHT t+h
   vals += "&c0t1=";   vals += c0t1.sact;
   vals += "&c0h1=";   vals += c0h1.sact; 
   
   // BME t+h
   vals += "&c0t2=";   vals += c0t2.sact;
   vals += "&c0h2=";   vals += c0h2.sact;
   
   // BME hPa
   vals += "&c0p1=";   vals += (int)c0p1.vact;
   
   // esp ADC
   vals += "&c0espA0="; vals += sfespA0;
   
   // ads ADC
   vals += "&c0adc0=";  vals += sfadc0;
   vals += "&c0adc1=";  vals += sfadc1;
   vals += "&c0adc2=";  vals += sfadc2;
   vals += "&c0adc3=";  vals += sfadc3;


   Serial.print("Sensorwerte: ");
   Serial.println( vals );
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
         dashboard(LEDmode);

         Serial.println("Verbindungsaufbau nicht moeglich!!!");
         if (versuche>10) {
            Serial.println("Klappt nicht, ich versuche es spaeter noch mal!");
            client.stop();

            //WiFi.disconnect();
            //ESP.deepSleep( 10*60000000); //Etwas später neu probieren, solange schlafen
            delay(5000);
         }

      }

      delay(1000);
   } while (erg!=1);

   //---------------------------------------
   // msg string to server

   String url = script; //Url wird zusammengebaut: script = "/client/client0/";
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
         dashboard(LEDmode);
         delay(1000);
      }
      else {
         Serial.println("\nAbbruch, keine Rueckantwort vom Server!\n");
         break;
      }
   }

   timeout=millis();
   String msgline;
   //if(client.available())
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
         cstringarg(cmsg, "c0out1", carg);  // out1  switch on/off/rev
         Sargval=(String)carg;
         if(Sargval!="") {
            c0out1=Sargval.toInt();
         }

         cstringarg(cmsg, "c0out2", carg);  // out2  switch on/off
         Sargval=(String)carg;
         if(Sargval!="") {
            c0out2=Sargval.toInt();
         }

         cstringarg(cmsg, "c0out3", carg);  // out3  switch on/off
         Sargval=(String)carg;
         if(Sargval!="") {
            c0out3=Sargval.toInt();
         }

      }

      // debug
      Serial.println( (String)"c0out1=" + (String)c0out1 + " <<<<<<<<<<<<<<<<<<<<<<<<<<" );
      Serial.println( (String)"c0out2=" + (String)c0out2 + " " );
      Serial.println( (String)"c0out3=" + (String)c0out3 + " " );


      if(c0out1==1) {                    //
         analogWrite(PIN_P1, 1023);
         digitalWrite(PIN_O1, LOW);
      }
      else if(c0out1==0) {              //
         analogWrite(PIN_P1, 0);
         digitalWrite(PIN_O1, LOW);
      }
      else if(c0out1==0) {              //
         analogWrite(PIN_P1, 1023);
         digitalWrite(PIN_O1, HIGH);
      }


      if(c0out2==1) {                    //
         analogWrite(PIN_P2, 1023);
         digitalWrite(PIN_O2, LOW);
      }
      else if(c0out2==0) {              //
         analogWrite(PIN_P2, 0);
         digitalWrite(PIN_O2, LOW);
      }

      if(c0out3==1)
         digitalWrite(PIN_O3, HIGH);
      else if(c0out3==0)
         digitalWrite(PIN_O3, LOW);
   }

   client.flush();

   //---------------------------------------
   client.stop();
   //WiFi.disconnect();
   //Serial.println("\nVerbindung beendet");

   //---------------------------------------
   dashboard(LEDmode);
   LEDmode++;
   if(LEDmode>8) LEDmode=0;

   Serial.println("\nSchlafe jetzt ... \n");
   //ESP.deepSleep( 15*60000000); //Angabe in Minuten - hier 15
   delay(1000);
}



//----------------------------------------------------------------------------
// References
//----------------------------------------------------------------------------

/*
    https://github.com/ThingPulse/esp8266-oled-ssd1306/issues ;
*/
