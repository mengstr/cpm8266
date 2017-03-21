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

static const char z80code[] RODATA_ATTR = {
#include "CPM22/boot.data"
};
#include "z80/z80emu.h"
#include "z80/z80user.h"
#include "monitor.h"
#include "utils.h"

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
void Execute(bool canbreak) {
  uint32_t flushCnt=0;
  machine.is_done = 0;
  ets_printf("Starting excution at 0x%04X\r\n", machine.state.pc);
  cycles=0;
  do {
    cycles+=Z80Emulate(&machine.state, 2, &machine);
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

  Z80Reset(&machine.state);
  machine.is_done = 0;

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
      machine.is_done = 1;
      break;

    case CONOUT:
      ets_printf("%c", C);
      break;

    case LIST:
      ets_printf("%c", C);
      break;

    case PUNCH:
      ets_printf("%c", C);
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

    //
    // Return the ready status of the list device used by the DESPOOL
    // program to improve console response during its operation The value
    // 00 is returned in A if the list device is not ready to accept a
    // character and 0FFH if a character can be sent to the printer.A 00
    // value should be returned if LIST status is not implemented
    //
    case LISTST:
      m->state.registers.byte[Z80_A] = 0x00;
      break;

    //
    // The next character is read from the currently assigned reader
    // device into register A with zero parity (high-order bit must be
    // zero); an end-of- file condition is reported by returning an
    // ASCII CTRL-Z(1AH).
    //
    case READER:
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
      z80_trk = BC & 0xFF;
      break;

    // SETSEC	Register BC contains the sector number, 1 through 26, for
    //		subsequent disk accesses on the currently selected drive.
    //		The sector number in BC is the same as the number returned
    //		from the SECTRAN entry point. You can choose to send this
    //		information to the controller at this point or delay sector
    //		selection until a read or write operation occurs.
    case SETSEC:
      z80_sec = BC - 1;
      break;

    // HOME	The disk head of the currently selected disk (initially
    //		disk A) is moved to the track 00 position. If the controller
    //		allows access to the track 0 flag from the drive, the head
    //		is stepped until the track 0 flag is detected. If the
    //		controller does not support this feature, the HOME call is
    //		translated into a call to SETTRK with a parameter of 0.
    case HOME:
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
      WriteDiskBlock(z80_dma, z80_sec, z80_trk, z80_dsk);
      m->state.registers.byte[Z80_A] = 0x00;
      break;

    case CYCLES:
      ets_printf("\r\n{CYCLES:%u}\r\n",cycles);
      break;

    case HALT:
      ets_printf("\r\n{Exiting}\r\n");
      machine.is_done = 1;
      break;

    default:
      ets_printf("\nINVALID argument to IN/OUT, ARG=0x%02x PC=%04x\r\n", arg, m->state.pc);
      machine.is_done = 1;
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

  cpm_main();
}
#endif



#ifdef WIFI
#endif
