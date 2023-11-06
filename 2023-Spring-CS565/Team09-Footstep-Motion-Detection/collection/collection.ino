#include <Arduino_LSM9DS1.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Started");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  delay(5000);

  Serial.println("timestamp,accX,accY,accZ,gyrX,gyrY,gyrZ");
}

void loop() {
  float accX, accY, accZ, gyrX, gyrY, gyrZ;

  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    IMU.readAcceleration(accX, accY, accZ);
    IMU.readGyroscope(gyrX, gyrY, gyrZ);
    Serial.print(millis());
    Serial.print(',');
    Serial.print(accX);
    Serial.print(',');
    Serial.print(accY);
    Serial.print(',');
    Serial.print(accZ);
    Serial.print(',');
    Serial.print(gyrX);
    Serial.print(',');
    Serial.print(gyrY);
    Serial.print(',');
    Serial.println(gyrZ);
  }
}
