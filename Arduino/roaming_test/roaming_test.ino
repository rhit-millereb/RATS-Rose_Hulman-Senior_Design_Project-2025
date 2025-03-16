//roamingnode2
#include <WiFi.h>
#include <esp_wifi.h>

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

static uint8_t frame_seq_nb = 0;
static uint8_t rx_buffer[50];
static uint32_t status_reg = 0;
extern dwt_txconfig_t txconfig_options;
unsigned long time_current = 0;
unsigned long time_offset;
unsigned long nextRangeCheck = 0;
char RoamingID[7];


void setup()
{
  UART_init();  // Or whatever your serial init is

  spiBegin(PIN_IRQ, PIN_RST); 
  spiSelect(PIN_SS);

  delay(200); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

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

  dwt_softreset();
  delay(200);

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

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();

  uint8_t baseMac[6];
  esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  snprintf(RoamingID, sizeof(RoamingID), "%02x%02x%02x", baseMac[3], baseMac[4], baseMac[5]);
  Serial.printf("RoamingID: %s\n", RoamingID);

  Serial.println("Roaming Node");
  Serial.println("Setup over........");
}


void loop()
{
  while (time_current == 0){
    TimeTx();
    TimeRx();
  }
  RangeStart();
}

void RangeStart(){
  Serial.println("Pulse Run");
  char tx_msg[30];
  time_offset = millis();
  snprintf(tx_msg, sizeof(tx_msg), "AR");

  int frame_length = (sizeof(tx_msg) + FCS_LEN); // The real length that is going to be transmitted

    /* Write frame data to DW IC and prepare transmission. See NOTE 3 below.*/
  dwt_writetxdata(frame_length - FCS_LEN, (uint8_t *)tx_msg, 0); /* Zero offset in TX buffer. */

  /* In this example since the length of the transmitted frame does not change,
   * nor the other parameters of the dwt_writetxfctrl function, the
   * dwt_writetxfctrl call could be outside the main while(1) loop.
   */
  dwt_writetxfctrl(frame_length, 0, 0); /* Zero offset in TX buffer, no ranging. */

  /* Start transmission. */
  dwt_starttx(DWT_START_TX_IMMEDIATE);
  
  delay(10); // Sleep(TX_DELAY_MS);

  /* Poll DW IC until TX frame sent event set. See NOTE 4 below.
   * STATUS register is 4 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
   * function to access it.*/
  while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
  {
  };

  /* Clear TX frame sent event. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

  test_run_info((unsigned char *)"TX Frame Sent");

  /* Execute a delay between transmissions. */
  delay(500);

  /* Increment the blink frame sequence number (modulo 256). */
  return;
}


void TimeTx()
{
  Serial.println("TimeTx Run");
  char tx_msg[30];
  time_offset = millis();
  snprintf(tx_msg, sizeof(tx_msg), "AT");

  int frame_length = (sizeof(tx_msg) + FCS_LEN); // The real length that is going to be transmitted

    /* Write frame data to DW IC and prepare transmission. See NOTE 3 below.*/
  dwt_writetxdata(frame_length - FCS_LEN, (uint8_t *)tx_msg, 0); /* Zero offset in TX buffer. */

  /* In this example since the length of the transmitted frame does not change,
   * nor the other parameters of the dwt_writetxfctrl function, the
   * dwt_writetxfctrl call could be outside the main while(1) loop.
   */
  dwt_writetxfctrl(frame_length, 0, 0); /* Zero offset in TX buffer, no ranging. */

  /* Start transmission. */
  dwt_starttx(DWT_START_TX_IMMEDIATE);
  
  delay(10); // Sleep(TX_DELAY_MS);

  /* Poll DW IC until TX frame sent event set. See NOTE 4 below.
   * STATUS register is 4 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
   * function to access it.*/
  while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
  {
  };

  /* Clear TX frame sent event. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

  test_run_info((unsigned char *)"TX Frame Sent");

  /* Execute a delay between transmissions. */
  delay(500);

  /* Increment the blink frame sequence number (modulo 256). */
  return;
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

   
    if (rx_buffer[0] == 'R' && rx_buffer[1] == 'T') { //look for the prefix
      char *p = (char *)rx_buffer;
      p += 2; // skip A and R
      time_current = strtoul(p, NULL, 10) - time_offset + millis(); //string to usngned long
      Serial.print("TimeRx Anchor timestamp = ");
      Serial.println(time_current);
    }
  }
  else {
    
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
  }
  return;
}

