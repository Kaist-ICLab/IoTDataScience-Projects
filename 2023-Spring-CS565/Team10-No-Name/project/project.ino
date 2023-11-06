#include <Arduino_LSM9DS1.h>
#include <PDM.h>

// buffer to read samples into, each sample is 16-bits
short sampleBuffer[256];

// number of samples read
volatile int samplesRead;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Started");

  // configure the data receive callback
  PDM.onReceive(onPDMdata);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM!");
    while (1);
  }

  // if (!IMU.accelerationAvailable() || !IMU.gyroscopeAvailable() || !IMU.magneticFieldAvailable()) {
  //   Serial.println("Failed to detect sensors!");
  //   while (1);
  // }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");

  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");

  Serial.print("Magnetometer sample rate = ");
  Serial.print(IMU.magneticFieldSampleRate());
  Serial.println(" Hz");

  Serial.println("timestamp,accX,accY,accZ,gyrX,gyrY,gyrZ,micValue");
}

void loop() {
  unsigned long timestamp = millis();
  float accX, accY, accZ, gyrX, gyrY, gyrZ, magX, magY, magZ;

  // if (samplesRead) {
  //   // print samples to the serial monitor or plotter
  //   for (int i = 0; i < samplesRead; i++) {
  //     Serial.print(",,,,,,,");
  //     Serial.println(sampleBuffer[i]);
      // // check if the sound value is higher than 500
      // if (sampleBuffer[i]>=500){
      //   digitalWrite(LEDR,LOW);
      //   digitalWrite(LEDG,HIGH);
      //   digitalWrite(LEDB,HIGH);
      // }
      // // check if the sound value is higher than 250 and lower than 500
      // if (sampleBuffer[i]>=250 && sampleBuffer[i] < 500){
      //   digitalWrite(LEDB,LOW);
      //   digitalWrite(LEDR,HIGH);
      //   digitalWrite(LEDG,HIGH);
      // }
      // //check if the sound value is higher than 0 and lower than 250
      // if (sampleBuffer[i]>=0 && sampleBuffer[i] < 250){
      //   digitalWrite(LEDG,LOW);
      //   digitalWrite(LEDR,HIGH);
      //   digitalWrite(LEDB,HIGH);
      // }
  //   }
  //   // clear the read count
  //   samplesRead = 0;
  // }

  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    Serial.print(timestamp);
    Serial.print(',');

    IMU.readAcceleration(accX, accY, accZ);

    Serial.print(accX);
    Serial.print(',');
    Serial.print(accY);
    Serial.print(',');
    Serial.print(accZ);
    Serial.print(',');

    IMU.readGyroscope(gyrX, gyrY, gyrZ);

    Serial.print(gyrX);
    Serial.print(',');
    Serial.print(gyrY);
    Serial.print(',');
    Serial.print(gyrZ);
    Serial.println(',');
  }
}

void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}
