/* Copyright 2023 The TensorFlow Authors. All Rights Reserved.

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

#include "constants.h"
#include "main_functions.h"
#include "model.h"
#include "output_handler.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Globals
namespace {
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  int inference_count = 0;
  
  constexpr int kTensorArenaSize = 50000;
  uint8_t tensor_arena[kTensorArenaSize];

  float ppg_window[PPG_INPUT_DIM];

  #ifdef USE_SAMPLE_VALUES
  float sample_ppg_window[] = {
    -0.7308375444346406,-0.9632044683253919,-0.9632044683253919,-1.0793879302707676,-0.8470210063800162,-0.4984706205438892,-0.6146540824892649,-0.4984706205438892,-0.6146540824892649,-0.9632044683253919,-0.9632044683253919,-0.38228715859851353,-0.14992023470776217,-0.7308375444346406,-0.2661036966531378,0.7795474608552432,1.4766482325274972,2.289932466145127,2.7546663139266294,2.7546663139266294,3.219400161708132,3.335583623653508,3.335583623653508,2.870849775872005,2.0575655422543755,2.6384828519812538,1.8251986183636242,1.244281308636746,0.08244668918298917,0.6633639989098675,0.19863015112836482,-0.7308375444346406,-0.38228715859851353,-0.7308375444346406,-0.7308375444346406,-0.9632044683253919,-1.1955713922161433,-0.8470210063800162,-0.7308375444346406,-0.4984706205438892,-1.0793879302707676,-1.0793879302707676,-0.4984706205438892,-1.4279383161068946,-0.14992023470776217,-1.1955713922161433,-0.4984706205438892,-0.38228715859851353,-0.38228715859851353,-0.7308375444346406,-1.0793879302707676,-0.7308375444346406,-0.14992023470776217,-0.8470210063800162,-0.8470210063800162,-0.38228715859851353,-0.2661036966531378,-0.38228715859851353,-0.6146540824892649,-0.8470210063800162,-1.311754854161519,-1.1955713922161433,-0.38228715859851353,-0.9632044683253919,-1.311754854161519,-0.4984706205438892,-0.7308375444346406,-0.6146540824892649,-1.1955713922161433,-1.1955713922161433,-1.1955713922161433,-0.8470210063800162,-0.7308375444346406,-0.8470210063800162,-0.38228715859851353,-0.4984706205438892,-1.311754854161519,-1.1955713922161433,-0.9632044683253919,-0.6146540824892649,-0.6146540824892649,-0.2661036966531378,-0.38228715859851353,-0.4984706205438892,-0.2661036966531378,-0.4984706205438892,-1.311754854161519,-1.311754854161519,-0.4984706205438892,-0.7308375444346406,0.3148136130737405,0.3148136130737405,1.1280978466913703,0.7795474608552432,1.7090151564182485,1.9413820803089998,2.522299390035878,3.335583623653508,2.9870332378173807,3.1032166997627564,2.6384828519812538,1.5928316944728729,2.173749004199751,1.0119143847459946,1.0119143847459946,0.7795474608552432,-0.033736772762386506,-0.9632044683253919,-0.38228715859851353,-0.7308375444346406,-1.1955713922161433,-1.311754854161519,-0.8470210063800162,-1.660305239997646,-1.1955713922161433,-0.9632044683253919,-1.660305239997646,-1.311754854161519,-1.5441217780522702,-0.6146540824892649,-0.7308375444346406,-0.7308375444346406,-0.9632044683253919,-0.2661036966531378,-1.1955713922161433,-0.14992023470776217,-0.2661036966531378,-0.38228715859851353,-0.2661036966531378,-0.4984706205438892,-0.4984706205438892,-1.0793879302707676,-0.7308375444346406,-0.9632044683253919,-0.4984706205438892,-0.6146540824892649,-0.6146540824892649,-0.4984706205438892,-0.6146540824892649,-0.6146540824892649,-0.7308375444346406,-1.4279383161068946,-0.7308375444346406,-0.7308375444346406,-1.660305239997646,-1.1955713922161433,-0.4984706205438892,-0.8470210063800162,-1.660305239997646,-0.7308375444346406,-0.4984706205438892,-0.9632044683253919,-0.4984706205438892,-0.38228715859851353,-1.0793879302707676,-1.5441217780522702,-0.38228715859851353,-0.38228715859851353,-0.4984706205438892,-0.6146540824892649,-0.7308375444346406,-0.9632044683253919,-0.9632044683253919,-1.311754854161519,-1.1955713922161433,-0.38228715859851353,-0.8470210063800162,-0.7308375444346406,-0.6146540824892649,-0.14992023470776217,-0.14992023470776217,1.1280978466913703,1.1280978466913703,2.173749004199751,2.173749004199751,3.335583623653508,3.219400161708132,3.451767085598884,2.522299390035878,3.219400161708132,2.4061159280905025,2.289932466145127,1.8251986183636242,1.4766482325274972,1.244281308636746,0.3148136130737405,0.19863015112836482,-0.6146540824892649,-0.38228715859851353,-0.4984706205438892,-0.6146540824892649,-1.311754854161519,-1.4279383161068946,-1.0793879302707676,-1.0793879302707676,-1.0793879302707676,-1.0793879302707676,-1.4279383161068946,-1.1955713922161433,-1.5441217780522702,-0.6146540824892649,-0.38228715859851353,-0.6146540824892649,-1.0793879302707676,-0.6146540824892649,-0.033736772762386506,-0.033736772762386506,-0.7308375444346406,-0.6146540824892649,-1.1955713922161433,-0.2661036966531378,-0.2661036966531378,-0.4984706205438892,-0.14992023470776217,-0.6146540824892649,-0.8470210063800162,-0.4984706205438892,-0.9632044683253919,-1.0793879302707676,-1.0793879302707676,-1.311754854161519,-0.4984706205438892,-1.1955713922161433,-0.9632044683253919,-0.7308375444346406,-0.6146540824892649,-0.8470210063800162,-1.1955713922161433,-0.6146540824892649,-0.6146540824892649,-1.0793879302707676,-0.7308375444346406,-0.6146540824892649,-0.7308375444346406,-1.1955713922161433,-0.8470210063800162,-0.8470210063800162,-0.4984706205438892,-0.7308375444346406,-0.6146540824892649
  }; // Should print 0.01479022 0.98251802
  #endif

  unsigned long record_start_time;
} // namespace

void setup() {
  delay(5000); // Wait serial connection
  MicroPrintf("Start setup\n");

  // Map the model into a usable data structure. No copy or parse
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf(
      "Model provided is schema version %d not equal "
      "to supported version %d.",
      model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Pull in all the op implentaions we need
  static tflite::AllOpsResolver resolver;

  // Build an interpreter
  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, kTensorArenaSize
  );
  interpreter = &static_interpreter;

  // Allocate memeory for the tensors from the tensor arena
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  // Obtain pointers to io tensors
  input = interpreter->input(0);
  output = interpreter->output(0);


  Serial.println("Type 'r' to start");
  // Wait until start. Type 'r' to start.
  while (true) {
    if (Serial.available()) {
      char ch = Serial.read();
      if (ch == 'r') {
        record_start_time = millis();
        break;
      }
    }
  }
}

void loop() {
  // Calculate an x value to feed into the model. 
  // We compare the current inference_count to #inferences/cycle 
  // to determine our position within the range of possible x values
  // the model wos trained on, and use this to calculate a value.


  // Get ppg values to construct an input for infernce.
  // TODO: gathering ppg values and conducting inference should be asynchronously work to make a live inference. 
  //       (Not sure. We may not need it.)
  int ppg_pos;
  int ppg_read;
  float ppg;

  unsigned long loop_start_time;
  unsigned long sample_start_time;

  // Start a 10-sec inference cycle. (10-sec is defined as INFERENCE_PERIOD in milliseconds)

  loop_start_time = millis();

  #ifndef USE_SAMPLE_VALUES
  // PROCESS 1. READ PPG VALUES

  ppg_pos = 0;
  while(ppg_pos < PPG_INPUT_DIM) {
    
    sample_start_time = micros();
    ppg_read = analogRead(PPG_PIN);

    #ifndef RUN_BY_USER
    // Print read ppg value
    Serial.print(sample_start_time);
    Serial.print(",");
    Serial.println(ppg_read);  
    #endif

    // Save the values to the window for inference
    ppg = static_cast<float>(ppg_read);
    ppg = OutlierModifed(ppg);
    ppg = Normalized(ppg);

    ppg_window[ppg_pos] = ppg;
    ppg_pos++;

    // Delay to match the sampling rate
    delayMicroseconds(SAMPLE_PERIOD * 1000 - (micros() - sample_start_time));
  }

  // Place the input in the model's input tensor
  for (int i = 0; i < PPG_INPUT_DIM; i++) {
    input->data.f[i] = ppg_window[i];
  } 
  #endif

  #ifdef USE_SAMPLE_VALUES
  Serial.println("Use sample values");

  for (int i = 0; i < PPG_INPUT_DIM; i++) {
    input->data.f[i] = sample_ppg_window[i];
  } 
  #endif

  // PROCESS 2: INFER THE VALENCE

  // Run inference and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    MicroPrintf("Invoke failed\n");
    return;
  }

  // Output the results.
  HandleOutput(output->data.f, millis() - record_start_time); // Output is in output->data.f

  // Have delay to match inference cycle
  delay(INFERENCE_PERIOD - (millis() - loop_start_time));
}

float Normalized(float x) {
  // TRAINED_MEAN, TRAINED_STD 
  return (x - TRAINED_MEAN) / TRAINED_STD;
}

float OutlierModifed(float x) {
  // TRAINED_MED, TRAINED_MAD, OUTLIER_THRESHOLD
  float modified_z_scores = 0.6745 * (x - TRAINED_MED) / TRAINED_MAD;
  if (Abs(modified_z_scores) > OUTLIER_THRESHOLD) {
    return TRAINED_MED;
  } else {
    return x;
  }
}

float Abs(float x) {
  if (x >= 0) {
    return x;
  } else {
    return -x;
  }
}