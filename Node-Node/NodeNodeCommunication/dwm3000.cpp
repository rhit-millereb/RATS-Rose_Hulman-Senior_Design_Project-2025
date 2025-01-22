#include "dwm3000.h"


void setup_device() {
  // SPI Setup Stage
  Serial.println("Starting SPI...");
  setup_spi();
  Serial.println("SPI Setup Successful.");

  // Hardware Reset Stage
  Serial.println("Resetting device...");
  reset_DWIC();
  Serial.println("Device reset, awaiting wake...");
  
  delay(10);

  while(!digitalRead(EXTON)) {
    Serial.println("\tWaiting...");
    delay(10);
  }

  Serial.println("Device Reset Successful.");

  delay(1000);

  
  // Check Data Accuracy Stage
  Serial.println("Checking device ID...");
  while (!check_device_ID()) {
    Serial.println("Error with device, checking again in 5sec ...");
    delay(5000);
  }
  Serial.println("Device ID OK.");

  // Await Boot Stage
  Serial.println("Awaiting Device to Boot...");
  delay(1);
  while(get_tse_state() != 1) {
    Serial.print("\tWaiting...");
    delay(10);
  }
  Serial.println("Device Booted Successfully.");

  // Enable LEDs Stage
  enable_led_usage();
  enable_led_blink(true);
  turn_on_leds(true, true, true, true);

  // Complete
  Serial.println("DWM3000 Connected Successfully!");
}


DeviceID get_device_ID() {
  // see DWM3000 user manual, pg 74 to see the DEV_ID fields
  uint8_t* data = read(GEN_CFG_AES_0, DEV_ID, DEV_ID_LEN);

  uint8_t rev = data[0] & 0x0F;

  uint8_t ver = data[0] & 0xF0;

  uint8_t model = data[1];

  uint16_t tag = data[2] + (data[3] << 8);

  return DeviceID(rev, ver, model, tag);
}

String get_device_ID_string() {
  DeviceID data = get_device_ID();

  String msg = "------DWM 3000 Info------\n";
  msg += "Rev:    0d" + String(data.revision) + "\n";
  msg += "Ver:    0d" + String(data.version) + "\n";
  msg += "Model:  0x" + String(data.model, HEX) + "\n";

  String tag_msg = String(data.ridtag, HEX);
  tag_msg.toUpperCase();
  msg += "Tag:    0x" + (tag_msg) + "\n";
}

bool check_device_ID() {
  DeviceID data = get_device_ID();

  // check that the device has a model num of three
  if (data.model != 3) {
    Serial.print("Error: DWM3000 must have model num \'0d03\' but instead found: 0d");
    Serial.println(data.model, DEC);
    return false;
  }

  // check that the device has the correct RID Tag
  if (data.ridtag != 0xDECA) {
    Serial.print("Error: DWM3000 must have RID Tag \'0xDECA\' but instead found: 0d");
    Serial.println(data.ridtag, HEX);
    return false;
  }

  // all other values are allowed, return true
  return true;
}


void reset_DWIC(void) {

  // set the reset pin to output
  pinMode(RESET, OUTPUT);

  // set the reset pin low
  digitalWrite(RESET, LOW);

  // wait 10 us
  delayMicroseconds(10);

  // set the pin back to high
  digitalWrite(RESET, HIGH);
}



MachineState get_machine_state() {
  // read the sys_state register
  uint8_t* data = read(DIG_DIAG, SYS_STATE, SYS_STATE_LEN);

  // set the btyes to their respective values
  //  see user manual, pg 216 for details
  return MachineState(data[0], data[1], data[2]);
}

uint8_t get_tx_state() {
  // get data from sys state register
  MachineState data = get_machine_state();

  return data.tx_state;
}

uint8_t get_rx_state() {
  // get data from sys state register
  MachineState data = get_machine_state();

  return data.rx_state;
}

uint8_t get_tse_state() {
  // get data from the sys state register
  MachineState data = get_machine_state();

  return data.tse_state;
}



bool set_channel(int chan) {
  if (chan != 5 && chan != 9) {
    Serial.println("Error: Invalid channel setting!");
    return false;
  }

  uint8_t data[4] = {0,0,0,0};

  if (chan == 5) {
    data[0] = CHANNEL5 & 0x00FF;
    data[1] = (CHANNEL5 & 0xFF00) >> 8;
  } else {
    data[0] = CHANNEL9 & 0x00FF;
    data[1] = (CHANNEL9 & 0xFF00) >> 8;
  }

  write(FS_CTRL, PLL_CFG, data, PLL_CFG_LEN);

  delay(1);

  uint8_t* read_data = read(FS_CTRL, PLL_CFG, PLL_CFG_LEN);

  if (read_data[0] != data[0] || read_data[1] != data[1]) {
    Serial.println("Error: Channel data not properly written.");
    return false;
  } 
  
  return true;
}


void set_to_idle() {
  // read the current data from the SEQ_CTRL reg 
  uint8_t* data = read(PMSC, SEQ_CTRL, SEQ_CTRL_LEN);

  // mask in the IDLE bit
  data[1] |= (AINIT2IDLE > 8);

  write(PMSC, SEQ_CTRL, data, SEQ_CTRL_LEN);
}




bool transmit_message(String msg, int len) {
  // ensure the data fits size requirements
  if (len <= 0) {
    Serial.println("Error: Invalid transmit frame length");
    return false;
  } else if (len >= 127) {
    Serial.println("Error: Transmit frame too large (127 bytes max)");
    return false;
  }

  // create data of size len+1 to include null terminator and end-of-text terminator
  uint8_t data[len+1] = {};

  msg.toCharArray(data, len+1, 0);

  // write the message to the transmit buffer
  write(TX_BUFFER, 0x0, data, len+1);



  // configure transmit parameters
  uint8_t* config_data = read(GEN_CFG_AES_0, TX_FCTRL_1, TX_FCTRL_1_LEN);
  // combine data into one 32bit int
  uint32_t tx_fctrl = 0;
  for (int i=0; i<4; i++) {
    tx_fctrl += (((uint32_t) config_data[i]) << (i*8));
  }

  // perform operations to clear previous values and set new ones
  // see page 85 of user manual for details
  // set frame length
  tx_fctrl &= ~0x1FF;
  tx_fctrl += len;
  // leave all other values at default

  // write data back to register
  write(GEN_CFG_AES_0, TX_FCTRL_1, data, TX_FCTRL_1_LEN);



  // send transmit start command
  fast_command(CMD_TX);


  // wait until message is sent


  return true;
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
  uint8_t flag = (uint8_t)enable;

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
  uint8_t mask = ((uint8_t)one) + (((uint8_t)two) << 1) + (((uint8_t)three) << 2) + (((uint8_t)four) << 3);

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
  for (int i = 0; i < len; i++) {
    data[i] = data[i] | mask[i];
  }
}

void bit_and_arrays(uint8_t* data, uint8_t* mask, int len) {
  for (int i = 0; i < len; i++) {
    data[i] = data[i] & mask[i];
  }
}