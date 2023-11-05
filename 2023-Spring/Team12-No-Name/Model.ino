#define DEBUG 0
#define enable_BLE 1

#include "net.h"
#include <ArduinoBLE.h>

#include <TensorFlowLite.h>
#include <Arduino_LSM9DS1.h>
#include <Adafruit_GPS.h>
#include <TaskScheduler.h>

#include "constants.h"
#include "model.h"
#include "pins.h"

// #include "tensorflow/lite/micro/kernels/micro_ops.h"
// #include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
  // tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;

  constexpr size_t kTensorArenaSize = 64*1024;
  // Keep aligned to 16 bytes for CMSIS
  alignas(16) uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

bool tone_on = false; 

float current_acceleration_data[3];
float current_gyroscope_data[3];

float prev_speed = 0;
float diff_speed = 0;

int gyrBufRow = 0;
int accBufRow = 0;
int gpsBufRow = 0;

#if enable_BLE
BLEService myService("D80D65F8-ED8A-417F-96C2-92C611E6A675");  // create service

// create switch characteristic and allow remote device to read and write
BLECharacteristic myBLE_Sender("0DDE4C84-3F91-4BC7-9F73-1D76F25FB94C", BLERead | BLENotify, big_union_size);
#endif

// what's the name of the hardware serial port?
#define GPSSerial Serial1

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

char* NMEA_;
char c;

#if enable_BLE
gpsPacket_t gps;
#endif

uint32_t timer = millis();

void readGPS() {
  timer = millis();
  while(!GPS.newNMEAreceived()){ //Loop until you have a good NMEA sentence
    c=GPS.read();
    if (millis()-timer > 12){
      return;
    }
  }
  NMEA_=GPS.lastNMEA();
  GPS.parse(NMEA_); //Parse that last good NMEA sentence

#if DEBUG
  // Serial.println(NMEA_);
#endif
}

// GPS bufferring
void taskBufGPS() {
  readGPS();
  if (GPS.fix) {
    diff_speed = (float)GPS.speed - prev_speed;
    prev_speed = (float)GPS.speed;
  }
}

#if enable_BLE
void taskSendGPS() {
  gps.structure.type_ms = htonl(millis());
  gps.structure.latitude = htonf(GPS.latitude);
  gps.structure.longitude = htonf(GPS.longitude);
  gps.structure.lat = htons(GPS.lat);
  gps.structure.lon = htons(GPS.lon);
  myBLE_Sender.writeValue(gps.byteArray, big_union_size);
}
#endif

// A buffer holding the last 20 sets of 3-channel values from the gyroscope.
constexpr int acceleration_data_length = WIDTH * NUM_CHANNELS;
constexpr int gyroscope_data_length = WIDTH * NUM_CHANNELS;
float acceleration_data[acceleration_data_length] = {};
float gyroscope_data[gyroscope_data_length] = {};
float orientation_data[gyroscope_data_length] = {};

// The next free entry in the data array.
float acceleration_sample_rate = 0.0f;
// The next free entry in the data array.
float gyroscope_sample_rate = 0.0f;

void SetupIMU() {
  // Make sure we are pulling measurements into a FIFO.
  // If you see an error on this line, make sure you have at least v1.1.0 of the
  // Arduino_LSM9DS1 library installed.
  IMU.setContinuousMode();

  acceleration_sample_rate = IMU.accelerationSampleRate();
  gyroscope_sample_rate = IMU.gyroscopeSampleRate();

#if DEBUG
  float rate_frac;
  float rate_int;
  rate_frac = modf(acceleration_sample_rate, &rate_int);
  MicroPrintf("Acceleration sample rate %d.%d Hz",
              static_cast<int32_t>(rate_int),
              static_cast<int32_t>(rate_frac * 100));
  rate_frac = modf(gyroscope_sample_rate, &rate_int);
  MicroPrintf("Gyroscope sample rate %d.%d Hz", static_cast<int32_t>(rate_int),
              static_cast<int32_t>(rate_frac * 100));
#endif  // DEBUG
}

Task t3(210, TASK_FOREVER, &taskBufGPS);

Scheduler runner;

void setup() {
#if DEBUG
  Serial.begin(115200);
  while(!Serial);
  Serial.println("start");
#endif
  
  GPS.begin(9600);
  delay(200);
  // Serial.print("Adafruit GPS logging start test!");
  // You can adjust which sentences to have the module emit, below
  // Default is RMC + GGA
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // Default is 1 Hz update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
  // Turn off antenna update nuisance data
  GPS.sendCommand("$PGCMD,33,0*6D");

  Serial.println("Pinmode");
  pinMode(buzzerPin, OUTPUT);
  delay(1000);

  // Map the model into a usable data structure.
  tflite::InitializeTarget();
  Serial.println("Model");
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    // MicroPrintf(
    //     "Model provided is schema version %d not equal "
    //     "to supported version %d.",
    //     model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // static tflite::AllOpsResolver resolver;

  Serial.println("Resolver");
  static tflite::MicroMutableOpResolver<8> micro_mutable_op_resolver;
  micro_mutable_op_resolver.AddQuantize();
  if (micro_mutable_op_resolver.AddExpandDims() != kTfLiteOk) {
    Serial.println("return 1");
    return;
  }
  if (micro_mutable_op_resolver.AddConv2D() != kTfLiteOk) {
    Serial.println("return 2");
    return;
  }
  micro_mutable_op_resolver.AddRelu();
  if (micro_mutable_op_resolver.AddReshape() != kTfLiteOk) {
    Serial.println("return 3");
    return;
  }
  micro_mutable_op_resolver.AddFullyConnected();
  micro_mutable_op_resolver.AddLogistic();
  micro_mutable_op_resolver.AddDequantize();

  // Build an interpreter to run the model
  static tflite::MicroInterpreter static_interpreter(
    model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize);//,
    // error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
#if DEBUG
  Serial.println("ALLLOC");
#endif
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    // error_reporter->Report("AllocateTensors() failed");
    Serial.println("Allocation");
    while(1);
  }
  
  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);
  
#if DEBUG
  Serial.print("Number of dimensions: ");
  Serial.println(input->dims->size);
  Serial.print("Dim 1 size: ");
  Serial.println(input->dims->data[0]);
  Serial.print("Dim 2 size: ");
  Serial.println(input->dims->data[1]);
  Serial.print("Input type: ");
  Serial.println(input->type);
#endif

  while (!IMU.begin()) {
#if DEBUG
    Serial.println("Failed to initialize IMU!");
#endif
  };

  while(!(IMU.accelerationAvailable() & IMU.gyroscopeAvailable())){
#if DEBUG
    Serial.println("First Reading");
#endif
  }

#if DEBUG
  Serial.println("End");
#endif

  while (!GPS.fix) {
    readGPS();
#if DEBUG
    Serial.println("Not yet fixed");
#endif
  }
  
  runner.addTask(t3);
  t3.enable();

#if enable_BLE
  while (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
  };
  // set device name
  BLE.setDeviceName("NanoJun13");

  // set the local name peripheral advertises
  BLE.setLocalName("myService");
  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(myService);

  // add the characteristics to the service
  myService.addCharacteristic(myBLE_Sender);
  // add the service
  BLE.addService(myService);
  // start advertising
  BLE.advertise();
#endif

  digitalWrite(buzzerPin, HIGH);
  delay(200);
  digitalWrite(buzzerPin, LOW);
#if DEBUG
  Serial.println("setup finished");
#endif
} //setup

void loop() {
#if enable_BLE
  BLE.poll();
#endif
#if DEBUG
    // Serial.println("Iteration");
#endif
  int gps_ran=0;
  
  if (accBufRow < WIDTH){
    if (IMU.accelerationAvailable()) {
      // Read each sample, removing it from the device's FIFO buffer
      IMU.readAcceleration(current_acceleration_data[0],
                                current_acceleration_data[1],
                                current_acceleration_data[2]);
                                
      int accBufPos = accBufRow * NUM_CHANNELS;
      input->data.f[accBufPos+0] = current_acceleration_data[0];
      input->data.f[accBufPos+1] = current_acceleration_data[1];
      input->data.f[accBufPos+2] = current_acceleration_data[2];

      // Update the buffer position
      accBufRow++;
      
      if (gpsBufRow < WIDTH) {
        int gpsBufPos = gpsBufRow * NUM_CHANNELS;
        input->data.f[gpsBufPos+6] = (float)GPS.speed;
        input->data.f[gpsBufPos+7] = diff_speed;

        // Update the buffer position
        gpsBufRow++;
        gps_ran = 1;
      }
    }
  }
  
  if (gyrBufRow < WIDTH){
    if (IMU.gyroscopeAvailable()) {
      // Read each sample, removing it from the device's FIFO buffer
      IMU.readGyroscope(current_gyroscope_data[0], current_gyroscope_data[1],
                            current_gyroscope_data[2]);

      int gyrBufPos = gyrBufRow * NUM_CHANNELS;
      input->data.f[gyrBufPos+3] = current_gyroscope_data[0];
      input->data.f[gyrBufPos+4] = current_gyroscope_data[1];
      input->data.f[gyrBufPos+5] = current_gyroscope_data[2];

      // Update the buffer position
      gyrBufRow++;

      //GPS
      if (!gps_ran & (gpsBufRow < WIDTH)){
        int gpsBufPos = gpsBufRow * NUM_CHANNELS;
        input->data.f[gpsBufPos+6] = (float)GPS.speed;
        input->data.f[gpsBufPos+7] = diff_speed;

        // Update the buffer position
        gpsBufRow++;
      }
    }
  }

  runner.execute();

  if ((gyrBufRow != WIDTH) | (accBufRow != WIDTH)){
    return;
  } else if (gpsBufRow != WIDTH){
    while (gpsBufRow != WIDTH){
      int gpsBufPos = gpsBufRow * NUM_CHANNELS;
      input->data.f[gpsBufPos+6] = (float)GPS.speed;
      input->data.f[gpsBufPos+7] = diff_speed;

      // Update the buffer position
      gpsBufRow++;
    }
  }

  Serial.print("Invoking: ");
  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    // MicroPrintf("Invoke failed on x: %f\n", static_cast<double>(x));
    // MicroPrintf("Error");
    return;
  }

  float ot = output->data.f[0];
  Serial.println(ot);

  if (ot > 0.5){
#if enable_BLE
    taskSendGPS();
#endif
    digitalWrite(buzzerPin, HIGH);
    delay(20);
    digitalWrite(buzzerPin, LOW);
  }

  gyrBufRow = 0;
  accBufRow = 0;
  gpsBufRow = 0;
}