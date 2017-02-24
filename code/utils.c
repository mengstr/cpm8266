#include "utils.h"

//
// This reads a byte from the ESP8266 irom0 area
//
uint8_t ICACHE_FLASH_ATTR readRomByte(const uint8_t* addr) {
    uint32_t bytes;
    bytes = *(uint32_t*)((uint32_t)addr & ~3);
    return ((uint8_t*)&bytes)[(uint32_t)addr & 3];
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


