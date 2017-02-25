#include "c_types.h"
#include "eagle_soc.h"
#include "espincludes.h"
#include "ets_sys.h"
#include "gpio.h"
#include "nosdk8266.h"
#include "uart.h"
#include "uart_dev.h"
#include "uart_register.h"

#define RODATA_ATTR  __attribute__((section(".irom.text"))) __attribute__((aligned(4)))
#define ROMSTR_ATTR  __attribute__((section(".irom.text.romstr"))) __attribute__((aligned(4)))

volatile uint32_t KEEPMEHERE __attribute__((section(".irom.text"))) = 0xDEADBEEF;

static const char z80code[] RODATA_ATTR = {
  #include "hex/boot.data"
};

#include "z80/z80emu.h"
#include "z80/z80user.h"

#include "disasm/disasm.h"
#include "monitor.h"
#include "utils.h"

MACHINE machine;

uint16_t z80_dma = 0x80;
uint16_t z80_trk = 0x00;
uint16_t z80_sec = 0x01;
uint16_t z80_dsk = 0x00;
uint8_t biostrace = 0;
uint8_t breakatbios = 0;

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

#define BREAKKEY '`'

#define SECTORSIZE 128
#define SECTORSPERTRACK 26
#define TRACKSPERDISK 77
#define DISKFLASHOFFSET 0x40000 // Location in flash for first disk
#define DISKFLASHSIZE 0x40000   // Number of bytes in flash for each disk
#define DISKCOUNT 15            // We have 15 disks A..O
#define FLASHBLOCKSIZE 4096

static uint16_t flashSectorNo = 0xFFFF;
static uint8_t flashBuf[FLASHBLOCKSIZE];


//
//
//
void ICACHE_FLASH_ATTR ReadDiskBlock(uint16_t mdest, uint8_t sectorNo,
                                     uint8_t trackNo, uint8_t diskNo) {

  if (diskNo > (DISKCOUNT - 1))          diskNo = DISKCOUNT - 1;
  if (trackNo > (TRACKSPERDISK - 1))     trackNo = TRACKSPERDISK - 1;
  if (sectorNo > (SECTORSPERTRACK - 1))  sectorNo = SECTORSPERTRACK - 1;

  uint32_t lba = SECTORSPERTRACK * trackNo + sectorNo;
  uint32_t flashloc = DISKFLASHOFFSET + DISKFLASHSIZE * diskNo + SECTORSIZE * lba;
  uint16_t myFlashSectorNo = flashloc / FLASHBLOCKSIZE;

#ifdef DEBUG
  printf("(read Sec=%d, Trk=%d Dsk=%d) LBA=%d myFlashSectorNo=%04X flashSectorNo=%04X  \n",
           sectorNo, trackNo, diskNo, lba, myFlashSectorNo, flashSectorNo);
#endif

  if (myFlashSectorNo != flashSectorNo) {
    Cache_Read_Disable();
    SPIUnlock();
    flashSectorNo = myFlashSectorNo;
#ifdef DEBUG
    printf("(FLASH READ sector %04X %08X\n", flashSectorNo, flashSectorNo * FLASHBLOCKSIZE);
#endif
#ifdef DEBUG1
    printf("r");
#endif
    SPIRead(flashSectorNo * FLASHBLOCKSIZE, (uint32_t *)flashBuf, FLASHBLOCKSIZE);
    Cache_Read_Enable(0, 0, 1);
  }

  uint16_t fl = flashloc % FLASHBLOCKSIZE;
#ifdef DEBUG
  printf("(fl=0x%04X\n", fl);
#endif
  for (uint8_t i = 0; i < SECTORSIZE; i++) {
    machine.memory[mdest+i] = flashBuf[fl + i];
  }

#ifdef DEBUG
  HexDump(mdest, 128, false);
#endif
}

//
//
//
void ICACHE_FLASH_ATTR WriteDiskBlock(uint16_t msrc, uint8_t sectorNo,
                                      uint8_t trackNo, uint8_t diskNo) {

  if (diskNo > (DISKCOUNT - 1))          diskNo = DISKCOUNT - 1;
  if (trackNo > (TRACKSPERDISK - 1))     trackNo = TRACKSPERDISK - 1;
  if (sectorNo > (SECTORSPERTRACK - 1))  sectorNo = SECTORSPERTRACK - 1;

  uint32_t lba = SECTORSPERTRACK * trackNo + sectorNo;
  uint32_t flashloc = DISKFLASHOFFSET + DISKFLASHSIZE * diskNo + SECTORSIZE * lba;
  uint16_t myFlashSectorNo = flashloc / FLASHBLOCKSIZE;

#ifdef DEBUG
  printf("(read Sec=%d, Trk=%d Dsk=%d) LBA=%d myFlashSectorNo=%04X flashSectorNo=%04X\n",
         sectorNo, trackNo, diskNo, lba, myFlashSectorNo, flashSectorNo);
#endif

  Cache_Read_Disable();
  SPIUnlock();
  if (myFlashSectorNo != flashSectorNo) {
    flashSectorNo = myFlashSectorNo;
#ifdef DEBUG
    printf("(FLASH READ(for erase/write) sector %04X %08X\n", flashSectorNo, flashSectorNo * FLASHBLOCKSIZE);
#endif
    SPIRead(flashSectorNo * FLASHBLOCKSIZE, (uint32_t *)flashBuf, FLASHBLOCKSIZE);
  }
#ifdef DEBUG
  printf("(FLASH ERASE/WRITE sector %04X %08X\n", flashSectorNo, flashSectorNo * FLASHBLOCKSIZE);
#endif
  uint16_t fl = flashloc % FLASHBLOCKSIZE;
#ifdef DEBUG
  printf("(fl=0x%04X\n", fl);
#endif
  for (uint8_t i = 0; i < SECTORSIZE; i++) {
    flashBuf[fl + i] = machine.memory[msrc+i];
  }
  printf("w");
  SPIEraseSector(flashSectorNo);
  SPIWrite(flashSectorNo * FLASHBLOCKSIZE, (uint32_t *)flashBuf, FLASHBLOCKSIZE);
  Cache_Read_Enable(0, 0, 1);

#ifdef DEBUG
  HexDump(msrc, 128, false);
#endif
}


//
//
//
void Execute(bool canbreak) {
  machine.is_done = 0;
  printf("Starting excution at 0x%04X\n", machine.state.pc);
  do {
    Z80Emulate(&machine.state, 1, &machine);
    if (canbreak && (GetKey(false) == BREAKKEY)) {
      break;
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

    if (cmd=='B') {
      // Read first sector at first track first disk to memory 0x0000
      ReadDiskBlock(0x0000, 0, 0, 0);	
      machine.state.pc = 0x0000;
      Execute(true);
      continue;
    }

    if (cmd == '+') {
      biostrace=1;
      printf("Biostrace enabled\n");
      continue;
    }
    if (cmd == '-') {
      biostrace=0;
      printf("Biostrace disabled\n");
      continue;
    }

    if (cmd == '!') {
      breakatbios=CONIN;
      printf("Break at CONSOLE IN enabled\n");
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
      Execute((cmd=='g'));  
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
    if (biostrace) printf("{EX}");
    machine.is_done = 1;
    break;

  case CONOUT:
    if (biostrace) printf("{CO}");
    printf("%c", C);
    break;

  case LIST:
    if (biostrace) printf("{LO}");
    printf("%c", C);
    break;

  case PUNCH:
    if (biostrace) printf("{PO}");
    printf("%c", C);
    break;

  // CONIN	The next console character is read into register A, and the
  // 		parity bit is set, high-order bit, to zero. If no console
  //		character is ready, wait until a character is typed before
  //		returning.
  case CONIN:
    if (biostrace) printf("{CI}");
    m->state.registers.byte[Z80_A] = GetKey(true);
    if (breakatbios==CONIN) {
      machine.is_done = 1;
      printf("CONIN bios break\n");
      ShowAllRegisters();
    }

    break;

  // CONST	You should sample the status of the currently assigned
  //		console device and return 0FFH in register A if a
  //		character is ready to read and 00H in register A if no
  //		console characters are ready.
  case CONST:
    if (biostrace) printf("{CS}");
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
    if (biostrace) printf("{LS}");
    m->state.registers.byte[Z80_A] = 0x00;
    break;

  //
  // The next character is read from the currently assigned reader 
  // device into register A with zero parity (high-order bit must be 
  // zero); an end-of- file condition is reported by returning an 
  // ASCII CTRL-Z(1AH).
  //
  case READER:
    if (biostrace) printf("{RI}");
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
    if (biostrace) printf("{SD}");
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
    if (biostrace) printf("{ST}");
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
    if (biostrace) printf("{SS}");
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
    if (biostrace) printf("{HO}");
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
    if (biostrace) printf("{SD}");
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
    if (biostrace) printf("{RD}");
#ifdef DEBUG
    printf("(READ D:%d T:%d S:%d)\n", z80_dsk, z80_trk, z80_sec);
#endif

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
    if (biostrace) printf("{WR}");
#ifdef DEBUG
    printf("(WRITE)\n");
#endif
    WriteDiskBlock(z80_dma, z80_sec, z80_trk, z80_dsk);
    m->state.registers.byte[Z80_A] = 0x00;
    break;

  default:
    printf("\nINVALID argument to IN/OUT, ARG=0x%02x PC=%04x\n", arg,
           m->state.pc);
    machine.is_done = 1;
  }
  if (biostrace) printf("\n");
}
