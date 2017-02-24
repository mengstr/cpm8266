#ifndef _MONITOR_H
#define _MONITOR_H

#include "c_types.h"

#define NONUM 0x10000
#define LOWERNULL 0x20

char ICACHE_FLASH_ATTR GetKey(bool wait);
char ICACHE_FLASH_ATTR *GetLine(void);
uint32_t ICACHE_FLASH_ATTR getHexNum(char **p);
char ICACHE_FLASH_ATTR *getString(char **p);
void ICACHE_FLASH_ATTR HexDump(uint16_t address, uint16_t len, bool intel);
void ICACHE_FLASH_ATTR ModifyMemory(uint16_t address);
void ICACHE_FLASH_ATTR LoadIntelHex(uint16_t offset);
void ICACHE_FLASH_ATTR ShowAllRegisters();
void ICACHE_FLASH_ATTR ModifyRegister(char *regname, uint16_t val);

#endif
