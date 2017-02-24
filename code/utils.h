#ifndef _UTILS_H
#define _UTILS_H

#include "c_types.h"

#define TOLOWER(x) (x | 0x20)

uint8_t ICACHE_FLASH_ATTR readRomByte(const uint8_t* addr);
bool ICACHE_FLASH_ATTR isHex(char ch);
uint8_t ICACHE_FLASH_ATTR hexDec1(char ch);
uint8_t ICACHE_FLASH_ATTR hexDec2p(char *p);
uint16_t ICACHE_FLASH_ATTR hexDec4p(char *p);


#endif
