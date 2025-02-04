
#include "spi.h"


bool full_speed = false;


uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void setup_spi() {
  // set the chip select pin to output
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
  
  // set the master out, slave in to output (messages sent on this pin)
  pinMode(MOSI, OUTPUT);
  // set the master in, slave out to input (messages received on this pin)
  pinMode(MISO, INPUT);

  // init the SPI communication library
  SPI.begin();
}

void start(uint8_t reg, uint8_t offset, bool write) {
  if (reg > 0x1F) {
    Serial.println("Error: Invalid Register Address");
    return;
  }

  if (offset > 0x7F) {
    Serial.println("Error: Invalid Register Offset");
    return;
  }

  uint8_t* header = new uint8_t[2];

  // prepare the first octet
  header[0] = ((uint8_t) write) << 7;
  header[0] |= 0b01000000; // set the full address flag
  header[0] |= (reg << 1); // add in the register address
  header[0] |= offset >> 6; //(reversed_offset >> 1); // or in the 7th bit of the offset
  
  //prepare the second octet
  header[1] = 0x00;
  header[1] |= (offset) << 2; // add in the 6 remaining bits in register offset

  // determine the transfer speed that should be used
  int speed = 0;
  if (full_speed) {
    speed = SPI_FULL_SPEED;
  } else {
    speed = SPI_LOW_SPEED;
  }

  // start an SPI transation with settings
  SPI.beginTransaction(SPISettings(speed, MSBFIRST, SPI_MODE0));
  // set the chip select to LOW
  digitalWrite(SS, LOW);

  // write the header over the SPI wire
  for (int i=0; i<2; i++) {
    SPI.transfer(header[i]);
  }

  free(header);

  return;
}

void end() {
  // set the chip select to high
  digitalWrite(SS, HIGH);
  // end the transaction
  SPI.endTransaction();

  return;
}

void write(uint8_t reg, uint8_t offset, uint8_t* data, int len) {
  // run function to start the transaction and send the header
  start(reg, offset, true);

  // send the data to the device
  for (int i=0; i<len; i++) {
    SPI.transfer(data[i]);
  }

  // run function to end the transaction
  end();

  return;
}

uint8_t* read(uint8_t reg, uint8_t offset, uint8_t* data, uint8_t bytes_to_read) {
  // run function to start the transation and tell device the register to read
  start(reg, offset, false);

  // read the data from the device
  for (int i=0; i<bytes_to_read; i++) {
    data[i] = (SPI.transfer(0x00));
  }

  // run function to end the transation
  end();

  return data;
}


void fast_command(uint8_t cmd) {
  // ensure the command is valid
  if (cmd > 0x13) {
    Serial.println("Error: Invalid fast command");
    return;
  }

  // create the send byte
  uint8_t data = 0;

  // set the first two bits to fast header
  data |= 0b10000000;
  // set the trailer bit
  data |= 0b00000001;

  // set the command code
  data |= (cmd << 1);

  // start an SPI transation with settings
  SPI.beginTransaction(SPISettings(130000, MSBFIRST, SPI_MODE0));
  // set the chip select to LOW
  digitalWrite(SS, LOW);

  // transfer the fast command
  SPI.transfer(data);

  end();

  return;
}

