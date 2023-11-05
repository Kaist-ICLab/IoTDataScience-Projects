/**
* TinyML Trainer Example. 
* Version: v005
*
* This is an example generated by Tiny Motion Trainer 
* It is pre-filled with your trained model and your capture settings 
* 
* Required Libraries:
* - TensorFlow Lite for Microcontrollers (tested with v2.4-beta)
* - Arduino_LSM9DS1
*
* Usage:
* - Make sure the Arduino BLE 33 board is installed (tools->board->Board Manager) and connected
* - Make sure you have the required libraries installed
* - Build and upload the sketch
* - Keep the Arduino connected via USB and open the Serial Monitor (Tools->Serial Monitor)
* - The sketch will log out the label with the highest score once detected.
*
* This is meant as an example for you to develop your own code, and is not production ready.
*
**/

// MPU Imports 
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
//

//==============================================================================
// Includes
//==============================================================================

#include <Arduino_LSM9DS1.h>
#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>


//==============================================================================
// Your custom data / settings
// - Editing these is not recommended
//==============================================================================

// This is the model you trained in Tiny Motion Trainer, converted to 
// a C style byte array.
#include "model.h"

// Values from Tiny Motion Trainer
#define MOTION_THRESHOLD 2
#define CAPTURE_DELAY 200 // This is now in milliseconds
#define NUM_SAMPLES 119

// Array to map gesture index to a name
const char *GESTURES[] = {
    "pinch", "tap", "twist"
};


//==============================================================================
// Capture variables
//==============================================================================

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

bool isCapturing = true;

// Num samples read from the IMU sensors
// "Full" by default to start in idle
int numSamplesRead = 0;

Adafruit_MPU6050 mpu;




//==============================================================================
// TensorFlow variables
//==============================================================================

// Global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// Auto resolve all the TensorFlow Lite for MicroInterpreters ops, for reduced memory-footprint change this to only 
// include the op's you need.
tflite::AllOpsResolver tflOpsResolver;


// Setup model
const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TensorFlow Lite for MicroInterpreters, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize];


void SetupExternalMpu(){
  Serial.println("MPU6050 Initialisation!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);

}
//==============================================================================
// Setup / Loop
//==============================================================================

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
        
  Serial.begin(9600);

  // Wait for serial monitor to connect
  while (!Serial);

  // Initialize IMU sensors
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // Print out the samples rates of the IMUs
  Serial.print("Accelerometer sample rate: ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroscope sample rate: ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");

  Serial.println();

  // Get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
  SetupExternalMpu();
}

void loop() {
  // Variables to hold IMU data
  float aX, aY, aZ, gX, gY, gZ;
  Serial.print("Capturing");
  Serial.print(isCapturing);
  // Wait for motion above the threshold setting
  while (false && !isCapturing) {
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      // Sum absolute values
      float aSum = fabs(aX) + fabs(aY) + fabs(aZ)+ fabs(a.acceleration.x) + fabs(a.acceleration.y) + fabs(a.acceleration.z);

      // float average = fabs(aX / 4.0) + fabs(aY / 4.0) + fabs(aZ / 4.0) + fabs(gX / 2000.0) + fabs(gY / 2000.0) + fabs(gZ / 2000.0);
      // average /= 6.
      Serial.print("Waiting");
      // Above the threshold?
      if (aSum >= MOTION_THRESHOLD) {
        isCapturing = true;
        numSamplesRead = 0;
        break;
      }
    }
  }

  while (isCapturing) {
    Serial.print("Should be here");
    // Check if both acceleration and gyroscope data is available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      Serial.print("IMU available");
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      // Normalize the IMU data between -1 to 1 and store in the model's
      // input tensor. Accelerometer data ranges between -4 and 4,
      // gyroscope data ranges between -2000 and 2000
      tflInputTensor->data.f[numSamplesRead * 6 + 0] = aX;// / 4.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 1] = aY;// / 4.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 2] = aZ;// / 4.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 3] = gX;// / 2000.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 4] = gY;// / 2000.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 5] = gZ;// / 2000.0;

      tflInputTensor->data.f[numSamplesRead * 6 + 6] = a.acceleration.x;// / 4.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 7] = a.acceleration.y;// / 4.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 8] = a.acceleration.z;// / 4.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 9] = g.gyro.x;// / 2000.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 10] = g.gyro.y;// / 2000.0;
      tflInputTensor->data.f[numSamplesRead * 6 + 11] = g.gyro.z;// / 2000.0;

      numSamplesRead++;
      Serial.print("1");
      // Do we have the samples we need?
      if (numSamplesRead == NUM_SAMPLES) {
        numSamplesRead = 0;
        }
        // Stop capturing
        //isCapturing = false;
        Serial.print("2");
        // Run inference
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Error: Invoke failed!");
          while (1);
          return;
        }
        Serial.print("3");

        // Loop through the output tensor values from the model
        int maxIndex = 0;
        float maxValue = 0;
        for (int i = 0; i < NUM_GESTURES; i++) {
          float _value = tflOutputTensor->data.f[i];
          if(_value > maxValue){
            Serial.print("4");
            maxValue = _value;
            maxIndex = i;
          }
        }
        Serial.print("Current Mode :");
        Serial.print(GESTURES[maxIndex]);
        Serial.println();

        // Add delay to not double trigger
        delay(CAPTURE_DELAY);
    }
  }
}