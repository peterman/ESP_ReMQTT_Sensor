#include <FS.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>


#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>

#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <AsyncElegantOTA.h>
#include <elegantWebpage.h>
#include <Hash.h>



//----------------------------------------------------------------
struct Config {
  char ssid[20]       = "MikroTik-220CF6";
  char wlankey[20]    = "Sanifar123!";
  char hostName[20]   = "esp-async";
  char http_user[10]  = "admin";
  char http_pw[10]    = "admin";
  char www_user[10]   = "admin";
  char www_pw[10]     = "admin";
  int port;
};

const char *filename = "/settings.json";  // <- SD library uses 8.3 filenames
Config config;
// ---------------------------- SKETCH BEGIN ---------------------
AsyncWebServer server(80);

char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Sonnabend"};

long ntpTM = 0;
long ntpTO = 5000;


#include "webserver.h"
#include "settings.h"

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);




    
void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  SPIFFS.begin();
  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  loadConfiguration(filename, config);
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(config.hostName);
  WiFi.begin(config.ssid, config.wlankey);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(config.ssid, config.wlankey);
  }

  //Send OTA events to the browser
 

  MDNS.addService("http","tcp",80);

  
  
  
  
  server.addHandler(new SPIFFSEditor(config.http_user,config.http_pw));
  
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setTemplateProcessor(processor);
  
  // Serve a file with no cache so every tile It's downloaded
  server.serveStatic("/settings.json", SPIFFS, "/settings.json", "no-cache, no-store, must-revalidate");

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
  // NTP-Client start
  timeClient.begin();
  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);    
  server.begin();
}

void loop(){
  AsyncElegantOTA.loop();
  if (millis() > ntpTO + ntpTM ) {
    ntpTM = millis();
    timeClient.update();

    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(", ");
    Serial.print(timeClient.getHours());
    Serial.print(":");
    Serial.print(timeClient.getMinutes());
    Serial.print(":");
    Serial.println(timeClient.getSeconds());
    Serial.println(timeClient.getFormattedTime());
  }
  
  
}
