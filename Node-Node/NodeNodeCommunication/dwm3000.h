#include "spi.h"
#include "HardwareSerial.h"

// REGISTER FILE NAMES
#define GEN_CFG_AES_0 0x00
#define GEN_CFG_AES_1 0x01
#define STS_CFG       0x02
#define RX_TUNE       0x03
#define EXT_SYNC      0x04
#define GPIO_CTRL     0x05

#define RF_CONF       0x07

#define FS_CTRL       0x09

#define DIG_DIAG      0x0F

#define PMSC          0x11
#define RX_BUFFER_0   0x12
#define RX_BUFFER_1   0x13
#define TX_BUFFER     0x14
  #define TX_BUFFER_LEN 0x3000


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

#define TX_FCTRL_1        0x24  // first reg with frame control: offset, length
  #define TX_FCTRL_1_LEN  0x04
#define TX_FCTRL_2        0x28  // second reg with frame control: Fine PSR
  #define TX_FCTRL_2_LEN  0x04

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


// RF CONFIGURATION DATA OFFETS
#define RF_TX_CTRL_2      0x1C  // configuration of the TX channel register
  #define RF_TX_CTRL_2_LEN 0x04



// FREQUENCY CONTROL DATA OFFSETS
#define PLL_CFG           0x00  // configuration of the PLL
  #define PLL_CFG_LEN     0x04
#define PLL_CC            0x04  // coarse code for the PLL
  #define PLL_CC_LEN      0x04
#define PLL_CAL           0x08  // calibration config for the PLL
  #define PLL_CAL_LEN     0x04
#define XTAL              0x14  // calibration of the crystal
  #define XTAL_LEN        0x01


// Digital diagnostics register DATA OFFSETS
#define EVC_CTRL          0x00  // event counter control
  #define EVC_CTRL_LEN    0x04  
#define DIAG_TMC          0x24  // test mode control register
  #define DIAG_TMC_LEN    0x04
#define SPI_MODE          0x2C  // current mode for SPI control
  #define SPI_MODE_LEN    0x04
#define SYS_STATE         0x30  // the current state in the STATE MACHINE of the system
  #define SYS_STATE_LEN   0x04
#define FCMD_STAT         0x3C  // current state of the use of fast commands
  #define FCMD_STAT_LEN   0x01


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




#define CHANNEL5    0x1F3C  // reg config value for channel 5
#define CHANNEL9    0x0F3C  // reg config value for channel 9



#define AINIT2IDLE  0b100000000 // index of bit for the AINIT2IDLE in the SEQ_CTRL 


#define DEFAULT_SYS_CONFIG {0xC8, 0x11, 0x04, 0x00}


void setup_device();





class DeviceID {
  public:
    uint8_t revision = -1;
    uint8_t version = -1;

    uint8_t model = -1;
    uint16_t ridtag = -1;

    DeviceID(uint8_t rev, uint8_t ver, uint8_t mod, uint16_t tag) {
      revision = rev;
      version = ver;
      model = mod;
      ridtag = tag;
    };
};

// Function to get Device ID
DeviceID get_device_ID();
String get_device_ID_string();
bool check_device_ID();

// Function to perform hardware reset
void reset_DWIC();

// function to set the UWB channel
//  chan = 5: data transferrance on UWB channel 5
//  chan = 9: data transferrance on UWB channel 9
//  chan != 5,9: error, returns false
// returns true if value properly set
bool set_channel(int chan);

// function to set the device to IDLE_PLL, when in IDLE_RC
void set_to_idle();

class MachineState {
  public:
    uint8_t tx_state = -1;
    uint8_t rx_state = -1;
    uint8_t tse_state = -1;

    MachineState(uint8_t tx, uint8_t rx, uint8_t tse) {
      tx_state = tx;
      rx_state = rx;
      tse_state = tse;
    };
};
// function to get current machine states
MachineState get_machine_state();
uint8_t get_tx_state();
uint8_t get_rx_state();
uint8_t get_tse_state();



// functions to transmit a message from the device
bool transmit_message(String msg, int len);
class TransmitStatus{
  public:
    bool start_send = false;
    bool sent_preamble = false;
    bool sent_header = false;
    bool sent_frame = false;

    TransmitStatus(bool start, bool preamble, bool header, bool frame) {
      start_send = start;
      sent_preamble = preamble;
      sent_header = header;
      sent_frame = frame;
    };
};
TransmitStatus get_transmit_status();
bool has_started_transmit();
bool has_sent_frame();
void clear_transmit_status();


// functions to receive a message
String get_received_message();
class ReceiveStatus {
  public:
    bool preamble_detected = false;
    bool sfd_detected = false;
    bool header_detected = false;
    bool data_frame_ready = false;

    ReceiveStatus(bool preamble, bool sfd, bool header, bool data) {
      preamble_detected = preamble;
      sfd_detected = sfd;
      header_detected = header;
      data_frame_ready = data;
    }
};
ReceiveStatus get_receive_status();
bool has_received_frame();
void clear_receive_flags();


// LED FUNCTIONS
void enable_led_usage();
uint8_t* get_led_ctrl_reg();
void set_led_time(uint8_t time);
void enable_led_blink(bool enable);
void turn_on_leds(bool one, bool two, bool three, bool four);



void bit_or_arrays(uint8_t* data, uint8_t* mask, int len);
void bit_and_arrays(uint8_t* data, uint8_t* mask, int len);
void zero_array(uint8_t* data, int len);
void print_full_reg(uint8_t* data, int len);










