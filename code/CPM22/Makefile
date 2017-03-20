SHELL		=  /bin/bash

CPMMEMORY	?=  65536
VERSION		?=  0

ASM		=  /usr/bin/z80asm
XXD		=  /usr/bin/xxd


all: BOOT.bin BIOS.bin BDOS.bin CCP.bin

BOOT.bin: BOOT.Z80
	@echo [Z80ASM] $<
	@echo "MEM: EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION: EQU $(VERSION)" >> SETTINGS.Z80
	@$(ASM) $< -o $@

BIOS.bin: BIOS.Z80
	@echo [Z80ASM] $<
	@echo "MEM: EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION: EQU $(VERSION)" >> SETTINGS.Z80
	@$(ASM) $< -o $@

BDOS.bin: BDOS.Z80
	@echo [Z80ASM] $<
	@echo "MEM: EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION: EQU $(VERSION)" >> SETTINGS.Z80
	@$(ASM) $< -o $@

CCP.bin: CCP.Z80
	@echo [Z80ASM] $<
	@echo "MEM: EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION: EQU $(VERSION)" >> SETTINGS.Z80
	@$(ASM) $< -o $@

clean :
	@rm -rf *~
	@rm -rf *.{bin,lst}


