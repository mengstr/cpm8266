#include "c_types.h"
#include "eagle_soc.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "ets_sys.h"
#include "gpio.h"
#include "nosdk8266.h"
#include "uart_dev.h"
#include "uart_register.h"

#include "uart.h"

typedef void (*int_handler_t)(void *);
void ets_isr_attach(int intr, int_handler_t handler, void *arg);
void ets_isr_unmask(int intr);
extern UartDevice UartDev;

static volatile uint8_t rxBuf[1024];
static volatile uint16_t rxR = 0;
static volatile uint16_t rxW = 0;

//
// Reading a CR at 115k and 9600 baud gives the followin results
//
//                  Receiver @    Receiver @
//                  115200 baud   9600 baud
//
// Sender at 115200 0D
// Sender at 57600  E6 80
// Sender at 38400  1C 00
// Sender at 19200  E0 E0 00
// Sender at 9600   00 00 00      0D
// Sender at 4800                 E6 80
// Sender at 2400                 78 00
// Sender at 1200                 80 80 00
// Sender at 300                  00 00 00
//
uint32_t ICACHE_FLASH_ATTR AutoBaud(void) {
  uint8_t v;
  for (;;) {
    uart_div_modify(UART0, (PERIPH_FREQ * 1000000) / 115200);
    v=GetRxChar();  // Wait for one character received
    if (v==0x0D) return 115200;
    if (v==0xE6) return 57600;
    if (v==0x1C) return 38400;
    if (v==0xE0) return 19200;
    ets_delay_us(50000); // Delay to trap possible additional chars
    FlushUart();  
    uart_div_modify(UART0, (PERIPH_FREQ * 1000000) / 9600);
    v=GetRxChar();
    if (v==0x0D) return 9600;
    if (v==0xE6) return 4800;
    if (v==0x78) return 2400;
    if (v==0x80) return 1200;
    if (v==0x00) return 300;
    ets_delay_us(50000); // Delay to trap possible additional chars
    FlushUart();  
  }
  return 0;
}


//
//
//
uint16_t ICACHE_FLASH_ATTR GetRxCnt(void) {
  if (rxW == rxR) return 0;
  if (rxW > rxR) return rxW - rxR;
  return (sizeof(rxBuf) - rxR) + rxW;
}


//
//
//
uint8_t ICACHE_FLASH_ATTR GetRxChar(void) {
  int i;
  while (!GetRxCnt()) continue;  // Wait for character to become available
  rxR = (rxR + 1) & (sizeof(rxBuf) - 1);
  return rxBuf[rxR];
}


//
//
//
void ICACHE_FLASH_ATTR FlushUart(void) {
    while (GetRxCnt()) GetRxChar();

}


//
//
//
LOCAL void uart0_rx_intr_handler(void *para) {
  if (UART_FRM_ERR_INT_ST ==
      (READ_PERI_REG(UART_INT_ST(UART0)) & UART_FRM_ERR_INT_ST)) {
    printf("FRM_ERR\r\n");
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_FRM_ERR_INT_CLR);
  } else if (UART_RXFIFO_FULL_INT_ST ==
             (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST)) {
    int cnt = 0;
    while ((READ_PERI_REG(UART_STATUS(UART0)) >> UART_RXFIFO_CNT_S) &
           UART_RXFIFO_CNT) {
      uint8_t ch = READ_PERI_REG(UART_FIFO(UART0));
      cnt++;
      if (GetRxCnt() < sizeof(rxBuf) - 1) {
        rxW = (rxW + 1) & (sizeof(rxBuf) - 1);
        rxBuf[rxW] = ch;
      }
    }
    //      printf("[cnt=%d rxR=%d rxW=%d]",cnt,rxR,rxW);
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
  } else if (UART_RXFIFO_TOUT_INT_ST ==
             (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST)) {
    int cnt = 0;
    while ((READ_PERI_REG(UART_STATUS(UART0)) >> UART_RXFIFO_CNT_S) &
           UART_RXFIFO_CNT) {
      uint8_t ch = READ_PERI_REG(UART_FIFO(UART0));
      cnt++;
      if (GetRxCnt() < sizeof(rxBuf) - 1) {
        rxW = (rxW + 1) & (sizeof(rxBuf) - 1);
        rxBuf[rxW] = ch;
      }
    }
    //        printf("(cnt=%d rxR=%d rxW=%d)",cnt,rxR,rxW);
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
  } else if (UART_TXFIFO_EMPTY_INT_ST ==
             (READ_PERI_REG(UART_INT_ST(UART0)) & UART_TXFIFO_EMPTY_INT_ST)) {
    printf("e");
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_TXFIFO_EMPTY_INT_CLR);
  } else if (UART_RXFIFO_OVF_INT_ST ==
             (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_OVF_INT_ST)) {
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_OVF_INT_CLR);
    printf("RX OVF!!\r\n");
  }
}

//
//
//
void ICACHE_FLASH_ATTR InitUart(void) {
  ETS_UART_INTR_ATTACH(uart0_rx_intr_handler, &(UartDev.rcv_buff));
  PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
  // WRITE_PERI_REG(
  //     UART_CONF0(UART0),
  //     ((UartDev.exist_parity & UART_PARITY_EN_M)  <<  UART_PARITY_EN_S)
  //     |
  //     ((UartDev.parity & UART_PARITY_M)  <<UART_PARITY_S ) |
  //     ((UartDev.stop_bits & UART_STOP_BIT_NUM) << UART_STOP_BIT_NUM_S)
  //     |
  //     ((UartDev.data_bits & UART_BIT_NUM) << UART_BIT_NUM_S));
  SET_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
  CLEAR_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
  WRITE_PERI_REG(
      UART_CONF1(UART0),
      ((1 & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S) |
          ((1 & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S) | UART_RX_TOUT_EN |
          ((0x10 & UART_TXFIFO_EMPTY_THRHD) << UART_TXFIFO_EMPTY_THRHD_S));
  WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);
  SET_PERI_REG_MASK(UART_INT_ENA(UART0),
                    UART_RXFIFO_FULL_INT_ENA | UART_RXFIFO_OVF_INT_ENA);
  ETS_UART_INTR_ENABLE();
}
