// Create A Simple ESP32 Web Server In Arduino IDE
// https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <WiFiServer.h>


/*Put your SSID & Password*/
const char* ssid = "WLAN-3YA7LD";  // Enter SSID here
const char* password = "18658446694458594657";  //Enter Password here
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



#define wwwport 8008
#define lanport 8081
WebServer  webserver(wwwport);   // router http port www
WiFiServer wifiserver(lanport);  // router wifi port lan 



// allows you to set the realm of authentication Default:"Login Required"
const char* www_realm = "Custom Auth Realm";
// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";


uint8_t LED1pin = LED_BUILTIN;
bool LED1status = LOW;

uint8_t LED2pin = 12;
bool LED2status = LOW;



//----------------------------------------------------------
void handleNotFound() {
 
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
 
}





//----------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);
  WiFi.config(this_ip, gateway, subnet);   // static IP  

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());


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
    handle_OnConnect();
  });
  
 /*
  webserver.on("/inline", []() {
    webserver.send(200, "text/plain", "this works as well");
  });
 */ 
 
  webserver.on("/led1on", handle_led1on);
  webserver.on("/led1off", handle_led1off);
  webserver.on("/led2on", handle_led2on);
  webserver.on("/led2off", handle_led2off);
  webserver.onNotFound(handle_NotFound);

  webserver.begin();
  
  Serial.println("HTTP webserver started");
}



//----------------------------------------------------------
void loop() {
  ArduinoOTA.handle();
  
  webserver.handleClient();
  if(LED1status)
  {digitalWrite(LED1pin, HIGH);}
  else
  {digitalWrite(LED1pin, LOW);}
  
  if(LED2status)
  {digitalWrite(LED2pin, HIGH);}
  else
  {digitalWrite(LED2pin, LOW);}
}



//----------------------------------------------------------
void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  Serial.println("GPIO13 Status: OFF | GPIO12 Status: OFF");
  webserver.send(200, "text/html", SendHTML(LED1status,LED2status)); 
}

//----------------------------------------------------------
void handle_led1on() {
  LED1status = HIGH;
  Serial.println("GPIO13 Status: ON");
  webserver.send(200, "text/html", SendHTML(true,LED2status)); 
}

//----------------------------------------------------------
void handle_led1off() {
  LED1status = LOW;
  Serial.println("GPIO13 Status: OFF");
  webserver.send(200, "text/html", SendHTML(false,LED2status)); 
}

//----------------------------------------------------------
void handle_led2on() {
  LED2status = HIGH;
  Serial.println("GPIO12 Status: ON");
  webserver.send(200, "text/html", SendHTML(LED1status,true)); 
}

//----------------------------------------------------------
void handle_led2off() {
  LED2status = LOW;
  Serial.println("GPIO12 Status: OFF");
  webserver.send(200, "text/html", SendHTML(LED1status,false)); 
}

//----------------------------------------------------------
void handle_NotFound(){
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
  //webserver.send(404, "text/plain", "Not found");
}

//----------------------------------------------------------
String SendHTML(uint8_t led1stat,uint8_t led2stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
    ptr +="<h3>Using Station(STA) Mode</h3>\n";
  
   if(led1stat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";}
  else
  {ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";}

  if(led2stat)
  {ptr +="<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";}
  else
  {ptr +="<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
