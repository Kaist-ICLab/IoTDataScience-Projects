#include <ArduinoBLE.h>

#include <Arduino_LSM9DS1.h>

#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model.h"


// Both Version 1 UUIDs generated with https://www.uuidgenerator.net/ (last access 28 Apr 2023)
// Set unique UUID to distinguish between services od the BLE device.
const char* SERVICE_UUID = "eeeb91c8-e564-11ed-b5ea-0242ac120002";
// Set unique UUID to distinguish between data structures containing data corresponding to devices funtion.
const char* CHARACTERISTICS_UUID = "33f5a498-e565-11ed-b5ea-0242ac120002";

// BLE address 35:40:8a:32:68:c0

BLEService service(SERVICE_UUID); // Create BLE service.
BLEByteCharacteristic characteristic(CHARACTERISTICS_UUID,BLEWrite | BLERead | BLENotify); // Create BLE characteristic.


const float accelerationThreshold = 1.8; // threshold 
const int totalNumSamples = 128;

int currentSamples = totalNumSamples;

// tflite::MicroErrorReporter tflErrorReporter;

tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

constexpr int tensorArenaSize = 32 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

const char* GESTURES[] = {
  "up",
  "left",
  "down",
  "right",
  // "counterclockwise",
  // "clockwise"
};

const int NUM_GESTURES = (sizeof(GESTURES) / sizeof(GESTURES[0]));

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if(!BLE.begin()) { // Try to start the BLE service.
    Serial.println("Failed to initialize BLE!");
    while (1);
  }
  
  service.addCharacteristic(characteristic); // Add the characteristic to the service.
  BLE.addService(service); // Start the service.

  BLEAdvertisingData scanData;
  scanData.setLocalName("Musiccontoller"); // Set the local name of the service
  BLE.setScanResponseData(scanData);
  BLE.setAdvertisedService(service); // Advertise the service

  BLE.advertise(); // Start advertising

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize); //, &tflErrorReporter);

  tflInterpreter->AllocateTensors();

  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
}

void loop() {

  float accX, accY, accZ, gyrX, gyrY, gyrZ;

  // Wait for significant magnitude to collect samples
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

  // Collect samples
  while (currentSamples < totalNumSamples) {

      IMU.readAcceleration(accX, accY, accZ);
      IMU.readGyroscope(gyrX, gyrY, gyrZ);

      // Put normalized values into input tensor
      // Values are notmalized following max and min possible values for acceleration and gyroscope unit
      tflInputTensor->data.f[currentSamples * 6 + 0] = (accX + 4.0) / 8.0;
      tflInputTensor->data.f[currentSamples * 6 + 1] = (accY + 4.0) / 8.0;
      tflInputTensor->data.f[currentSamples * 6 + 2] = (accZ + 4.0) / 8.0;
      tflInputTensor->data.f[currentSamples * 6 + 3] = (gyrX + 2000.0) / 4000.0;
      tflInputTensor->data.f[currentSamples * 6 + 4] = (gyrY + 2000.0) / 4000.0;
      tflInputTensor->data.f[currentSamples * 6 + 5] = (gyrZ + 2000.0) / 4000.0;

      currentSamples++;

      if (currentSamples == totalNumSamples) {
    
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Invoke failed!");
          while (1);
          return;
        }

        int mostProbableGestureIndex = 0;
        float maxProbability = 0;
        // Get recognized gesture
        for (int i = 0; i < NUM_GESTURES; i++) {
          float probability = tflOutputTensor->data.f[i];
          if (probability > maxProbability) {
            mostProbableGestureIndex = i;
            maxProbability = probability;
          }
        }
        Serial.print(GESTURES[mostProbableGestureIndex]);
        Serial.println();
        int value = mostProbableGestureIndex;
        characteristic.setValue(value);
        Serial.print("Sending ");
        Serial.println(value);
        Serial.println();
      }
  }
  
  // Wait for a BLE connection
  BLEDevice central = BLE.central();
  if (central) {
    while (central.connected()) {
    }
  }
}
