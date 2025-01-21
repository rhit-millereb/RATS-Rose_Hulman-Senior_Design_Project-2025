#include "dwm3000.h"

void setup() {
  delay(3000);
  // setup a serial connection to the board
  Serial.begin(115200);
  // init the uno LED
  pinMode(LED_BUILTIN, OUTPUT);
  // show that the program started sucessfully by flashing LED 5 times
  showUploadComplete();

  // run function to reset the device, and boot it 
  setup_device();

  // function to set the channel to UWB 5
  while(!set_channel(5)) {
    Serial.println("Error: Unable to set channel");
    delay(5000);
  }

  /*

    Perform additional set up here

  */
  print_full_reg(read(PMSC, SEQ_CTRL, SEQ_CTRL_LEN), SEQ_CTRL_LEN);
  // set the device to idle mode
  set_to_idle();
}

void loop() {
  
  delay(1000);

  print_full_reg(read(PMSC, SEQ_CTRL, SEQ_CTRL_LEN), SEQ_CTRL_LEN);

  Serial.println(get_tse_state());
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


void print_full_reg(uint8_t* data, int len) {
  uint32_t full_reg = 0;

  for (int i = 0; i<len; i++) {
    full_reg += (((uint32_t) data[i]) << i*8);
  }

  Serial.println(full_reg, HEX);
}