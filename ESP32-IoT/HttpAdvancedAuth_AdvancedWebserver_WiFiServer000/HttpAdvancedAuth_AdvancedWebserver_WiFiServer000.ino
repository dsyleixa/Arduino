/*
  HTTP Advanced Authentication example
  plus AdvancedWebserver example, unified
  Created Mar 16, 2017 by Ahmed El-Sharnoby.
  This example code is in the public domain.
  Reworked by dsyleixa 2019
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <WiFiServer.h>

#define wwwport 8008
#define lanport 8081

WebServer  webserver(wwwport);   // router http port www
WiFiServer wifiserver(lanport);  // router wifi port lan 


char ssid[64]     = "WLAN-3YA7LD";
char password[64] = "18658446694458594657";
/*
char* ssid[64]     = "****ssid****";
char* password[64] = "****pswd****";
*/
char www_username[64] = "admin";
char www_password[64] = "esp32";


#define     this_ip_4    202      // <<< static webserver  ip (4th byte)

IPAddress   this_ip(192, 168, 2, this_ip_4); // <<< static local IP of this ESP-webserver
IPAddress   gateway(192, 168, 2, 1);         // <<< LAN Gateway IP
IPAddress   subnet(255, 255, 255, 0);        // <<< LAN Subnet Mask
char url[128] = "";


// allows you to set the realm of authentication Default:"Login Required"
const char* www_realm = "Custom Auth Realm";
// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";



//----------------------------------------------------------
// GPIOs
char led = LED_BUILTIN;




//----------------------------------------------------------
void handleRoot() {
  
  char temp[255];  
  String script;
  
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  // time-cstr
  sprintf(temp,"%02d:%02d:%02d", hr, min%60, sec%60);
  
  digitalWrite(led, HIGH);

  script=
     (String)"<html>" +
     "<head>" +
       "<meta http-equiv='refresh' content='5'; charset=utf-8; URL="+url+"/>" +
       "<title>ESP32 Demo</title>" +
       "<style>" +
         "body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }" +       
         "</style>" +
     "</head>" +
     "<body>" +
       "<h1>Hello from ESP32 </h1>" + url + (String)" °!°" + // debug: ° <> utf-8 test!
       "<p>Uptime:" + temp + "</p>" +
       "<img src=\"/test.svg\" />" +
     "</body>" +
     "</html>"
     ;
  script += (String)hr + min%60 + sec%60 ;  
  
  webserver.send(200, "text/html", script);
  
  digitalWrite(led, LOW);
}


//----------------------------------------------------------
void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += (webserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";

  for (uint8_t i = 0; i < webserver.args(); i++) {
    message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
  }

  webserver.send(404, "text/plain", message);
  digitalWrite(led, 0);
}


//----------------------------------------------------------
void setup() {
  sprintf(url, "%s:%d/","http://ford.sytes.net:",wwwport); 
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(this_ip, gateway, subnet);   // static IP   
  
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
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

    handleRoot();
  });
  
  webserver.on("/test.svg", drawGraph);
  webserver.on("/inline", []() {
    webserver.send(200, "text/plain", "this works as well");
  });
  webserver.onNotFound(handleNotFound);
  webserver.begin();
  
  Serial.println("HTTP webserver started");

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.print(wwwport);
  Serial.println("/ in your browser to see it working");
}


//----------------------------------------------------------
void loop() {
  ArduinoOTA.handle();
  webserver.handleClient();
}


//----------------------------------------------------------
void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  webserver.send(200, "image/svg+xml", out);
}

//----------------------------------------------------------
// EOF
