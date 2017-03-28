 #include "c_types.h"
 #include "os_type.h"
 #include "espincludes.h"
#ifdef NOSDK
 #include "eagle_soc.h"
 #include "ets_sys.h"
 #include "gpio.h"
 #include "nosdk8266.h"
#endif
#ifdef WIFI
 #include "osapi.h"
 #include "wifi.h"
#endif

#include "uart.h"
#include "uart_dev.h"
#include "uart_register.h"
#include "gpio16.h"
#include "flash.h"
#include "conio.h"

// Make sure there always is something in the irom segment or the uploader
// will barf
volatile uint32_t KEEPMEHERE __attribute__((section(".irom.text"))) = 0xDEADBEEF;

#include "machine.h"
#include "z80/z80emu.h"
#include "z80/z80user.h"
#include "monitor.h"
#include "utils.h"

#ifdef NOSDK
void ICACHE_FLASH_ATTR main(void) {
  uint32_t baud;

  gpio16_output_conf();
  gpio16_output_set(1);	

  nosdk8266_init();
  InitUart();
  baud=AutoBaud();
  uart_div_modify(UART0, (PERIPH_FREQ * 1000000) / baud);

  ets_printf("\r\ncpm8266 - Z80 Emulator and CP/M 2.2 system version %d.%d\r\n",VERSION/10,VERSION%10);
  ets_delay_us(250000);
  EmptyComBuf();

  InitMachine();
  LoadBootSector();
  for (;;) {          
    RunMachine(1);  // 400MC takes 32 seconds
  }
}
#endif



#ifdef WIFI
#endif
