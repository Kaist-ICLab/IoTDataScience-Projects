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

#include <TensorFlowLite.h>

#include "main_functions.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "constants.h"
#include "model.h"
#include "output_handler.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include <Arduino_LSM9DS1.h>

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
int inference_count = 0;
int offset = 0;

// Track wich state of knocking we just where in
unsigned char last_state = 0;
// the timestamp for the last state change
unsigned int timestamp = 0;
// Morse code buffer
unsigned int buffer[6] = {0, 0, 0, 0, 0, 0};
// Current index of the "write head" in the buffer
unsigned int buffer_offset = 0;

constexpr int kTensorArenaSize = 4000;
uint8_t tensor_arena[kTensorArenaSize];


float mean_[20] = {1.0022442170817063, 0.9969739768681839, 0.978042704626188, 0.9774699733094633, 0.9885242437721001, 0.9912844750888297, 0.9855393683272535, 0.9825700622774335, 0.9850900800710289, 0.98709630782904, 0.9885487099642646, 0.9860542704624838, 0.9812700177934515, 0.9842415480425646, 0.9943527580069735, 0.9891570284696032, 0.9749644128112537, 0.9777680160141133, 1.0058463078290607, 1.0034842081849298};

float scale_[20] = {0.05701648761363816, 0.04319244611142525, 0.046883705124575015, 0.042844716062582694, 0.039743557294275894, 0.038103191961104434, 0.03919704542517741, 0.038188382004254784, 0.03629396192285551, 0.039044248721854705, 0.04310507194035964, 0.03947357881955402, 0.03882232144002704, 0.04378275001672554, 0.049458627264599125, 0.04295937918789043, 0.042593299062307084, 0.051431795092334316, 0.06028583272898376, 0.05562282799706009};

// Time to wait before reseting the morse code buffer
int RESET_TIME_LIMIT = 4000;
// Timelimit for a short tap
int SHORT_TIME_LIMIT = 1000;
}  // namespace




void setup() {
    Serial.begin(9600);
  while (!Serial);
  Serial.println("Started");

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
  if (model->version() != TFLITE_SCHEMA_VERSION) {
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
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);

  // Keep track of how many inferences we have performed.
  inference_count = 0;
  offset = 0;
}

void loop() {
  float x, y, z;
  
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
	  // Capture sensor data to be feed to the model
      IMU.readAcceleration(x, y, z);


      float scaled_z = (z- mean_[offset]) / scale_[offset];

      input->data.f[offset] = scaled_z;

      offset++;

      if (offset >= 20) {
        offset = 0;
      } else {
        return;
      }


      // Run inference, and report any error
      TfLiteStatus invoke_status = interpreter->Invoke();
      if (invoke_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x: %f\n",
                            static_cast<double>(x));
        return;
      }

      // Obtain the quantized output from model's output tensor
      float no = output->data.f[0];
      float tap = output->data.f[1];

      handle_prediction(no, tap);
    }
}

bool is_valid(int i, int first_is_short){
  if (i > 1){
    int prev_is_short = buffer[i-1] - buffer[i-2] < SHORT_TIME_LIMIT;
    int current_is_short = buffer[i] - buffer[i-1] < SHORT_TIME_LIMIT;
    if(first_is_short && !prev_is_short && current_is_short){
      return false;
    }
    else if(!first_is_short && prev_is_short && !current_is_short){
      return false;
    }
    else{
      return true;
    }
  }
}

void handle_prediction(float no_tap, float yes_tap) {
  unsigned char state = (yes_tap > no_tap);

  // Reset the morse code buffer if we recive no input for a long time
  if (millis() - timestamp > RESET_TIME_LIMIT) {
    if (buffer_offset > 0) {
      Serial.println("Reset");
    }
    buffer_offset = 0;
  }
  
  // We are only intreseted in state transition to a knocking/tapping state
  if (state != last_state && state) {
    buffer[buffer_offset] = millis();
    buffer_offset++;

    Serial.print("-> ");
    for (int i = 1; i < buffer_offset; i++) {
      if(buffer[i] - buffer[i-1] < SHORT_TIME_LIMIT) {
        Serial.print(".");
      } else {
        Serial.print("-");
      }
    }
    Serial.println("");


    bool valid_number = true;

    if (buffer_offset >= 6) {
      // Do match
      int no_short = 0;
      int first_is_short = buffer[1] - buffer[0] < SHORT_TIME_LIMIT;

      for (int i = 0; i < 6; i++) {
        if(i<5){
          no_short += buffer[i+1] - buffer[i] < SHORT_TIME_LIMIT;
        }
        valid_number = is_valid(i, first_is_short);
      }

      
      
      if(!valid_number){
         Serial.println("Invalid Number");
      }
      else if (first_is_short) {
        Serial.println(no_short);
      } 
      else {
        if (no_short > 0) {
          Serial.println(10-no_short);
        } else{
          Serial.println(0);
        }

      }

      // Reset
      buffer_offset = 0;
    }
    timestamp = millis();
  }

  last_state = state;
}
