/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

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

#include <algorithm>

#include "Arduino.h"
#include "constants.h"
#include "output_handler.h"
#include "tensorflow/lite/micro/micro_log.h"

void HandleOutput(float* y, unsigned long time) {
  
  #ifndef RUN_BY_USER
  // Log the predictions
  Serial.print("Inference: ");
  Serial.print(y[0], 6);
  Serial.print(" ");
  Serial.println(y[1], 6);
  #endif

  #ifdef RUN_BY_USER
  // Example: (7456ms) Emotion in diary: low valence
  Serial.print("("); Serial.print(time); Serial.print("ms) Emotion in diary: ");
  if (y[0] >= y[1]) {
    Serial.println("low valence");
  } else {
    Serial.println("high valence");
  }
  #endif
}
