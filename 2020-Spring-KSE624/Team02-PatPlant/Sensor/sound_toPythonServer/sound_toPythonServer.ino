#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>

#define PROTOTYPE_NAME "collectorS"
#define SERVER_NAME "MyServer"
#define HOST_SERVER "143.248.250.70"

const long interval = 50; 
unsigned long previousMillis = 0;
int soundPin = A0;
int sensorVal = 0;
int dummy = -11;

void setup(){
  Serial.begin(115200);
  Serial.println('serial begin');
    
  while(smartWiFi(PROTOTYPE_NAME));  
  launchWeb(1);
  //socket.begin(HOST_SERVER);
  //socket.on(SERVER_NAME, event);
}

void loop(){
  mdns_setup();  
  unsigned long currentMillis = millis();  

  if (currentMillis - previousMillis >= interval) {    
    previousMillis = currentMillis;
    
    DynamicJsonDocument data_to_server(1024);

    sensorVal = analogRead (A0);
    data_to_server["type"] = "VOICE";
    data_to_server["gx"] = dummy;
    data_to_server["gy"] = dummy;
    data_to_server["gz"] = dummy;
    data_to_server["ax"] = dummy;
    data_to_server["ay"] = dummy;
    data_to_server["az"] = dummy;
    data_to_server["vc"] = sensorVal;
    
    String jsonData;
    serializeJson(data_to_server, jsonData);    

    HTTPClient http;
    //socket.emit(PROTOTYPE_NAME, jsonData.c_str()); 
    http.begin("http://192.168.0.59:5000/user");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonData.c_str());
    String payload = http.getString();
     
    Serial.println(httpCode);
    Serial.println(payload);
    http.end();
    Serial.print("sound = "); Serial.println(sensorVal);
    
    
  }
  //socket.loop();   
  
}
