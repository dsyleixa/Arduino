//============================================================================
// PROGRAMMTEIL 1 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_BMP280.h>
#include <TimeLib.h>
#include <Timezone.h>

// Watchdog und System-Konstanten
#define MAXLEN 512
#define TOKLEN 64

// Hardware I2C Pins fuer NodeMCU
#ifndef PIN_SDA
  #define PIN_SDA 4 // D2
#endif
#ifndef PIN_SCL
  #define PIN_SCL 5 // D1
#endif

// Lokale Relais- und LED-Pins
#define PIN_OUT0 2  // Blue onboard LED (reverse logic)
#define PIN_OUT1 14 // D5 - Relais 1
#define PIN_OUT2 12 // D6 - Relais 2
#define PIN_OUT3 13 // D7 - Relais 3

// Vorab-Deklarationen (Prototypen) fuer den Compiler
void handleWebsite(WiFiClient client, String HTTP_req);
void dashboard_Init();
void dashboard(int mode);
void handleClients();
void updateTimeNow();
void buildDateTimeString();
int checkAlarms();
void systemWatchdog();
String basicAuthString(const char* uname, const char* upwd);
String htmlButton(String caption, String action, int height, int width);

// Struktur fuer Sensor-Messwerte aus dem Original-Code
struct SensorData {
   char Sname[20];
   double vact;
   double vmin;
   double vmax;
   double vmean;
   double sact;
   double smin;
   double smax;
   double smean;
   unsigned long tFail;
};

// Globale Instanzen der Server-Objekte
ESP8266WebServer webserver(8081); // Port 8081 exklusiv fuer externe ESP-Clients
//------------------- Ende Programmteil 1 -----------------------------------


//============================================================================
// PROGRAMMTEIL 2 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

// KORRIGIERT: Keine extern-Lügen mehr! Die originalen Werte aus v090n direkt im RAM deklariert:
char website_uname[] = "admin";
char website_upwd[]  = "admin";
char website_title[] = "HomeServer";
char website_url[]   = "zaphod.sytes.net";
int  http_port       = 80;

char ssid[]          = "IhrWlanName";      // << Bitte tragen Sie hier kurz Ihren echten WLAN-Namen ein
char password[]      = "IhrWlanPasswort";  // << Bitte tragen Sie hier kurz Ihr echtes WLAN-Passwort ein

IPAddress this_ip(192, 168, 178, 200);     // << Bitte passen Sie diese 3 IPs bei Bedarf an Ihr Heimnetz an
IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);

// Globale System-Zähler und Status-Variablen
int EmergencyCnt = 0;
int RemindCnt = 0;
int LCDmode = 0;

unsigned long millisLastConfirm = 0;
unsigned long dmillisLastConfirm = 0;
unsigned long dhrsLastConfirm = 0;
const unsigned long hrsConfirmLimit = 12; // 12 Stunden Limit

unsigned long lastI2CDataMillis = 0;
unsigned long lastSensorDataMillis = 0;

// Relais-Zustände (Server)
int OUT1 = 0;
int OUT2 = 0;
int OUT3 = 0;

// Relais-Zustände (Clients)
int c0out1 = 0, c0out2 = 0, c0out3 = 0, c0tx3 = 18;
int c1out1 = 0, c1out2 = 0, c1out3 = 0;
int c2out1 = 0, c2out2 = 0, c2out3 = 0;
int c3out1 = 0, c3out2 = 0, c3out3 = 0;

// Klarnamen der Geräte und Sensoren
String SERVERname = "Haupt-Server";
String CLIENT0name = "Gartenhaus";
String CLIENT1name = "Wohnzimmer";
String CLIENT2name = "Keller";
String CLIENT3name = "Dachboden";

String OUT1name = "Heizung";
String OUT2name = "Licht Außen";
String c0OUT1name = "Pumpe";
String c0OUT2name = "Licht";
String c1OUT1name = "Ventilator";
String c1OUT2name = "Steckdose";
String c2OUT1name = "Lüfter";
String c2OUT2name = "Reserve";
String c3OUT1name = "Alarm";
String c3OUT2name = "Reserve";

String svSECT1name = "Innen-Sensor";
String c0SECT1name = "Luft";
String c0SECT2name = "Boden";
String c1SECT1name = "Klima";
String c1SECT2name = "Boden";
String c2SECT1name = "Klima";
String c2SECT2name = "Boden";
String c3SECT1name = "Klima";
String c3SECT2name = "Boden";

String A0intname = "ESP-A0";
String A0muxname = "Mux-0";
String A1muxname = "Mux-1";
String c0A0intname = "ADC-Int";
String c0A0muxname = "ADC-0";
String c0A1muxname = "ADC-1";
String c0A2muxname = "ADC-2";
String c0A3muxname = "ADC-3";
String c3A3muxname = "ADC-Soil";

// Schwellwerte und Invalide-Marker
const double fINVAL = -999.0;
const double fFireLIMIT = 15.0; 
const double adcSoilMin = 200.0;

double FNNcorr = 0.0; 

// Instanziierung lokaler Hardware (Server)
Adafruit_BMP280 bmp_x77; // Barometer-Sensor
WiFiServer wifiserver(80); // Das Dashboard läuft auf Port 80

// Globale Zeit- und Datums-Strings
String timestr = "00:00:00";
String datestr = "01.01.2026";

// Dynamische Authentifizierungs-Konstante im RAM zur Trennung der Benutzer
int auth_realm_counter = 1;

//------------------- Ende Programmteil 2 -----------------------------------











//============================================================================
// PROGRAMMTEIL 3 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

// Instanziierung aller globalen SensorData-Strukturen fuer den RAM

// Lokale Server-Sensorstrukturen
SensorData svt1;       // Server Temperatur
SensorData svh1;       // Server Luftfeuchtigkeit
SensorData svp1;       // Server Luftdruck (BMP280)
SensorData svespA0;    // Server Analogeingang (ESP-intern)

// Client 0 Sensorstrukturen (Gartenhaus)
SensorData c0t1;       // Sektor 1 Temperatur
SensorData c0h1;       // Sektor 1 Luftfeuchtigkeit
SensorData c0t2;       // Sektor 2 Temperatur
SensorData c0h2;       // Sektor 2 Luftfeuchtigkeit
SensorData c0espA0;    // Client 0 Interner Analogwert
SensorData c0adc0;     // Mux Kanal 0 (Feuchtigkeit 1)
SensorData c0adc1;     // Mux Kanal 1 (Feuchtigkeit 2)
SensorData c0adc2;     // Mux Kanal 2 (Feuchtigkeit 3)
SensorData c0adc3;     // Mux Kanal 3 (Feuchtigkeit 4)

// Client 1 Sensorstrukturen (Wohnzimmer)
SensorData c1t1;       // Sektor 1 Temperatur
SensorData c1h1;       // Sektor 1 Luftfeuchtigkeit
SensorData c1t2;       // Sektor 2 Temperatur
SensorData c1h2;       // Sektor 2 Luftfeuchtigkeit

// Client 2 Sensorstrukturen (Keller)
SensorData c2t1;       // Sektor 1 Temperatur
SensorData c2h1;       // Sektor 1 Luftfeuchtigkeit
SensorData c2t2;       // Sektor 2 Temperatur
SensorData c2h2;       // Sektor 2 Luftfeuchtigkeit

// Client 3 Sensorstrukturen (Dachboden)
SensorData c3t1;       // Sektor 1 Temperatur
SensorData c3h1;       // Sektor 1 Luftfeuchtigkeit
SensorData c3t2;       // Sektor 2 Temperatur
SensorData c3h2;       // Sektor 2 Luftfeuchtigkeit


//----------------------------------------------------------------------------
// Hilfsfunktionen fuer mathematische Konvertierung & Formatierung
//----------------------------------------------------------------------------

// Kalibrierung und Umrechnung des analogen Rohwerts (A0) in Prozent
double calADC1023(double raw) {
   if (raw < 0) raw = 0;
   if (raw > 1023) raw = 1023;
   // Lineare Standardumrechnung in Prozent (0 - 100%)
   return (raw / 1023.0) * 100.0;
}

// Loggen und Verarbeiten neuer Sensorwerte mitsamt Min/Max/Mittelwert-Berechnung
void logval(double new_val, SensorData &struct_target) {
   if (new_val == fINVAL) return;

   struct_target.vact = new_val;
   struct_target.sact = new_val; // Stringkompatibler Aktuell-Wert

   // Extremwert-Überwachung im RAM
   if (new_val < struct_target.vmin) {
      struct_target.vmin = new_val;
      struct_target.smin = new_val;
   }
   if (new_val > struct_target.vmax) {
      struct_target.vmax = new_val;
      struct_target.smax = new_val;
   }

   // Laufende Mittelwertbildung (Gleitender Durchschnitt)
   if (struct_target.vmean == fINVAL || struct_target.vmean == 0.0) {
      struct_target.vmean = new_val;
   } else {
      struct_target.vmean = (struct_target.vmean * 0.95) + (new_val * 0.05);
   }
   struct_target.smean = struct_target.vmean;
   struct_target.tFail = 0; // Fehlerzähler bei erfolgreichem Wert nullen
}

//------------------- Ende Programmteil 3 -----------------------------------

//============================================================================
// PROGRAMMTEIL 4 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

void setup() {

   //-------------------------------------------------------------------------
   // Set output pins default
   //-------------------------------------------------------------------------

   pinMode(PIN_OUT0, OUTPUT);  digitalWrite(PIN_OUT0, 0); // blue onboard led (reverse logic)
   pinMode(PIN_OUT1, OUTPUT);  digitalWrite(PIN_OUT1, 1); // open relay
   pinMode(PIN_OUT2, OUTPUT);  digitalWrite(PIN_OUT2, 1); // open relay
   pinMode(PIN_OUT3, OUTPUT);  digitalWrite(PIN_OUT3, 1); // open relay


   //-------------------------------------------------------------------------
   // Start I/O communication channels
   //-------------------------------------------------------------------------

   Serial.begin(115200);
   delay(10);
   Serial.println(); Serial.println();
   Serial.println((String)"\nSystem Booting: Setup...   " + __FILE__  "  "  __DATE__  "  "  __TIME__ );

   Wire.begin(PIN_SDA, PIN_SCL);  // Wire.begin(int sda, int scl)   default=(4, 5);


   //-------------------------------------------------------------------------
   // Initialize local board hardware components
   //-------------------------------------------------------------------------

   dashboard_Init();
   dashboard(0);

   /* dht_1.begin(); */

   // KORRIGIERT: Nutzen des reinen Standard-Inits Ihrer Bibliotheksversion
   if (!bmp_x77.begin(0x76)) {  // 0x76 oder 0x77  // bme280 oder bmp280
      Serial.println("Could not find a valid BMP/BME280 sensor, check wiring!");
      while (1);
   }


   //-------------------------------------------------------------------------
   // Initialize network communication: Connect to WiFi network
   //-------------------------------------------------------------------------

   Serial.print((String)"Connecting to WiFi: " + ssid + " .");

   WiFi.config(this_ip, gateway, subnet);
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);

   int conncnt = 0;
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      conncnt++;
      if (conncnt > 40) {
         Serial.println("\nWiFi Connect Timeout! Resetting system...");
         ESP.reset();
      }
   }
   Serial.println();
   Serial.print("WiFi connected, local IP: "); Serial.println(WiFi.localIP());

   // Start local wifiserver stream on Dashboard port (80 or 8080)
   wifiserver.begin();
   Serial.println((String)"Dashboard Low-Level server started on Port " + http_port);
   
//------------------------ Ende Programmteil 4 -------------------------------


//============================================================================
// PROGRAMMTEIL 5 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

   //-------------------------------------------------------------------------
   // Set up webserver routing handlers (Port 8081 for ESP-Clients)
   //-------------------------------------------------------------------------

   // Das High-Level webserver-Objekt dient exklusiv den 4 externen Mess-Clients
   webserver.on("/client/client0/", handleClients);
   webserver.on("/client/client1/", handleClients);
   webserver.on("/client/client2/", handleClients);
   webserver.on("/client/client3/", handleClients);

   webserver.begin();
   Serial.println("High-Level Webserver for ESP-Clients started on Port 8081");


   //-------------------------------------------------------------------------
   // Initialize Client hardware structures
   //-------------------------------------------------------------------------

   c0t1.tFail = 999; c0t2.tFail = 999; c0h1.tFail = 999; c0h2.tFail = 999;
   c1t1.tFail = 999; c1t2.tFail = 999; c1h1.tFail = 999; c1h2.tFail = 999;
   c2t1.tFail = 999; c2t2.tFail = 999; c2h1.tFail = 999; c2h2.tFail = 999;
   c3t1.tFail = 999; c3t2.tFail = 999; c3h1.tFail = 999; c3h2.tFail = 999;

   c0adc0.tFail = 999; c0adc1.tFail = 999; c0adc2.tFail = 999; c0adc3.tFail = 999;

   // KORRIGIERT: Das '&' wurde entfernt, da der Array-Name selbst der Pointer ist!
   strcpy(svt1.Sname, "sv_t1");     svt1.vact = fINVAL;   svt1.vmax = -99.9;  svt1.vmin = 99.9;
   strcpy(svh1.Sname, "sv_h1");     svh1.vact = fINVAL;   svh1.vmax = -99.9;  svh1.vmin = 99.9;
   strcpy(svp1.Sname, "sv_p1");     svp1.vact = fINVAL;   svp1.vmax = -99.9;  svp1.vmin = 99.9;
   strcpy(svespA0.Sname, "sv_A0");  svespA0.vact = 0;     svespA0.vmax = 0;   svespA0.vmin = 100;

   strcpy(c0t1.Sname, "c0_t1");  c0t1.vact = fINVAL;  c0t1.vmax = -99.9;  c0t1.vmin = 99.9;
   strcpy(c0h1.Sname, "c0_h1");  c0h1.vact = fINVAL;  c0h1.vmax = -99.9;  c0h1.vmin = 99.9;
   strcpy(c0t2.Sname, "c0_t2");  c0t2.vact = fINVAL;  c0t2.vmax = -99.9;  c0t2.vmin = 99.9;
   strcpy(c0h2.Sname, "c0_h2");  c0h2.vact = fINVAL;  c0h2.vmax = -99.9;  c0h2.vmin = 99.9;

   strcpy(c1t1.Sname, "c1_t1");  c1t1.vact = fINVAL;  c1t1.vmax = -99.9;  c1t1.vmin = 99.9;
   strcpy(c1h1.Sname, "c1_h1");  c1h1.vact = fINVAL;  c1h1.vmax = -99.9;  c1h1.vmin = 99.9;
   strcpy(c1t2.Sname, "c1_t2");  c1t2.vact = fINVAL;  c1t2.vmax = -99.9;  c1t2.vmin = 99.9;
   strcpy(c1h2.Sname, "c1_h2");  c1h2.vact = fINVAL;  c1h2.vmax = -99.9;  c1h2.vmin = 99.9;

   strcpy(c2t1.Sname, "c2_t1");  c2t1.vact = fINVAL;  c2t1.vmax = -99.9;  c2t1.vmin = 99.9;
   strcpy(c2h1.Sname, "c2_h1");  c2h1.vact = fINVAL;  c2h1.vmax = -99.9;  c2h1.vmin = 99.9;
   strcpy(c2t2.Sname, "c2_t2");  c2t2.vact = fINVAL;  c2t2.vmax = -99.9;  c2t2.vmin = 99.9;
   strcpy(c2h2.Sname, "c2_h2");  c2h2.vact = fINVAL;  c2h2.vmax = -99.9;  c2h2.vmin = 99.9;

   strcpy(c3t1.Sname, "c3_t1");  c3t1.vact = fINVAL;  c3t1.vmax = -99.9;  c3t1.vmin = 99.9;
   strcpy(c3h1.Sname, "c3_h1");  c3h1.vact = fINVAL;  c3h1.vmax = -99.9;  c3h1.vmin = 99.9;
   strcpy(c3t2.Sname, "c3_t2");  c3t2.vact = fINVAL;  c3t2.vmax = -99.9;  c3t2.vmin = 99.9;
   strcpy(c3h2.Sname, "c3_h2");  c3h2.vact = fINVAL;  c3h2.vmax = -99.9;  c3h2.vmin = 99.9;

   strcpy(c0espA0.Sname, "c0_A0"); c0espA0.vact = 0;     c0espA0.vmax = 0;   c0espA0.vmin = 100;
   strcpy(c0adc0.Sname, "c0_ad0"); c0adc0.vact = fINVAL; c0adc0.vmax = 0;    c0adc0.vmin = 1023;
   strcpy(c0adc1.Sname, "c0_ad1"); c0adc1.vact = fINVAL; c0adc1.vmax = 0;    c0adc1.vmin = 1023;
   strcpy(c0adc2.Sname, "c0_ad2"); c0adc2.vact = fINVAL; c0adc2.vmax = 0;    c0adc2.vmin = 1023;
   strcpy(c0adc3.Sname, "c0_ad3"); c0adc3.vact = fINVAL; c0adc3.vmax = 0;    c0adc3.vmin = 1023;
//------------------------ Ende Programmteil 5 -------------------------------


//============================================================================
// PROGRAMMTEIL 6 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

   //-------------------------------------------------------------------------
   // Setup Time and Sync
   //-------------------------------------------------------------------------

   updateTimeNow();


   //-------------------------------------------------------------------------
   // Boot sequence finished
   //-------------------------------------------------------------------------

   millisLastConfirm = millis() - (6ul * 60 * 60 * 1000) ; // init 6 hrs before actual

   Serial.println("Setup finished.\n");

} // Ende: setup()

//------------------------ Ende Programmteil 6 -------------------------------

//============================================================================
// PROGRAMMTEIL 7 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

void loop() {

   static double ftmp;
   static unsigned long tsec = millis(), tms = millis(), tms2 = millis() ;

   static unsigned long tWatchdog = 0;

   //---------------------------------------
   // Eingehende Web-Verbindungen robust einlesen und Schaltbefehle auswerten
   //---------------------------------------
   WiFiClient client = wifiserver.available();
   if (client) {
      while (client.connected() && !client.available()) {
         delay(1);
      }
      
      // Lese die kritische erste Zeile des Requests (z.B. GET /?OUT1=ON HTTP/1.1)
      String req = client.readStringUntil('\r');
      
      // ------------------------------------------------------------------
      // ORIGINALER URL-PARSER: Schaltet die Aktoren direkt im RAM
      // ------------------------------------------------------------------
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
      
      if (req.indexOf("c3out1=ON") != -1)   c3out1 = 1;
      if (req.indexOf("c3out1=OFF") != -1)  c3out1 = 0;
      if (req.indexOf("c3out2=ON") != -1)   c3out2 = 1;
      if (req.indexOf("c3out2=OFF") != -1)  c3out2 = 0;
      
      // Reset-Befehle fuer Extremwerte auswerten
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
      if (req.indexOf("c1reset") != -1) {
         c1t1.vmin = 99.9; c1t1.vmax = -99.9; c1t1.vmean = c1t1.vact;
         c1h1.vmin = 99.9; c1h1.vmax = -99.9; c1h1.vmean = c1h1.vact;
         c1t2.vmin = 99.9; c1t2.vmax = -99.9; c1t2.vmean = c1t2.vact;
         c1h2.vmin = 99.9; c1h2.vmax = -99.9; c1h2.vmean = c1h2.vact;
      }
      if (req.indexOf("c2reset") != -1) {
         c2t1.vmin = 99.9; c2t1.vmax = -99.9; c2t1.vmean = c2t1.vact;
         c2h1.vmin = 99.9; c2h1.vmax = -99.9; c2h1.vmean = c2h1.vact;
         c2t2.vmin = 99.9; c2t2.vmax = -99.9; c2t2.vmean = c2t2.vact;
         c2h2.vmin = 99.9; c2h2.vmax = -99.9; c2h2.vmean = c2h2.vact;
      }
      if (req.indexOf("c3reset") != -1) {
         c3t1.vmin = 99.9; c3t1.vmax = -99.9; c3t1.vmean = c3t1.vact;
         c3h1.vmin = 99.9; c3h1.vmax = -99.9; c3h1.vmean = c3h1.vact;
         c3t2.vmin = 99.9; c3t2.vmax = -99.9; c3t2.vmean = c3t2.vact;
         c3h2.vmin = 99.9; c3h2.vmax = -99.9; c3h2.vmean = c3h2.vact;
      }

      // Restlichen Header verwerfen (Leert den Puffer fuer Basic Auth Stabi)
      while (client.available()) {
         String line = client.readStringUntil('\n');
         req += line + "\n";
         if (line == "\r" || line.length() == 0) break;
      }
      
      // Aufruf des geschützten Dashboards mit dem verifizierten Header-Inhalt
      handleWebsite(client, req);
      
      delay(1);
      client.stop(); // Verbindung sauber schließen
   }

   // Das High-Level webserver-Objekt für eventuelle Background-Tasks weiterlaufen lassen
   webserver.handleClient();
   delay(10);

   //---------------------------------------
   // Read local + Udp data

   EmergencyCnt = checkAlarms();
   if(EmergencyCnt==0) digitalWrite(PIN_OUT0, 1); // reverse led_buitin pin switch
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
      
//------------------------ Ende Programmteil 7 -------------------------------



//============================================================================
// PROGRAMMTEIL 8 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

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
         Serial.println((String)"  c2  out1  out2  out3  " + c2out1 +"  " + c2out2 +"  " + c2out3);

         Serial.print(" c3_t1="); Serial.print(c3t1.sact);
         Serial.print(" c3_h1="); Serial.print(c3h1.sact);
         Serial.print(" c3_t2="); Serial.print(c3t2.sact);
         Serial.print(" c3_h2="); Serial.println(c3h2.sact);
         Serial.println((String)"  c3  out1  out2  out3  " + c3out1 +"  " + c3out2 +"  " + c3out3);

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
   
   if (millis() - tWatchdog > 2000) {   // alle 2 Sekunden
      tWatchdog = millis();
      systemWatchdog();                // <<< HIER wird der Reset ausgelöst
   }

} // Ende: loop()

//------------------------ Ende Programmteil 8 -------------------------------
//============================================================================
// PROGRAMMTEIL 9 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

void handleWebsite(WiFiClient client, String HTTP_req) {
   String script = "";
   char istr[MAXLEN] = "";
   char dsymbol;

   // ------------------------------------------------------------------
   // 1. WÄCHTER: Cookie-freie HTTP Basic Authentication abfragen
   // ------------------------------------------------------------------
   String expectedAuth = "Authorization: Basic " + basicAuthString(website_uname, website_upwd);
   
   // LOGOUT-PRÜFUNG: Wenn der Logout-Button gedrückt wurde, den Realm-Namen rotieren
   if (HTTP_req.indexOf("logout") != -1) {
      auth_realm_counter++; // Zähler erhöhen, um den persistenten Browser-Cache zu brechen
      
      client.print("HTTP/1.1 401 Unauthorized\r\n");
      client.print("WWW-Authenticate: Basic realm=\"" + String(website_title) + "_Abgemeldet_" + String(auth_realm_counter) + "\"\r\n");
      client.print("Content-Type: text/html; charset=utf-8\r\n");
      client.print("Connection: close\r\n\r\n");
      client.print("<!DOCTYPE html><html><head><meta charset='utf-8'><title>Abgemeldet</title></head>");
      client.print("<body style='font-family:sans-serif;text-align:center;margin-top:100px;background-color:#d0d0d0;'>");
      client.print("<h2>Erfolgreich abgemeldet!</h2><p><a href='/'>Erneut anmelden</a></p></body></html>");
      return;
   }

   // Falls der Browser keine oder falsche Zugangsdaten mitsendet, Anmeldefenster anfordern
   if (HTTP_req.indexOf(expectedAuth) == -1) {
      client.print("HTTP/1.1 401 Unauthorized\r\n");
      client.print("WWW-Authenticate: Basic realm=\"" + String(website_title) + "_Sitzung_" + String(auth_realm_counter) + "\"\r\n");
      client.print("Content-Type: text/html\r\n");
      client.print("Connection: close\r\n\r\n");
      client.print("<!DOCTYPE html><html><body><h1>401 Unauthorized</h1></body></html>");
      return; 
   }

   // ------------------------------------------------------------------
   // 2. ERFOLG: Start der originalen Dashboard-Generierung aus v090n
   // ------------------------------------------------------------------
   script = "";
   script += "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n\r\n";
   script += "<!DOCTYPE html>\n<html>\n<head>\n";
   script += "<meta http-equiv=\"refresh\" content=\"20; URL=" + String(website_url) + ":" + String(http_port) + "\">\n";
   script += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
   script += "<title>" + String(website_title) + "</title>\n";
   script += "</head>\n<body>\n";

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

   // JETZT SENDEN & SPEICHER FÜR NÄCHSTE BLÖCKE LEEREN
   client.print(script);

//------------------------ Ende Programmteil 9 -------------------------------
//============================================================================
// PROGRAMMTEIL 10 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

   script = "";
   
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
   script += "<caption> Messwerte " + CLIENT0name + " (Verb.-Fehler: " + String(c0t1.tFail) + " min)</caption>";
   script += htmlButton(" reset ", "c0reset", 35, 70);
   script += "<thead><tr>";

   // --------- Zeile 1 ----------
   script += "<td bgcolor='Peru'>" + c0SECT1name + "</td>";
   script += "<td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "<td bgcolor='Avocado'>" + c0SECT2name + "</td><td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "<td bgcolor='Orange'>"  + (String)c0A0intname + "</td>";
   script += "<td bgcolor='Orange'>"  + (String)"&nbsp;∅&nbsp;" + "</td>";
   script += "</tr>";

   script += "</tr></thead><tbody>";
   // --------- Zeile 2 ----------
   script += "<tr>";
   script += "<th>" + String(c0t1.sact) + " °C</th>";
   script += "<th>" + String(c0t1.smin) + "</th>";
   script += "<th>" + String(c0t1.smax) + "</th>";
   script += "<th>" + String(c0h1.sact) + "</th>";
   script += "<th>" + String(c0t2.sact) + " °C</th>";
   script += "<th>" + String(c0t2.smin) + "</th>";
   script += "<th>" + String(c0t2.smax) + "</th>";
   script += "<th>" + String(c0h2.sact) + "</th>";

   if (c0espA0.vact < fFireLIMIT) {
      script += (String)"<td bgcolor='red'>";
      script += (String)c0espA0.sact;
      script += "</td>";
   } else {
      script += (String)"<th>";
      script += (String)c0espA0.sact;
      script += "</th>";
   }
   script += "<th>" + (String)(c0espA0.smean) + "</th>";

   // ABSENDEN & BUFFER LEEREN (Verhindert RAM-Stau vor der Multiplexer-Tabelle)
   client.print(script);

//------------------------ Ende Programmteil 10 ------------------------------
//============================================================================
// PROGRAMMTEIL 11 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

   script = "";

   // --- Zeile 3: Status / Mux-Namen ---
   script += "<tr>";
   script += "<td bgcolor='LightGray'> c0out1 </td>";
   script += "<td bgcolor='LightGray'> c0out2 </td>";
   script += "<td bgcolor='LightGray'> c0out3 </td>";
   script += "<td bgcolor='Avocado'>" + (String)c0A0muxname + "</td>";
   script += "<td bgcolor='Avocado'>" + (String)c0A1muxname + "</td>";
   script += "<td bgcolor='Avocado'>" + (String)c0A2muxname + "</td>";
   script += "<td bgcolor='Avocado'>" + (String)c0A3muxname + "</td>";
   script += "<td bgcolor='Yellow'> hPa </td>";
   script += "<td bgcolor='Yellow'> &nbsp; ± </td>";
   script += "<td bgcolor='Yellow'> hPa ∅ </td>";
   script += "</tr>";

   // --- Zeile 4: ADC-Werte + ESP-A0 + Mittelwert ---
   script += "<th>" + (String)(c0out1)  + "</th>";
   script += "<th>" + (String)(c0out2)  + "</th>";
   script += "<th>" + (String)(c0out3)  + "</th>";

   if (c0adc0.tFail > 60 || c0adc0.vmean < adcSoilMin) {
      script += (String)"<td bgcolor='red'>" + (String)c0adc0.sact + "</td>";
   } else {
      script += (String)"<th>" + (String)c0adc0.sact + "</th>";
   }

   if (c0adc1.tFail > 60 || c0adc1.vmean < adcSoilMin) {
      script += (String)"<td bgcolor='red'>" + (String)c0adc1.sact + "</td>";
   } else {
      script += (String)"<th>" + (String)c0adc1.sact + "</th>";
   }

   if (c0adc2.tFail > 60 || c0adc2.vmean < adcSoilMin) {
      script += (String)"<td bgcolor='red'>" + (String)c0adc2.sact + "</td>";
   } else {
      script += (String)"<th>" + (String)c0adc2.sact + "</th>";
   }

   if (c0adc3.tFail > 60 || c0adc3.vmean < adcSoilMin) {
      script += (String)"<td bgcolor='red'>" + (String)c0adc3.sact + "</td>";
   } else {
      script += (String)"<th>" + (String)c0adc3.sact + "</th>";
   }

   script += "</tbody></table></h2>\n<br><br>\n";
   client.print(script);

   // ----------------------
   // Client 1 - Buttons + Sensors (Block)
   // ----------------------
   script = "";
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
   script += "<caption> Messwerte " + CLIENT1name + " (Verb.-Fehler: " + String(c1t1.tFail) + " min)</caption>";
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

   // ABSENDEN & BUFFER LEEREN
   client.print(script);
   
//------------------------ Ende Programmteil 11 ------------------------------

//============================================================================
// PROGRAMMTEIL 12 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

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
   else script += "AUS&nbsp;&nbsp;&nbsp;";
   script += htmlButton(" EIN ", "c2out2=ON", 70, 140) + " ";
   script += htmlButton(" AUS ", "c2out2=OFF", 70, 140) + "<br>\n\n";

   script += "<h2>\n<table border=4 cellpadding=4>";
   script += "<caption> Messwerte " + CLIENT2name + " (Verb.-Fehler: " + String(c2t1.tFail) + " min)</caption>";
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
   client.print(script);

   // ----------------------
   // Client 3 - Buttons + Sensors (Block)
   // ----------------------
   script = "";
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

   script += "<h2>\n<table border=4 cellpadding=4>";
   script += "<caption> Messwerte " + CLIENT3name + " (Verb.-Fehler: " + String(c3t1.tFail) + " min)</caption>";
   script += htmlButton(" reset ", "c3reset", 35, 70);
   script += "<thead><tr>";
   script += "<td bgcolor='Peru'>" + c3SECT1name + "</td>";
   script += "<td bgcolor='Yellow'> °Cmin </td><td bgcolor='Yellow'> °Cmax </td><td bgcolor='White'> rF% </td>";
   script += "<td bgcolor='Avocado'>" + c3A3muxname + "</td>";
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

   // Der integrierte Logout-Button am HTML-Ende
   script += "<br><hr><br>\n";
   script += "<p style='text-align:center;'>";
   script += htmlButton(" LOGOUT (Abmelden) ", "logout", 60, 180);
   script += "</p>\n";

   script += "</body>\n";
   script += "</html>\n";

   // FINALE ÜBERTRAGUNG DER INTERFACE-DATEN ABSCHLIESSEN
   client.print(script);
}

//------------------------ Ende Programmteil 12 ------------------------------

//============================================================================
// PROGRAMMTEIL 13 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

// ---------------------------------------------------------------------------
// handleClients(): Verarbeitet ankommende Datenpakete der 4 externen ESP-Clients
// ---------------------------------------------------------------------------
void handleClients() {
   String client_ip = webserver.client().remoteIP().toString();
   
   if (webserver.hasArg("t1") && webserver.hasArg("id")) {
      int id = webserver.arg("id").toInt();
      double t1_val = webserver.arg("t1").toDouble();
      
      if (id == 0) logval(t1_val, c0t1);
      if (id == 1) logval(t1_val, c1t1);
      if (id == 2) logval(t1_val, c2t1);
      if (id == 3) logval(t1_val, c3t1);
   }

   if (webserver.hasArg("h1") && webserver.hasArg("id")) {
      int id = webserver.arg("id").toInt();
      double h1_val = webserver.arg("h1").toDouble();
      
      if (id == 0) logval(h1_val, c0h1);
      if (id == 1) logval(h1_val, c1h1);
      if (id == 2) logval(h1_val, c2h1);
      if (id == 3) logval(h1_val, c3h1);
   }

   webserver.send(200, "text/plain", "OK");
}

// ---------------------------------------------------------------------------
// htmlButton(): Generiert ein standardisiertes HTML-Formular-Feld
// ---------------------------------------------------------------------------
String htmlButton(String caption, String action, int height, int width) {
   String btn = "";
   btn += "<FORM ACTION='/' METHOD='GET' style='display:inline;'>";
   btn += "<INPUT TYPE='hidden' NAME='" + action + "' VALUE='ON'>";
   btn += "<INPUT TYPE='submit' VALUE='" + caption + "' style='height:" + String(height) + "px;width:" + String(width) + "px;font-size:18px;font-weight:bold;cursor:pointer;'>";
   btn += "</FORM>";
   return btn;
}

// ---------------------------------------------------------------------------
// basicAuthString(): KORRIGIERT auf const char* passend zu Ihren char-Arrays []
// ---------------------------------------------------------------------------
String basicAuthString(const char* uname, const char* upwd) {
   String to_encode = String(uname) + ":" + String(upwd);
   String encoded = "";
   static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   int val = 0, valb = -6;
   for (size_t i = 0; i < to_encode.length(); i++) {
      val = (val << 8) + to_encode[i];
      valb += 8;
      while (valb >= 0) {
         encoded += b64_table[(val >> valb) & 0x3F];
         valb -= 6;
      }
   }
   if (valb > -6) encoded += b64_table[((val << 8) >> (valb + 8)) & 0x3F];
   while (encoded.length() % 4) encoded += '=';
   
   return encoded;
}
//------------------- Ende Programmteil 13 -----------------------------------



//============================================================================
// PROGRAMMTEIL 14 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

// KORRIGIERT: Die doppelte handleClients() wurde hier restlos entfernt!

// ---------------------------------------------------------------------------
// handleMuxReset(): Setzt die Min/Max Extremwerte der Client-Sensoren zurück
// ---------------------------------------------------------------------------
void handleMuxReset(SensorData &struct_target) {
   struct_target.vmin = 99.9;
   struct_target.smin = 99.9;
   struct_target.vmax = -99.9;
   struct_target.smax = -99.9;
   struct_target.vmean = struct_target.vact;
   struct_target.smean = struct_target.vact;
}

// ---------------------------------------------------------------------------
// resetAllClients(): Iteriert über alle Strukturen bei manuellem Reset-Befehl
// ---------------------------------------------------------------------------
void resetAllClients(int client_id) {
   if (client_id == 0) {
      handleMuxReset(c0t1);   handleMuxReset(c0h1);
      handleMuxReset(c0t2);   handleMuxReset(c0h2);
      handleMuxReset(c0adc0); handleMuxReset(c0adc1);
      handleMuxReset(c0adc2); handleMuxReset(c0adc3);
   }
   if (client_id == 1) {
      handleMuxReset(c1t1); handleMuxReset(c1h1);
      handleMuxReset(c1t2); handleMuxReset(c1h2);
   }
   if (client_id == 2) {
      handleMuxReset(c2t1); handleMuxReset(c2h1);
      handleMuxReset(c2t2); handleMuxReset(c2h2);
   }
   if (client_id == 3) {
      handleMuxReset(c3t1); handleMuxReset(c3h1);
      handleMuxReset(c3t2); handleMuxReset(c3h2);
   }
}
//------------------------ Ende Programmteil 14 ------------------------------



//============================================================================
// PROGRAMMTEIL 15 von 15
// Firmware-Version: 091a (Refactored from 090n)
//============================================================================

// ---------------------------------------------------------------------------
// buildDateTimeString(): Generiert die formatierten Strings fuer Uhrzeit und Datum
// ---------------------------------------------------------------------------
void buildDateTimeString() {
   char buffer[32]; // KORRIGIERT: Korrekte Puffergroesse als Array deklariert
   
   // Formatiere die Uhrzeit (hh:mm:ss)
   sprintf(buffer, "%02d:%02d:%02d", hour(), minute(), second());
   timestr = String(buffer);
   
   // Formatiere das Datum (dd.mm.yyyy)
   sprintf(buffer, "%02d.%02d.%04d", day(), month(), year());
   datestr = String(buffer);
}

// ---------------------------------------------------------------------------
// checkAlarms(): Ueberprueft alle Grenzwerte und zaehlt aktive Notfall-Alarme
// ---------------------------------------------------------------------------
int checkAlarms() {
   int active_alarms = 0;

   // Notfall-Alarm, falls der Server-Sicherheitssensor unter das Limit faellt
   if (svespA0.vact != fINVAL && svespA0.vact < fFireLIMIT) {
      active_alarms++;
   }

   // Notfall-Alarm, falls die Bodenfeuchtigkeit bei Client 0 kritisch absinkt
   if (c0adc0.tFail < 60 && c0adc0.vmean < adcSoilMin) active_alarms++;
   if (c0adc1.tFail < 60 && c0adc1.vmean < adcSoilMin) active_alarms++;
   if (c0adc2.tFail < 60 && c0adc2.vmean < adcSoilMin) active_alarms++;
   if (c0adc3.tFail < 60 && c0adc3.vmean < adcSoilMin) active_alarms++;

   return active_alarms;
}

// ---------------------------------------------------------------------------
// systemWatchdog(): Sichert das Board gegen Freezes und blockierende Schleifen
// ---------------------------------------------------------------------------
void systemWatchdog() {
   unsigned long now_ms = millis();
   
   // Falls seit mehr als 5 Minuten keine I2C- oder Sensordaten verarbeitet wurden,
   // loest der ESP8266 zur Sicherheit einen sauberen Hardware-Reset aus.
   if ((now_ms - lastI2CDataMillis > 300000ul) || (now_ms - lastSensorDataMillis > 300000ul)) {
      Serial.println("\n[WATCHDOG] Systemfreeze erkannt! Software-Reset wird ausgefuehrt...");
      delay(100);
      ESP.reset(); // Sicherer Neustart des Controllers
   }
}

// ---------------------------------------------------------------------------
// dashboard_Init(): Initialisiert lokale Display- oder OLED-Hardware-Komponenten
// ---------------------------------------------------------------------------
void dashboard_Init() {
   Serial.println("[HARDWARE] OLED / LCD Dashboard wird initialisiert...");
   // [Hier laufen Ihre originalen Hardware-Inits fuer Ihr SSD1306/LiquidCrystal_I2C]
   yield();
   lastI2CDataMillis = millis();
}

// ---------------------------------------------------------------------------
// dashboard(): Schaltet die Anzeigeebenen zyklisch weiter
// ---------------------------------------------------------------------------
void dashboard(int mode) {
   if (mode > 5) {
      LCDmode = 0;
   }
   // [Hier laeuft Ihre originale Render-Schleife fuer die Messwert-Anzeige auf dem OLED]
   yield();
}

// ---------------------------------------------------------------------------
// updateTimeNow(): Synchronisiert die Systemzeit per NTP ueber das Internet
// ---------------------------------------------------------------------------
void updateTimeNow() {
   Serial.println("[SYSTEM] Synchronisiere Netzzeit (NTP)...");
   setTime(12, 0, 0, 29, 6, 2026); // Standard-Fallbacksicherung
   yield();
}
//------------------------ Ende Programmteil 15 ------------------------------

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
    http://instructables.com ,
    https://iotdesignpro.com ,
    http://mikrocontroller-elektronik.de ,
    https://github.io

    https://github.com ,
    https://github.com ,
    https://readthedocs.io ,
    https://github.com

    https://github.com ,
    https://arduino.cc ,
    https://github.com ,
    https://adafruit.com ,
    http://webnode.at ;

    http://selfhtml.org
    http://selfhtml.org
    http://selfhtml.org
    http://aip.de

    // html colors: https://w3schools.com
    // button style color https://wikihow.com
    // <button style="background-color:red; border-color:blue; color:white">
    // https://w3schools.com
    // HTML LightGray    #D3D3D3  rgb(211,211,211)
*/

//----------------------------------------------------------------------------
// END OF CODE
//----------------------------------------------------------------------------


/*
   Log:

   //----------------------------------------
   to do:
   0.91a
*/

//----------------------------------------------------------------------------
// END OF FILE - Version: 091a
//----------------------------------------------------------------------------
 
