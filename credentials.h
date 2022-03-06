/*
TITLE: 
    credentials.h

BRIEF:
    header only

DESC:
    header with customizable vars

SOURCE: 
    https://github.com/Zheng-Bote/esp32_dht_webserver_mqtt-client

SYNTAX:
#include "credentials.h"

RETURNS:
    vars

HISTORIE:
Version | Date       | Developer  | Comments
------- | ---------- | ---------- | -----------------------------------------------
0.5.0   | 2022-02-26 | RZheng     | created 
1.0.0   | 2022-02-27 | RZheng     | NTP added
1.1.0   | 2022-02-27 | RZheng     | changed ESP32 deep sleep type of TIME_TO_SLEEP

*/

// ##### WiFi credentials
char *wifiSsid = "<your wifi ssid>";
char *wifiPassword = "<your wifi password";

// ##### DHT credentials
//=> see rz_dht.h|cpp

// ##### BME680 credentials
//=> see rz_bme680.h|cpp

// ##### MQTT credentials
// IPAddress mqttServer(192, 168, 178, 99);
IPAddress mqttServer(<your MQTT server address, each octet comma separated>);
//int mqttPort = 1883;
int mqttPort = <your MQTT server port>;
char *mqttUser = "<your MQTT user";
char *mqttPwd = "<your MQTT user password";

// run MQTT and do intervall publishing 
bool enableMQTT = true;

// Intervall Timer (Milliseconds) for MQTT publish
const long mqttInterval = 300000; // 5 Mins
//const long mqttInterval = 900000; // 15 Mins
//const long mqttInterval = 1800000; // 30 Mins
//const long mqttInterval = 3600000; // 60 Mins


// ##### Webserver credentials
// run webserver
bool enableWebserver = true; 
int webserverPort = 80;

// ##### System credentials
// time sync
char* ntpServer = "de.pool.ntp.org";
// offset in seconds between your time zone and GMT
long gmtOffset_sec = 3600;
// offset in seconds for daylight saving time.
int daylightOffset_sec = 3600;

// run deepsleep and e.g. run mqtt publish after wakeup
bool deepSleep = true;
// Time ESP32 will go to sleep (in seconds)
//const unsigned long long TIME_TO_SLEEP = 300;  // 5 Mins
const unsigned long long TIME_TO_SLEEP = 900;  // 15 Mins
//const unsigned long long TIME_TO_SLEEP = 1800; // 30 Mins
//const unsigned long long TIME_TO_SLEEP = 3600; // 60 Mins

// Serial Speed
const uint32_t SERIAL_SPEED{115200};

// check for firmware updates
const bool CHECK_FIRMWARE = true;
// do firmware update => not yet implemented
const bool DO_FIRMWARE_UPDATE = false;
