#include <Arduino_LSM9DS1.h>
#include <Adafruit_GPS.h>
#include <SPI.h> 
#include <SD.h>
#include <TaskScheduler.h>

#define DEBUG false

// pin numbers
const int chipSelect = D10;
// const int buzzerPin = D2;

// what's the name of the hardware serial port?
#define GPSSerial Serial1

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

File myACCData;
File myGYRData;
File myGPSData;

String filename_ACC;
String filename_GYR;
String filename_GPS;

const int max_buf = 500*119;
int buffer_ACC_len = 0;
int buffer_GYR_len = 0;
int buffer_GPS_len = 0;

const int buf_size = int(1.1*max_buf);
char* buffer_ACC;
char* buffer_GYR;
char* buffer_GPS;

bool tone_on = false; 

uint32_t timer = millis();

float x, y, z;

// GPS bufferring
void taskBufGPS() {
  readGPS();

  if (GPS.fix) {
    // buffer_GPS_len += snprintf(buffer_GPS + buffer_GPS_len, max_buf - buffer_GPS_len, "%lu,%d,%d,%d,%d", millis(), GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds);
    // buffer_GPS_len += snprintf(buffer_GPS + buffer_GPS_len, max_buf - buffer_GPS_len, ",%.3f\n", GPS.speed * 1.852); // knot to km/h
    timer = millis();
    memcpy(buffer_GPS + buffer_GPS_len, &timer, sizeof(timer));
    buffer_GPS_len += sizeof(timer);

    uint16_t hour = (uint16_t)GPS.hour;
    memcpy(buffer_GPS + buffer_GPS_len, &hour, sizeof(hour));
    buffer_GPS_len += sizeof(hour);

    uint16_t minute = (uint16_t)GPS.minute;
    memcpy(buffer_GPS + buffer_GPS_len, &minute, sizeof(minute));
    buffer_GPS_len += sizeof(minute);

    uint16_t seconds = (uint16_t)GPS.seconds;
    memcpy(buffer_GPS + buffer_GPS_len, &seconds, sizeof(seconds));
    buffer_GPS_len += sizeof(seconds);

    uint16_t milliseconds = (uint16_t)GPS.milliseconds;
    memcpy(buffer_GPS + buffer_GPS_len, &milliseconds, sizeof(milliseconds));
    buffer_GPS_len += sizeof(milliseconds);

    float speed_knots = (float)GPS.speed; // convert knots to km/h *1.852
    memcpy(buffer_GPS + buffer_GPS_len, &speed_knots, sizeof(speed_knots));
    buffer_GPS_len += sizeof(speed_knots);
// #if DEBUG
//     Serial.print(timer);
//     Serial.print("\t");
//     Serial.print(hour);
//     Serial.print("\t");
//     Serial.print(minute);
//     Serial.print("\t");
//     Serial.print(seconds);
//     Serial.print("\t");
//     Serial.print(milliseconds);
//     Serial.print("\t");
//     Serial.println(speed_knots);
// #endif
  }
}

Task t3(210, TASK_FOREVER, &taskBufGPS);

char* NMEA_;
char c;

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

int nextFileNumber = 0;

void findMinUnusedFileNumber() {
  for (int i = 0; i <= 9999; i++) {
    char fileName[16];
    sprintf(fileName, "%04dACC.txt", i);  // check for any type of file, here "ACC"
    if (!SD.exists(fileName)) {
      nextFileNumber = i; // Store the next file number globally
      break;
    }
  }
}

String getNextFileName(String type) {
  char fileName[16];
  sprintf(fileName, "%04d%s.txt", nextFileNumber, type.c_str());
  return String(fileName);
}

void writeToFile(String filename, char* data, int len) {
#if DEBUG
  Serial.print(filename);
  Serial.print(" ");
  Serial.println(millis());
#endif
  File file = SD.open(filename, FILE_WRITE);
  if (file) {
    file.write((uint8_t *)data, len);
    file.flush();  // Ensure data is written and saved to the SD card
    // delay(10000);
    file.close();
#if DEBUG
  } else {
    Serial.println("Error writing to " + filename);
#endif
  }
#if DEBUG
  Serial.print(filename);
  Serial.print(" ");
  Serial.println(millis());
#endif
}

Scheduler runner;

void SetupIMU() {
  if (!IMU.begin()) {
#if DEBUG
    Serial.println("Failed to initialize IMU!");
#endif
    while (1);
  }
  IMU.setContinuousMode();

  float acceleration_sample_rate = IMU.accelerationSampleRate();
  float gyroscope_sample_rate = IMU.gyroscopeSampleRate();

  delay(1000);
#if DEBUG
  float rate_frac;
  float rate_int;
  rate_frac = modf(acceleration_sample_rate, &rate_int);
  Serial.print("Acceleration sample rate ");
  Serial.print(static_cast<int32_t>(rate_int));
  Serial.print(".");
  Serial.println(static_cast<int32_t>(rate_frac * 100));

  rate_frac = modf(gyroscope_sample_rate, &rate_int);
  Serial.print("Gyroscope sample rate ");
  Serial.print(static_cast<int32_t>(rate_int));
  Serial.print(".");
  Serial.println(static_cast<int32_t>(rate_frac * 100));
#endif  // DEBUG
  delay(1000);
}

// int n_bufs=0;

void setup() {
  Serial.begin(115200);
  // 9600 NMEA is the default baud rate for MTK - some use 4800
  GPS.begin(9600);
  delay(100);
  // Serial.print("Adafruit GPS logging start test!");
  // You can adjust which sentences to have the module emit, below
  // Default is RMC + GGA
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // Default is 1 Hz update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
  // Turn off antenna update nuisance data
  GPS.sendCommand("$PGCMD,33,0*6D");

  // pinMode(buzzerPin, OUTPUT);
  pinMode(chipSelect, OUTPUT);

  while (!SD.begin(chipSelect)) {
#if DEBUG
    Serial.print("Card failed, or not present");
#endif
    delay(100);
  }
#if DEBUG
  Serial.println("card initialized.");
#endif

  while (!IMU.begin()) {
#if DEBUG
    Serial.println("Failed to initialize IMU!");
#endif
  };
#if DEBUG
  Serial.println("IMU initialized.");
#endif
  //Accel
  findMinUnusedFileNumber();

  filename_ACC = getNextFileName("ACC");

//   myACCData = SD.open(filename_ACC, FILE_WRITE);

//   Serial.print(filename_ACC);
//   if (myACCData) {
//     myACCData.print("Time Launched,"); 
//     myACCData.print("x,"); 
//     myACCData.print("y,"); 
//     myACCData.println("z"); 
//     myACCData.close();
// #if DEBUG
//     Serial.println(": Created");
//   } else {
//     Serial.println(": Error opening ACCData.txt");
// #endif
//   }

  //Gyro
  filename_GYR = getNextFileName("GYR");
//   myGYRData = SD.open(filename_GYR, FILE_WRITE);

//   Serial.print(filename_GYR);
//   if (myGYRData) {
//     myGYRData.print("Time Launched,"); 
//     myGYRData.print("x,"); 
//     myGYRData.print("y,"); 
//     myGYRData.println("z"); 
//     myGYRData.close();
// #if DEBUG
//     Serial.println(": Created");
//   } else {
//     Serial.println(": Error opening GYRData.txt");
// #endif
//   }

  //GPS
  filename_GPS = getNextFileName("GPS");
//   myGPSData = SD.open(filename_GPS, FILE_WRITE);

//   Serial.print(filename_GPS);
//   if (myGPSData) {
//     myGPSData.print("Hour,Minute,Second,Millisecond,Speed\n"); 
//     myGPSData.println(GPS.speed * 1.852);
//     myGPSData.close();
// #if DEBUG
//     Serial.println(": Created");
//   } else {
//     Serial.println(": Error opening GPSData.txt");
// #endif
//   }
  
  runner.addTask(t3);
  t3.enable();

#if DEBUG
  Serial.println("memory allocating");
#endif
  buffer_ACC = new char[buf_size];
  buffer_GYR = new char[buf_size];
  buffer_GPS = new char[int(buf_size/10)];

#if DEBUG
  Serial.print("Address of buffer_ACC: "); Serial.println((uint32_t)buffer_ACC);
  Serial.print("Address of buffer_GYR: "); Serial.println((uint32_t)buffer_GYR);
  Serial.print("Address of buffer_GPS: "); Serial.println((uint32_t)buffer_GPS);
#endif

  memset(buffer_ACC, 0, buf_size);
  memset(buffer_GYR, 0, buf_size);
  memset(buffer_GPS, 0, int(buf_size/10));

  while (!GPS.fix) {
    readGPS();
  }

  delay(100);
#if DEBUG
  Serial.println("setup finished");
#endif
}

// void bufferring(char* buffer, int* stored_len, float* x){

// }

void loop() {
  //Accel
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    // buffer_ACC_len += snprintf(buffer_ACC + buffer_ACC_len, max_buf - buffer_ACC_len, "%lu,%f,%f,%f\n", millis(), x, y, z);
    timer = millis();
    memcpy(buffer_ACC + buffer_ACC_len, &timer, sizeof(timer));
    buffer_ACC_len += sizeof(timer);

    memcpy(buffer_ACC + buffer_ACC_len, &x, sizeof(x));
    buffer_ACC_len += sizeof(x);
  
    memcpy(buffer_ACC + buffer_ACC_len, &y, sizeof(y));
    buffer_ACC_len += sizeof(y);
    
    memcpy(buffer_ACC + buffer_ACC_len, &z, sizeof(z));
    buffer_ACC_len += sizeof(z);
  }

  //Gyro
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);
    // buffer_GYR += String(millis()) + "," + String(x) + "," + String(y) + "," + String(z) + "\n";
    // buffer_GYR_len += snprintf(buffer_GYR + buffer_GYR_len, max_buf - buffer_GYR_len, "%lu,%f,%f,%f\n", millis(), x, y, z);
    timer = millis();
    memcpy(buffer_GYR + buffer_GYR_len, &timer, sizeof(timer));
    buffer_GYR_len += sizeof(timer);

    memcpy(buffer_GYR + buffer_GYR_len, &x, sizeof(x));
    buffer_GYR_len += sizeof(x);
  
    memcpy(buffer_GYR + buffer_GYR_len, &y, sizeof(y));
    buffer_GYR_len += sizeof(y);
    
    memcpy(buffer_GYR + buffer_GYR_len, &z, sizeof(z));
    buffer_GYR_len += sizeof(z);
  }

  if (buffer_ACC_len > max_buf) {
    writeToFile(filename_GPS, buffer_GPS, buffer_GPS_len);
#if DEBUG
    Serial.println(buffer_GPS_len);
#endif
    memset(buffer_GPS, 0, buffer_GPS_len);
    buffer_GPS_len=0;

    writeToFile(filename_GYR, buffer_GYR, buffer_GYR_len);
    memset(buffer_GYR, 0, buffer_GYR_len);
    buffer_GYR_len=0;
    // delay(10000);

    writeToFile(filename_ACC, buffer_ACC, buffer_ACC_len);
#if DEBUG
    Serial.println(buffer_ACC_len);
#endif
    memset(buffer_ACC, 0, buffer_ACC_len);
    buffer_ACC_len=0;

    // n_bufs=0;
    nextFileNumber++;
    
    filename_ACC = getNextFileName("ACC");
    filename_GPS = getNextFileName("GPS");
    filename_GYR = getNextFileName("GYR");
  }

  //GPS
  runner.execute();
}

// String createGPSDataString() {
//   String data = "";
//   data += String(timer) + "," +
//           String(2000 + (int) GPS.year) + "," +
//           String(GPS.month) + "," +
//           String(GPS.day) + "," +
//           String(mergeTimeToMilliseconds((uint8_t) GPS.hour, (uint8_t) GPS.minute, (uint8_t) GPS.seconds, (uint16_t) GPS.milliseconds)) + "," +
//           String(GPS.latitude, 4) + "," +
//           String(GPS.lat) + "," +
//           String(GPS.longitude, 4) + "," +
//           String(GPS.lon) + "," +
//           String(GPS.altitude) + "," +
//           String(GPS.speed * 1.852) + "," +
//           String(GPS.angle) + "\n";
//   return data;
// }