#ifndef _DISASM_H
#define _DISASM_H

#include "../espincludes.h"

int ICACHE_FLASH_ATTR Z80_Dasm(uint8_t *buffer, char *dest, uint16_t PC);

#endif