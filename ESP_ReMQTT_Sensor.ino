#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>


#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>

#include <PubSubClient.h>

#include <AsyncElegantOTA.h>
#include <elegantWebpage.h>
#include <Hash.h>

//----------------------------------------------------------------
// SKETCH BEGIN
AsyncWebServer server(80);

const char* ssid = "MikroTik-220CF6";
const char* password = "Sanifar123!";
const char * hostName = "esp-async";
const char* http_username = "admin";
const char* http_password = "admin";

char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Sonnabend"};

long ntpTM = 0;
long ntpTO = 5000;


// Define NTP Client to get time
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);


String processor(const String& var) {
      if(var == "HELLO_FROM_TEMPLATE") return String(ESP.getFreeHeap());
      if(var == "WIFIMODE") return String(WiFi.getMode());
      return String();
    }

    
void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(hostName);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(ssid, password);
  }

  //Send OTA events to the browser
 

  MDNS.addService("http","tcp",80);

  SPIFFS.begin();
  
  
  
  server.addHandler(new SPIFFSEditor(http_username,http_password));
  
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setTemplateProcessor(processor);

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
    //Serial.println(timeClient.getFormattedTime());
  }
  
  
}
