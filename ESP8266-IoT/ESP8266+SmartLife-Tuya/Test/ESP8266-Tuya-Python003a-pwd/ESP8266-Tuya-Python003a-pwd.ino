// Test: auslesen von sämtlichen 3rd party Sensoren im bestehenen Tuya Heimnetzwerk;
// Projekt ist bei Tuya registriert
// Fehler: IDs korrekt, aber timestamp scheitert offenbar

/*
// === WLAN Daten ===
const char* ssid = "*******";
const char* password = "*******";

// === Tuya Cloud Daten ===
const char* tuyaEndpoint = "https://openapi.tuyaeu.com"; // EU Endpoint
const char* accessId  = "*******";
const char* accessKey = "*******"; 

*/


/*********************************************************************
 *  ESP8266 → Tuya Cloud Client (ver 002.d ==> 003a – fix13)
 *  - Token Request in UTC-Sekunden (Tuya)
 *  - OLED zeigt lokale Berlin-Zeit (CET/CEST)
 *  - Sommerzeit automatisch berechnet
 *  - HMAC-SHA256 via br_hmac()
 *  - WLAN + NTP + Debug-Ausgaben
 *********************************************************************/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <BearSSLHelpers.h>
#include <string.h>
#include <time.h>


// === WLAN Daten ===
const char* ssid = "*******";
const char* password = "*******";

// === Tuya Cloud Daten ===
const char* tuyaEndpoint = "https://openapi.tuyaeu.com"; // EU Endpoint
const char* accessId  = "*******";
const char* accessKey = "*******"; 




// === OLED Setup ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === Globale Variablen ===
std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
HTTPClient https;

String accessToken = "";
unsigned long tokenExpiry = 0;

// === NTP & Zeitverwaltung ===
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC, alle 60s

// === Hilfsfunktion: Unix-Zeit in Sekunden für Tuya Token ===
String getUnixTimeSecondsUTC() {
  return String(timeClient.getEpochTime()); // UTC Sekunden
}

// === HMAC-SHA256 Funktion ===
void hmac_sha256(const char* key, const char* msg, uint8_t* out32) {
  br_hmac_key_context kc;
  br_hmac_context hc;

  br_hmac_key_init(&kc, &br_sha256_vtable, (const uint8_t*)key, strlen(key));
  br_hmac_init(&hc, &kc, 0);
  br_hmac_update(&hc, (const uint8_t*)msg, strlen(msg));
  br_hmac_out(&hc, out32);
}

// === Sommerzeit Berechnung für Berlin ===
int getBerlinOffset(int utcSec) {
  // Berechne Jahr, Monat, Tag
  time_t t = utcSec;
  struct tm* tm_utc = gmtime(&t);
  int year = tm_utc->tm_year + 1900;
  int month = tm_utc->tm_mon + 1;
  int day = tm_utc->tm_mday;
  int wday = tm_utc->tm_wday; // Sonntag = 0
  int hour = tm_utc->tm_hour;

  // Letzter Sonntag im März
  int lastSundayMarch = 31;
  for (int d = 31; d >= 25; d--) {
    t = timeClient.getEpochTime() + (d - day)*86400;
    struct tm* tmp = gmtime(&t);
    if (tmp->tm_mon + 1 == 3 && tmp->tm_wday == 0) { lastSundayMarch = d; break; }
  }
  // Letzter Sonntag im Oktober
  int lastSundayOct = 31;
  for (int d = 31; d >= 25; d--) {
    t = timeClient.getEpochTime() + (d - day)*86400;
    struct tm* tmp = gmtime(&t);
    if (tmp->tm_mon + 1 == 10 && tmp->tm_wday == 0) { lastSundayOct = d; break; }
  }

  int offset = 1; // Standard +1h
  if ((month > 3 && month < 10) ||
      (month == 3 && day >= lastSundayMarch) ||
      (month == 10 && day < lastSundayOct)) {
    offset = 2; // Sommerzeit +2h
  }

  return offset;
}

// === Konvertiere UTC → Berlin Zeit String HH:MM:SS ===
String getBerlinTimeStr() {
  unsigned long utc = timeClient.getEpochTime();
  int offsetH = getBerlinOffset(utc);
  time_t t = utc + offsetH*3600;
  struct tm* tm_local = gmtime(&t);
  char buf[16];
  sprintf(buf, "%02d:%02d:%02d", tm_local->tm_hour, tm_local->tm_min, tm_local->tm_sec);
  return String(buf);
}

// === WLAN verbinden ===
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Verbinde mit WLAN %s ...\n", ssid);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Verbinde WLAN...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWLAN verbunden!");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WLAN verbunden!");
  display.println(WiFi.localIP());
  display.display();
}

// === Zeit synchronisieren ===
void syncTime() {
  Serial.print("Synchronisiere Zeit...");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Synchronisiere Zeit...");
  display.display();

  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
    Serial.print(".");
    delay(500);
  }
  Serial.println(" OK!");
  display.println("OK!");
  display.display();

  delay(1000); // kurze Pause für stabile Zeit
}

// === Tuya Token abrufen ===
void getTuyaToken() {
  String url = String(tuyaEndpoint) + "/v1.0/token?grant_type=1";
  String t = getUnixTimeSecondsUTC();           // UTC Sekunden
  String signInput = String(accessId) + t;     // client_id + UTC Sekunden

  uint8_t hmacResult[32];
  hmac_sha256(accessKey, signInput.c_str(), hmacResult);

  char signHex[65];
  for (int i = 0; i < 32; i++) sprintf(signHex + i*2, "%02x", hmacResult[i]);
  signHex[64] = 0;

  // Debug
  Serial.println("-----DEBUG Tuya Sign-----");
  Serial.println("SignInput: " + signInput);
  Serial.println("SignHex:   " + String(signHex));
  Serial.println("------------------------");

  client->setInsecure();
  https.begin(*client, url);
  https.addHeader("client_id", accessId);
  https.addHeader("sign", signHex);
  https.addHeader("t", t);
  https.addHeader("sign_method", "HMAC-SHA256");

  int httpCode = https.GET();
  Serial.printf("Token HTTP Code: %d\n", httpCode);

  if (httpCode > 0) {
    String payload = https.getString();
    Serial.println("Token Antwort:");
    Serial.println(payload);

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error && doc["success"] == true) {
      accessToken = doc["result"]["access_token"].as<String>();
      int expire = doc["result"]["expire_time"];
      tokenExpiry = millis() + (expire * 1000UL);

      Serial.println("Token erfolgreich erhalten:");
      Serial.println(accessToken);

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Token OK!");
      display.println("Berlin Zeit: " + getBerlinTimeStr());
      display.display();
    } else {
      Serial.println("Fehler beim Token holen oder JSON-Parsing");
      display.clearDisplay();
      display.println("Token FEHLER!");
      display.display();
    }
  } else {
    Serial.println("Token Request fehlgeschlagen!");
  }

  https.end();
}

// === SETUP ===
void setup() {
  Serial.begin(115200);
  delay(500);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED nicht gefunden!");
  }
  display.clearDisplay();

  connectWiFi();
  syncTime();
  getTuyaToken();
}

// === LOOP ===
void loop() {
  // Token prüfen / ggf. erneuern
  if (millis() > tokenExpiry - 30000UL || accessToken == "") {
    Serial.println("Token abgelaufen – hole neu...");
    getTuyaToken();
  }

  // OLED aktuelle Berlin-Zeit anzeigen
  display.setCursor(0, 10);
  display.println("Berlin Zeit: " + getBerlinTimeStr());
  display.display();

  delay(15000); // zyklische Kontrolle
}
