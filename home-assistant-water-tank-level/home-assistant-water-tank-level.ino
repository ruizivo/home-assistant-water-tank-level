#include "network.h"
#include "configuration.h"



// defines variables
long duration;
int distance;
int myArray[10];


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

// Water tank lenght in cm
// water tank width in cm

  //284x150x144 -14

  int sensorHeightWater = 14;
  int lenght = 284;
  int width = 150;
  int height = 130;
  int fullTank = (lenght * width * height)/1000;
  int volume = 0;
  int percent;
  
  int distance = precisionSensorRead() - sensorHeightWater;
  if (height < distance || distance < 0) {                                                       //we don't want to display negative values
      distance = 0;
      volume = 0;
      percent = 0;
  } else {
      volume = (lenght * width * (height-distance)) / 1000;  
      percent = ((float) volume / fullTank) * 100;
  }

  // Prints the distance on the Serial Monitor
//  Serial.print("Distance: ");
//  Serial.println(distance);
//  Serial.print("volume: ");
//  Serial.println(volume);
//  Serial.print("percent: ");
//  Serial.println(percent);

  
  publishData(volume,percent,distance);
   delay(2000);
}

int precisionSensorRead(){
  
  for (int i = 0; i < 10; i = i + 1) {
    myArray[i] = sensorRead();
    delay(1);
    //Serial.println(myArray[i]);
  }
  int count=0;
  int bigger = 0;
  int smaller = 9999999999999;
  for (int i = 0; i < 10; i = i + 1) {
    if(myArray[i] > bigger){
      bigger = myArray[i];
    }
    if(myArray[i] < smaller){
      bigger = myArray[i];
    }
  }
  int sum=0;
  for (int i = 0; i < 10; i = i + 1) {
    if(myArray[i] < bigger && myArray[i] > smaller){
      sum += myArray[i];
      count++;
    }
    
  }
  float total;
  
  if(sum != 0){
    total = (float)sum/count;
  } else{
    total = myArray[0];
  }
  
//  Serial.print("total: ");
//  Serial.println(total);
//  Serial.print("totalR: ");
//  Serial.println(round(total));
  return round(total);
}

int sensorRead(){
    // Clears the trigPin
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(12);
  digitalWrite(pinTrig, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(pinEcho, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
 
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
