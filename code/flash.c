#include "c_types.h"
#include "eagle_soc.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "ets_sys.h"
#include "nosdk8266.h"
#include "z80/z80emu.h"
#include "z80/z80user.h"
#include "gpio16.h"
#include "flash.h"

#define SECTORSIZE      128
#define SECTORSPERTRACK 26
#define TRACKSPERDISK   77

#define DISKFLASHOFFSET 0x40000 // Location in flash for first disk
#define DISKFLASHSIZE   0x40000 // Number of bytes in flash for each disk
#define DISKCOUNT       15      // We have 15 disks A..O
#define FLASHBLOCKSIZE  4096    // Nr of bytes for erase/write to flash

extern MACHINE machine;

static uint8_t flashBuf[FLASHBLOCKSIZE];
static uint16_t flashSectorNo = 0xFFFF;
static bool dirty=false;

//
//
//
void ICACHE_FLASH_ATTR ReadDiskBlock(uint16_t mdest, uint8_t sectorNo,
                                     uint8_t trackNo, uint8_t diskNo) {
  if (diskNo > (DISKCOUNT - 1)) diskNo = DISKCOUNT - 1;
  if (trackNo > (TRACKSPERDISK - 1)) trackNo = TRACKSPERDISK - 1;
  if (sectorNo > (SECTORSPERTRACK - 1)) sectorNo = SECTORSPERTRACK - 1;

  uint32_t lba = SECTORSPERTRACK * trackNo + sectorNo;
  uint32_t flashloc = DISKFLASHOFFSET + DISKFLASHSIZE * diskNo + SECTORSIZE * lba;
  uint16_t myFlashSectorNo = flashloc / FLASHBLOCKSIZE;
  uint16_t fl = flashloc % FLASHBLOCKSIZE;
 
  // Do we already have this disk sector in the bigger flash block that is already
  // read into memory (and might have dirty blocks on it that needs to be flushed first)
  if (myFlashSectorNo != flashSectorNo) {
    Cache_Read_Disable();
    SPIUnlock();
    if (dirty) {
        FlushDisk(false); // Flush Disk in non-standalone move
    }
    flashSectorNo = myFlashSectorNo;
    SPIRead(flashSectorNo * FLASHBLOCKSIZE, (uint32_t *)flashBuf, FLASHBLOCKSIZE);
//    printf("{R%d}",flashSectorNo);
    Cache_Read_Enable(0, 0, 1);
  }// else printf("{r%d}",flashSectorNo);

  ets_memcpy(&machine.memory[mdest], &flashBuf[fl], SECTORSIZE);
}

//
//
//
void ICACHE_FLASH_ATTR WriteDiskBlock(uint16_t msrc, uint8_t sectorNo,
                                      uint8_t trackNo, uint8_t diskNo) {

  if (diskNo > (DISKCOUNT - 1)) diskNo = DISKCOUNT - 1;
  if (trackNo > (TRACKSPERDISK - 1)) trackNo = TRACKSPERDISK - 1;
  if (sectorNo > (SECTORSPERTRACK - 1)) sectorNo = SECTORSPERTRACK - 1;

  uint32_t lba = SECTORSPERTRACK * trackNo + sectorNo;
  uint32_t flashloc = DISKFLASHOFFSET + DISKFLASHSIZE * diskNo + SECTORSIZE * lba;
  uint16_t myFlashSectorNo = flashloc / FLASHBLOCKSIZE;
  uint16_t fl = flashloc % FLASHBLOCKSIZE;

  if (myFlashSectorNo != flashSectorNo) {
   Cache_Read_Disable();
    SPIUnlock();
    if (dirty) {
      FlushDisk(false); // Flush Disk in non-standalone mode
    }
    flashSectorNo = myFlashSectorNo;
    SPIRead(flashSectorNo * FLASHBLOCKSIZE, (uint32_t *)flashBuf, FLASHBLOCKSIZE);
    Cache_Read_Enable(0, 0, 1);
//    printf("{W%d}",flashSectorNo);
  } //else printf("{w%d}",flashSectorNo);
  
  ets_memcpy(&flashBuf[fl], &machine.memory[msrc],  SECTORSIZE);
  dirty=true;
}


//
//
//
void ICACHE_FLASH_ATTR FlushDisk(bool standalone) {
  if (!dirty) return;
  if (standalone) {
    Cache_Read_Disable();
    SPIUnlock();
  }
//  printf("{E%d}",flashSectorNo);
  gpio16_output_set(0);	
  SPIEraseSector(flashSectorNo);
  SPIWrite(flashSectorNo * FLASHBLOCKSIZE, (uint32_t *)flashBuf, FLASHBLOCKSIZE);
  gpio16_output_set(1);	
  if (standalone) {
   Cache_Read_Enable(0, 0, 1);
  }
  dirty=false;
}

