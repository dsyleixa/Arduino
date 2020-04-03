/*  ESP8266 NodeMCU
 *  ESP8266WiFi Client für Remote Sensor Werte
 *  Client 2, Port 8081, wifi server .200
 *   
 * 
 *  Quelle website:
 *  http://www.mikrocontroller-elektronik.de/nodemcu-esp8266-tutorial-wlan-board-arduino-ide/
 *
 *  Temperatur wird über angeschlossenen Sensor DS18B20 ausgelesen und per 
 *  Url an einen Server uebertragen.
 *  Als Server (also Empfaenger) kann ebenfalls ein NodeMcu-Board verwendet werden.
 *  Ein Beispiel-Empfaenger empfehlen wir das Script "NodeMCU-Server-TFT-Temperatur" auf unser
 *  Projektseite. 
 * 
 *  Arduino IDE 1.8.9  
 */

 /*
  PINOUT:
  =======                         --v--
  digital:         default        myStd         bRk               ESP_motorShield
   D0     16       WAKE           ---           ---               ---
   D1      5       I2C SCL        I2C SCL       I2C SCL           out D1 motorA
   D2      4       I2C SDA        I2C SDA       I2C SDA           out D2 motorB
   D3      0       FLASH/LED      in D3       built-in btn+LED    out D3 motorA 
   D4      2       TX1            in/out        ---               out D4 motorB
   D5     14       SPI SCK        in D5 (DHT)   in D5 (DHT)       SDA
   D6     12       SPI MISO       out D6        out D6            SCL
   D7     13       SPI MOSI       in D7 (DHT)   in D7 (DHT)       in  D7 
   D8     15       MTD0 PWM       out D8        ---               out D8
   D9      3       UART RX0       USB, Serial   USB, Serial       USB, Serial
   D10     1       UART TX0       USB, Serial   USB, Serial       USB, Serial
  (D12)   10       GPIO intern    TFT reset     ---               TFT reset
 *  
 */

 char * clientname = "Client 2"; // Küche  
 char * ver = "2.016";


#include <stringEx.h>  // cstringarg( char* haystack, char* vname, char* sarg )

//----------------------------------------------------

const double  fINVAL = -99.9;
char     sNEXIST[10]= "--";
char     sINVAL[10] = "??";

//----------------------------------------------------
// logval vlog struct

typedef struct {
   double    vact=fINVAL, vmin=fINVAL, vmax=fINVAL, vmean=fINVAL ; // min,act,max,mean
   uint32_t  tact=0, tmin=0, tmax=0, tmean=0, talert=0 ; // time (millis)
   char      sact[20]="--", smin[20]="--", smax[20]="--",smean[20]="--";
} vlog;


static vlog c2t1, c2h1, 
            c2t2, c2h2,    // local temperature, humidity 
            c2p1, c2q1;    // local barometr.air pressure, quality 

            
//----------------------------------------------------------------------------
#include <Wire.h>
 
#define ESPSDA   D2   // GPIO4 
#define ESPSCL   D1   // GPIO5 

//----------------------------------------------------------------------------
// IO pins  
//----------------------------------------------------------------------------

#define PIN_RESETAlarm  D3      // Btn 0=D3 reset Alarm
#define PIN_OUT0        D6      //  alert
#define PIN_OUT2        D8      //  out



//----------------------------------------------------------------------------
// OLED SSD1306

#include <ESP_SSD1306.h>    // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>   // Needs a little change in original Adafruit library (See README.txt file)
#include <Fonts/FreeMono9pt7b.h> 
#include <Fonts/FreeMono8pt7b.h>    

ESP_SSD1306    display(-1);

#define CHR_DEGREE (unsigned char)247               // ° symbol for OLED font

 

//----------------------------------------------------------------------------
// IO pins
//----------------------------------------------------------------------------
#define  PIN_RESETAlarm   D3   // bRk built-in Btn 0=D3
int      btnRST=0;        // Btn state of PIN_RESETAlarm 
int8     LocalAlive=0;
int      emergencycnt=0;      // sensor alalrm
int      RemindCnt=99;    // remind action 


// outputs + signals
#define   PIN_OUT0    D6   // Alert LED Pin D6=GPIO12 


int16_t  c2out0=0,
         c2out1=0,
         c2out2=0;  


//----------------------------------------------------
// DHT 
String  DIGIN1name = "DHT1";
String  DIGIN2name = "DHT2";


// digital 1-wire pins for DHT sensors

#define PIN_DHT1    D5       //  DHT Sensor DHT22
#define PIN_DHT2    D7       //  DHT Sensor DHT11
   
//----------------------------------------------------
// analog pins

String A0internname = "iA0";  // intern: ESP8266

int16_t intA0;
float   fintA0;
char    sfintA0[20];

    



//----------------------------------------------------
#include <DHT.h> 
// DHT sensor defs
 
DHT  DHT_1(PIN_DHT1, DHT22),  // 1.DHT Typ definieren
     DHT_2(PIN_DHT2, DHT11);  // 2.DHT Typ definieren
     

//----------------------------------------------------------------------------
#include <ESP8266WiFi.h>

#include "data\settings.h"  // sets + initializes passwords

// WiFi Router 
extern char* website_pwd;     //  website password log in  "xyz"
extern char* website_title;   //  website caption          "MySyte"
extern char* website_url;     //  website url              "http:\\mysite.com"
extern const char* ssid;      //  local WiFi ssid          "myLocalWifiName"
extern const char* password;  //  local wifi password      "1234567890"


const char* host = "192.168.2.200"; // Server der die temperatur empfangen soll 

const char* script = "/client/client2/"; // URL/Verzeichnis das wir gewaehlt haben

const char* idnr = "1"; //Hier kann man dem Sensor eine beliebe ID zuteilen (derzeit ungenutzt)
 
volatile long timeoutcnt=0;  // timeouts to server



//----------------------------------------------------------------------------

char * dashboard(int mode) {
   
  display.setRotation(2);
  display.clearDisplay();
  
  if(mode==1) {   
       display.setFont();   
                    
       String ip_oled= "IP: " + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] 
                            + "."  + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3] 
                            + "-"  + (String)WiFi.RSSI() ; 
       display.setCursor( 0, 0);  display.print(ip_oled );

       //display.setFont();
       display.setCursor( 0, 9);  display.print("timeout svr="+String(timeoutcnt) );   
       display.setFont(&FreeMono8pt7b);           
       display.setCursor( 0,26);  display.print("1." + (String)c2t1.sact +  "*C");       
          //display.setCursor(75,26);  display.print("F:"+(String)c2h1.sact );
       display.setCursor( 0,39);  display.print("2." + (String)c2t2.sact +  "*C" );       
          //display.setCursor(75,39);  display.print("F:"+(String)c2h2.sact );       
       display.setFont();
       display.setCursor( 0,44);  display.print(ver);

       display.setFont(&FreeMono8pt7b);
       display.setCursor( 0,63);  display.print("ALARM " +(String)emergencycnt );  // Svr alarm state
           if(LocalAlive) {
           display.setCursor(75,63);  display.print("ALIVE"); 
           }
           else {
           display.setCursor(75,63);  display.print((String)"  (" +RemindCnt +")" );  
           }
       display.setFont();  
       display.display();    
  }  
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
      { v.vact = f; }
   else
      { v.vact = (v.vact +f )/2; }
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
   else   
   if( v.vact<=v.vmin )  {      
      v.vmin= 0.9*v.vmin + 0.1*v.vact;
      v.tmin= v.tact;
   }
    
   if (v.vact > v.vmax+10.0 ) {   // error? => small adjust max  
      v.vmax+=0.5;
      v.tmax = v.tact;
   }            
   else
   if( v.vact>=v.vmax ) {
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
// checkRESETBtn
//----------------------------------------------------------------------------
 void checkRESETBtn(){
    btnRST=digitalRead(PIN_RESETAlarm);           // btn press bRk built-in Btn 0=D3
    if(LocalAlive==0 && !btnRST)  LocalAlive=1;   // store local alive=true when btn press (inverse)!
    digitalWrite(LED_BUILTIN, btnRST);            // show btnRST press
    delay(1); 
 }

 
//----------------------------------------------------------------------------
// SETUP
//----------------------------------------------------------------------------

void setup() {
  
  Serial.begin(115200);
  delay(10);

  pinMode( PIN_OUT0, OUTPUT);
  pinMode( PIN_OUT2, OUTPUT);
  pinMode( LED_BUILTIN, OUTPUT);
  
  pinMode(PIN_RESETAlarm, INPUT);   

  
  // i2c + OLED 
  Wire.begin(ESPSDA,ESPSCL); // SDA, SCL
  delay(1);
 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
  display.setRotation(2);
  
  display.setFont();  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor( 0, 0);  display.print("OLED TEST OK");
  display.display();  

  // WiFi start
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
  delay(500);
  
}



//----------------------------------------------------------------------------
// LOOP
//----------------------------------------------------------------------------

// Bei deepSleep wird die loop Schleife eigentlich nur einmal durchlaufen

void loop() {  
  
   static float ftmp;
   static unsigned long tms=0;  
   static unsigned long dtms=0;
   unsigned long timeout;
   
   WiFiClient client;
   const int httpPort = 8081;   
    
   checkRESETBtn();
     
   delay(10);    
     
   ftmp = DHT_1.readTemperature();          // 1. Temperatur auslesen (Celsius)
     delay(10);
     if (isnan(ftmp)) ftmp=fINVAL;       
     logval(ftmp, c2t1); 
     delay(10);     

   ftmp = DHT_1.readHumidity();             // 1. Feuchtigkeit auslesen (Prozent)
     delay(1);
     if (isnan(ftmp)) ftmp=fINVAL;       
     logval(ftmp, c2h1); 
     delay(10);    

   ftmp = DHT_2.readTemperature();          // 1. Temperatur auslesen (Celsius)
     delay(10);
     if (isnan(ftmp)) ftmp=fINVAL;       
     logval(ftmp, c2t2); 
     delay(10);     

   ftmp = DHT_2.readHumidity();             // 1. Feuchtigkeit auslesen (Prozent)
     delay(1);
     if (isnan(ftmp)) ftmp=fINVAL;       
     logval(ftmp, c2h2); 
     delay(10);    
  

   //---------------------------------------
   // display on OLED
   dashboard(1);

   checkRESETBtn();
   
   //---------------------------------------
   // msg string
   
   String vals = "";
   vals += "&c2t1=";
   vals += (String)c2t1.vact;
   vals += "&c2h1=";
   vals += (String)c2h1.vact;   
   vals += "&c2t2=";
   vals += (String)c2t2.vact;
   vals += "&c2h2=";
   vals += (String)c2h2.vact;
   vals += "&LocalAlive=";
   vals += (String)LocalAlive;  
   
   Serial.print("Sensorwerte: "); 
   Serial.println( vals ); 
   delay(1);
   if(LocalAlive) LocalAlive=0;  // reset local alive signal
   /*
   Serial.print("Reset LocalAlive: "); 
   Serial.println( (String)LocalAlive ); 
   */
  
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
       dashboard(1);
         
       Serial.println("Verbindungsaufbau nicht moeglich!!!");
       if (versuche>10){
         Serial.println("Klappt nicht, versuche es spaeter noch mal!");
         client.stop();
         
         //WiFi.disconnect(); 
         //ESP.deepSleep( 10*60000000ul); //Etwas später neu probieren, solange schlafen
         delay(2000);
       }
        
     }

     checkRESETBtn();
     
     delay(1000);
   } while (erg!=1);

   //---------------------------------------
   // msg string to server
   
   String url = script; //Url wird zusammengebaut: script = "/client/client2/";
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
        dashboard(1); 
        checkRESETBtn();       
        delay(1000);    
     }
     else {
        Serial.println("\nAbbruch, keine Rueckantwort vom Server!\n");
        break;
     }
   }  

   checkRESETBtn();
   
   timeout=millis();
   String msgline;
   //if(client.available())    
   Serial.println("Rueckgabe vom Server:\n");
   if(client.available()){
     timeoutcnt=0;
     while(client.available()){
        msgline = client.readStringUntil('\r');
        //Serial.print("msgline="); Serial.println(msgline); 
     }
   String Sargval=""; 
   char carg[TOKLEN], ctok[TOKLEN], cmsg[MAXLEN];    
   Sargval="";
   msgline.toCharArray(cmsg, MAXLEN-1);
   Serial.println();
   Serial.print("cmsg="); Serial.println(cmsg);

   c2out0=-1; // undefined
   c2out1=0;
   c2out2=0;
   if(cmsg!="") {         
        cstringarg(cmsg, "c2out0", carg);  // alert pin (D6) switch on/off
        Sargval=(String)carg;
        if(Sargval!="") {
           c2out0=Sargval.toInt(); 
        }   

        cstringarg(cmsg, "c2out1", carg);  // switch on/off
        Sargval=(String)carg;
        if(Sargval!="") {
           c2out1=Sargval.toInt(); 
        }  

        cstringarg(cmsg, "c2out2", carg);  // switch on/off
        Sargval=(String)carg;
        if(Sargval!="") {
           c2out2=Sargval.toInt(); 
        }  
        
        cstringarg(cmsg, "remindcnt", carg);
        Sargval=(String)carg;
        if(Sargval!="") {
           RemindCnt=Sargval.toInt(); 
        }  

        cstringarg(cmsg, "emergencycnt", carg);
        Sargval=(String)carg;
        if(Sargval!="") {
           emergencycnt=Sargval.toInt(); 
        }  
        
        /* 
        cstringarg(cmsg, "c2out3", carg);
        Sargval=(String)carg;
        if(Sargval!="") {
           c2out3=Sargval.toInt(); 
        }
        */
   }
    
   // debug
   Serial.println( (String)"c2out0=" + (String)c2out0 + " <<<<<<<<<<<<" );
   Serial.println( (String)"c2out1=" + (String)c2out1 + " <<<<<<<<<<<<" );
   Serial.println( (String)"c2out2=" + (String)c2out2 + " " );
   //  Serial.println( (String)"c2out3=" + (String)c2out3 + " " ); 
   Serial.println( (String)"RemindCnt=" + (String)RemindCnt + " <<<<<<<<<<<<" );
      
   
   if(c2out0>0 || emergencycnt>0 ) {           // Server message: alarm, LED ON, please confirm global alive
      digitalWrite( PIN_OUT0, HIGH);
   }
   else if(c2out0==0 && emergencycnt==0 ) {    // Server message: no alarm, LED OFF, global still alive
      digitalWrite( PIN_OUT0, LOW);
   }
   if(c2out2==0) digitalWrite(PIN_OUT2, LOW); else digitalWrite(PIN_OUT2, HIGH);
   
   //  if(c2out3==0) digitalWrite(PIN_OUT3, LOW); else digitalWrite(PIN_O3, HIGH);
     
   }         
   client.flush();

   //---------------------------------------
   client.stop();
   //WiFi.disconnect(); 
   //Serial.println("\nVerbindung beendet");

   //---------------------------------------
   dashboard(1);

   checkRESETBtn();
   
   Serial.println("\nSchlafe jetzt ... \n");
   //ESP.deepSleep( 15*60000000); //Angabe in Minuten - hier 15
   delay(3000);
}
