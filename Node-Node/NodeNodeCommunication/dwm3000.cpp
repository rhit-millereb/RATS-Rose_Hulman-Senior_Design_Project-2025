#include "dwm3000.h"


void setup_device() {
  setup_spi();
}

void reset_device() {
  // set the SYS_CLK in CLK_CTRL to 01

  // clear SOFTRESET to all zeros

  // set SOFTRESET to all ones
}

String get_device_ID() {
  // see DWM3000 user manual, pg 74 to see the DEV_ID fields
  uint8_t* data = read(GEN_CFG_AES_0, DEV_ID, DEV_ID_LEN);

  uint8_t rev = data[0] & 0x0F;
  uint8_t ver = data[0] & 0xF0;

  uint8_t model = data[1];
  if (model != 3) {
    Serial.print("Error: DWM3000 Must have Model Num \'0d3\' but instead found: 0d");
    Serial.println(model);
    return "Error";
  }

  uint16_t tag = data[2] + (data[3] << 8);
  if (tag != 0xDECA) {
    Serial.print("Error: DECA devices must have model number \'0xDECA\' but found: 0d");
    Serial.println(tag, HEX);
    return "Error";
  }

  String msg = "------DWM 3000 Info------\n";
  msg += "Rev:    0d"+String(rev)+"\n";
  msg += "Ver:    0d"+String(ver)+"\n";
  msg += "Model:  0x"+String(model, HEX)+"\n";

  String tag_msg = String(tag, HEX);
  tag_msg.toUpperCase();
  msg += "Tag:    0x"+(tag_msg)+"\n";

  return msg;
}

uint8_t* get_led_ctrl_reg() {
  // get the current state of the register
  return read(PMSC, LED_CTRL, LED_CTRL_LEN);
}

void enable_led_usage() {
  // SET UP THE GPIO TO ALLOW LED DRIVING
  uint8_t* reg = read(GPIO_CTRL, GPIO_MODE, GPIO_MODE_LEN);

  // mask the values for the RXOKLED
  reg[0] = (reg[0] & 0xF8) | 0x1;
  // mask the values for the SFDLED
  reg[0] = (reg[0] & 0xC7) | (0x1 << 3);
  // mask the values for the RXLED
  reg[0] = (reg[0] & 0x3F) | (0x1 << 6);
  // mask the values for the TXLED
  reg[1] = (reg[1] & 0xF1) | (0x1 << 1);

  // write back to register
  write(GPIO_CTRL, GPIO_MODE, reg, GPIO_MODE_LEN);

  // ENABLE THE CLOCK
  reg = read(PMSC, CLK_CTRL, CLK_CTRL_LEN);

  // toggle the DCLK enable flag
  reg[2] |= 0x1 << 2;

  //write data back to register
  write(PMSC, CLK_CTRL, reg, CLK_CTRL_LEN);
}

void set_led_time(uint8_t time) {
  // get the LED CTRL register
  uint8_t* reg = get_led_ctrl_reg();

  // delay time is the first eight bits, bit and with the given time
  reg[0] &= time;

  // write the data to the register
  write(PMSC, LED_CTRL, reg, LED_CTRL_LEN);

  // read back from the register
  reg = read(PMSC, LED_CTRL, LED_CTRL_LEN);
  // confirm the data was written
  if (reg[0] != time) {
    Serial.println("Error: Delay time was not written to register");
  } else {
    Serial.println("Success: Delay time set to board");
  }
}

void enable_led_blink(bool enable) {
  uint8_t* reg = get_led_ctrl_reg();

  // convert bool to uint8
  uint8_t flag = (uint8_t) enable;

  // bit 8 of the reg is the enable flag, and with the user given flag
  reg[1] = ((reg[1] & 0xFE) | flag);

  // write the new data to the register
  write(PMSC, LED_CTRL, reg, LED_CTRL_LEN);

  reg = read(PMSC, LED_CTRL, LED_CTRL_LEN);
  if (reg[1] & 0x01 != enable) {
    Serial.println("Error: Enable was not written");
  } else {
    Serial.println("Success: Enable bit set");
  }
}

void turn_on_leds(bool one, bool two, bool three, bool four) {
  // get the current register value
  uint8_t* reg = get_led_ctrl_reg();

  // get a mask of the given values
  uint8_t mask = ((uint8_t) one) + (((uint8_t) two) << 1) + (((uint8_t) three) << 2) + (((uint8_t) four) << 3);

  // mask the third byte of the register
  reg[2] = (reg[2] & 0xF0) | mask;

  // write data to the register
  write(PMSC, LED_CTRL, reg, LED_CTRL_LEN);

  //read data back from the register
  reg = read(PMSC, LED_CTRL, LED_CTRL_LEN);
  if ((reg[2] & 0x0F) != mask) {
    Serial.println("Error: LED toggles not written");
  } else {
    Serial.println("Success: LED toggles set");
  }
}

void bit_or_arrays(uint8_t* data, uint8_t* mask, int len) {
  for (int i=0; i<len; i++) {
    data[i] = data[i] | mask[i];
  }
}

void bit_and_arrays(uint8_t* data, uint8_t* mask, int len) {
  for (int i=0; i<len; i++) {
    data[i] = data[i] & mask[i];
  }
}