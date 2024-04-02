#include <esp_now.h>
#include <WiFi.h>
#include <HardwareSerial.h>
#include <math.h>

#define RXp2 8
#define TXp2 7

int EMG_ON_CUTOFF = 1000;
int EMG_OFF_CUTOFF = 400;
int SHORT_PULSE_CUTOFF = 3000;
int LONG_PULSE_CUTOFF = 5000;
int ANGLE_DEADBAND = 10;

int keyboard_mode = 1;

HardwareSerial MySerial1(1);

// structure to determine emg on off pulses
typedef struct emg_pulses {
  
  unsigned long last_pulse_start;
  unsigned long last_pulse_end;
  float last_emg_reading;
  int num_pulses;
  
} emg_pulses_t;

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

    float x_Gravity;
    float y_Gravity;
    float z_Gravity;
    
    float emgData;
    
} struct_message;

struct_message myData;
emg_pulses_t emg_pulses;

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
  
  MySerial1.begin(115200, SERIAL_8N1, RXp2, TXp2);
  while (!MySerial1) delay(10);
  
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
  MySerial1.println("ESP32 wifi up");
  MySerial1.flush();
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
  // printData("x_Gyro", myData.x_Gyro);
  // printData("y_Gyro", myData.y_Gyro);
  // printData("z_Gyro", myData.z_Gyro);

  // print linear acceleration data to Serial monitor
  // printData("x_LinAccel", myData.x_LinAccel);
  // printData("y_LinAccel", myData.y_LinAccel);
  // printData("z_LinAccel", myData.z_LinAccel, true);

  // print gravity data to Serial monitor
  printData("x_Grav", myData.x_Gravity);
  printData("y_Grav", myData.y_Gravity);
  printData("z_Grav", myData.z_Gravity);

  update_pulses(&emg_pulses, myData.emgData);

  if(keyboard_mode == 1) {
    up_down_arrows(myData.x_Orientation, myData.y_Orientation, myData.z_Orientation, 
      myData.x_Gravity, myData.y_Gravity, myData.z_Gravity);
  }
  
  delay(100);
}

void update_pulses(emg_pulses_t* emg_ptr, int emg_reading) {
  int current_time = millis();
  
  if(emg_reading > EMG_ON_CUTOFF){
    
    if(emg_ptr->last_emg_reading < EMG_ON_CUTOFF) {

      emg_ptr->last_pulse_start = current_time;
      
    } else if(current_time - emg_ptr->last_pulse_start > LONG_PULSE_CUTOFF) {
      // actions for long pulse
    }
  }

  if(emg_reading < EMG_OFF_CUTOFF && emg_ptr->last_emg_reading > EMG_OFF_CUTOFF){

    if(emg_ptr->num_pulses > 1 && (current_time - emg_ptr->last_pulse_end < SHORT_PULSE_CUTOFF)) {
      // actions for two short pulses
      
      if(keyboard_mode == 0) keyboard_mode = 1;
      if(keyboard_mode == 1) keyboard_mode = 0;
      
    }

    emg_ptr->num_pulses++;
    
    emg_ptr->last_pulse_end = current_time;
  }
  
  emg_ptr->last_emg_reading = emg_reading;
}

void up_down_arrows(float x, float y, float z, float grav_x, float grav_y, float grav_z){
  float mag_orientation = sqrt(sq(x)+sq(y)+sq(z));
  float mag_gravity = sqrt(sq(grav_x)+sq(grav_y)+sq(grav_z));
  float cos_angle = (-grav_y) / (mag_gravity);
  
  float angle = acos(cos_angle) * 180 / M_PI;

  if(angle > 90 + ANGLE_DEADBAND) {
    MySerial1.write("u");
    MySerial1.flush();
  } else if(angle < 90 - ANGLE_DEADBAND) {
    MySerial1.write("d");
    MySerial1.flush();
  } else {
    MySerial1.write("n");
    MySerial1.flush();
  }

  printData("keyboardMode", keyboard_mode);
  printData("angle", angle, true);
  
}
