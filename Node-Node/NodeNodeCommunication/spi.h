#include <SPI.h>
#include <string.h>
#include <HardwareSerial.h>

#define SCLK    13
#define MOSI    12
#define MISO    11
#define SS      10
#define WAKEUP  9
#define IRQ     8
#define RESET   7

#define EXTON A0

void setup_spi();

void start(uint8_t reg, uint8_t offset, bool write);
void end();

void write(uint8_t reg, uint8_t offset, uint8_t* data, int len);

uint8_t* read(uint8_t reg, uint8_t offset, uint8_t bytes_to_read);