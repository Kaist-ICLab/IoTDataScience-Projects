/*
  Arduino LSM9DS1 - Simple Gyroscope

  This example reads the gyroscope values from the LSM9DS1
  sensor and continuously prints them to the Serial Monitor
  or Serial Plotter.

  The circuit:
  - Arduino Nano 33 BLE Sense

  created 10 Jul 2019
  by Riccardo Rizzo

  This example code is in the public domain.
*/

#include <Arduino_LSM9DS1.h>

void setup() {




  Serial.begin(9600);
  while (!Serial);


  //print out column headers
  Serial.println("timestamp,accX,accY,accZ,gyrX,gyrY,gyrZ");


  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Gyroscope in degrees/second");
  Serial.println("X\tY\tZ");

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration in g's");
  Serial.println("X\tY\tZ");
  
}

void loop() {

  
  unsigned long myTime;
  
  float acc_x, acc_y, acc_z;

  float gyro_x, gyro_y, gyro_z;

  // "timestamp,accX,accY,accZ,gyrX,gurY,gyrZ"
  if (IMU.accelerationAvailable() & IMU.gyroscopeAvailable()) {
    
    IMU.readAcceleration(acc_x, acc_y, acc_z);

    IMU.readGyroscope(gyro_x, gyro_y, gyro_z);

    myTime = millis();

    Serial.print(myTime);
    Serial.print(',');

    Serial.print(acc_x);
    Serial.print(',');
    Serial.print(acc_y);
    Serial.print(',');
    Serial.print(acc_z);

    Serial.print(',');

    Serial.print(gyro_x);
    Serial.print(',');
    Serial.print(gyro_y);
    Serial.print(',');
    Serial.print(gyro_z);

    Serial.println();


  }

  

}

