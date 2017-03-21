#ifndef _CONIO_H
#define _CONIO_H

char ICACHE_FLASH_ATTR GetKey(bool wait);
uint16_t ICACHE_FLASH_ATTR GetRxCnt(void);
uint8_t ICACHE_FLASH_ATTR GetRxChar(void);
void ICACHE_FLASH_ATTR EmptyComBuf(void);
void StoreInComBuf(uint8_t ch);

#endif