#include <SPI.h>
#include <string.h>
#include <HardwareSerial.h>

// PIN NUMBERS
#define SCLK    13
#define MOSI    12
#define MISO    11
#define SS      10
#define WAKEUP  9
#define IRQ     8
#define RESET   7

#define EXTON A0


// FAST COMMAND CODES
#define CMD_TXRXOFF     0x0   // puts device in IDLE state, clears all events
#define CMD_TX          0x1   // immediate transmission of data
#define CMD_RX          0x2   // enable RX immediately

#define CMD_TX_W4R      0xC   // transmit immediately, then start reciever

#define CMD_CLEAR_IRQS  0x12  // clear all interrupt events


#define SPI_LOW_SPEED 2000000     // 2 MHz low speed transfer
#define SPI_FULL_SPEED 38000000   // 15 MHz full speed transfer

extern bool full_speed;

void setup_spi();

void start(uint8_t reg, uint8_t offset, bool write);
void end();

void write(uint8_t reg, uint8_t offset, uint8_t* data, int len);

uint8_t* read(uint8_t reg, uint8_t offset, uint8_t* data, uint8_t bytes_to_read);

void fast_command(uint8_t cmd);