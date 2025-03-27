//roamingnode2
#include <WiFi.h>
#include <esp_wifi.h>

#include "dw3000.h"

#define PIN_RST 15 //27
#define PIN_IRQ 4 //34
#define PIN_SS 5 //4

#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385
#define ALL_MSG_COMMON_LEN 10
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4
#define POLL_RX_TO_RESP_TX_DLY_UUS 450


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

static uint8_t rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
uint8_t tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t frame_seq_nb = 0;
static uint8_t rx_buffer[20];
static uint32_t status_reg = 0;
static uint64_t poll_rx_ts;
static uint64_t resp_tx_ts;
char RoamingID[7];

extern dwt_txconfig_t txconfig_options;

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
  for (int i = 0 ; i<sizeof(RoamingID); i++){
      tx_resp_msg[19+i] = RoamingID[i];
  }
  //time_offset = millis();
  Serial.printf("RoamingID: %s\n", RoamingID);

  Serial.println("Roaming Node");
  Serial.println("Setup over........");
}


void loop()
{
  // while (time_current == 0){
  //   TimeTx();
  //   TimeRx();
  // }
  // RangeStart();
  Range_Tx();
}

void Range_Tx(){
  /* Activate reception immediately. */
  dwt_rxenable(DWT_START_RX_IMMEDIATE);

  /* Poll for reception of a frame or error/timeout. See NOTE 6 below. */
  while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR)))
  {
  };

  if (status_reg & SYS_STATUS_RXFCG_BIT_MASK)
  {
    uint32_t frame_len;

    /* Clear good RX frame event in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

    /* A frame has been received, read it into the local buffer. */
    frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
    if (frame_len <= sizeof(rx_buffer))
    {
      dwt_readrxdata(rx_buffer, frame_len, 0);

      /* Check that the frame is a poll sent by "SS TWR initiator" example.
       * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
      rx_buffer[ALL_MSG_SN_IDX] = 0;
      if (memcmp(rx_buffer, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0)
      {
        uint32_t resp_tx_time;
        int ret;

        /* Retrieve poll reception timestamp. */
        poll_rx_ts = get_rx_timestamp_u64();

        /* Compute response message transmission time. See NOTE 7 below. */
        resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
        dwt_setdelayedtrxtime(resp_tx_time);

        /* Response TX timestamp is the transmission time we programmed plus the antenna delay. */
        resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

        /* Write all timestamps in the final message. See NOTE 8 below. */
        resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);
        resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts);

        /* Write and send the response message. See NOTE 9 below. */
        tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
        dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0); /* Zero offset in TX buffer. */
        dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);          /* Zero offset in TX buffer, ranging. */
        ret = dwt_starttx(DWT_START_TX_DELAYED);

        /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 10 below. */
        if (ret == DWT_SUCCESS)
        {
          /* Poll DW IC until TX frame sent event set. See NOTE 6 below. */
          while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
          {
          };

          /* Clear TXFRS event. */
          dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

          /* Increment frame sequence number after transmission of the poll message (modulo 256). */
          frame_seq_nb++;
        }
      }
    }
  }
  else
  {
    /* Clear RX error events in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
  }

}

// void RangeStart(){
//   Serial.println("Pulse Run");
//   char tx_msg[30];
//   long time_rn = time_current + micros();
//   snprintf(tx_msg, sizeof(tx_msg), "AP%s%ld", RoamingID, time_rn);
//   Serial.printf("Message: %s\n", tx_msg);
//   int frame_length = (sizeof(tx_msg) + FCS_LEN); // The real length that is going to be transmitted

//     /* Write frame data to DW IC and prepare transmission. See NOTE 3 below.*/
//   dwt_writetxdata(frame_length - FCS_LEN, (uint8_t *)tx_msg, 0); /* Zero offset in TX buffer. */

//   /* In this example since the length of the transmitted frame does not change,
//    * nor the other parameters of the dwt_writetxfctrl function, the
//    * dwt_writetxfctrl call could be outside the main while(1) loop.
//    */
//   dwt_writetxfctrl(frame_length, 0, 0); /* Zero offset in TX buffer, no ranging. */

//   /* Start transmission. */
//   dwt_starttx(DWT_START_TX_IMMEDIATE);
  
//   delay(10); // Sleep(TX_DELAY_MS);

//   /* Poll DW IC until TX frame sent event set. See NOTE 4 below.
//    * STATUS register is 4 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
//    * function to access it.*/
//   while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
//   {
//   };

//   /* Clear TX frame sent event. */
//   dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

//   test_run_info((unsigned char *)"TX Frame Sent");

//   /* Execute a delay between transmissions. */
//   delay(500);

//   /* Increment the blink frame sequence number (modulo 256). */
//   return;
// }


// void TimeTx()
// {
//   Serial.println("TimeTx Run");
//   char tx_msg[30];
//   snprintf(tx_msg, sizeof(tx_msg), "AT");

//   int frame_length = (sizeof(tx_msg) + FCS_LEN); // The real length that is going to be transmitted

//     /* Write frame data to DW IC and prepare transmission. See NOTE 3 below.*/
//   dwt_writetxdata(frame_length - FCS_LEN, (uint8_t *)tx_msg, 0); /* Zero offset in TX buffer. */

//   /* In this example since the length of the transmitted frame does not change,
//    * nor the other parameters of the dwt_writetxfctrl function, the
//    * dwt_writetxfctrl call could be outside the main while(1) loop.
//    */
//   dwt_writetxfctrl(frame_length, 0, 0); /* Zero offset in TX buffer, no ranging. */

//   /* Start transmission. */
//   dwt_starttx(DWT_START_TX_IMMEDIATE);
  
//   delay(10); // Sleep(TX_DELAY_MS);

//   /* Poll DW IC until TX frame sent event set. See NOTE 4 below.
//    * STATUS register is 4 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
//    * function to access it.*/
//   while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
//   {
//   };

//   /* Clear TX frame sent event. */
//   dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

//   test_run_info((unsigned char *)"TX Frame Sent");

//   /* Execute a delay between transmissions. */
//   delay(100);

//   /* Increment the blink frame sequence number (modulo 256). */
//   return;
// }

// void TimeRx()
// {
//   memset(rx_buffer, 0, sizeof(rx_buffer));
//   dwt_rxenable(DWT_START_RX_IMMEDIATE);

//   uint32_t statusReg = 0;
//   /* Poll for reception of a frame or error/timeout. See NOTE 6 below. */
//   while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR))) {
    
//   }

//   if (statusReg & SYS_STATUS_RXFCG_BIT_MASK) { // Check for good frame
//     uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_BIT_MASK;
//     if (frame_len <= sizeof(rx_buffer)) {
//       dwt_readrxdata(rx_buffer, frame_len, 0);
//     }
    
//     dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

//     Serial.print("TimeRx received: ");
//     Serial.println((char *)rx_buffer);

   
//     if (rx_buffer[0] == 'R' && rx_buffer[1] == 'T') { //look for the prefix
//       char *p = (char *)rx_buffer;
//       p += 2; // skip A and R
//       time_current = strtoul(p, NULL, 10) - time_offset + micros(); //string to usngned long
//       Serial.print("TimeRx Anchor timestamp = ");
//       Serial.println(time_current);
//     }
//   }
//   else {
    
//     dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
//   }
//   return;
// }

