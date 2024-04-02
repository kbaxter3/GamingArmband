#include <HID-Project.h>

bool WASD = true;

bool pressed = false;
char input = 'n';
char last_input = 'n';

void setup() {
  // Sends a clean report to the host. This is important on any Arduino type.
  Keyboard.begin();
  
  // Begin USB CDC communication for logging
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("setup");

  Serial1.begin(115200);
  while(!Serial1) delay(10);

  Serial.println("esp32 uart setup");
  Serial1.println("yay");
}


void loop() {
  // put your main code here, to run repeatedly:
  if(Serial1.available()) { 
    input = Serial1.read();
    Serial.println(input);
    up_down_hold(last_input, input);
    last_input = input;
  }
}

void up_down_hold (char last, char current) {
  if(last != current) {
    Keyboard.releaseAll();
    
    if(current == 'u' ) {
      Keyboard.press(WASD ? 'w' : KEY_UP_ARROW);
    } else if (current == 'd') {
      Keyboard.press(WASD ? 's' : KEY_DOWN_ARROW);
    }
  }
}
