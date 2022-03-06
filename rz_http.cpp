// 1.0.0

#include "Arduino.h"
#include "rz_http.h"


#include <HTTPClient.h>

HTTPClient http;

RZ_HTTP::RZ_HTTP() {
  String _payload = "";
}

std::string RZ_HTTP::checkFirmware() {
   Serial.println("- checking Firmware");
   http.begin("https://raw.githubusercontent.com/Zheng-Bote/ESP32_BME680_Webserver_MQTT-Client/main/version");
   int httpCode = http.GET();
   if (httpCode > 0) {
    _payload = http.getString();
    _payload.trim();
    Serial.print("-- Github Version: "); Serial.println(_payload);
  }
  http.end();
  return _payload.c_str();
}

std::string RZ_HTTP::getUpdVersion() {
  return _payload.c_str();
}
