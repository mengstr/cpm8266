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

#define BREAKKEY '`'

//
//
//
void Execute(bool canbreak) {
  uint32_t flushCnt=0;
  machine.is_done = 0;
  ets_printf("Starting excution at 0x%04X\r\n", machine.state.pc);
  do {
    Z80Emulate(&machine.state, 2, &machine);
//    if (canbreak && (GetKey(false) == BREAKKEY)) break;
    if ((flushCnt++>2000000)) {
      FlushDisk(true);  // Flush Disk in standalone mode
      flushCnt=0;
    }
  } while (!machine.is_done);
  ets_printf("\n");
}





//
//
//
void ICACHE_FLASH_ATTR cpm_main() {
  char *line;
  //  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);
  //  PIN_DIR_OUTPUT=_BV(0);
  gpio16_output_conf();
  gpio16_output_set(1);	// PIN_OUT_SET=_BV(0);

  InitMachine();

  while (1) {
    ets_printf("EMON:");
    line = GetLine();
    ets_printf("\r\n");
    char cmd = line[0];
    line++;  // Skip to next char

    //
    // Show help text
    //
    if (cmd == '?') {
      ets_printf("EMON v0.2\r\n");
      ets_printf("\td ADR [LEN] - Hexdump LEN bytes of memory starting at ADR\r\n");
      ets_printf("\tD ADR [LEN] - Intel hexdump LEN bytes of memory starting at ADR\r\n");
      ets_printf("\tL [OFFSET]  - Load an Intel hexdump into memory with OFFSET\r\n");
      ets_printf("\tm ADR       - Modify memory contents starting at ADR\r\n");
      ets_printf("\tr [REG VAL] - Display all registers, or set REG to VAL\r\n");
      ets_printf("\tg [ADR]     - Start execution at PC or [ADR], can break with `\r\n");
      ets_printf("\tG [ADR]     - Start execution at PC or [ADR]\r\n");
      ets_printf("\ts [NUM]     - Single step 1 or [NUM] instructions, print once\r\n");
      ets_printf("\tS [NUM]     - Single step 1 or [NUM] instructions, print every step\r\n");
      ets_printf("\tB           - Load D0/T0/S0 to 0x0000 and execute\r\n");
      continue;
    }

    //
    //
    //
    if (cmd == 'B') {
      // Read first sector at first track first disk to memory 0x0000
      ReadDiskBlock(0x0000, 0, 0, 0);
      machine.state.pc = 0x0000;
      Execute(true);
      continue;
    }

    //
    // Load INTEL Hex into memory
    //
    if (cmd == 'L') {
      uint32_t offset = getHexNum(&line);
      if (offset == NONUM)
        offset = 0;
      LoadIntelHex(offset);
      continue;
    }

    //
    // Start execution at PC or specified address.
    //
    if (TOLOWER(cmd) == 'g') {
      uint32_t pc = getHexNum(&line);
      if (pc != NONUM) {
        machine.state.pc = pc;
      }
      Execute((cmd == 'g'));
      continue;
    }

    //
    // Single step 1 or multiple instructions with or without printing
    // of registers. When stepping multiple instructions pressing the
    // period/dot button aborts execution.
    //
    if (TOLOWER(cmd) == 's') {
      uint32_t cnt;
      cnt = getHexNum(&line);
      if ((cnt & 0xFFFF) == 0)
        cnt = 1;
      for (int i = 0; i < cnt; i++) {
        Z80Emulate(&machine.state, 1, &machine);
        if (cmd == 'S') ShowAllRegisters();
//        if (GetKey(false) == BREAKKEY) break;
      }
      if (cmd == 's') ShowAllRegisters();
      continue;
    }

    //
    // Hexdump memory
    //
    if (TOLOWER(cmd) == 'd') {
      uint16_t loc = getHexNum(&line);
      uint16_t len = getHexNum(&line);
      HexDump(loc, len, (cmd == 'D'));
      continue;
    }

    //
    // Modify memory
    //
    if (cmd == 'm') {
      uint16_t loc = getHexNum(&line);
      ModifyMemory(loc);
      continue;
    }

    //
    // Display and modify Z80-registers
    //
    if (cmd == 'r') {
      char *regp = getString(&line);
      uint32_t val = getHexNum(&line);
      if (*regp) {
        ModifyRegister(regp, val);
      } else {
        ShowAllRegisters();
      }
      continue;
    }
  }
}



#ifdef NOSDK
void ICACHE_FLASH_ATTR main(void) {
  uint32_t baud;

  nosdk8266_init();
  InitUart();
  baud=AutoBaud();
  uart_div_modify(UART0, (PERIPH_FREQ * 1000000) / baud);

  ets_printf("\r\ncpm8266 - Z80 Emulator and CP/M 2.2 system version %d.%d\r\n",VERSION/10,VERSION%10);
  ets_delay_us(250000);
  EmptyComBuf();

//  cpm_main();
  InitMachine();
  LoadBootSector();
  for (;;) {
    RunMachine(1);
  }
}
#endif



#ifdef WIFI
#endif
