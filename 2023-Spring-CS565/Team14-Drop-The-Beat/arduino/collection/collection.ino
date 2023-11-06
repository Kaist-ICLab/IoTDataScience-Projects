#include <Arduino_LSM9DS1.h>

const float accelerationThreshold = 1.8;
const int totalNumSamples = 128;

int currentSamples = totalNumSamples;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
}

void loop() {

  float accX, accY, accZ, gyrX, gyrY, gyrZ;

  // Wait for accelerometer readings to surpass certain magnitude
  while (currentSamples == totalNumSamples) {
    if (IMU.accelerationAvailable()) {
  
      IMU.readAcceleration(accX, accY, accZ);

      float magnitude = sqrt(pow(accX, 2) + pow(accY, 2) + pow(accZ, 2));

      if (magnitude >= accelerationThreshold) {
        currentSamples = 0;
        break;
      }
    }
  }

  // Read samples and print them in csv format
  while (currentSamples < totalNumSamples) {

      IMU.readAcceleration(accX, accY, accZ);
      IMU.readGyroscope(gyrX, gyrY, gyrZ);

      Serial.print(accX);
      Serial.print(",");
      Serial.print(accY);
      Serial.print(",");
      Serial.print(accZ);
      Serial.print(",");
      Serial.print(gyrX);
      Serial.print(",");
      Serial.print(gyrY);
      Serial.print(",");
      Serial.println(gyrZ);

      currentSamples = currentSamples + 1;
  }
  Serial.println("");

}
