#include "network.h"


ESP8266WebServer server(80);

WiFiClient wifiClient;

/***************************************************
          MQTT Server Settings
***************************************************/
const char* mqtt_server = "192.168.31.55";            //MQTT server ip address
const int mqttPort = 1883;                            //MQTT broker port

const char* mqttUser = "mqtt";                        //MQTT broker username
const char* mqttPassword = "123456";                  //MQTT broker user password

char const * waterTankSensorMqtt = "/house/watersump/";
char const * deviceName = "WaterLevelSensor-" + ESP.getChipId();

void callback(char * topic, byte * payload, unsigned int length);

PubSubClient client(mqtt_server, mqttPort, callback, wifiClient);
/**************************************************/

void configOTA(){
  ArduinoOTA.onStart([]() {
    Serial.println("Inicio...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("nFim!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Autenticacao Falhou");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Falha no Inicio");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Falha na Conexao");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Falha na Recepcao");
    else if (error == OTA_END_ERROR) Serial.println("Falha no Fim");
  });
  ArduinoOTA.begin();
}


void configWifiManager(){
    WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.autoConnect(deviceName);
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  //WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  IPAddress myIP = WiFi.localIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}


String macToStr(const uint8_t * mac) {                                // Generate unique name from MAC addr
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5) {
      result += ':';
    }
  }
  return result;
}

void reconnectMQTT() {
  delay(1000);
  if (WiFi.status() == WL_CONNECTED) {                                // Make sure we are connected to WIFI before attemping to reconnect to MQTT    
    while (!client.connected()) {                                     // Loop until we're reconnected to the MQTT server
      Serial.print("Attempting MQTT connection...");
      String clientName;                                              // Generate client name based on MAC address and last 8 bits of microsecond counter
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {  // Delete "mqtt_username", and "mqtt_password" here if you are not using any
        Serial.println("MQTT Connected");
        //client.subscribe(firmwareUpdateTopic);                        // If connected, subscribe to the topic(s) we want to be notified about
        //client.subscribe(waterTankSensorMqtt);
        //client.publish("/house/watersump/Confirmfirmware/", "0");     // Sending message to MQTT server to turn off MQTT firmware upgrade button if its on
      } else {
        Serial.println("\tFailed.");
        abort();
      }
    }
  } else {
    Serial.println("Wifi is not connected");
  }
}

void callback(char * topic, byte * payload, unsigned int length) {
  String topicStr = topic;                                            // Convert topic to string to make it easier to work with
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);                                           // Note:  the "topic" value gets overwritten everytime it receives confirmation (callback) message from MQTT

  if (topicStr == "/house/watersump/firmware/") {
    Serial.print("Firmware update calling ");
    if (payload[0] == '1') {                                          // Turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
      Serial.print("Firmware switch on ");
      //checkforupdate();
    } else if (payload[0] == '0') {                                   // Turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
      Serial.print("Firmware switch off ");
    }
  } else {
    //publishData(volume, percent, distance, distanceinFeet, BUILD_NUMBER);
  }
}
