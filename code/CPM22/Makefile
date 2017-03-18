SHELL		=  /bin/bash

EMULATIONBAUD	?=  9600
DISKSIZE	?=  0x3E900 # 256256 bytes (IMB 8-inch 26sec, 77trk, SS)
CPMMEMORY	?=  65536

ZASM		=  /usr/local/bin/zasm
XXD			=  /usr/bin/xxd

FLAGSHEX	= --z80 -v0 -u -w -x
FLAGSBIN	= --z80 -v0 -u -w -b

all: BOOT.bin BIOS.bin BDOS.bin CCP.bin

BOOT.bin: BOOT.Z80
	@echo [ZASM] $<
	@echo "MEM EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION EQU $(VERSION)" >> SETTINGS.Z80
	@$(ZASM) $(FLAGSBIN) $< -o $@

BOOT.hex: BOOT.Z80
	@echo [ZASM] $<
	@echo "MEM EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION EQU $(VERSION)" >> SETTINGS.Z80
	@$(ZASM) $(FLAGSHEX) $< -o $@

BIOS.bin: BIOS.Z80
	@echo [ZASM] $<
	@echo "MEM EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION EQU $(VERSION)" >> SETTINGS.Z80
	@$(ZASM) $(FLAGSBIN) $< -o $@

BIOS.hex: BIOS.Z80
	@echo [ZASM] $<
	@echo "MEM EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION EQU $(VERSION)" >> SETTINGS.Z80
	@$(ZASM) $(FLAGSHEX) $< -o $@

BDOS.bin: BDOS.Z80
	@echo [ZASM] $<
	@echo "MEM EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION EQU $(VERSION)" >> SETTINGS.Z80
	@$(ZASM) $(FLAGSBIN) $< -o $@

BDOS.hex: BDOS.Z80
	@echo [ZASM] $<
	@echo "MEM EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION EQU $(VERSION)" >> SETTINGS.Z80
	@$(ZASM) $(FLAGSHEX) $< -o $@

CCP.bin: CCP.Z80
	@echo [ZASM] $<
	@echo "MEM EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION EQU $(VERSION)" >> SETTINGS.Z80
	@$(ZASM) $(FLAGSBIN) $< -o $@

CCP.hex: CCP.Z80
	@echo [ZASM] $<
	@echo "MEM EQU $(CPMMEMORY)" > SETTINGS.Z80
	@echo "VERSION EQU $(VERSION)" >> SETTINGS.Z80
	@$(ZASM) $(FLAGSHEX) $< -o $@

clean :
	@rm -rf *~
	@rm -rf *.{rom,bin,hex,lst}
	@rm -rf *.data


