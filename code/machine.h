#ifndef _MACHINE_H_
#define _MACHINE_H_

#include "z80/z80emu.h"
#include "z80/z80user.h"

void InitMachine();
void ICACHE_FLASH_ATTR LoadBootSector();
void RunMachine(int cycles);

extern MACHINE machine;

#endif
