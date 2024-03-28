
void setup() {
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
    char byte = Serial1.read();
    Serial.write(&byte, 1);
  }
  
}
