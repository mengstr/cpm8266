SHELL		=  /bin/bash

ESPPORT		?= /dev/ttyUSB0
ESP8266SDK	?= /Volumes/case-sensitive/esp-open-sdk/
MAIN_MHZ	=  80 #Pick from *52, *80, 104 or *115, 160, *173, *189#, 231, 346, 378#  * = peripheral clock at processor clock. # = Mine won't boot + on ESP8285, Clock Lower and unreliable.  Warning. Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why)

FLASHPARAM 	=  --flash_freq 80m --flash_mode dout
FLASHBAUD	=  921600

EMULATIONBAUD	=  9600
DISKSIZE	=  0x3E900 # 256256 bytes (IMB 8-inch 26sec, 77trk, SS)

export CPMMEMORY =  65536

TARGET	 	=  image
FW1 		=  $(TARGET).elf-0x10000.bin
FW2 		=  $(TARGET).elf-0x00000.bin

ZASM		=  /usr/local/bin/zasm
XXD			=  /usr/bin/xxd
CPMCP		=  /usr/bin/cpmcp
DD			=  /bin/dd
ESPTOOL		=  $(ESP8266SDK)/esptool/esptool.py
BINS		=  $(ESP8266SDK)/xtensa-lx106-elf/bin
SIZE		=  $(BINS)/xtensa-lx106-elf-size
OBJDUMP		=  $(BINS)/xtensa-lx106-elf-objdump
OBJCOPY		=  $(BINS)/xtensa-lx106-elf-objcopy
GCC			=  $(BINS)/xtensa-lx106-elf-gcc
AS			=  $(BINS)/xtensa-lx106-elf-as

CFLAGS		=  -std=c99 -flto -Os -g -Iinclude -nostdlib -mno-serialize-volatile -mlongcalls
CFLAGS		:= $(CFLAGS) -DMAIN_MHZ=$(MAIN_MHZ) -DEMULATIONBAUD=$(EMULATIONBAUD) -DCPMMEMORY=$(CPMMEMORY)
#CFLAGS		:= $(CFLAGS) -DDEBUG
LDFLAGS		=  -T ld/linkerscript.ld -T ld/addresses.ld
SRCS		=  main.c uart.c monitor.c utils.c disasm/disasm.c z80/z80emu.c nosdk/startup.S nosdk/nosdk8266.c
OBJS		=  main.o uart.o monitor.c utils.c disasm/disasm.o z80/z80emu.o nosdk/startup.o nosdk/nosdk8266.o

bins: $(OBJS)
	@echo [LINK] $(OBJS)
	@$(GCC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $(TARGET).elf
	@$(SIZE) $(TARGET).elf
	@nm -S -n $(TARGET).elf > $(TARGET).map
	@$(OBJDUMP) -S $(TARGET).elf > $(TARGET).lst
	@export PATH=$(BINS):$$PATH; $(ESPTOOL) elf2image $(TARGET).elf

all: clean bins flash serial

main.o: main.c z80/z80emu.h z80/z80user.h hex/boot.data
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

uart.o: uart.c uart.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

utils.o: utils.c utils.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

monitor.o: monitor.c monitor.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

disasm/mnemonic_cb.data: disasm/mnemonic_cb.txt
	@$(XXD) -i < $< > $@

disasm/mnemonic_main.data: disasm/mnemonic_main.txt
	@$(XXD) -i < $< > $@

disasm/mnemonic_xx.data: disasm/mnemonic_xx.txt
	@$(XXD) -i < $< > $@

disasm/mnemonic_ed.data: disasm/mnemonic_ed.txt
	@$(XXD) -i < $< > $@

disasm/mnemonic_xx_cb.data: disasm/mnemonic_xx_cb.txt
	@$(XXD) -i < $< > $@

disasm/disasm.o: disasm/disasm.c disasm/disasm.h \
		 disasm/mnemonic_xx_cb.data \
		 disasm/mnemonic_cb.data \
		 disasm/mnemonic_main.data \
		 disasm/mnemonic_xx.data \
		 disasm/mnemonic_ed.data
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

CPM22/BOOT.bin :
	$(MAKE) -C CPM22 BOOT.bin

data:
	$(XXD) -i < CPM22/BOOT.bin > hex/boot.data

CPM22/BIOS.bin :
	 $(MAKE) -C CPM22 BIOS.bin

CPM22/BDOS.bin :
	 $(MAKE) -C CPM22 BDOS.bin

CPM22/CCP.bin :
	 $(MAKE) -C CPM22 CCP.bin

flash : $(FW1) $(FW2)
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x00000 $(FW2) 0x10000 $(FW1) $(FLASHPARAM)

systemdiskA : CPM22/BOOT.bin CPM22/BIOS.bin CPM22/BDOS.bin CPM22/CCP.bin
	@$(DD) of=disks/DISK_A.DSK if=CPM22/BOOT.bin seek=0    count=128  bs=1 conv=notrunc
	@$(DD) of=disks/DISK_A.DSK if=CPM22/CCP.bin  seek=128  count=2048 bs=1 conv=notrunc
	@$(DD) of=disks/DISK_A.DSK if=CPM22/BDOS.bin seek=2176 count=3584 bs=1 conv=notrunc
	@$(DD) of=disks/DISK_A.DSK if=CPM22/BIOS.bin seek=5760 count=896  bs=1 conv=notrunc


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

getdiskA:
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x040000 $(DISKSIZE) ../private/diska.bin


serial :
#	minicom -b -D $(ESPPORT) $(EMULATIONBAUD)
	screen $(ESPPORT) $(EMULATIONBAUD)

clean :
	@rm -rfv $(FW1) $(FW2)
	@rm -rfv hex/CPM22.data
	@rm -rfv {.,nosdk,z80,disasm}/*.{data,s,o,i,elf,res,map,bin,lst,out,bak}
	@rm -rfv {.,nosdk,z80,disasm}/*~
	$(MAKE) -C CPM22 clean


