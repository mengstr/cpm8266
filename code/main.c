#include "c_types.h"
#include "eagle_soc.h"
#include "espincludes.h"
#include "ets_sys.h"
#include "gpio.h"
#include "nosdk8266.h"
#include "uart.h"
#include "uart_dev.h"
#include "uart_register.h"

#include "disasm/disasm.h"

#define RODATA_ATTR  __attribute__((section(".irom.text"))) __attribute__((aligned(4)))
#define ROMSTR_ATTR  __attribute__((section(".irom.text.romstr"))) __attribute__((aligned(4)))


//   text	   data	    bss	    dec	    hex	filename
//  29192	      6	  71040	 100238	  1878e	image.elf

//   text	   data	    bss	    dec	    hex	filename
//  29189	      6	  71040	 100235	  1878b	image.elf

#define TOLOWER(x) (x | 0x20)
#define BREAKKEY '`'

static const char z80code[] RODATA_ATTR = {
  #include "hex/CPM22.data"
};

#include "z80/z80emu.h"
#include "z80/z80user.h"

static MACHINE machine;

uint16_t z80_dma = 0x80;
uint16_t z80_trk = 0x00;
uint16_t z80_sec = 0x01;
uint16_t z80_dsk = 0x00;

static inline uint8_t ICACHE_FLASH_ATTR readRomByte(const uint8_t* addr) {
    uint32_t bytes;
    bytes = *(uint32_t*)((uint32_t)addr & ~3);
    return ((uint8_t*)&bytes)[(uint32_t)addr & 3];
}



//
// Returns the pressed key. This function waits until a key is
// pressed if "wait" is true. If wait is false and no key is available
// then 0x00 is returned
//
char ICACHE_FLASH_ATTR GetKey(bool wait) {
  if (!wait && GetRxCnt() == 0)
    return 0x00;
  return GetRxChar();
}

//
//
//
char ICACHE_FLASH_ATTR *GetLine(void) {
  char ch;
  static char line[128];
  line[0] = 0;
  for (;;) {
    ch = GetKey(true);
    if (ch == 13 || ch == 10)
      return line;
    if ((ch == 8 || ch == 127) && ets_strlen(line) > 0) {
      printf("\b \b");
      line[ets_strlen(line) - 1] = 0;
      continue;
    }
    if (ets_strlen(line) < sizeof(line) - 1) {
      printf("%c", ch);
      line[ets_strlen(line) + 1] = 0;
      line[ets_strlen(line)] = ch;
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
uint8_t ICACHE_FLASH_ATTR hexDec2p(char *p) {
  uint8_t v1 = hexDec1(*p);
  p++;
  uint8_t v2 = hexDec1(*p);
  return v1 * 16 + v2;
}

//
//
//
uint16_t ICACHE_FLASH_ATTR hexDec4p(char *p) {
  uint8_t v1 = hexDec2p(p);
  p += 2;
  uint8_t v2 = hexDec2p(p);
  return v1 * 256 + v2;
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
    while (len > 0) {
      uint16_t l = len;
      if (l > 16)
        l = 16;
      len -= l;
      printf(":%02X%04X00", l, address);
      uint8_t chksum = l + (address >> 8) + (address & 0xFF) + 0x00;
      for (int i = 0; i < l; i++) {
        chksum += machine.memory[address];
        printf("%02X", machine.memory[address++]);
      }
      chksum = ~chksum;
      chksum = chksum + 1;
      printf("%02X\n", chksum);
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
void ICACHE_FLASH_ATTR LoadIntelHex(uint16_t offset) {
  char *line;
  uint16_t minAddress = 0xFFFF;
  uint16_t maxAddress = 0x0000;

  for (;;) {
    printf("i ");
    line = GetLine();
    printf("\n");
    char *p = line;
    // Skip leading blanks
    while ((*p) == ' ')
      p++;
    // Ignore empty lines
    if (!(*p))
      continue;
    // Exit if bad data
    if ((*p++) != ':') {
      printf("Missing : in '%s'\n", line);
      break;
    }

    uint8_t len = hexDec2p(p);
    p += 2;
    uint16_t address = hexDec4p(p) + offset;
    p += 4;
    uint8_t typ = hexDec2p(p);
    p += 2;
    if (typ == 0x01) {
      printf("Loaded data between %04x and %04x\n", minAddress, maxAddress);
      break;
    } else if (typ == 0x00) {
      for (int i = 0; i < len; i++) {
        if (address < minAddress)
          minAddress = address;
        if (address > maxAddress)
          maxAddress = address;
        machine.memory[address] = hexDec2p(p);
        p += 2;
        address++;
      }
    } else {
      //      printf("Invalid record type in '%s'\n",line);
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

  reg = -1;
  c1 = TOLOWER(regname[0]);
  c2 = TOLOWER(regname[1]);

  if (c1 == 'p' && c2 == 'c') {
    machine.state.pc = val;
    ShowAllRegisters();
    return;
  }
  if (c2 == LOWERNULL) {
    siz = 8;
    if (c1 == 'a')
      reg = Z80_A;
    if (c1 == 'b')
      reg = Z80_B;
    if (c1 == 'c')
      reg = Z80_C;
    if (c1 == 'd')
      reg = Z80_D;
    if (c1 == 'e')
      reg = Z80_E;
    if (c1 == 'h')
      reg = Z80_H;
    if (c1 == 'l')
      reg = Z80_L;
  } else {
    siz = 16;
    if (c1 == 'i' && c2 == 'x')
      reg = Z80_IX;
    if (c1 == 'i' && c2 == 'y')
      reg = Z80_IY;
    if (c1 == 's' && c2 == 'p')
      reg = Z80_SP;
  }

  if (reg == -1) {
    printf("Invalid register name.\n");
    return;
  }

  if (siz == 8) {
    machine.state.registers.byte[reg] = val;
  } else {
    machine.state.registers.word[reg] = val;
  }
  ShowAllRegisters();
}

#define SECTORSIZE 128
#define SECTORSPERTRACK 26
#define TRACKSPERDISK 77
#define DISKFLASHOFFSET 0x40000 // Location in flash for first disk
#define DISKFLASHSIZE 0x40000   // Number of bytes in flash for each disk
#define DISKCOUNT 15            // We have 15 disks A..O
#define FLASHBLOCKSIZE 4096

static uint16_t flashSectorNo = 0xFFFF;
static uint8_t flashBuf[FLASHBLOCKSIZE];

void ICACHE_FLASH_ATTR hexdumpflash() {
  uint16_t a = 0;
  char cleartext[17];
  int len = 512;

  cleartext[16] = 0;
  while (a < len) {
    printf("%04X:", a);
    for (uint16_t i = 0; i < 16; i++) {
      cleartext[i] = '.';
      char mem = flashBuf[a];
      printf(" %02X", mem);
      if (mem >= 32 && mem < 127)
        cleartext[i] = mem;
      a++;
    }
    printf("  %s\n", cleartext);
  }
}

//
//
//
void ICACHE_FLASH_ATTR ReadDiskBlock(void *buf, uint8_t sectorNo,
                                     uint8_t trackNo, uint8_t diskNo) {
  if (diskNo > DISKCOUNT - 1)
    diskNo = DISKCOUNT - 1;
  if (trackNo > (TRACKSPERDISK - 1))
    trackNo = TRACKSPERDISK - 1;
  if (sectorNo > (SECTORSPERTRACK - 1))
    sectorNo = SECTORSPERTRACK - 1;

  uint32_t lba = SECTORSPERTRACK * trackNo + sectorNo;
  uint32_t flashloc =
      DISKFLASHOFFSET + DISKFLASHSIZE * diskNo + SECTORSIZE * lba;
  uint16_t myFlashSectorNo = flashloc / FLASHBLOCKSIZE;

#ifdef DEBUG
  printf("(read Sec=%d, Trk=%d Dsk=%d) LBA=%d myFlashSectorNo=%04X "
         "flashSectorNo=%04X  \n",
         sectorNo, trackNo, diskNo, lba, myFlashSectorNo, flashSectorNo);
#endif

  if (myFlashSectorNo != flashSectorNo) {
    Cache_Read_Disable();
    SPIUnlock();
    flashSectorNo = myFlashSectorNo;
#ifdef DEBUG
    printf("(FLASH READ sector %04X %08X\n", flashSectorNo,
           flashSectorNo * FLASHBLOCKSIZE);
#endif
    SPIRead(flashSectorNo * FLASHBLOCKSIZE, (uint32_t *)flashBuf,
            FLASHBLOCKSIZE);
    Cache_Read_Enable(0, 0, 1);
  }

  uint16_t fl = flashloc % FLASHBLOCKSIZE;
#ifdef DEBUG
  printf("(fl=0x%04X\n", fl);
#endif
  for (uint8_t i = 0; i < SECTORSIZE; i++) {
    ((uint8_t *)buf)[i] = flashBuf[fl + i];
  }

#ifdef DEBUG
  HexDump(z80_dma, 128, false);
#endif
}

//
//
//
void ICACHE_FLASH_ATTR WriteDiskBlock(void *buf, uint8_t sectorNo,
                                      uint8_t trackNo, uint8_t diskNo) {
  if (diskNo > DISKCOUNT - 1)
    diskNo = DISKCOUNT - 1;
  if (trackNo > (TRACKSPERDISK - 1))
    trackNo = TRACKSPERDISK - 1;
  if (sectorNo > (SECTORSPERTRACK - 1))
    sectorNo = SECTORSPERTRACK - 1;

  uint32_t lba = SECTORSPERTRACK * trackNo + sectorNo;
  uint32_t flashloc =
      DISKFLASHOFFSET + DISKFLASHSIZE * diskNo + SECTORSIZE * lba;
  uint16_t myFlashSectorNo = flashloc / FLASHBLOCKSIZE;

#ifdef DEBUG
  printf("(read Sec=%d, Trk=%d Dsk=%d) LBA=%d myFlashSectorNo=%04X "
         "flashSectorNo=%04X  \n",
         sectorNo, trackNo, diskNo, lba, myFlashSectorNo, flashSectorNo);
#endif

  Cache_Read_Disable();
  SPIUnlock();
  if (myFlashSectorNo != flashSectorNo) {
    flashSectorNo = myFlashSectorNo;
#ifdef DEBUG
    printf("(FLASH READ(for erase/write) sector %04X %08X\n", flashSectorNo,
           flashSectorNo * FLASHBLOCKSIZE);
#endif
    SPIRead(flashSectorNo * FLASHBLOCKSIZE, (uint32_t *)flashBuf,
            FLASHBLOCKSIZE);
  }
#ifdef DEBUG
  printf("(FLASH ERASE/WRITE sector %04X %08X\n", flashSectorNo,
         flashSectorNo * FLASHBLOCKSIZE);
#endif
  uint16_t fl = flashloc % FLASHBLOCKSIZE;
#ifdef DEBUG
  printf("(fl=0x%04X\n", fl);
#endif
  for (uint8_t i = 0; i < SECTORSIZE; i++) {
    flashBuf[fl + i] = ((uint8_t *)buf)[i];
  }
  SPIEraseSector(flashSectorNo);
  SPIWrite(flashSectorNo * FLASHBLOCKSIZE, (uint32_t *)flashBuf,
           FLASHBLOCKSIZE);
  Cache_Read_Enable(0, 0, 1);

#ifdef DEBUG
  HexDump(z80_dma, 128, false);
#endif
}

//  Cache_Read_Disable();
//	SPIUnlock();
//	SPIEraseBlock(0x40000>>16);
//	SPIWrite(0x40004, &stuff, 4);
//	stuff = 0;
//	SPIRead(0x40000, &sectorbuf, siezof(sectorbuf));
//	Cache_Read_Enable(0,0,1);

//
//
//
int main() {
  char *line;

  nosdk8266_init();
  InitUart();
  ets_delay_us(250000);
  printf("\n");

  Z80Reset(&machine.state);
  machine.is_done = 0;

  while (1) {
    printf("EMON:");
    line = GetLine();
    printf("\n");
    char cmd = line[0];
    line++; // Skip to next char

    //
    // Show help text
    //
    if (cmd == '?') {
      printf("EMON v0.2\n");
      printf("\td ADR [LEN] - Hexdump LEN bytes of memory starting at ADR\n");
      printf("\tD ADR [LEN] - Intel hexdump LEN bytes of memory starting at "
             "ADR\n");
      printf("\tL [OFFSET]  - Load an Intel hexdump into memory with OFFSET\n");
      printf("\tm ADR       - Modify memory contents starting at ADR\n");
      printf("\tr [REG VAL] - Display all registers, or set REG to VAL\n");
      printf("\tg [ADR]     - Start execution at PC or [ADR]\n");
      printf(
          "\ts [NUM]     - Single step 1 or [NUM] instructions, print once\n");
      printf("\tS [NUM]     - Single step 1 or [NUM] instructions, print every "
             "step\n");
      continue;
    }

    if (cmd == '1') {
      uint16_t ccp=1024*(CPMMEMORY-8);
      uint16_t bios=ccp+0x1600;
      uint8_t *p=z80code;
 
//      ets_memcpy(machine.memory + ccp, z80code, sizeof(z80code));
      printf("Patching in data into z80 %04x..%04x reading from flash %p (%p)\n",ccp,ccp+sizeof(z80code),z80code,p);
      for (uint16_t i=0; i<sizeof(z80code); i++) {
        machine.memory[ccp+i]=readRomByte(p++);
      }
      machine.state.pc = bios;
      ShowAllRegisters();
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
      if (pc != NONUM)
        machine.state.pc = pc;
      machine.is_done = 0;
      printf("Starting excution at 0x%04X\n", machine.state.pc);
      do {
        Z80Emulate(&machine.state, 1, &machine);
        if (cmd == 'g' && GetKey(false) == BREAKKEY)
          break;
      } while (!machine.is_done);
      printf("\n");
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
        if (cmd == 'S') {
          int i;
          unsigned char buffer[16];
          char dest[32];
          ets_memcpy(buffer, &machine.memory[machine.state.pc], 16);
          for (i = 0; i < 16; ++i)
            printf("%02X ", buffer[i]);
          printf("\n");
          i = Z80_Dasm(buffer, dest, 0);
          printf("%s  - %d bytes\n", dest, i);
        }
        Z80Emulate(&machine.state, 1, &machine);
        if (cmd == 'S')
          ShowAllRegisters();
        if (GetKey(false) == BREAKKEY)
          break;
      }
      if (cmd == 's')
        ShowAllRegisters();
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

#define EXIT 0x00
#define CONOUT 0x01
#define LIST 0x02
#define PUNCH 0x03
#define CONIN 0x04
#define CONST 0x05
#define SETDMA 0x06
#define SETTRK 0x07
#define SETSEC 0x08
#define HOME 0x09
#define SELDSK 0x0A
#define READ 0x0B
#define WRITE 0x0C

//
//
//
void ICACHE_FLASH_ATTR SystemCall(MACHINE *m, int opcode, int val, int instr) {
  uint8_t A = m->state.registers.byte[Z80_A];
  uint8_t B = m->state.registers.byte[Z80_B];
  uint8_t C = m->state.registers.byte[Z80_C];
  uint8_t D = m->state.registers.byte[Z80_D];
  uint8_t E = m->state.registers.byte[Z80_E];
  uint16_t BC = m->state.registers.word[Z80_BC];
  uint16_t DE = m->state.registers.word[Z80_DE];
  uint16_t HL = m->state.registers.word[Z80_HL];
  uint8_t argm1 = m->memory[m->state.pc - 1];
  uint8_t arg = m->memory[m->state.pc];
  uint8_t argp1 = m->memory[m->state.pc + 1];

#ifdef DEBUG
//   printf("opcode=%04x val=%04x instr=%04x arg-1=%02x arg=%02x arg+1=%02x
//   \n",opcode,val,instr,argm1,arg,argp1);
#endif

  switch (arg) {

  case EXIT:
    machine.is_done = 1;
    break;

  case CONOUT:
  case LIST:
  case PUNCH:
    printf("%c", C);
    break;

  // CONIN	The next console character is read into register A, and the
  // 		parity bit is set, high-order bit, to zero. If no console
  //		character is ready, wait until a character is typed before
  //		returning.
  case CONIN:
    m->state.registers.byte[Z80_A] = GetKey(true);
    break;

  // CONST	You should sample the status of the currently assigned
  //		console device and return 0FFH in register A if a
  //		character is ready to read and 00H in register A if no
  //		console characters are ready.
  case CONST:
    if (GetRxCnt() == 0) {
      m->state.registers.byte[Z80_A] = 0x00;
    } else {
      m->state.registers.byte[Z80_A] = 0xFF;
    }
    break;

  // SETDMA	Register BC contains the DMA (Disk Memory Access) address
  //		for subsequent read or write operations. For example, if
  //		B = 00H and C = 80H when SETDMA is called, all subsequent
  //		read operations read their data into 80H through 0FFH and
  //		all subsequent write operations get their data from 80H
  //		through 0FFH, until the next call to SETDMA occurs. The
  //		initial DMA address is assumed to be 80H. The controller
  //		need not actually support Direct Memory Access. If, for
  //		example, all data transfers are through I/O ports, the
  //		CBIOS that is constructed uses the 128 byte area starting
  //		at the selected DMA address for the memory buffer during
  //		the subsequent read or write operations.
  case SETDMA:
#ifdef DEBUG
    printf("(SETDMA %04x)\n", BC);
#endif
    z80_dma = BC;
    break;

  // SETTRK	Register BC contains the track number for subsequent disk
  //		accesses on the currently selected drive. The sector number
  //		in BC is the same as the number returned from the SECTRAN
  //		entry point. You can choose to seek the selected track at
  //		this time or delay the seek until the next read or write
  //		actually occurs. Register BC can take on values in the
  //		range 0-76 corresponding to valid track numbers for
  //		standard floppy disk drives and 0- 65535 for nonstandard
  //		disk subsystems.
  case SETTRK:
#ifdef DEBUG
    printf("(SETTRK B=%d C=%d)\n", B, C);
#endif
    z80_trk = BC & 0xFF;
    break;

  // SETSEC	Register BC contains the sector number, 1 through 26, for
  //		subsequent disk accesses on the currently selected drive.
  //		The sector number in BC is the same as the number returned
  //		from the SECTRAN entry point. You can choose to send this
  //		information to the controller at this point or delay sector
  //		selection until a read or write operation occurs.
  case SETSEC:
#ifdef DEBUG
    printf("(SETSEC B=%d C=%d)\n", B, C);
#endif
    z80_sec = BC - 1;
    break;

  // HOME	The disk head of the currently selected disk (initially
  //		disk A) is moved to the track 00 position. If the controller
  //		allows access to the track 0 flag from the drive, the head
  //		is stepped until the track 0 flag is detected. If the
  //		controller does not support this feature, the HOME call is
  //		translated into a call to SETTRK with a parameter of 0.
  case HOME:
#ifdef DEBUG
    printf("(HOME)\n");
#endif
    z80_trk = 0;
    break;

  // SELDSK	The disk drive given by register C is selected for further
  //		operations, where register C contains 0 for drive A, 1 for
  //		drive B, and so on up to 15 for drive P (the standard CP/M
  //		distribution version supports four drives). On each disk
  //		select, SELDSK must return in HL the base address of a
  //		16-byte area, called the Disk Parameter Header, described
  //		in Section 6.10. For standard floppy disk drives, the
  //		contents of the header and associated tables do not change;
  //		thus, the program segment included in the sample CBIOS
  //		performs this operation automatically.
  //
  //		If there is an attempt to select a nonexistent drive,
  //		SELDSK returns HL = 0000H as an error indicator. Although
  //		SELDSK must return the header address on each call, it is
  //		advisable to postpone the physical disk select operation
  //		until an I/O function (seek, read, or write) is actually
  //		performed, because disk selects often occur without
  //		ultimately performing any disk I/O, and many controllers
  //		unload the head of the current disk before selecting the
  //		new drive. This causes an excessive amount of noise and
  //		disk wear. The least significant bit of register E is zero
  //		if this is the first occurrence of the drive select since
  //		the last cold or warm start
  case SELDSK:
#ifdef DEBUG
    printf("(SELDSK)\n");
#endif
    z80_dsk = C & 0x0F;
    break;

  // READ 	Assuming the drive has been selected, the track has been
  //		set, and the DMA address has been specified, the READ
  //		subroutine attempts to read eone sector based upon these
  //		parameters and returns the following error codes in
  //		register A:
  //
  //		0 - no errors occurred
  //
  //		1 - nonrecoverable error condition occurred
  //
  //		Currently, CP/M responds only to a zero or nonzero value as
  //		the return code. That is, if the value in register A is 0,
  //		CP/M assumes that the disk operation was completed properly.
  //		If an error occurs the CBIOS should attempt at least 10
  //		retries to see if the error is recoverable. When an error is
  //		reported the BDOS prints the message BDOS ERR ON x: BAD
  //		SECTOR. The operator then has the option of pressing a
  //		carriage return to ignore the error, or CTRL-C to abort.
  case READ:
#ifdef DEBUG
    printf("(READ D:%d T:%d S:%d)\n", z80_dsk, z80_trk, z80_sec);
#endif

    ReadDiskBlock(&(m->memory[z80_dma]), z80_sec, z80_trk, z80_dsk);
    m->state.registers.byte[Z80_A] = 0x00;

    break;

  // WRITE	Data is written from the currently selected DMA address to
  //		the currently selected drive, track, and sector. For floppy
  //		disks, the data should be marked as nondeleted data to
  //		maintain compatibility with other CP/M systems. The error
  //		codes given in the READ command are returned in register A,
  //		with error recovery attempts as described above.
  case WRITE:
#ifdef DEBUG
    printf("(WRITE)\n");
#endif
    WriteDiskBlock(&(m->memory[z80_dma]), z80_sec, z80_trk, z80_dsk);
    m->state.registers.byte[Z80_A] = 0x00;
    break;

  default:
    printf("\nINVALID argument to IN/OUT, ARG=0x%02x PC=%04x\n", arg,
           m->state.pc);
    machine.is_done = 1;
  }
}
