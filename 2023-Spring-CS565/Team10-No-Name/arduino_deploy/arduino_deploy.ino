#include <Arduino_LSM9DS1.h>
#include <TensorFlowLite.h>

// #include "main_functions.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "model.h"
// #include "output_handler.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 160000;
uint8_t tensor_arena[kTensorArenaSize];

float ax, ay, az;  // acceleration data
float gx, gy, gz;  // gyroscope data
float a_mag, g_mag;  // magnitudes

const int numSamples = 119;
const int numFeatures = 8;
float inputBuffer[numSamples][numFeatures];  // buffer to store the input tensor
int sampleIndex = 0;
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Started");

  tflite::InitializeTarget();

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf(
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
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  inference_count = 0;

  if (!IMU.begin()) {
    MicroPrintf("Failed to initialize IMU");
    while (1);
  }
}

// The name of this function is important for Arduino compatibility.
void loop() {
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);

    a_mag = sqrt(ax * ax + ay * ay + az * az);
    g_mag = sqrt(gx * gx + gy * gy + gz * gz);

    // Fill the input buffer
    inputBuffer[sampleIndex][0] = ax;
    inputBuffer[sampleIndex][1] = ay;
    inputBuffer[sampleIndex][2] = az;
    inputBuffer[sampleIndex][3] = gx;
    inputBuffer[sampleIndex][4] = gy;
    inputBuffer[sampleIndex][5] = gz;
    inputBuffer[sampleIndex][6] = a_mag;
    inputBuffer[sampleIndex][7] = g_mag;

    sampleIndex++;

    if (sampleIndex == numSamples) {
      // All samples collected, time to perform inference
      for (int i = 0; i < numSamples; i++) {
        for (int j = 0; j < numFeatures; j++) {
          input->data.f[i * numFeatures + j] = inputBuffer[i][j];
        }
      }

      TfLiteStatus invoke_status = interpreter->Invoke();
      if (invoke_status != kTfLiteOk) {
        MicroPrintf("Invoke failed on sample_index: %d",
                            sampleIndex);
        return;
      }

      float y_output = output->data.f[0]; // Interpret the output as per your requirement

      // HandleOutput(error_reporter, sampleIndex, y_output);

      // Reset sampleIndex for next set of samples
      sampleIndex = 0;
    }
  }
}
