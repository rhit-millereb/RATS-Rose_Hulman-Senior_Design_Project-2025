
#include "spi.h"


uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void setup_spi() {
  // set the IRQ to input
  pinMode(IRQ, INPUT);

  // set the chip select pin to output
  pinMode(SS, OUTPUT);

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

  // start an SPI transation with settings
  SPI.beginTransaction(SPISettings(130000, MSBFIRST, SPI_MODE0));
  // set the chip select to LOW
  digitalWrite(SS, LOW);

  delay(5);

  // write the header over the SPI wire
  for (int i=0; i<2; i++) {
    SPI.transfer(header[i]);
  }
}

void end() {
  delay(5);
  // set the chip select to high
  digitalWrite(SS, HIGH);
  // end the transaction
  SPI.endTransaction();
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
}

uint8_t* read(uint8_t reg, uint8_t offset, uint8_t bytes_to_read) {
  uint8_t* data = new uint8_t[bytes_to_read];

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


