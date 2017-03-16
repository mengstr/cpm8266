#ifndef _FLASH_H
#define _FLASH_H

void ICACHE_FLASH_ATTR FlushDisk(bool standalone);
void ICACHE_FLASH_ATTR ReadDiskBlock(uint16_t mdest, uint8_t sectorNo, uint8_t trackNo, uint8_t diskNo);
void ICACHE_FLASH_ATTR WriteDiskBlock(uint16_t msrc, uint8_t sectorNo, uint8_t trackNo, uint8_t diskNo);

#endif
