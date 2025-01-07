#include "spi.h"

// REGISTER FILE NAMES
#define GEN_CFG_AES_0 0x00
#define GEN_CFG_AES_1 0x01
#define STS_CFG       0x02
#define RX_TUNE       0x03
#define EXT_SYNC      0x04
#define GPIO_CTRL     0x05

#define PMSC          0x11
#define RX_BUFFER_0   0x12
#define RX_BUFFER_1   0x13
#define TX_BUFFER     0x14


//GEN CONFIG DATA OFFSETS
// GEN_CFG 0 File
#define DEV_ID            0x00  // id for the device, should always be 0
  #define DEV_ID_LEN      0x04
#define EUI_64            0x04  // extended device identifier
  #define EUI_64_LEN      0x08
#define PANADR            0x0C  // PAN identifier and short address
  #define PANADR_LEN      0x04
#define SYS_CFG           0x10  // system configuration flags
  #define SYS_CFG_LEN     0x04
#define FF_CFG            0x14  // frame filter config flags
  #define FF_CFG_LEN      0x04
#define SPI_RD_CRC        0x18  // SPI CRC read status
  #define SPI_RD_CRC_LEN  0x04
#define SYS_TIME          0x1C  // System time counter
  #define SYS_TIME_LEN    0x08

#define SYS_CTRL          0x38  // system control flags
  #define SYS_CTL_LEN     0x04
#define SYS_ENABLE        0x3C  // system event enable mask
  #define SYS_ENABLE_LEN  0x08
#define SYS_STATUS        0x44  // system even status flags
  #define SYS_STATUS_LEN  0x08
#define RX_FINFO          0x4C  // recieved frame information
  #define RX_FINFO_LEN    0x18
#define RX_TIME           0x64  // received frame time stamp
  #define RX_TIME_LEN     0x10
#define TX_TIME           0x74  // transmitted frame time stamp
  #define TX_TIME_LEN     0x10

// GEN_CFG 1 File
#define CHAN_CTRL         0x14  // channel control register flags
  #define CHAN_CTRL_LEN   0x02
#define SPI_COLLISION     0x20  // status on SPI collisions
  #define SPI_CLN_LEN     0x04
//

// GPIO CONTROL AND STATUS OFFSETS
#define GPIO_MODE         0x00  // mode control register
  #define GPIO_MODE_LEN   0x04 
#define GPIO_PULL_EN      0x04  // drive strength and pull control
  #define GPIO_PULL_LEN   0x04  
#define GPIO_DIR          0x08  // direction control register
  #define GPIO_DIR_LEN    0x04
#define GPIO_OUT          0x0C  // data output register
  #define GPIO_OUT_LEN    0x04
#define GPIO_IRQE         0x10  // interrupt enable register 
  #define GPIO_IRQE_LEN   0x04
#define GPIO_ISTS         0x14  // interrupt status register
  #define GPIO_ISTS_LEN   0x04

// Power management, timing, and sequence control (PMSC) DATA OFFSETS
#define SOFT_RST          0x00  // soft reset of the device blocks
  #define SOFT_RST_LEN    0x04
#define CLK_CTRL          0x04  // control flags for the clock
  #define CLK_CTRL_LEN    0x04
#define SEQ_CTRL          0x08  // control register
  #define SEQ_CTRL_LEN    0x04
#define TXFSEQ            0x12  // fine grain transmission sequencing control flags
  #define TXFSEQ_LEN      0x04
#define LED_CTRL          0x16  // control flags for the LED
  #define LED_CTRL_LEN    0x04
#define RX_SNIFF          0x1A  // config flags for sniff mode
  #define RX_SNIFF_LEN    0x05  
#define BIAS_CTRL         0x1F  // calibration values of the analog blocks
  #define BIAS_CTRL_LEN   0x01
//


void setup_device();

String get_device_ID();

void reset_device();

void enable_led_usage();
uint8_t* get_led_ctrl_reg();
void set_led_time(uint8_t time);
void enable_led_blink(bool enable);
void turn_on_leds(bool one, bool two, bool three, bool four);

void bit_or_arrays(uint8_t* data, uint8_t* mask, int len);

void bit_and_arrays(uint8_t* data, uint8_t* mask, int len);











