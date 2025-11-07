// ============================================================
// ESP8266 Tuya Cloud Token Request + OLED & Watch
// Version: 003.d-fix3 (alias 003.d-fixOLED-watch)
// ============================================================

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Crypto.h>
#include <SHA256.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_GFX.h>
#include <ESP_SSD1306.h>

// === OLED Setup ===
#define OLED_RESET -1
ESP_SSD1306 display(OLED_RESET);

// --- WLAN-Konfiguration ---
const char* ssid     = "xxxxxxxxxxxxxxxxxxxx";
const char* password = "xxxxxxxxxxxxxxxxxxxx";

// --- Tuya Cloud-Konfiguration ---
const char* regionUrl  = "https://openapi.tuyaeu.com";
const char* accessId   = "xxxxxxxxxxxxxxxxxxxx";
const char* accessKey  = "xxxxxxxxxxxxxxxxxxxx";

// --- Zeit & Offset ---
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60 * 60 * 1000);
long long timeOffsetMs = 0;

// --- Globale Variablen ---
String lastWanIP = "";
unsigned long lastTokenTime = 0;
String lastToken = "";

// ------------------------------------------------------------
// Hilfsfunktionen (Crypto, Zeit, WAN-IP)
// ------------------------------------------------------------
void hmacSHA256(const char* key, const char* msg, uint8_t* outHash) {
  SHA256 sha;
  sha.resetHMAC((const uint8_t*)key, strlen(key));
  sha.update((const uint8_t*)msg, strlen(msg));
  sha.finalizeHMAC((const uint8_t*)key, strlen(key), outHash, 32);
}

String toHex(const uint8_t* data, size_t len) {
  String hex = "";
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 0x10) hex += "0";
    hex += String(data[i], HEX);
  }
  hex.toUpperCase();
  return hex;
}

String randomHex32() {
  uint8_t r[16];
  for (int i = 0; i < 16; i++) r[i] = os_random() & 0xFF;
  return toHex(r, 16);
}

unsigned long long getUTCMillis() {
  timeClient.forceUpdate();
  unsigned long epoch = timeClient.getEpochTime();
  unsigned long long ms = (unsigned long long)epoch * 1000ULL;
  return ms + timeOffsetMs;
}

String getWanIP() {
  WiFiClient client;
  HTTPClient http;
  String wanIP = "";
  if (http.begin(client, "http://api.ipify.org")) {
    int code = http.GET();
    if (code == 200) wanIP = http.getString();
    http.end();
  }
  return wanIP;
}

// ------------------------------------------------------------
// Signaturerstellung (Tuya Spec 2024)
// accessId + t + nonce + method + "\n" + bodyHash + "\n\n" + pathWithQuery
// ------------------------------------------------------------
String createSign(String clientId, String t, String nonce, String key, String pathWithQuery) {
  String method = "GET";
  String bodyHash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"; // leerer Body
  String signInput = clientId + t + nonce + method + "\n" + bodyHash + "\n\n" + pathWithQuery;

  uint8_t hash[32];
  hmacSHA256(key.c_str(), signInput.c_str(), hash);
  String sign = toHex(hash, 32);

  Serial.println("SignInput:");
  Serial.println(signInput);
  Serial.println("Signatur:");
  Serial.println(sign);
  return sign;
}

// ------------------------------------------------------------
// Token-Anforderung
// ------------------------------------------------------------
bool requestTuyaToken() {
  unsigned long long ts = getUTCMillis();
  char tsBuf[32];
  sprintf(tsBuf, "%llu", ts);
  String tStr = String(tsBuf);
  String nonce = randomHex32();

  String pathWithQuery = "/v1.0/token?grant_type=1";
  String sign = createSign(accessId, tStr, nonce, accessKey, pathWithQuery);

  Serial.println("\n==== Tuya Token Request ====");
  Serial.printf("UTC (ms): %s  Offset: %lld\n", tsBuf, timeOffsetMs);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String url = String(regionUrl) + pathWithQuery;
  if (!https.begin(client, url)) {
    Serial.println("HTTPS-Start fehlgeschlagen!");
    return false;
  }

  https.addHeader("client_id", accessId);
  https.addHeader("sign", sign);
  https.addHeader("t", tStr);
  https.addHeader("nonce", nonce);
  https.addHeader("sign_method", "HMAC-SHA256");

  int httpCode = https.GET();
  Serial.printf("HTTP Status: %d\n", httpCode);

  String payload = https.getString();
  https.end();
  Serial.println("Antwort:");
  Serial.println(payload);

  int idx = payload.indexOf("\"t\":");
  if (idx > 0 && payload.length() >= idx + 17) {
    String tServerStr = payload.substring(idx + 4, idx + 17);
    unsigned long long serverT = strtoull(tServerStr.c_str(), nullptr, 10);
    long long diff = (long long)serverT - (long long)ts;
    if (llabs(diff) > 50) {
      timeOffsetMs += diff;
      Serial.printf("⚙️ Offset angepasst um %lld ms\n", diff);
    }
  }

  bool success = payload.indexOf("\"success\":true") > 0;
  if (success) {
    Serial.println("✅ Token erfolgreich empfangen!");
    int tokStart = payload.indexOf("\"access_token\":\"") + 16;
    int tokEnd = payload.indexOf("\"", tokStart);
    lastToken = payload.substring(tokStart, tokEnd);
  } else {
    Serial.println("❌ Tokenfehler – siehe Log!");
  }
  return success;
}

// ------------------------------------------------------------
// OLED Anzeige aktualisieren
// ------------------------------------------------------------
void showStatus(String line1, String line2 = "", String line3 = "", String line4 = "") {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(line1);
  if (line2 != "") display.println(line2);
  if (line3 != "") display.println(line3);
  if (line4 != "") display.println(line4);
  display.display();
}

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== ESP8266 Tuya Cloud Token Request 003.d-fix3 ===");

  display.begin(SSD1306_SWITCHCAPVCC);
  display.setRotation(2);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  showStatus("Starte...", "", "", "");

  WiFi.begin(ssid, password);
  showStatus("Verbinde WLAN...");
  Serial.print("Verbinde mit WLAN");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.printf("\nWLAN verbunden! IP: %s\n", WiFi.localIP().toString().c_str());
  showStatus("WLAN OK", WiFi.localIP().toString());

  timeClient.begin();
  Serial.print("Warte auf NTP...");
  while (!timeClient.update()) { delay(500); Serial.print("."); }
  Serial.println("\nNTP-Sync OK.");
  showStatus("Zeit OK", "Hole WAN-IP...");

  lastWanIP = getWanIP();
  showStatus("Zeit OK", "WAN-IP:", lastWanIP);
  Serial.println("Aktuelle WAN-IP: " + lastWanIP);

  bool ok = requestTuyaToken();
  showStatus("Token:", ok ? "OK" : "Fehler");
  lastTokenTime = millis();
}

// ------------------------------------------------------------
// Loop: Token-Refresh + IP-Watch
// ------------------------------------------------------------
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    showStatus("⚠️ WLAN getrennt!");
    delay(2000);
    return;
  }

  String ipNow = getWanIP();
  if (ipNow != "" && ipNow != lastWanIP) {
    Serial.println("⚠️ WAN-IP geändert!");
    lastWanIP = ipNow;
    showStatus("⚠️ WAN-IP geändert!", ipNow);
  }

  if (millis() - lastTokenTime > 3600000UL) {
    Serial.println("🔁 Token-Refresh...");
    bool ok = requestTuyaToken();
    showStatus("Token-Refresh", ok ? "OK" : "Fehler");
    lastTokenTime = millis();
  }

  delay(15000);
}



/*
serMon log:

...........
WLAN verbunden! IP: 192.168.2.114
Warte auf NTP...
NTP-Sync OK.
Aktuelle WAN-IP: 79.213.11.94
SignInput:
g5dd7pq3dn8ggqava4r5176252257800090C63861FB912B7615396BF42526352CGET
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855

/v1.0/token?grant_type=1
Signatur:
940FD67659038F93DF84B68D01E9687F95A878B6D7E336E0CB0CEAF9650FBC97

==== Tuya Token Request ====
UTC (ms): 1762522578000  Offset: 0
HTTP Status: 200
Antwort:
{"result":{"access_token":"3c59b474199810245c7dabf79d2eea33","expire_time":6051,"refresh_token":"d880cc41b03050a99f827e6a77b8ded2","uid":"bay17621120861336JRd"},"success":true,"t":1762522579418,"tid":"be0b3e90bbde11f0805d96c8b66247b7"}
⚙️ Offset angepasst um 1418 ms
✅ Token erfolgreich empfangen!

*/
