#include <esp_now.h>
#include <WiFi.h>
#include <HardwareSerial.h>
#include <math.h>

#define RXp2 8
#define TXp2 7

int EMG_ON_CUTOFF = 2000;
int EMG_OFF_CUTOFF = 1000;
int MIN_PULSE_CUTOFF = 300;
int SHORT_PULSE_CUTOFF = 1000;
int SHORT_PULSE_INTERVAL_CUTOFF = 3000;
int LONG_PULSE_CUTOFF = 4000;
int ANGLE_DEADBAND = 5;
int ANGLE_CENTER = 15;
int HIGH_SAMPLE_CUTOFF = 4;

int emg_high_counter = 0;
int keyboard_mode = 1;

HardwareSerial MySerial1(1);

// structure to determine emg on off pulses
typedef struct emg_pulses {
  
  unsigned int last_pulse_start;
  unsigned int last_pulse_end;
  float last_emg_reading;
  int num_pulses;
  bool emg_on;
  
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
emg_pulses_t emg_pulses{ .emg_on = false};

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

  // Serial.println("setup");
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
  printData("x_Gyro", myData.x_Gyro);
  printData("y_Gyro", myData.y_Gyro);
  printData("z_Gyro", myData.z_Gyro);

  // print linear acceleration data to Serial monitor
  printData("x_LinAccel", myData.x_LinAccel);
  printData("y_LinAccel", myData.y_LinAccel);
  printData("z_LinAccel", myData.z_LinAccel);

  // print gravity data to Serial monitor
  // printData("x_Grav", myData.x_Gravity);
  // printData("y_Grav", myData.y_Gravity);
  // printData("z_Grav", myData.z_Gravity, true);

  update_pulses(&emg_pulses, myData.emgData);

  if(keyboard_mode == 1) {
    up_down_arrows(myData.x_Orientation, myData.y_Orientation, myData.z_Orientation, 
      myData.x_Gravity, myData.y_Gravity, myData.z_Gravity);
  } else {
    MySerial1.write("n");
    MySerial1.flush();
  }
  
  digitalWrite(10, keyboard_mode);
  
  delay(100);
}

void update_pulses(emg_pulses_t* emg_ptr, int emg_reading) {
  unsigned int current_time = millis();
  
  if(emg_high_counter != 0) {
    // make sure enough high samples before triggering a pulse
    if(emg_reading < EMG_ON_CUTOFF) emg_high_counter = 0;
    else emg_high_counter++;

    if(emg_high_counter > HIGH_SAMPLE_CUTOFF) {
      // treat as pulse start
      emg_ptr->last_pulse_start = current_time;
      emg_ptr->emg_on = true;
      emg_high_counter = 0;
    }
    
  } else if(emg_reading > EMG_ON_CUTOFF && !emg_ptr->emg_on) {
      // emg just turned on
      emg_high_counter = 1;
      
  } else if(emg_ptr->emg_on && emg_reading < EMG_OFF_CUTOFF){
    // emg just turned off
    emg_ptr->emg_on = false;
    unsigned int pulse_length = current_time - emg_ptr->last_pulse_start;
    
    if(pulse_length < SHORT_PULSE_CUTOFF && pulse_length > MIN_PULSE_CUTOFF) {
      // pulse was a short pulse
      // Serial.print("\nShort pulse: ");
      // Serial.println(pulse_length);
      emg_ptr->num_pulses++;

      if(emg_ptr->num_pulses > 1){
        //two short pulses
        if(current_time - emg_ptr->last_pulse_end < SHORT_PULSE_INTERVAL_CUTOFF){
          // two short pulses in a short time, switch keyboard mode on / off
          if(keyboard_mode == 0) keyboard_mode = 1;
          else if(keyboard_mode == 1) keyboard_mode = 0;
          emg_ptr->num_pulses = 0;
        } else {
          // two short pulses in a long amount of time
          emg_ptr->num_pulses = 1;
        }
      }
    } else if(pulse_length > MIN_PULSE_CUTOFF){
      // pulse was not a short pulse
      emg_ptr->num_pulses = 0;
      
      // Serial.print("\nLong pulse: ");
      // Serial.println(pulse_length);;
    }
    
    emg_ptr->last_pulse_end = current_time;
  }

  if(emg_ptr->emg_on && current_time - emg_ptr->last_pulse_start > LONG_PULSE_CUTOFF) {
    // actions during a long pulse
    // Serial.println("\nLong pulse actions");
    
  } 
  
  emg_ptr->last_emg_reading = emg_reading;
}

void up_down_arrows(float x, float y, float z, float grav_x, float grav_y, float grav_z){
  float mag_orientation = sqrt(sq(x)+sq(y)+sq(z));
  float mag_gravity = sqrt(sq(grav_x)+sq(grav_y)+sq(grav_z));
  float cos_angle = (-grav_y) / (mag_gravity);
  
  float angle = acos(cos_angle) * 180 / M_PI;

  if(angle > ANGLE_CENTER + ANGLE_DEADBAND) {
    MySerial1.write("u");
    MySerial1.flush();
  } else if(angle < ANGLE_CENTER - ANGLE_DEADBAND) {
    MySerial1.write("d");
    MySerial1.flush();
  } else {
    MySerial1.write("n");
    MySerial1.flush();
  }

  
  // printData("angle", angle);
  
}
