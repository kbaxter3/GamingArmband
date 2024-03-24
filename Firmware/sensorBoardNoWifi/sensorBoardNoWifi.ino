#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define EMG_PIN A2

uint16_t BNO055_SAMPLERATE_DELAY_MS = 100;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

void printData(char *dataName, int value, bool isEnd = false);

void setup() {
  //set up EMG input pin
  pinMode(EMG_PIN, INPUT);
  
  // initialize serial port
  Serial.begin(115200);
  while (!Serial) delay(10);


  // Initialise the sensor
  if (!bno.begin())
  {
    // There was a problem detecting the BNO055 ... check your connections 
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  delay(1000);
}

void loop() {
  //define variables
  sensors_event_t orientationData , angVelocityData , linearAccelData;
  uint16_t emgData;

  // get and print EMG data to Serial monitor
  emgData = analogRead(EMG_PIN);
  printData("EMG_Data",emgData);

  // get IMU data
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);

  // print orientation data to Serial monitor
  printData("x_Orientation", orientationData.orientation.x);
  printData("y_Orientation", orientationData.orientation.y);
  printData("z_Orientation", orientationData.orientation.z);

  // print gyro data to Serial monitor
  printData("x_Gyro", angVelocityData.gyro.x);
  printData("y_Gyro", angVelocityData.gyro.y);
  printData("z_Gyro", angVelocityData.gyro.z);

  // print linear acceleration data to Serial monitor
  printData("x_LinAccel", linearAccelData.acceleration.x);
  printData("y_LinAccel", linearAccelData.acceleration.y);
  printData("z_LinAccel", linearAccelData.acceleration.z, true);

  delay(BNO055_SAMPLERATE_DELAY_MS);
}


void printData(char *dataName, int value, bool isEnd) {
  Serial.print(dataName);
  Serial.print(":");
  Serial.print(value);
  if(isEnd)
    Serial.println();
   else
    Serial.print(",");
}
