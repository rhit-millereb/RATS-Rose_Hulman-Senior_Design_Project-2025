#include "dwm3000.h"

#define TX_EN true;


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

  // set the device to idle mode
  set_to_idle();

  // set sleep parameters
  uint8_t data[2] = {0b1100, 0b0001};
  write(0x0A, 0x14, data, 0x02);
}

void loop() {
  //print_full_reg(read(0x0A, 0x14, 0x2), 0x2);

  delay(1000);

  if (TX_EN) {
    transmis_message("Hello World!!", 13);
  }

  //fast_command(CMD_RX);

  //Serial.println(has_received_frame());

  transmit_message("Hello World!!", 13);

  Serial.println(get_tse_state());
}


void showUploadComplete() {
  int i = 0;
  while (i<4) {
    blink_led(100);
    i+=1;
  }

  Serial.println("Upload complete. Program starting...");
}

void blink_led(int period) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(period);
  digitalWrite(LED_BUILTIN, LOW);
  delay(preiod);
}
