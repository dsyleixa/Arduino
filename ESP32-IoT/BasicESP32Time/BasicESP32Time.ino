// Basic example sketch NTPClient.h lib
// https://github.com/arduino-libraries/NTPClient

#include <NTPClient.h>
//#include <ESP8266WiFi.h> // for ESP8266
#include <WiFi.h> // for ESP32 & WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>

const char* ssid = "WLAN-3YA7LD";
const char* password = "18658446694458594657";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup(){
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
}

void loop() {
  timeClient.update();

  Serial.println(timeClient.getFormattedTime());

  delay(1000);
}
