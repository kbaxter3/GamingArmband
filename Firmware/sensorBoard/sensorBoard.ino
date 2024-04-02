#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <esp_now.h>
#include <WiFi.h>

#define EMG_PIN A2

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x70, 0x04, 0x1D, 0x2A, 0xF4, 0x54};

uint16_t BNO055_SAMPLERATE_DELAY_MS = 100;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

// Structure to send data, matches receiver structure
typedef struct struct_message {
    float x_Orientation;
    float y_Orientation;
    float z_Orientation;
    
    float x_Gyro;
    float y_Gyro;
    float z_Gyro;
    
    float x_LinAccel;
    float y_LinAccel;
    float z_LinAccel;

    float x_Gravity;
    float y_Gravity;
    float z_Gravity;
    
    float emgData;
    
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  //set up EMG input pin
  pinMode(EMG_PIN, INPUT);
  pinMode(10, OUTPUT);

  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);  
  delay(1000);
  
  // initialize serial port
  Serial.begin(115200);
  while (!Serial) delay(10);

  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);  
  delay(1000);
  
  // Initialise the sensor
  if (!bno.begin())
  {
    // There was a problem detecting the BNO055 ... check your connections 
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);  
  delay(1000);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);  
  delay(1000);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);  
  delay(1000);
  Serial.println("setup");
  delay(1000);
}

void loop() {
  //define variables
  sensors_event_t orientationData , angVelocityData , linearAccelData, gravityData;
  uint16_t emgData;

  // get and print EMG data to Serial monitor
  emgData = analogRead(EMG_PIN);
  myData.emgData = emgData;

  // get IMU data
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
  bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);
  
  // store orientation data to send
  myData.x_Orientation = orientationData.orientation.x;
  myData.y_Orientation = orientationData.orientation.y;
  myData.z_Orientation = orientationData.orientation.z;
  
  // store gyro data to send
  myData.x_Gyro = angVelocityData.gyro.x;
  myData.y_Gyro = angVelocityData.gyro.y;
  myData.z_Gyro = angVelocityData.gyro.z;

  // store lin accel data to send
  myData.x_LinAccel = linearAccelData.acceleration.x;
  myData.y_LinAccel = linearAccelData.acceleration.y;
  myData.z_LinAccel = linearAccelData.acceleration.z;

  // store gravity data to send
  myData.x_Gravity = gravityData.acceleration.x;
  myData.y_Gravity = gravityData.acceleration.y;
  myData.z_Gravity = gravityData.acceleration.z;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
