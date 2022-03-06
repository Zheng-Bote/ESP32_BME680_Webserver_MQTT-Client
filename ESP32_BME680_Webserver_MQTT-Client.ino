/*
PROG:
  ESP32 temperature, humidity, pressure, gas WIFi Webserver & MQTT client

DESC:
  Arduino prog for BME680 sensor with WiFi, dis-/enabled Webserver and dis-/enabled MQTT client
  - optional: show Temperature and Humidity on Website (refresh every 15 seconds)
  - optional: MQTT client intervall publishing (customizable)
  - optional: MQTT publish data and do ESP32 deep sleep intervall (customizable) 

SOURCE:
  https://github.com/Zheng-Bote/esp32_bme680_webserver_mqtt-client

DEPENDENCIES:
  - MQTT PubSubClient
  - Zanshin_BME680 (https://github.com/Zanduino/BME680)

Version | Date       | Developer  | Comments
------- | ---------- | ---------- | ---------------------------------------------------------------
0.5.0   | 2022-02-19 | RZheng     | created
1.0.0   | 2022-02-20 | RZheng     | added Webserver and MQTT client
1.1.0   | 2022-02-27 | RZheng     | separated to libs, added ESP32 deep sleep
*/

 // ##### 
const char* appTitle = "ESP32 Temperature/Humidity/Pressure/Gas Wifi Webserver & MQTT client";
const char *appVersion = "1.1.0";

bool firmwareUpdateAvailable = false;
const int led = 13;
// ##### Timer
unsigned long previousMillis = 0;
// #####

#include "credentials.h"
#include "rz_wifi.h"
#include "rz_version.h"
#include "rz_system.h"
#include "rz_http.h"
#include "rz_bme680.h"
#include "rz_mqtt.h"
#include "rz_webserver.h"

// create instances
RZ_System *esp32System = new RZ_System(); 
RZ_WiFi *mywifi = new RZ_WiFi(wifiSsid, wifiPassword);
RZ_Version *versions = new RZ_Version();
RZ_HTTP *httpclient = new RZ_HTTP();
RZ_BME680 *bme680sensor = new RZ_BME680();

// ##### Setup
void setup() {
  std::string bufferStr;
  
  Serial.begin(SERIAL_SPEED); 
  
  Serial.print("\nStarting\n- "); 
  digitalWrite(led, 1);
  Serial.print(appTitle); Serial.print(" v"); Serial.println(appVersion);
  Serial.print("-- on "); Serial.println(esp32System->getHostName().c_str());
  
  Serial.print(F("- Initializing WiFi\n"));
  if(mywifi->startWiFi()) {
    Serial.print("\n-- Connected to ");
    Serial.println(wifiSsid);
    Serial.print("-- IP-Address "); Serial.println(mywifi->getIpAddr());

    Serial.print(F("- Initializing MDNS responder\n"));
    if(mywifi->startMDNS(esp32System->getHostName())) {
      bufferStr = esp32System->getHostName() + ".local";
      Serial.print("-- MDNS started: "); Serial.println(bufferStr.c_str());
    }
    else {
      Serial.println("-- MDNS failed");
    }
  }
  else {
    Serial.println("\n-- WiFi connect failed. => restart in 10 seconds\n");
    delay(10000);
    esp32System->doRestart();
  }

  if(CHECK_FIRMWARE) {
    checkVersion(versions->checkVersions(appVersion, httpclient->checkFirmware()));  
  }

  Serial.print(F("- Initializing BME680 sensor\n"));
  bme680sensor->readData();
  if(bme680sensor->getSensorStatus()) {
    Serial.println("\n-- BME680 sensor failed. => restart in 10 seconds\n");
    delay(10000);
    esp32System->doRestart();
  }
  
  Serial.print("-- Temperature: "); Serial.println(bme680sensor->getTemperature());
  Serial.print("-- Humidity: "); Serial.println(bme680sensor->getHumidity());
  Serial.print("-- Pressure: "); Serial.println(bme680sensor->getPressure());
  Serial.print("-- Gas: "); Serial.println(bme680sensor->getGas());

  if(enableMQTT) {
    char buffer[20];
    Serial.print(F("- Initializing MQTT client\n"));
    Serial.print("-- MQTT publish interval in seconds: ");
    Serial.println(mqttInterval / 1000);
    rz_mqttclient_start(esp32System->getHostName());
    bufferStr = esp32System->getHostName() + "/status";
    esp32System->ntpLocalTime(ntpServer, gmtOffset_sec, daylightOffset_sec);
    strcpy(buffer, esp32System->getDateTimeString().c_str()); 
    rz_mqtt_sendMsg(bufferStr, buffer); 
    sendMqttData();
    delay(1000);
  }

  if(enableWebserver) {
    Serial.print(F("- Initializing Webserver\n"));
    webServer.on("/", handleRoot);
    webServer.on("/getjsonvals", getDataJson);
    webServer.on("/checkversion", getVersion);
    webServer.begin();
    Serial.println("-- HTTP server started");
    bufferStr = "-- http://" + esp32System->getHostName() + ".local:";
    Serial.print(bufferStr.c_str()); Serial.println( + webserverPort);
  }
  
  digitalWrite(led, 0);
}
// ##### Setup

void getVersion() {
  char json[100];
  char buf[100];
  char version[10];
  char isUpdate[] = "false";

  if(firmwareUpdateAvailable) {
    strcpy(version, httpclient->getUpdVersion().c_str()); 
    strcpy(isUpdate, "true");
  }
  else {
    strcpy(version, appVersion);
    strcpy(isUpdate, "false");
  }

  strcpy(json, "{\"version\": \"");
  sprintf(buf, "%s", version);
  strcat(json, buf);
  strcat(json, "\",\"isupdate\": \"");
  sprintf(buf, "%s", isUpdate);
  strcat(json, buf);
  strcat(json, "\"}");
  
  // Serial.println(json);

  webServer.send(200, "application/json", json);
}

// checkVersion(versions->checkVersions(v1, v2));
// esp32System->getHostId();
void checkVersion(int ret) {
  switch (ret) {
    case 0: {Serial.println("-- current version is up to date"); break;}
    case 1: {Serial.println("-- current version is greater than Github version. Pls check"); break;}
    case 2: {
      Serial.println("-- a new Firmware is available on Github"); 
      firmwareUpdateAvailable = true;
      if(DO_FIRMWARE_UPDATE) {  }
      break;
    }
    default:
      {Serial.println("case default"); break;}
  }
}

void sendMqttData() {
  std::string bufferStr;
  char buffer[10];

  bme680sensor->readData();
    
  bufferStr = esp32System->getHostName() + "/temperature";
  strcpy(buffer, bme680sensor->getTemperature().c_str()); 

  /*Serial.println("sendMqttData");
  Serial.println(buffer);
  Serial.println(bufferStr.c_str());*/
  
  rz_mqtt_sendMsg(bufferStr, buffer); 
  
  bufferStr = esp32System->getHostName() + "/humidity";
  strcpy(buffer, bme680sensor->getHumidity().c_str()); 

  /*Serial.println("sendMqttData");
  Serial.println(buffer);
  Serial.println(bufferStr.c_str());*/
  
  rz_mqtt_sendMsg(bufferStr, buffer);   

  bufferStr = esp32System->getHostName() + "/pressure";
  strcpy(buffer, bme680sensor->getPressure().c_str()); 

  /*Serial.println("sendMqttData");
  Serial.println(buffer);
  Serial.println(bufferStr.c_str());*/
  
  rz_mqtt_sendMsg(bufferStr, buffer);   

  bufferStr = esp32System->getHostName() + "/gas";
  strcpy(buffer, bme680sensor->getGas().c_str()); 

  /*Serial.println("sendMqttData");
  Serial.println(buffer);
  Serial.println(bufferStr.c_str());*/
  
  rz_mqtt_sendMsg(bufferStr, buffer);   
  
}

void getDataJson() {
  char json[100];
  char buf[100];
  
  bme680sensor->readData();

  strcpy(json, "{\"temperature\": \"");
  sprintf(buf, "%s", bme680sensor->getTemperature().c_str());
  strcat(json, buf);
  strcat(json, "\",\"humidity\": \"");
  sprintf(buf, "%s", bme680sensor->getHumidity().c_str());
  strcat(json, buf);
  strcat(json, "\",\"pressure\": \"");
  sprintf(buf, "%s", bme680sensor->getPressure().c_str());
  strcat(json, buf);
  strcat(json, "\",\"gas\": \"");
  sprintf(buf, "%s", bme680sensor->getGas().c_str());
  strcat(json, buf);
  strcat(json, "\"}");
  
  //Serial.println(json);

  webServer.send(200, "application/json", json); 
}

// ##### LOOP
void loop()
{
  if(enableMQTT) {
    // Serial.println("MQTT Timer Loop");
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= mqttInterval) {
      previousMillis = currentMillis;
//      sendMqttData();
    }
  }

  if(enableWebserver) {
    webServer.handleClient();
  }
  delay(2);
  
  if(deepSleep) {
    Serial.print("- Initializing ESP32 Deep Sleep\n-- duration: "); Serial.print(TIME_TO_SLEEP); Serial.println(" seconds");
    esp32System->doStartDeepSleep(TIME_TO_SLEEP);
  }
}
