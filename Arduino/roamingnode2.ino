//roamingnode2

#include "dw3000.h"

#define PIN_RST 15 //27
#define PIN_IRQ 4 //34
#define PIN_SS 5 //4

#define RNG_DELAY_MS 1000
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385
#define ALL_MSG_COMMON_LEN 10
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#define RESP_RX_TIMEOUT_UUS 400

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
  5,                /* Channel number. */
  DWT_PLEN_128,     /* Preamble length. Used in TX only. */
  DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
  9,                /* TX preamble code. Used in TX only. */
  9,                /* RX preamble code. Used in RX only. */
  1,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
  DWT_BR_6M8,       /* Data rate. */
  DWT_PHRMODE_STD,  /* PHY header mode. */
  DWT_PHRRATE_STD,  /* PHY header rate. */
  (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
  DWT_STS_MODE_OFF, /* STS disabled */
  DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
  DWT_PDOA_M0       /* PDOA mode off */
};

uint8_t tx_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
uint8_t rx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t frame_seq_nb = 0;
static uint8_t rx_buffer[20];
static uint32_t status_reg = 0;
extern dwt_txconfig_t txconfig_options;
unsigned long time_current = 0;
unsigned long time_offset;
unsigned long nextRangeCheck = 0;   

void setup()
{
  UART_init();  // Or whatever your serial init is

  spiBegin(PIN_IRQ, PIN_RST); 
  spiSelect(PIN_SS);

  delay(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

  while (!dwt_checkidlerc()) // Need to make sure DW IC is in IDLE_RC before proceeding
  {
    UART_puts("IDLE FAILED\r\n");
    while (1)
      ;
  }

  if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
  {
    UART_puts("INIT FAILED\r\n");
    while (1)
      ;
  }

  // Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards.
  dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

  /* Configure DW IC. See NOTE 6 below. */
  if (dwt_configure(&config)) // if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device
  {
    UART_puts("CONFIG FAILED\r\n");
    while (1)
      ;
  }

  /* Configure the TX spectrum parameters (power, PG delay and PG count) */
  dwt_configuretxrf(&txconfig_options);

  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);

  Serial.println("Roaming Node");
  Serial.println("Setup over........");
}


void loop()
{
  unsigned long now = millis();
  if (now >= nextRangeCheck) {
    TimeTx(); // send range request to anchor node
    nextRangeCheck = now + RNG_DELAY_MS;
  }

  // after transmit, wait for anchor respnse
  TimeRx();
}


void TimeTx()
{

  char tx_msg[30];
  unsigned long tNow = millis();
  unsigned long combinedTime = tNow + time_offset;

  snprintf(tx_msg, sizeof(tx_msg), "AR %lu", combinedTime);

  Serial.print("TimeTx: sending: ");
  Serial.println(tx_msg);

  /* Write frame data to DW IC and prepare transmission. See NOTE 7 below. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
  size_t msg_len = strlen(tx_msg) + 1; // +1 for null  term

  dwt_writetxdata(msg_len, (uint8_t *)tx_msg, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(msg_len, 0, 0); /* Zero offset in TX buffer, ranging. */

   /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
  * set by dwt_setrxaftertxdelay() has elapsed. */
  dwt_starttx(DWT_START_TX_IMMEDIATE);

  while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK)) {
  
  }

  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

  Serial.println("TimeTx finish");
}

void TimeRx()
{
  memset(rx_buffer, 0, sizeof(rx_buffer));
  dwt_rxenable(DWT_START_RX_IMMEDIATE);

  uint32_t statusReg = 0;
  /* Poll for reception of a frame or error/timeout. See NOTE 6 below. */
  while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR))) {
    
  }

  if (statusReg & SYS_STATUS_RXFCG_BIT_MASK) { // Check for good frame
    uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_BIT_MASK;
    if (frame_len <= sizeof(rx_buffer)) {
      dwt_readrxdata(rx_buffer, frame_len, 0);
    }
    
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

    Serial.print("TimeRx received: ");
    Serial.println((char *)rx_buffer);

   
    if (rx_buffer[0] == 'A' && rx_buffer[1] == 'R') { //look for the prefix
      char *p = (char *)rx_buffer;
      p += 2; // skip A and R
      unsigned long anchorTime = strtoul(p, NULL, 10); //string to usngned long
      Serial.print("TimeRx Anchor timestamp = ");
      Serial.println(anchorTime);
    }
  }
  else {
    
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
  }
}
