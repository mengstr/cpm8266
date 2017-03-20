#include "c_types.h"
#include "eagle_soc.h"

#ifdef NOSDK
 #include "esp8266_auxrom.h"
 #include "esp8266_rom.h"
 #include "nosdk8266.h"
#endif

#include "ets_sys.h"
#include "gpio.h"
#include "uart_dev.h"
#include "uart_register.h"

#include "espincludes.h"
#include "conio.h"
#include "uart.h"

typedef void (*int_handler_t)(void *);
extern UartDevice UartDev;

#ifdef WIFI
 #define PERIPH_FREQ 160    // TODO
#endif


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
    EmptyComBuf();  
    uart_div_modify(UART0, (PERIPH_FREQ * 1000000) / 9600);
    v=GetRxChar();
    if (v==0x0D) return 9600;
    if (v==0xE6) return 4800;
    if (v==0x78) return 2400;
    if (v==0x80) return 1200;
    if (v==0x00) return 300;
    ets_delay_us(50000); // Delay to trap possible additional chars
    EmptyComBuf();  
  }
  return 0;
}


//
//
//
void UartOutChar(char c) {
  uint8_t fifo_cnt;
  do {
    fifo_cnt=((READ_PERI_REG(UART_STATUS(UART0))>>UART_TXFIFO_CNT_S)& UART_TXFIFO_CNT);
  } while (fifo_cnt>125);
  WRITE_PERI_REG(UART_FIFO(UART0), c);
}




//
//
//
LOCAL void uart0_rx_intr_handler(void *para) {
  if (UART_FRM_ERR_INT_ST ==
      (READ_PERI_REG(UART_INT_ST(UART0)) & UART_FRM_ERR_INT_ST)) {
    ets_printf("FRM_ERR\r\n");
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_FRM_ERR_INT_CLR);
  } else if (UART_RXFIFO_FULL_INT_ST ==
             (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST)) {
    while ((READ_PERI_REG(UART_STATUS(UART0)) >> UART_RXFIFO_CNT_S) & UART_RXFIFO_CNT) {
      uint8_t ch = READ_PERI_REG(UART_FIFO(UART0));
      StoreInComBuf(ch);
    }
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
  } else if (UART_RXFIFO_TOUT_INT_ST ==
             (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST)) {
    while ((READ_PERI_REG(UART_STATUS(UART0)) >> UART_RXFIFO_CNT_S) & UART_RXFIFO_CNT) {
      uint8_t ch = READ_PERI_REG(UART_FIFO(UART0));
      StoreInComBuf(ch);
    }
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
  } else if (UART_TXFIFO_EMPTY_INT_ST ==
             (READ_PERI_REG(UART_INT_ST(UART0)) & UART_TXFIFO_EMPTY_INT_ST)) {
    ets_printf("e");
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_TXFIFO_EMPTY_INT_CLR);
  } else if (UART_RXFIFO_OVF_INT_ST ==
             (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_OVF_INT_ST)) {
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_OVF_INT_CLR);
    ets_printf("RX OVF!!\r\n");
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
  SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_RXFIFO_FULL_INT_ENA | UART_RXFIFO_OVF_INT_ENA);
  ETS_UART_INTR_ENABLE();
  ets_install_putc1((void *)UartOutChar);
}

