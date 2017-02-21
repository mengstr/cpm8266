SHELL		=  /bin/bash

ESPPORT	    	?= /dev/ttyUSB0
ESP8266SDK	?= /Volumes/case-sensitive/esp-open-sdk/
MAIN_MHZ	=  80 #Pick from *52, *80, 104 or *115, 160, *173, *189#, 231, 346, 378#  * = peripheral clock at processor clock. # = Mine won't boot + on ESP8285, Clock Lower and unreliable.  Warning. Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why)

FLASHPARAM 	=  --flash_freq 80m --flash_mode dout
FLASHBAUD	=  921600

EMULATIONBAUD	=  9600
DISKSIZE	=  0x3E900 # 256256 bytes (IMB 8-inch 26sec, 77trk, SS)

CPMMEMORY	=  48

TARGET	 	=  image
FW_1 		=  $(TARGET).elf-0x10000.bin
FW_2 		=  $(TARGET).elf-0x00000.bin

ZASM		=  /usr/local/bin/zasm
XXD		=  /usr/bin/xxd
ESPTOOL		=  $(ESP8266SDK)/esptool/esptool.py
BINS		=  $(ESP8266SDK)/xtensa-lx106-elf/bin
SIZE		=  $(BINS)/xtensa-lx106-elf-size
OBJDUMP		=  $(BINS)/xtensa-lx106-elf-objdump
OBJCOPY		=  $(BINS)/xtensa-lx106-elf-objcopy
GCC		=  $(BINS)/xtensa-lx106-elf-gcc
AS		=  $(BINS)/xtensa-lx106-elf-as

CFLAGS		=  -std=c99 -flto -Os -g -Iinclude -nostdlib -mno-serialize-volatile -mlongcalls
CFLAGS		:= $(CFLAGS) -DMAIN_MHZ=$(MAIN_MHZ) -DEMULATIONBAUD=$(EMULATIONBAUD) -DCPMMEMORY=$(CPMMEMORY)
#CFLAGS		:= $(CFLAGS) -DDEBUG 
LDFLAGS		=  -T ld/linkerscript.ld -T ld/addresses.ld
SRCS		=  main.c uart.c disasm.c z80/z80emu.c nosdk/startup.S nosdk/nosdk8266.c
OBJS		=  main.o uart.o disasm.o z80/z80emu.o nosdk/startup.o nosdk/nosdk8266.o

bins:   $(OBJS)
	@echo [LINK] $(OBJS)
	@$(GCC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $(TARGET).elf
	@$(SIZE) $(TARGET).elf
	@nm -S -n $(TARGET).elf > $(TARGET).map
	@$(OBJDUMP) -S $(TARGET).elf > $(TARGET).lst
	@export PATH=$(BINS):$$PATH; $(ESPTOOL) elf2image $(TARGET).elf

all:	clean bins flash serial

main.o: main.c z80/z80emu.h z80/z80user.h hex/CPM22.data
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

uart.o: uart.c uart.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

disasm.o: disasm.c disasm.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

z80/z80emu.o: z80/z80emu.c z80/z80emu.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

nosdk/nosdk8266.o: nosdk/nosdk8266.c
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

nosdk/startup.o: nosdk/startup.S
	@echo [AS] $<
	@$(AS) -o $@ -c $<

hex/CPM22.data: CPM22/CPM22.Z80 CPM22/BDOS.Z80 CPM22/BIOS.Z80 CPM22/CCP.Z80  
	@echo "MEM EQU $(CPMMEMORY)" > CPM22/MEMORY.Z80
	@$(ZASM) --z80 -u -w -b CPM22/CPM22.Z80 CPM22/CPM22.bin
	@$(XXD) -i < CPM22/CPM22.bin > hex/CPM22.data

flash : $(FW_FILE_1) $(FW_FILE_2)
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x00000 image.elf-0x00000.bin $(FLASHPARAM)

writealldisks : writediskA writediskB writediskC writediskC writediskD writediskE \
		writediskF writediskG writediskH writediskI writediskJ writediskK \
		writediskL writediskM writediskN writediskO

readalldisks :  readdiskA readdiskB readdiskC readdiskC readdiskD readdiskE \
		readdiskF readdiskG readdiskH readdiskI readdiskJ readdiskK \
		readdiskL readdiskM readdiskN readdiskO

writediskA:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x040000 disks/DISK_A.DSK $(FLASHPARAM)

writediskB:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x080000 disks/DISK_B.DSK $(FLASHPARAM)

writediskC:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x0C0000 disks/DISK_C.DSK $(FLASHPARAM)

writediskD:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x100000 disks/DISK_D.DSK $(FLASHPARAM)

writediskE:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x140000 disks/DISK_E.DSK $(FLASHPARAM)

writediskF:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x180000 disks/DISK_F.DSK $(FLASHPARAM)

writediskG:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x1C0000 disks/DISK_G.DSK $(FLASHPARAM)

writediskH:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x200000 disks/DISK_H.DSK $(FLASHPARAM)

writediskI:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x240000 disks/DISK_I.DSK $(FLASHPARAM)

writediskJ:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x280000 disks/DISK_J.DSK $(FLASHPARAM)

writediskK:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x2C0000 disks/DISK_K.DSK $(FLASHPARAM)

writediskL:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x300000 disks/DISK_L.DSK $(FLASHPARAM)

writediskM:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x340000 disks/DISK_M.DSK $(FLASHPARAM)

writediskN:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x380000 disks/DISK_N.DSK $(FLASHPARAM)

writediskO:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x3C0000 disks/DISK_O.DSK $(FLASHPARAM)


readdiskA:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x040000 $(DISKSIZE) disks/DISK_A.DSK 

readdiskB:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x080000 $(DISKSIZE) disks/DISK_B.DSK 

readdiskC:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x0C0000 $(DISKSIZE) disks/DISK_C.DSK 

readdiskD:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x100000 $(DISKSIZE) disks/DISK_D.DSK 

readdiskE:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x140000 $(DISKSIZE) disks/DISK_E.DSK 

readdiskF:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x180000 $(DISKSIZE) disks/DISK_F.DSK 

readdiskG:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x1C0000 $(DISKSIZE) disks/DISK_G.DSK 

readdiskH:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x200000 $(DISKSIZE) disks/DISK_H.DSK 

readdiskI:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x240000 $(DISKSIZE) disks/DISK_I.DSK 

readdiskJ:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x280000 $(DISKSIZE) disks/DISK_J.DSK 

readdiskK:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x2C0000 $(DISKSIZE) disks/DISK_K.DSK 

readdiskL:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x300000 $(DISKSIZE) disks/DISK_L.DSK 

readdiskM:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x340000 $(DISKSIZE) disks/DISK_M.DSK 

readdiskN:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x380000 $(DISKSIZE) disks/DISK_N.DSK 

readdiskO:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x3C0000 $(DISKSIZE) disks/DISK_O.DSK

serial :
#	minicom -b -D $(ESPPORT) $(EMULATIONBAUD) 
	screen $(ESPPORT) $(EMULATIONBAUD)

clean :
	@rm -rfv $(FW_1) $(FW_2) 
	@rm -rfv hex/CPM22.data CPM22/*.{rom,bin,hex}
	@rm -rfv {.,CPM22,nosdk,z80}/*.{s,o,i,elf,res,map,bin,lst,out,bak} 
	@rm -rfv {.,nosdk,z80}/*~


