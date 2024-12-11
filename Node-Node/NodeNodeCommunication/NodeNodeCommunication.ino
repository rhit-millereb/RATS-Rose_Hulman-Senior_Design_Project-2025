void setup() {
  // setup a serial connection to the board
  Serial.begin(115200);
  // init the uno LED
  pinMode(LED_BUILTIN, OUTPUT);
  // show that the program started sucessfully by flashing LED 5 times
  showUploadComplete();
}

void loop() {
  
}


void showUploadComplete() {
  int i = 0;
  while (i<4) {
    digitalWrite(LED_BUILTIN, 1);
    delay(100);
    digitalWrite(LED_BUILTIN, 0);
    delay(100);
    i+=1;
  }

  Serial.println("Upload complete. Program starting...");
}
