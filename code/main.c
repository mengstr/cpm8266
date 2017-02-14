#include "c_types.h"
#include "eagle_soc.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "ets_sys.h"
#include "gpio.h"
#include "nosdk8266.h"

size_t strlen(const char *s);

static unsigned char z80code[] = {
#include "hex/hello.data"
};
#include "z80/z80emu.h"
#include "z80/z80user.h"

static MACHINE machine;

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
char *GetLine(void) {
  char ch;
  static char line[80];
  line[0] = 0;
  for (;;) {
    ch = GetKey(true);
    //    printf("(%d)", ch);
    if (ch == 13)
      return line;
    if ((ch == 8 || ch == 127) && strlen(line) > 0) {
      printf("\b \b");
      line[strlen(line) - 1] = 0;
      continue;
    }
    if (strlen(line) < sizeof(line) - 1) {
      printf("%c", ch);
      line[strlen(line) + 1] = 0;
      line[strlen(line)] = ch;
    }
  }
}

//
//
//
bool isHex(char ch) {
  if (ch >= '0' && ch <= '9')
    return true;
  if (ch >= 'A' && ch <= 'F')
    return true;
  if (ch >= 'a' && ch <= 'f')
    return true;
  return false;
}

//
//
//
uint8_t hexDec1(char ch) {
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  if (ch >= 'A' && ch <= 'F')
    return ch - 'A' + 10;
  if (ch >= 'a' && ch <= 'f')
    return ch - 'a' + 10;
  return 0xFF;
}

//
//
//
uint16_t getHexNum(char **p) {
  uint16_t address = 0;
  char ch;

  // Skip leading blanks
  while ((**p) == ' ')
    (*p)++;
  // Build number while hex-characters
  while (isHex(**p)) {
    address = address * 16 + hexDec1(**p);
    (*p)++;
  }
  return address;
}

//
// 01234567890123456789012345678901234567890123456789012345678901234567890123
// 0000: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff    ................
void HexDump(uint16_t address, uint16_t len) {
  uint16_t a;
  char cleartext[17];

  if (len == 0)
    len = 16;
  a = address & 0xFFF0;
  cleartext[16] = 0;
  while (a < address + len) {
    printf("%04X:", a);
    for (uint16_t i = 0; i < 16; i++) {
      cleartext[i] = '.';
      char mem = machine.memory[a];
      if (a >= address && a < address + len) {
        printf(" %02X", mem);
        if (mem >= 32 && mem < 127)
          cleartext[i] = mem;
      } else {
        printf(" ..");
      }
      a++;
    }
    printf("  %s\n", cleartext);
  }
}

void ModifyMemory(uint16_t address) {
  char *line;
  uint8_t v1, v2;
  for (;;) {
    printf("%04x = %02x : ", address, machine.memory[address]);
    line = GetLine();
    printf("(%d)", *line);
    if (*line == '.') { // DOT exits
      break;
    } else if (*line == 0) { // Empty line goes to next memory location
      address++;
    } else if (*line == '-') { // Minus goes to previous memory location
      address--;
    } else { // Two proper hex digits stores & goes to next location
      v1 = hexDec1(*line);
      v2 = hexDec1(*(line + 1));
      if (v1 < 16 && v2 < 16) {
        machine.memory[address++] = v1 * 16 + v2;
      } else { // Message for bad input and stay at same location
        printf(" INVALID ENTRY");
      }
    }
    printf("\n");
  }
  printf("\n");
}

//
//
//
void ShowAllRegisters() {
  printf(
      "A=%02X B=%02X C=%02X D=%02X E=%02X H=%02X L=%02X "
      "IX=%04X IY=%04X SP=%04X PC=%04X\n",
      machine.state.registers.byte[Z80_A], machine.state.registers.byte[Z80_B],
      machine.state.registers.byte[Z80_C], machine.state.registers.byte[Z80_D],
      machine.state.registers.byte[Z80_E], machine.state.registers.byte[Z80_H],
      machine.state.registers.byte[Z80_L], machine.state.registers.word[Z80_IX],
      machine.state.registers.word[Z80_IY],
      machine.state.registers.word[Z80_SP], machine.state.pc);
}

//
//
//
int main() {
  char *line;
  uint16_t loc;
  uint16_t len;

  nosdk8266_init();
  ets_delay_us(250000);
  while (1) {
    printf("EMON:");
    line = GetLine();
    printf("\n");
    if (line[0] == '?') {
      printf("EMON v0.1\n");
      printf("\td AAAA [LLLL] - Dump LLLL bytes of memory starting at AAAA\n");
      printf("\tm AAAA        - Modify memory contents starting at AAAA\n");
      printf("\tr [REG]       - Display all, or modify REG, register(s)\n");
      continue;
    }
    if (line[0] == 'd') {
      line++; // Skip no next char
      loc = getHexNum(&line);
      len = getHexNum(&line);
      HexDump(loc, len);
      continue;
    }
    if (line[0] == 'm') {
      line++; // Skip no next char
      loc = getHexNum(&line);
      ModifyMemory(loc);
      continue;
    }
    if (line[0] == 'r') {
      line++; // Skip no next char
      ShowAllRegisters();
      continue;
    }
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
