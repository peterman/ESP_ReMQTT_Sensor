String processor(const String& var) {
      if(var == "HELLO_FROM_TEMPLATE") return String(ESP.getFreeHeap());
      if(var == "WIFIMODE") return String(WiFi.getMode());
      return String();
    }
    
