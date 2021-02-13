#include "network.h"
#include "configuration.h"



// defines variables
long duration;
int distance;



void configUltrasonicSensor(){
  pinMode(pinTrig, OUTPUT); // Sets the trigPin as an Output
  pinMode(pinEcho, INPUT); // Sets the echoPin as an Input
}


void configWebServer(){
  server.on("/cmd", handleCmd);
  server.on("/status", handleStatus);
  server.begin(); //Start the server
  Serial.println("server started");
}

void setup() {
  Serial.begin(115200);
  
  configWifiManager();

  configOTA();

  configWebServer();

  configUltrasonicSensor();
}

void loop() {
//  if (WiFi.status() != WL_CONNECTED) {                                    // Reconnect if connection is lost
//    reconnectWifi();
//  } else 
  if (!client.connected()) {
    reconnectMQTT();
  } else {
    client.loop();                                                        // Maintain MQTT connection
    delay(10);                                                            // MUST delay to allow ESP8266 WIFI functions to run
    server.handleClient();
    ArduinoOTA.handle();
  }
  

  publishData(0,0,sensorRead());

}

int sensorRead(){
    // Clears the trigPin
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(pinEcho, HIGH);
  
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  delay(2000);
  return distance;
}



void publishData(int p_volume, int p_percent, int p_distanceInCm) {    // function called to publish the temperature and the humidity  
  StaticJsonDocument < 200 > jsonDocument;                            // create a JSON object  
  jsonDocument["volume"] = (String) p_volume;                         // INFO: the data must be converted into a string; a problem occurs when using floats...
  jsonDocument["percentage"] = (String) p_percent;
  jsonDocument["distanceInCm"] = (String) p_distanceInCm;
  //jsonDocument["distanceInFeet"] = (String) p_distanceinFeet;
  //jsonDocument["buildNumber"] = (String) p_buildNumber;
  char data[200];
  serializeJson(jsonDocument, data);
  client.publish(waterTankSensorMqtt, data, true);                        //Publishing data to MQTT server as Json
  yield();
}



void checkBody() {
  if (server.hasArg("plain") == false) { //Check if body received
    server.send(200, "text/plain", "Body not received");
    return;
  }
}


void handleCmd() {
  checkBody();

  StaticJsonDocument<200> doc;
  deserializeJson(doc, server.arg("plain"));


  int iSteps = doc["steps"];
  int velocity = doc["velocity"];
  bool iTrack = doc["track"];
  String path = doc["path"].as<String>();
  String sensor = doc["mode"].as<String>();  

  Serial.println(server.arg("plain"));
 
  Serial.print("set x: ");
  //Serial.println(x);
  Serial.print("set y: ");
  //Serial.println(y);

  //server.send(200, "text/json", server.arg("plain"));
  server.send(200, "text/json");
}


void handleStatus(){
  server.send(200, "text/json");
}
