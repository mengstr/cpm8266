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
//
//
uint16_t GetRxCnt(void) {
  if (rxW == rxR) return 0;
  if (rxW > rxR) return rxW - rxR;
  return (sizeof(rxBuf) - rxR) + rxW;
}


//
//
//
uint8_t GetRxChar(void) {
  int i;
  while (!GetRxCnt()) continue;  // Wait for character to become available
  rxR = (rxR + 1) & (sizeof(rxBuf) - 1);
  return rxBuf[rxR];
}

void pbf(void) {
  //    printf("{%d/%d}",rxR,rxW);
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
void InitUart(void) {
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
