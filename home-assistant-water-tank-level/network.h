#ifndef network_h
#define network_h

#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>


extern ESP8266WebServer server;

extern WiFiClient wifiClient;

/***************************************************
          MQTT Server Settings
***************************************************/
extern const char* mqtt_server;               //MQTT server ip address
extern const int mqttPort;                    //MQTT broker port
extern const char* mqttUser;                  //MQTT broker username
extern const char* mqttPassword;              //MQTT broker user password
extern char const * waterTankSensorMqtt;
extern char const * deviceName;

void callback(char * topic, byte * payload, unsigned int length);

extern PubSubClient client;
/**************************************************/



void configOTA();

void configWifiManager();

void reconnectMQTT();

 
#endif
