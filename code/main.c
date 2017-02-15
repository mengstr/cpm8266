#include "c_types.h"
#include "eagle_soc.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "ets_sys.h"
#include "gpio.h"
#include "nosdk8266.h"

#define TOLOWER(x) (x|0x20)
size_t strlen(const char *s);
char *ets_strcpy(char *dest, const char *src);
char *strcpy(char *dest, const char *src);

#define BREAKKEY '`'


static unsigned char z80code[] = {
//#include "hex/hello.data"
#include "hex/zexdoc.data"
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
char ICACHE_FLASH_ATTR GetKey(bool wait) {
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
char ICACHE_FLASH_ATTR *GetLine(void) {
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
// Return true if the argument is hex (0..9 A..F a..f)
//
bool ICACHE_FLASH_ATTR isHex(char ch) {
  if (ch >= '0' && ch <= '9')
    return true;
  if (ch >= 'A' && ch <= 'F')
    return true;
  if (ch >= 'a' && ch <= 'f')
    return true;
  return false;
}

//
// Return the converted value of a hex digit (0..9 A..F a..f)
//
uint8_t ICACHE_FLASH_ATTR hexDec1(char ch) {
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
uint8_t hexDec2p(char *p) {
  uint8_t v1=hexDec1(*p);
  p++;
  uint8_t v2=hexDec1(*p);
  return v1*16+v2;
}

//
//
//
uint16_t hexDec4p(char *p) {
  uint8_t v1=hexDec2p(p);
  p+=2;
  uint8_t v2=hexDec2p(p);
  return v1*256+v2;
}




#define NONUM 0x10000

//
// Returns the value of the first word found in the string.
// If the string ends before anything is found then return NONUM.
// If the first found character is non-hex then return 0
//
uint32_t ICACHE_FLASH_ATTR getHexNum(char **p) {
  uint32_t address = 0;
  char ch;

  // Skip leading blanks
  while ((**p) == ' ')
    (*p)++;

  // Reached end of string?
  if (!(**p))
    return NONUM;
  // Build number while hex-characters
  while (isHex(**p)) {
    address = address * 16 + hexDec1(**p);
    (*p)++;
  }
  return address;
}

//
// Returns a pointer to the first word in the string. The word
// gets terminated by \0 and the line pointer is set to the next
// location after that.
//
char ICACHE_FLASH_ATTR *getString(char **p) {
  char *s;

  // Skip leading blanks
  while ((**p) == ' ')
    (*p)++;

  // Beginning of (possible) string
  s = *p;

  // Skip ahead while non-blank
  while ((**p) > ' ')
    (*p)++;

  // If at EOL then just return
  if (!(**p))
    return s;

  // If there still is data in the string we need to terminate
  // this word and bump up the string pointer one character
  (**p) = 0;
  (*p)++;

  return s;
}

//
// HEX    aaaa: dd dd dd dd dd dd dd dd dd dd dd dd dd dd dd dd ................
// INTEL  :llaaaatt[dd...]cc
//
void ICACHE_FLASH_ATTR HexDump(uint16_t address, uint16_t len, bool intel) {
  uint16_t a;
  char cleartext[17];

  // Default to 16 bytes of data to dump
  if (len == 0)
    len = 16;

  if (intel) {

    // Dump data in INTEL HEX format
    while (len>0) {
      uint16_t l=len;
      if (l>16) l=16;
      len-=l;
      printf(":%02X%04X00",l,address);
      uint8_t chksum=l+(address>>8)+(address&0xFF)+0x00;
      for (int i=0; i<l; i++) {
        chksum+=machine.memory[address];
        printf("%02X",machine.memory[address++]);
      }
      chksum=~chksum;
      chksum=chksum+1;
      printf("%02X\n",chksum);
    }
    printf(":00000001FF\n"); // EOF Record

  } else {

    // Dump memory in human readable HAX + ASCII format
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
}

//
//
//
void ICACHE_FLASH_ATTR ModifyMemory(uint16_t address) {
  char *line;
  uint8_t v1, v2;
  for (;;) {
    printf("%04x = %02x : ", address, machine.memory[address]);
    line = GetLine();
    if (*line == 0) { // Empty line goes to next memory location
      address++;
    } else if (*line == '-') { // Minus goes to previous memory location
      address--;
    } else { // Two proper hex digits stores & goes to next location
      v1 = hexDec1(*line);
      v2 = hexDec1(*(line + 1));
      if (v1 < 16 && v2 < 16) {
        machine.memory[address++] = v1 * 16 + v2;
      } else { // Any 'bad' input exits function
        break;
      }
    }
    printf("\n");
  }
  printf("\n");
}

//
//
// INTEL  :llaaaatt[dd...]cc
//
void LoadIntelHex(void) {
  char *line;
  uint16_t minAddress=0xFFFF;
  uint16_t maxAddress=0x0000;

  for (;;) {
    printf("i ");
    line=GetLine();
    printf("\n");
    char *p=line;
    // Skip leading blanks
    while ((*p) == ' ')
      p++;
      // Ignore empty lines
    if (!(*p)) 
      continue;
    // Exit if bad data
    if ((*p++)!=':') {
      printf("Missing : in '%s'\n",line);
      break;
    }

    uint8_t len=hexDec2p(p);
    p+=2;
    uint16_t address=hexDec4p(p);
    p+=4;
    uint8_t typ=hexDec2p(p);
    p+=2;
    if (typ==0x01) {
      printf("Loaded data between %04x and %04x\n",minAddress,maxAddress);
      break;
    } else if (typ==0x00) {
      for (int i=0; i<len; i++) {
        if (address<minAddress) minAddress=address;
        if (address>maxAddress) maxAddress=address;
        machine.memory[address]=hexDec2p(p);
        p+=2;
        address++;
      }
    } else {
      printf("Invalid record type in '%s'\n",line);
    }
  }
}



#define LOWERNULL 0x20

//
//
//
void ICACHE_FLASH_ATTR ShowAllRegisters() {
  uint8_t flags;
  char tmps[9];

  flags = machine.state.registers.byte[Z80_F];
  ets_strcpy(tmps, "sz-h-pnc");

  if (flags & Z80_S_FLAG)
    tmps[0] = 'S';
  if (flags & Z80_Z_FLAG)
    tmps[1] = 'Z';
  if (flags & Z80_H_FLAG)
    tmps[3] = 'H';
  if (flags & Z80_P_FLAG)
    tmps[5] = 'P';
  if (flags & Z80_N_FLAG)
    tmps[6] = 'N';
  if (flags & Z80_C_FLAG)
    tmps[7] = 'C';
  printf(
      "A=%02X B=%02X C=%02X D=%02X E=%02X H=%02X L=%02X "
      "IX=%04X IY=%04X SP=%04X PC=%04X %s\n",
      machine.state.registers.byte[Z80_A], machine.state.registers.byte[Z80_B],
      machine.state.registers.byte[Z80_C], machine.state.registers.byte[Z80_D],
      machine.state.registers.byte[Z80_E], machine.state.registers.byte[Z80_H],
      machine.state.registers.byte[Z80_L], machine.state.registers.word[Z80_IX],
      machine.state.registers.word[Z80_IY],
      machine.state.registers.word[Z80_SP], machine.state.pc, tmps);
}

//
//
//
void ICACHE_FLASH_ATTR ModifyRegister(char *regname, uint16_t val) {
  char c1, c2;
  int reg;
  uint8_t siz;

  reg=-1;
  c1 = TOLOWER(regname[0]);
  c2 = TOLOWER(regname[1]);

  if (c1=='p' && c2=='c') {
    machine.state.pc=val;
    ShowAllRegisters();
    return;
  }
  if (c2==LOWERNULL) {
    siz=8;
    if (c1=='a') reg=Z80_A;
    if (c1=='b') reg=Z80_B;
    if (c1=='c') reg=Z80_C;
    if (c1=='d') reg=Z80_D;
    if (c1=='e') reg=Z80_E;
    if (c1=='h') reg=Z80_H;
    if (c1=='l') reg=Z80_L;
  } else {
    siz=16;
    if (c1=='i' && c2=='x') reg=Z80_IX;
    if (c1=='i' && c2=='y') reg=Z80_IY;
    if (c1=='s' && c2=='p') reg=Z80_SP;
  }

  if (reg==-1) {
    printf("Invalid register name.\n");
    return;
  }

  if (siz==8) {
    machine.state.registers.byte[reg]=val;
  } else {
    machine.state.registers.word[reg]=val;
  }
  ShowAllRegisters();
}

//
//
//
int main() {
  char *line;

  nosdk8266_init();
  ets_delay_us(250000);

  Z80Reset(&machine.state);
  machine.is_done = 0;

  while (1) {
    printf("EMON:");
    line = GetLine();
    printf("\n");
    char cmd=line[0];
    line++; // Skip to next char

    //
    // Show help text
    //
    if (cmd == '?') {
      printf("EMON v0.2\n");
      printf("\td ADR [LEN] - Hexdump LEN bytes of memory starting at ADR\n");
      printf("\tD ADR [LEN] - Intel hexdump LEN bytes of memory starting at ADR\n");
      printf("\tL           - Load an Intel hexdump into memory\n");
      printf("\tm ADR       - Modify memory contents starting at ADR\n");
      printf("\tr [REG VAL] - Display all registers, or set REG to VAL\n");
      printf("\tg [ADR]     - Start execution at PC or [ADR]\n");
      printf("\ts [NUM]     - Single step 1 or [NUM] instructions, print once\n");
      printf("\tS [NUM]     - Single step 1 or [NUM] instructions, print every step\n");
      continue;
    }

    if (cmd=='1') {
      ets_memcpy(machine.memory + 0x100, z80code, sizeof(z80code));
      machine.state.pc=0x100;
      ShowAllRegisters();
      continue;
    }

    //
    // Load INTEL Hex into memory
    //
    if (cmd=='L') {
      LoadIntelHex();
      continue;
    }

    //
    // Start execution at PC or specified address.
    //
     if (TOLOWER(cmd)=='g') {
     uint32_t pc = getHexNum(&line);
      if (pc<NONUM) machine.state.pc=pc;
      PatchBiosVectors(&machine);
      machine.is_done = 0;
      do {
        Z80Emulate(&machine.state, 1000000, &machine);
        printf("_");
        if (cmd=='g' && GetKey(false)==BREAKKEY) break;
      } while (!machine.is_done);
      printf("\n");
    }


    //
    // Single step 1 or multiple instructions with or without printing
    // of registers. When stepping multiple instructions pressing the  
    // period/dot button aborts execution. 
    //
    if (TOLOWER(cmd)=='s') {
      uint32_t cnt;
      cnt = getHexNum(&line);
      if ((cnt&0xFFFF)==0) cnt=1;
      PatchBiosVectors(&machine);
      for (int i=0; i<cnt; i++) {
        Z80Emulate(&machine.state, 1, &machine);
        if (cmd=='S') ShowAllRegisters();
        if (GetKey(false)==BREAKKEY) break;
      }
      if (cmd=='s') ShowAllRegisters();
      continue;
    }

    //
    // Hexdump memory
    //
    if (TOLOWER(cmd) == 'd') {
      uint16_t loc = getHexNum(&line);
      uint16_t len = getHexNum(&line);
      HexDump(loc, len, (cmd=='D'));
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
        ModifyRegister(regp,val);
      } else {
        ShowAllRegisters();
      }
      continue;
    }
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
