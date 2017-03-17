 #include "c_types.h"
 #include "os_type.h"
#ifdef NOSDK
 #define os_printf printf
 #include "eagle_soc.h"
 #include "ets_sys.h"
 #include "gpio.h"
 #include "espincludes.h"
 #include "nosdk8266.h"
#endif
#ifdef WIFI
int os_printf(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
int os_snprintf(char *str, size_t size, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
int os_printf_plus(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
void ets_timer_arm_new(volatile os_timer_t *a, int b, int c, int isMstimer);
void ets_timer_disarm(volatile os_timer_t *a);
void ets_timer_setfn(volatile os_timer_t *t, ETSTimerFunc *fn, void *parg);
void ets_isr_attach(int intr, void *handler, void *arg);
void ets_isr_mask(unsigned intr);
void ets_isr_unmask(unsigned intr);
void ets_delay_us(int ms);
 #include "ets_sys.h"
 #include "osapi.h"
 #include "gpio.h"
 #include "user_interface.h"
 #include "user_config.h"
#endif

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
#include "monitor.h"
#include "utils.h"
#ifdef NOSDK
# include "disasm/disasm.h"
#endif

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
void ICACHE_FLASH_ATTR DebugBiosBdos() {
  switch (machine.state.pc) {
    case 0xF100: os_printf("(BIOS BOOT)"); break;
    case 0xF103: os_printf("(BIOS WBOOT)"); break;
    case 0xF106: os_printf("(BIOS CONST)"); break;
    case 0xF109: os_printf("(BIOS CONIN)"); break;
    case 0xF10C: os_printf("(BIOS CONOUT)"); break;
    case 0xF10F: os_printf("(BIOS LIST)"); break;
    case 0xF112: os_printf("(BIOS PUNCH)"); break;
    case 0xF115: os_printf("(BIOS READER)"); break;
    case 0xF118: os_printf("(BIOS HOME)"); break;
    case 0xF11B: os_printf("(BIOS SELDSK)"); break;
    case 0xF11E: os_printf("(BIOS SETTRK)"); break;
    case 0xF121: os_printf("(BIOS SETSEC)"); break;
    case 0xF124: os_printf("(BIOS SETDMA)"); break;
    case 0xF127: os_printf("(BIOS READ)"); break;
    case 0xF12A: os_printf("(BIOS WRITE)"); break;
    case 0xF12D: os_printf("(BIOS LISTST)"); break;
    case 0xF130: os_printf("(BIOS SECTRN)"); break;
    case 0xE311:  // BDOS ENTRY POINTS
      switch (machine.state.registers.byte[Z80_C]) {
        case 0: os_printf("(BDOS WBOOT)"); break;
        case 1: os_printf("(BDOS GETCON)"); break;
        case 2: os_printf("(BDOS OUTCON)"); break;
        case 3: os_printf("(BDOS GETRDR)"); break;
        case 4: os_printf("(BDOS PUNCH)"); break;
        case 5: os_printf("(BDOS LIST)"); break;
        case 6: os_printf("(BDOS DIRCIO %02x)", machine.state.registers.byte[Z80_E]); break;
        case 7: os_printf("(BDOS GETIOB)"); break;
        case 8: os_printf("(BDOS SETIOB)"); break;
        case 9: os_printf("(BDOS PRTSTR)"); break;
        case 10: os_printf("(BDOS RDBUFF)"); break;
        case 11: os_printf("(BDOS GETCSTS)"); break;
        case 12: os_printf("(BDOS GETVER)"); break;
        case 13: os_printf("(BDOS RSTDSK)"); break;
        case 14: os_printf("(BDOS SETDSK)"); break;
        case 15: os_printf("(BDOS OPENFIL)"); break;
        case 16: os_printf("(BDOS CLOSEFIL)"); break;
        case 17: os_printf("(BDOS GETFST)"); break;
        case 18: os_printf("(BDOS GETNXT)"); break;
        case 19: os_printf("(BDOS DELFILE)"); break;
        case 20: os_printf("(BDOS READSEQ)"); break;
        case 21: os_printf("(BDOS WRTSEQ)"); break;
        case 22: os_printf("(BDOS FCREATE)"); break;
        case 23: os_printf("(BDOS RENFILE)"); break;
        case 24: os_printf("(BDOS GETLOG)"); break;
        case 25: os_printf("(BDOS GETCRNT)"); break;
        case 26: os_printf("(BDOS PUTDMA)"); break;
        case 27: os_printf("(BDOS GETALOC)"); break;
        case 28: os_printf("(BDOS WRTPRTD)"); break;
        case 29: os_printf("(BDOS GETROV)"); break;
        case 30: os_printf("(BDOS SETATTR)"); break;
        case 31: os_printf("(BDOS GETPARM)"); break;
        case 32: os_printf("(BDOS GETUSER)"); break;
        case 33: os_printf("(BDOS RDRANDOM)"); break;
        case 34: os_printf("(BDOS WTRANDOM)"); break;
        case 35: os_printf("(BDOS FILESIZE)"); break;
        case 36: os_printf("(BDOS SETRAN)"); break;
        case 37: os_printf("(BDOS LOGOFF)"); break;
        case 38: os_printf("(BDOS RTN)"); break;
        case 39: os_printf("(BDOS RTN)"); break;
        case 40: os_printf("(BDOS WTSPECL)"); break;
        default: os_printf("(BDOS C=%d)", machine.state.registers.byte[Z80_C]); break;
      }
  }
}


//
//
//
void Execute(bool canbreak) {
  uint32_t flushCnt=0;
  machine.is_done = 0;
  os_printf("Starting excution at 0x%04X\n", machine.state.pc);
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
  os_printf("\n");
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
    os_printf("EMON:");
    line = GetLine();
    os_printf("\n");
    char cmd = line[0];
    line++;  // Skip to next char

    //
    // Show help text
    //
    if (cmd == '?') {
      os_printf("EMON v0.2\n");
      os_printf("\td ADR [LEN] - Hexdump LEN bytes of memory starting at ADR\n");
      os_printf("\tD ADR [LEN] - Intel hexdump LEN bytes of memory starting at ADR\n");
      os_printf("\tL [OFFSET]  - Load an Intel hexdump into memory with OFFSET\n");
      os_printf("\tm ADR       - Modify memory contents starting at ADR\n");
      os_printf("\tr [REG VAL] - Display all registers, or set REG to VAL\n");
      os_printf("\tg [ADR]     - Start execution at PC or [ADR], can break with `\n");
      os_printf("\tG [ADR]     - Start execution at PC or [ADR]\n");
      os_printf("\ts [NUM]     - Single step 1 or [NUM] instructions, print once\n");
      os_printf("\tS [NUM]     - Single step 1 or [NUM] instructions, print every step\n");
      os_printf("\tB           - Load D0/T0/S0 to 0x0000 and execute\n");
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
        //     os_printf("%02X ", buffer[i]);
        //   os_printf("\n");
        //   i = Z80_Dasm(buffer, dest, 0);
        //   os_printf("%s  - %d bytes\n", dest, i);
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
      if (DBG & 8) os_printf("{EX}");
      machine.is_done = 1;
      break;

    case CONOUT:
      if (DBG & 8) os_printf("{CO}");
      os_printf("%c", C);
      break;

    case LIST:
      if (DBG & 8) os_printf("{LO}");
      os_printf("%c", C);
      break;

    case PUNCH:
      if (DBG & 8) os_printf("{PO}");
      os_printf("%c", C);
      break;

    // CONIN	The next console character is read into register A, and the
    // 		parity bit is set, high-order bit, to zero. If no console
    //		character is ready, wait until a character is typed before
    //		returning.
    case CONIN:
      if (DBG & 8) os_printf("{CI}");
      m->state.registers.byte[Z80_A] = GetKey(true);
      break;

    // CONST	You should sample the status of the currently assigned
    //		console device and return 0FFH in register A if a
    //		character is ready to read and 00H in register A if no
    //		console characters are ready.
    case CONST:
      if (DBG & 8) os_printf("{CS}");
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
      if (DBG & 8) os_printf("{LS}");
      m->state.registers.byte[Z80_A] = 0x00;
      break;

    //
    // The next character is read from the currently assigned reader
    // device into register A with zero parity (high-order bit must be
    // zero); an end-of- file condition is reported by returning an
    // ASCII CTRL-Z(1AH).
    //
    case READER:
      if (DBG & 8) os_printf("{RI}");
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
      if (DBG & 8) os_printf("{SD}");
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
      if (DBG & 8) os_printf("{ST}");
      z80_trk = BC & 0xFF;
      break;

    // SETSEC	Register BC contains the sector number, 1 through 26, for
    //		subsequent disk accesses on the currently selected drive.
    //		The sector number in BC is the same as the number returned
    //		from the SECTRAN entry point. You can choose to send this
    //		information to the controller at this point or delay sector
    //		selection until a read or write operation occurs.
    case SETSEC:
      if (DBG & 8) os_printf("{SS}");
      z80_sec = BC - 1;
      break;

    // HOME	The disk head of the currently selected disk (initially
    //		disk A) is moved to the track 00 position. If the controller
    //		allows access to the track 0 flag from the drive, the head
    //		is stepped until the track 0 flag is detected. If the
    //		controller does not support this feature, the HOME call is
    //		translated into a call to SETTRK with a parameter of 0.
    case HOME:
      if (DBG & 8) os_printf("{HO}");
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
      if (DBG & 8) os_printf("{SD}");
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
      if (DBG & 8) os_printf("{RD}");
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
      if (DBG & 8) os_printf("{WR}");
      WriteDiskBlock(z80_dma, z80_sec, z80_trk, z80_dsk);
      m->state.registers.byte[Z80_A] = 0x00;
      break;

    case CYCLES:
      os_printf("{CYCLES:%u}",cycles);
      break;

    case HALT:
      os_printf("\n{Exiting}\n");
      machine.is_done = 1;
      break;

    default:
      os_printf("\nINVALID argument to IN/OUT, ARG=0x%02x PC=%04x\n", arg, m->state.pc);
      machine.is_done = 1;
  }
  if (DBG & 8) os_printf("\n");
}


#ifdef NOSDK
void ICACHE_FLASH_ATTR main(void) {
  uint32_t baud;

  nosdk8266_init();
  InitUart();
  baud=AutoBaud();
  uart_div_modify(UART0, (PERIPH_FREQ * 1000000) / baud);

  os_printf("\n\ncpm8266 - Z80 Emulator and CP/M 2.2 system version %d.%d\n\n",VERSION/10,VERSION%10);
  ets_delay_us(250000);
  FlushUart();

  cpm_main();
}
#endif



#ifdef WIFI
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t some_timer;

bool connected;
bool connecting;
bool dhcpc_started;
struct station_config wifi_conf;

void some_timerfunc(void *arg) {
  int wifi_status;

  if ((!connected) && (!connecting)){
//   char ssid[32] = "01111523287";
//   char password[64] = "jamsheed12" ;
//   char ssid[32] = "mats";
//   char password[64] = "6503181114" ;
    strcpy((char *)wifi_conf.ssid, "01111523287");
    strcpy((char *)wifi_conf.password, "jamsheed12");
    wifi_conf.bssid_set = FALSE;
    ETS_UART_INTR_DISABLE();
    wifi_station_set_config_current(&wifi_conf);
    ETS_UART_INTR_ENABLE();
    connected = wifi_station_connect();
  }

  if (connecting) {
    wifi_status = wifi_station_get_connect_status();
    switch (wifi_status) {
      case STATION_GOT_IP:
        connecting = FALSE;
        connected = TRUE;
        break;

      case STATION_IDLE:
      case STATION_CONNECTING:
        // Still going
        break;

      case STATION_WRONG_PASSWORD:
      case STATION_NO_AP_FOUND:
      case STATION_CONNECT_FAIL:
      default:
        connecting = FALSE;
        connected = FALSE;
        wifi_station_disconnect();
        break;
    }
  }

  if (connected && !dhcpc_started) {
    dhcpc_started = wifi_station_dhcpc_start();
  }
}

//Do nothing function
static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events) {
  os_delay_us(10);
}




#endif


#ifdef WIFI
void ICACHE_FLASH_ATTR user_init() {
  wifi_set_opmode_current(STATION_MODE);
  connected = FALSE;
  connecting = FALSE;
  dhcpc_started = FALSE;

  //Disarm timer
  os_timer_disarm(&some_timer);

  //Setup timer
  os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

  //Arm the timer
  //&some_timer is the pointer
  //1000 is the fire time in ms
  //0 for once and 1 for repeating
  os_timer_arm(&some_timer, 1000, 1);

  //Start os task
  system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

  cpm_main();
}
#endif
