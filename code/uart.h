#ifndef _UART_H
#define _UART_H

uint32_t AutoBaud(void);
void InitUart(void);
uint16_t GetRxCnt(void);
uint8_t GetRxChar(void);
void FlushUart(void);

#endif