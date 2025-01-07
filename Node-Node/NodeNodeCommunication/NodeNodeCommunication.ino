#include "dwm3000.h"

void setup() {
  delay(3000);
  // setup a serial connection to the board
  Serial.begin(115200);
  // init the uno LED
  pinMode(LED_BUILTIN, OUTPUT);
  // show that the program started sucessfully by flashing LED 5 times
  showUploadComplete();

  setup_device();

  Serial.println(get_device_ID());

  enable_led_usage();
  set_led_time(0b00100000);
  enable_led_blink(true);
  turn_on_leds(true, true, true, true);

  uint8_t* reg = get_led_ctrl_reg();
  for (int i=0; i<4; i++) {
    Serial.println(reg[i], BIN);
  }
}

void loop() {
  
  delay(1000);
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
