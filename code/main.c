#include "c_types.h"
#include "eagle_soc.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "ets_sys.h"
#include "gpio.h"
#include "nosdk8266.h"

static unsigned char z80code[] = {
#include "hex/hello.data"
};
#include "z80/z80emu.h"
#include "z80/z80user.h"

static MACHINE machine;
void SystemCall(MACHINE *m);

//
// Patch memory for taking care of the JP 0x0000 (RESTART) and
// the CALL 0x0005 (Generic BIOS call)
//
// The RESTART vector is replaced with a OUT instruction that will
// be ....
//
// The 0x0005 vector used by BIOS calls is replaced with an IN
// instruction that will be handled by the SystemCall() handler.
//
void ICACHE_FLASH_ATTR PatchBiosVectors(MACHINE *m) {
  machine.memory[0] = 0xd3; /* OUT N, A */
  machine.memory[1] = 0x00;

  machine.memory[5] = 0xdb; /* IN A, N */
  machine.memory[6] = 0x00;
  machine.memory[7] = 0xc9; /* RET */
}

//
// Returns the pressed key. This function waits until a key is
// pressed if "wait" is true. If wait is false and no key is available
// then 0x00 is returned
//
char GetKey(bool wait) {
  int res;
  char buf = 0;
  do {
    res = uart_rx_one_char(&buf);
  } while (res && wait);
  return buf;
}

//
//
//
int main() {
  char buf[10];
  char ch;

  nosdk8266_init();
  ets_delay_us(100000);
  while (1) {
    printf("emon:");
    ch = GetKey(true);
    if (ch >= ' ')
      printf("%c");
    if (ch == '?') {
      printf("\n");
      printf("? - Help\n");
      printf("g AAAA - Run code at location 0xAAAA\n");
      printf("d AAAA-BBBB - Dump memory between 0x#### and 0x#####\n");
      printf("d AAAA:CCCC - Dump 0xCCCC bytes of memory starting at 0xAAAA\n");
      continue;
    }
    if (ch == 13) {
      printf("\n");
      continue;
    }
    if (ch == 'g') {
      printf("'g' is not yet implemented\n");
      continue;
    }
    if (ch == 'd') {
      printf("'d' is not yet implemented\n");
      continue;
    }
    printf("Unknown comand. Enter '?' for a list\n");
  }

  while (1) {
    ets_memcpy(machine.memory + 0x100, z80code, sizeof(z80code));

    Z80Reset(&machine.state);
    machine.is_done = 0;
    machine.state.pc = 0x100;
    do {
      Z80Emulate(&machine.state, 10000000, &machine);
      //   call_delay_us(1000000);
    } while (!machine.is_done);
  }
}

//
// Emulate CP/M bdos call 5 functions
//
void ICACHE_FLASH_ATTR SystemCall(MACHINE *m) {

  if (m->state.registers.byte[Z80_C] == 2) {
    printf("%c", m->state.registers.byte[Z80_E]);
    return;
  }

  if (m->state.registers.byte[Z80_C] == 9) {
    int i;
    for (i = m->state.registers.word[Z80_DE]; m->memory[i] != '$'; i++) {
      printf("%c", m->memory[i & 0xffff]);
    }
  }
}
