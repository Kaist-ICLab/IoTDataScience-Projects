/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <Arduino_LSM9DS1.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

// #include <Keyboard.h>
#include "model.h"
#include "main_functions.h"
#include "output_handler.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace
{
  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *interpreter = nullptr;
  TfLiteTensor *input = nullptr;
  TfLiteTensor *output = nullptr;

  constexpr int kTensorArenaSize = 2000;
  uint8_t tensor_arena[kTensorArenaSize];
  float x_data[7];
  float y_pred[4];
  const int numSamples = 119;
  int samplesRead = 0;
} // namespace

// The name of this function is important for Arduino compatibility.
void setup()
{
  Serial.begin(9600);
  while (!Serial);

  // Set up IMU sensor
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);
}

// The name of this function is important for Arduino compatibility.
void loop()
{
  while (samplesRead < numSamples) {
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    // Read IMU data
    float accX, accY, accZ, gyrX, gyrY, gyrZ;
    IMU.readAcceleration(accX, accY, accZ);
    IMU.readGyroscope(gyrX, gyrY, gyrZ);

    // Store the data in the x_data array
    x_data[0] = accX;
    x_data[1] = accY;
    x_data[2] = accZ;
    x_data[3] = gyrX;
    x_data[4] = gyrY;
    x_data[5] = gyrZ;

    // Quantize the input from floating-point to integer
    for (int i = 0; i < 6; i++) {
      x_data[i] = (x_data[i] - input->params.zero_point) * input->params.scale ;
      input->data.f[samplesRead * 6 + i] = x_data[i];
    }
    samplesRead++;

    if(samplesRead == numSamples){
      // Run inference for each model, and report any error
      TfLiteStatus invoke_status = interpreter->Invoke();
      if (invoke_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
        return;
      }

      // Obtain the quantized outputs from the models' output tensors
      for (int j = 0; j < 4; j++) {
        y_pred[j] = (output->data.f[j] - output->params.zero_point) * output->params.scale;
      }

      // Find the index of the highest value using argmax
      int max_index = 0;
      for (int k = 1; k < 4; k++) {
        if (y_pred[k] > y_pred[max_index]) {
          max_index = k;
        }
      }
      
      // Print the index with the highest value
      Serial.print("Max Index: ");
      Serial.println(max_index);
    }
  
    // switch (max_index) {
    //   case 0:
    //     Keyboard.press(KEY_DOWN_ARROW);
    //     Keyboard.release(KEY_DOWN_ARROW);
    //     break;
    //   case 1:
    //     Keyboard.press(KEY_RIGHT_ARROW);
    //     Keyboard.release(KEY_RIGHT_ARROW);
    //     break;
    //   case 2:
    //     Keyboard.press(KEY_UP_ARROW);
    //     Keyboard.release(KEY_UP_ARROW);
    //     break;
    //   case 3:
    //     Keyboard.press(KEY_LEFT_ARROW);
    //     Keyboard.release(KEY_LEFT_ARROW);
    //     break;
    // }
    }
  }
}
