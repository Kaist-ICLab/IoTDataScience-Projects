#include <Arduino_LSM9DS1.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  Serial.println("timestamp,accX,accY,accZ,gyrX,gyrY,gyrZ");
}

void loop() {
  float x, y, z;

  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    Serial.print(millis());
    Serial.print(',');
    
    IMU.readAcceleration(x, y, z);

    Serial.print(x);
    Serial.print(',');
    Serial.print(y);
    Serial.print(',');
    Serial.print(z);
    Serial.print(',');
    

    IMU.readGyroscope(x, y, z);

    Serial.print(x);
    Serial.print(',');
    Serial.print(y);
    Serial.print(',');
    Serial.println(z);
  }
}
