#include <esp_now.h>
#include <WiFi.h>

// Structure to receive data, matches sender structure
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
    
    float emgData;
    
} struct_message;

struct_message myData;

void printData(char *dataName, int value, bool isEnd = false);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

void setup() {
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

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("setup");
  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);  
  delay(1000);
  delay(1000);
}

void loop() {
}

// print to Serial monitor
void printData(char *dataName, int value, bool isEnd) {
  Serial.print(dataName);
  Serial.print(":");
  Serial.print(value);
  if(isEnd)
    Serial.println();
   else
    Serial.print(",");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  // print emg data
  printData("EMG_Data",myData.emgData);
   
  // print orientation data to Serial monitor
  printData("x_Orientation", myData.x_Orientation);
  printData("y_Orientation", myData.y_Orientation);
  printData("z_Orientation", myData.z_Orientation);

  // print gyro data to Serial monitor
  printData("x_Gyro", myData.x_Gyro);
  printData("y_Gyro", myData.y_Gyro);
  printData("z_Gyro", myData.z_Gyro);

  // print linear acceleration data to Serial monitor
  printData("x_LinAccel", myData.x_LinAccel);
  printData("y_LinAccel", myData.y_LinAccel);
  printData("z_LinAccel", myData.z_LinAccel, true);

}
