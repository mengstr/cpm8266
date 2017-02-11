#include "c_types.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "eagle_soc.h"
#include "ets_sys.h"
#include "gpio.h"
#include "nosdk8266.h"


// Takes 2224 seconds to run


static unsigned char z80code[] = {
//	#include "hex/hello.data"
	#include "hex/zexdoc.data"
};
#include "z80/z80emu.h"
#include "z80/z80user.h"

#define call_delay_us( time ) { asm volatile( "mov.n a2, %0\n_call0 delay4clk" : : "r"(time*13) : "a2" ); }

static MACHINE machine;
extern void SystemCall (MACHINE *m);

int main() {
	int cnt=0;
	nosdk8266_init();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	PIN_DIR_OUTPUT = _BV(2); //Enable GPIO2 light off.

	while(1) {
		PIN_OUT_SET = _BV(2); //Turn GPIO2 light off.
		call_delay_us( 500000 );
		PIN_OUT_CLEAR = _BV(2); //Turn GPIO2 light on.
		call_delay_us( 500000 );

	memcpy(machine.memory+0x100, z80code, sizeof(z80code));

	machine.memory[0] = 0xd3;       /* OUT N, A */
	machine.memory[1] = 0x00;

	machine.memory[5] = 0xdb;       /* IN A, N */
	machine.memory[6] = 0x00;
	machine.memory[7] = 0xc9;       /* RET */

	Z80Reset(&machine.state);
	machine.is_done = 0;
	machine.state.pc = 0x100;
	do {
		if (cnt==0) {
			cnt=1;
			PIN_OUT_SET = _BV(2); //Turn GPIO2 light off.
		} else {
			cnt=0;
			PIN_OUT_CLEAR = _BV(2); //Turn GPIO2 light on.
		}
		Z80Emulate(&machine.state, 10000000, &machine);
		call_delay_us(1000000);
	} while (!machine.is_done);    
		
    }
}



/* Emulate CP/M bdos call 5 functions */
void ICACHE_FLASH_ATTR SystemCall (MACHINE *m) {

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
