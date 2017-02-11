#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"

#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"

#include "driver/uart.h"

//#include "smartconfig.h"
//#include "pwm.h"
//#include "ip_addr.h"
//#include "upgrade.h"
//#include "esp_sdk_ver.h"
//#include "wpa2_enterprise.h"
//#include "eagle_soc.h"
//#include "c_types.h"
//#include "simple_pair.h"
//#include "mem.h"
//#include "sntp.h"
//#include "espnow.h"
//#include "json/json.h"
//#include "json/jsontree.h"
//#include "json/jsonparse.h"
//#include "spi_flash.h"
//#include "queue.h"
//#include "at_custom.h"
//#include "gpio.h"
//#include "os_type.h"
//#include "mesh.h"
//#include "airkiss.h"
//#include "ets_sys.h"
//#include "user_interface.h"
//#include "ping.h"
//#include "osapi.h"
//#include "espconn.h"

#define Z80KB	32


#define procTaskPrio        0
#define procTaskQueueLen    1
static const int pin = 2;
static volatile os_timer_t some_timer;
os_event_t procTaskQueue[procTaskQueueLen];

static unsigned int counter=0;


static unsigned char z80code[] = {
    #include "hex/hello.data"
};

#include "z80emu.h"
#include "z80user.h"

extern void SystemCall (MACHINE *machine);

static volatile uint8_t memory[Z80KB*1024];


static void ICACHE_FLASH_ATTR emulate (const unsigned char *constcode, unsigned int len) {
  	MACHINE	context;
    memcpy(context.memory+0x100, constcode, len);

    /* Patch the memory of the program. Reset at 0x0000 is trapped by an
     * OUT which will stop emulation. CP/M bdos call 5 is trapped by an IN.
  	 * See Z80_INPUT_BYTE() and Z80_OUTPUT_BYTE() definitions in z80user.h.
     */

    context.memory[0] = 0xd3;       /* OUT N, A */
    context.memory[1] = 0x00;

    context.memory[5] = 0xdb;       /* IN A, N */
    context.memory[6] = 0x00;
    context.memory[7] = 0xc9;       /* RET */

    context.is_done = 0;

    /* Emulate. */
    Z80Reset(&context.state);
    context.state.pc = 0x100;
  	do {
      Z80Emulate(&context.state, 100, &context);    
    } while (!context.is_done);
}

/* Emulate CP/M bdos call 5 functions */
void ICACHE_FLASH_ATTR SystemCall (MACHINE *zextest) {
  if (zextest->state.registers.byte[Z80_C] == 2) {
    os_printf("%c", zextest->state.registers.byte[Z80_E]);
    return;
  }

  if (zextest->state.registers.byte[Z80_C] == 9) {
    int     i, c;
    for (i = zextest->state.registers.word[Z80_DE], c = 0; zextest->memory[i] != '$'; i++) {
      os_printf("%c", zextest->memory[i & 0xffff]);
      c++;
    }
  }
}


//There is no code in this project that will cause reboots if interrupts are disabled.
void EnterCritical() {}
void ExitCritical() {}



void some_timerfunc(void *arg) {
    os_printf("Timer interrupt %ld --------------------------------------------------\n",counter);
    

//  if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << pin)) {
//    gpio_output_set(0, (1 << pin), 0, 0); // set gpio low
//  } else {
//    gpio_output_set((1 << pin), 0, 0, 0); // set gpio high
//  }
}

//
//Tasks that happen all the time.
//
static void ICACHE_FLASH_ATTR procTask(os_event_t *events) {
    system_os_post(procTaskPrio, 0, 0 );
    counter++;
    os_delay_us(1000);
//    emulate(z80code,sizeof(z80code));
}



//
//
//
void ICACHE_FLASH_ATTR user_init() {
  
  // configure UART TXD to be GPIO1, set as output
  //  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1); 
  //  gpio_output_set(0, 0, (1 << pin), 0);

  //  gpio_init(); // init gpio sussytem
  uart_div_modify(0, UART_CLK_FREQ / 115200);

  os_delay_us(50000);
  os_printf("\n\nCPM8266 version 0.1\n");
  os_printf("ESP NOOS SDK version:%s\n", system_get_sdk_version());
  system_print_meminfo();

  // setup timer (10s, repeating)
  os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);
  os_timer_arm(&some_timer, 10000, 1);

  //Add a process
  system_os_task(procTask, procTaskPrio, procTaskQueue, procTaskQueueLen);
  system_os_post(procTaskPrio, 0, 0 );
}



//
// Required for NONOS 2.0
//
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


//
// Required for NONOS 2.0
//
void ICACHE_FLASH_ATTR user_rf_pre_init(void) {
}
