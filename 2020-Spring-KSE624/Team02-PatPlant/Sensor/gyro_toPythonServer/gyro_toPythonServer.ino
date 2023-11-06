#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>

#define PROTOTYPE_NAME "collectorS"
#define SERVER_NAME "MyServer"
#define HOST_SERVER "143.248.250.70"

const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,GyX,GyY,GyZ;
const long interval = 50; 
unsigned long previousMillis = 0;
int dummy = -11;

void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x68);
  Wire.write(0);
  Wire.endTransmission(true);
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
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,14,true);

    AcX=Wire.read()<<8|Wire.read();    
    AcY=Wire.read()<<8|Wire.read(); 
    AcZ=Wire.read()<<8|Wire.read();
    GyX=Wire.read()<<8|Wire.read();  
    GyY=Wire.read()<<8|Wire.read();
    GyZ=Wire.read()<<8|Wire.read();

    data_to_server["type"] = "MPU";
    data_to_server["gx"] = GyX;
    data_to_server["gy"] = GyY;
    data_to_server["gz"] = GyZ;
    data_to_server["ax"] = AcX;
    data_to_server["ay"] = AcY;
    data_to_server["az"] = AcZ;
    data_to_server["vc"] = dummy;
    
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
  }
  //socket.loop();   
  
}
