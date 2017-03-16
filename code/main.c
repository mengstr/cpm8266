#include "c_types.h"
#include "eagle_soc.h"
#include "espincludes.h"
#include "ets_sys.h"
#include "gpio.h"
#include "nosdk8266.h"
#include "uart.h"
#include "uart_dev.h"
#include "uart_register.h"
#include "gpio16.h"
#include "flash.h"

#define RODATA_ATTR __attribute__((section(".irom.text"))) __attribute__((aligned(4)))
#define ROMSTR_ATTR __attribute__((section(".irom.text.romstr"))) __attribute__((aligned(4)))

// Make sure there always is something in the irom segment or the uploader
// will barf
volatile uint32_t KEEPMEHERE __attribute__((section(".irom.text"))) = 0xDEADBEEF;

static const char z80code[] RODATA_ATTR = {
#include "hex/boot.data"
};

#include "z80/z80emu.h"
#include "z80/z80user.h"

#include "disasm/disasm.h"
#include "monitor.h"
#include "utils.h"

// 01=    02=r/w    04=bios/bdos  08=
const uint8_t DBG = 0x00;

MACHINE machine;
uint32_t cycles=0;

uint16_t z80_dma = 0x80;
uint16_t z80_trk = 0x00;
uint16_t z80_sec = 0x01;
uint16_t z80_dsk = 0x00;
uint8_t biostrace = 0;

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
#define READER 0x0D
#define LISTST 0x0E
#define CYCLES 0xFE
#define HALT 0xFF

#define BREAKKEY '`'


//
//
//
void DebugBiosBdos() {
  switch (machine.state.pc) {
    case 0xF100: printf("(BIOS BOOT)"); break;
    case 0xF103: printf("(BIOS WBOOT)"); break;
    case 0xF106: printf("(BIOS CONST)"); break;
    case 0xF109: printf("(BIOS CONIN)"); break;
    case 0xF10C: printf("(BIOS CONOUT)"); break;
    case 0xF10F: printf("(BIOS LIST)"); break;
    case 0xF112: printf("(BIOS PUNCH)"); break;
    case 0xF115: printf("(BIOS READER)"); break;
    case 0xF118: printf("(BIOS HOME)"); break;
    case 0xF11B: printf("(BIOS SELDSK)"); break;
    case 0xF11E: printf("(BIOS SETTRK)"); break;
    case 0xF121: printf("(BIOS SETSEC)"); break;
    case 0xF124: printf("(BIOS SETDMA)"); break;
    case 0xF127: printf("(BIOS READ)"); break;
    case 0xF12A: printf("(BIOS WRITE)"); break;
    case 0xF12D: printf("(BIOS LISTST)"); break;
    case 0xF130: printf("(BIOS SECTRN)"); break;
    case 0xE311:  // BDOS ENTRY POINTS
      switch (machine.state.registers.byte[Z80_C]) {
        case 0: printf("(BDOS WBOOT)"); break;
        case 1: printf("(BDOS GETCON)"); break;
        case 2: printf("(BDOS OUTCON)"); break;
        case 3: printf("(BDOS GETRDR)"); break;
        case 4: printf("(BDOS PUNCH)"); break;
        case 5: printf("(BDOS LIST)"); break;
        case 6: printf("(BDOS DIRCIO %02x)", machine.state.registers.byte[Z80_E]); break;
        case 7: printf("(BDOS GETIOB)"); break;
        case 8: printf("(BDOS SETIOB)"); break;
        case 9: printf("(BDOS PRTSTR)"); break;
        case 10: printf("(BDOS RDBUFF)"); break;
        case 11: printf("(BDOS GETCSTS)"); break;
        case 12: printf("(BDOS GETVER)"); break;
        case 13: printf("(BDOS RSTDSK)"); break;
        case 14: printf("(BDOS SETDSK)"); break;
        case 15: printf("(BDOS OPENFIL)"); break;
        case 16: printf("(BDOS CLOSEFIL)"); break;
        case 17: printf("(BDOS GETFST)"); break;
        case 18: printf("(BDOS GETNXT)"); break;
        case 19: printf("(BDOS DELFILE)"); break;
        case 20: printf("(BDOS READSEQ)"); break;
        case 21: printf("(BDOS WRTSEQ)"); break;
        case 22: printf("(BDOS FCREATE)"); break;
        case 23: printf("(BDOS RENFILE)"); break;
        case 24: printf("(BDOS GETLOG)"); break;
        case 25: printf("(BDOS GETCRNT)"); break;
        case 26: printf("(BDOS PUTDMA)"); break;
        case 27: printf("(BDOS GETALOC)"); break;
        case 28: printf("(BDOS WRTPRTD)"); break;
        case 29: printf("(BDOS GETROV)"); break;
        case 30: printf("(BDOS SETATTR)"); break;
        case 31: printf("(BDOS GETPARM)"); break;
        case 32: printf("(BDOS GETUSER)"); break;
        case 33: printf("(BDOS RDRANDOM)"); break;
        case 34: printf("(BDOS WTRANDOM)"); break;
        case 35: printf("(BDOS FILESIZE)"); break;
        case 36: printf("(BDOS SETRAN)"); break;
        case 37: printf("(BDOS LOGOFF)"); break;
        case 38: printf("(BDOS RTN)"); break;
        case 39: printf("(BDOS RTN)"); break;
        case 40: printf("(BDOS WTSPECL)"); break;
        default: printf("(BDOS C=%d)", machine.state.registers.byte[Z80_C]); break;
      }
  }
}


//
//
//
void Execute(bool canbreak) {
  uint32_t flushCnt=0;
  machine.is_done = 0;
  printf("Starting excution at 0x%04X\n", machine.state.pc);
  cycles=0;
  do {
    cycles+=Z80Emulate(&machine.state, 2, &machine);
    if (DBG & 4) DebugBiosBdos();
//    if (canbreak && (GetKey(false) == BREAKKEY)) break;
    if ((flushCnt++>2000000)) {
      FlushDisk(true);  // Flush Disk in standalone mode
      flushCnt=0;
    }
  } while (!machine.is_done);
  printf("\n");
}





//
//
//
int main() {
  char *line;

  nosdk8266_init();

  //  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);
  //  PIN_DIR_OUTPUT=_BV(0);
  gpio16_output_conf();
  gpio16_output_set(1);	// PIN_OUT_SET=_BV(0);

  ets_delay_us(250000);
  InitUart();

  uint32_t baud=AutoBaud();
  uart_div_modify(UART0, (PERIPH_FREQ * 1000000) / baud);

  printf("\n\ncpm8266 - Z80 Emulator and CP/M 2.2 system version %d.%d\n\n",VERSION/10,VERSION%10);
  ets_delay_us(250000);
  FlushUart();

  Z80Reset(&machine.state);
  machine.is_done = 0;

  while (1) {
    printf("EMON:");
    line = GetLine();
    printf("\n");
    char cmd = line[0];
    line++;  // Skip to next char

    //
    // Show help text
    //
    if (cmd == '?') {
      printf("EMON v0.2\n");
      printf("\td ADR [LEN] - Hexdump LEN bytes of memory starting at ADR\n");
      printf("\tD ADR [LEN] - Intel hexdump LEN bytes of memory starting at ADR\n");
      printf("\tL [OFFSET]  - Load an Intel hexdump into memory with OFFSET\n");
      printf("\tm ADR       - Modify memory contents starting at ADR\n");
      printf("\tr [REG VAL] - Display all registers, or set REG to VAL\n");
      printf("\tg [ADR]     - Start execution at PC or [ADR], can break with `\n");
      printf("\tG [ADR]     - Start execution at PC or [ADR]\n");
      printf("\ts [NUM]     - Single step 1 or [NUM] instructions, print once\n");
      printf("\tS [NUM]     - Single step 1 or [NUM] instructions, print every step\n");
      printf("\tB           - Load D0/T0/S0 to 0x0000 and execute\n");
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
        // if (cmd == 'S') {
        //   int i;
        //   unsigned char buffer[16];
        //   char dest[32];
        //   ets_memcpy(buffer, &machine.memory[machine.state.pc], 16);
        //   for (i = 0; i < 16; ++i)
        //     printf("%02X ", buffer[i]);
        //   printf("\n");
        //   i = Z80_Dasm(buffer, dest, 0);
        //   printf("%s  - %d bytes\n", dest, i);
        // }
        cycles+=Z80Emulate(&machine.state, 1, &machine);
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

  switch (arg) {
    case EXIT:
      if (DBG & 8) printf("{EX}");
      machine.is_done = 1;
      break;

    case CONOUT:
      if (DBG & 8) printf("{CO}");
      printf("%c", C);
      break;

    case LIST:
      if (DBG & 8) printf("{LO}");
      printf("%c", C);
      break;

    case PUNCH:
      if (DBG & 8) printf("{PO}");
      printf("%c", C);
      break;

    // CONIN	The next console character is read into register A, and the
    // 		parity bit is set, high-order bit, to zero. If no console
    //		character is ready, wait until a character is typed before
    //		returning.
    case CONIN:
      if (DBG & 8) printf("{CI}");
      m->state.registers.byte[Z80_A] = GetKey(true);
      break;

    // CONST	You should sample the status of the currently assigned
    //		console device and return 0FFH in register A if a
    //		character is ready to read and 00H in register A if no
    //		console characters are ready.
    case CONST:
      if (DBG & 8) printf("{CS}");
      if (GetRxCnt() == 0) {
        m->state.registers.byte[Z80_A] = 0x00;
      } else {
        m->state.registers.byte[Z80_A] = 0xFF;
      }
      break;

    //
    // Return the ready status of the list device used by the DESPOOL
    // program to improve console response during its operation The value
    // 00 is returned in A if the list device is not ready to accept a
    // character and 0FFH if a character can be sent to the printer.A 00
    // value should be returned if LIST status is not implemented
    //
    case LISTST:
      if (DBG & 8) printf("{LS}");
      m->state.registers.byte[Z80_A] = 0x00;
      break;

    //
    // The next character is read from the currently assigned reader
    // device into register A with zero parity (high-order bit must be
    // zero); an end-of- file condition is reported by returning an
    // ASCII CTRL-Z(1AH).
    //
    case READER:
      if (DBG & 8) printf("{RI}");
      m->state.registers.byte[Z80_A] = 0x1A;
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
      if (DBG & 8) printf("{SD}");
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
      if (DBG & 8) printf("{ST}");
      z80_trk = BC & 0xFF;
      break;

    // SETSEC	Register BC contains the sector number, 1 through 26, for
    //		subsequent disk accesses on the currently selected drive.
    //		The sector number in BC is the same as the number returned
    //		from the SECTRAN entry point. You can choose to send this
    //		information to the controller at this point or delay sector
    //		selection until a read or write operation occurs.
    case SETSEC:
      if (DBG & 8) printf("{SS}");
      z80_sec = BC - 1;
      break;

    // HOME	The disk head of the currently selected disk (initially
    //		disk A) is moved to the track 00 position. If the controller
    //		allows access to the track 0 flag from the drive, the head
    //		is stepped until the track 0 flag is detected. If the
    //		controller does not support this feature, the HOME call is
    //		translated into a call to SETTRK with a parameter of 0.
    case HOME:
      if (DBG & 8) printf("{HO}");
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
      if (DBG & 8) printf("{SD}");
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
      if (DBG & 8) printf("{RD}");
      ReadDiskBlock(z80_dma, z80_sec, z80_trk, z80_dsk);
      m->state.registers.byte[Z80_A] = 0x00;

      break;

    // WRITE	Data is written from the currently selected DMA address to
    //		the currently selected drive, track, and sector. For floppy
    //		disks, the data should be marked as nondeleted data to
    //		maintain compatibility with other CP/M systems. The error
    //		codes given in the READ command are returned in register A,
    //		with error recovery attempts as described above.
    case WRITE:
      if (DBG & 8) printf("{WR}");
      WriteDiskBlock(z80_dma, z80_sec, z80_trk, z80_dsk);
      m->state.registers.byte[Z80_A] = 0x00;
      break;

    case CYCLES:
      printf("{CYCLES:%u}",cycles);
      break;

    case HALT:
      printf("\n{Exiting}\n");
      machine.is_done = 1;
      break;

    default:
      printf("\nINVALID argument to IN/OUT, ARG=0x%02x PC=%04x\n", arg, m->state.pc);
      machine.is_done = 1;
  }
  if (DBG & 8) printf("\n");
}
