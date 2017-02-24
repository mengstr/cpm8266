SHELL		=  /bin/bash

ESPPORT		?= /dev/ttyUSB0
ESP8266SDK	?= /Volumes/case-sensitive/esp-open-sdk/
MAIN_MHZ	=  80 #Pick from *52, *80, 104 or *115, 160, *173, *189#, 231, 346, 378#  * = peripheral clock at processor clock. # = Mine won't boot + on ESP8285, Clock Lower and unreliable.  Warning. Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why)

FLASHPARAM 	=  --flash_freq 80m --flash_mode dout
FLASHBAUD	=  921600

EMULATIONBAUD	=  9600
DISKSIZE	=  0x3E900 # 256256 bytes (IMB 8-inch 26sec, 77trk, SS)
DISKBLOCK	=  262144  # 0x40000
DISKS		=  A B C D E F G H I J K L M N O

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

DDFLAGS		= status=none bs=1 conv=notrunc
CFLAGS		=  -std=c99 -flto -Os -g -Iinclude -nostdlib -mno-serialize-volatile -mlongcalls
CFLAGS		:= $(CFLAGS) -DMAIN_MHZ=$(MAIN_MHZ) -DEMULATIONBAUD=$(EMULATIONBAUD) -DCPMMEMORY=$(CPMMEMORY)
#CFLAGS		:= $(CFLAGS) -DDEBUG
LDFLAGS		=  -T ld/linkerscript.ld -T ld/addresses.ld
SRCS		=  main.c uart.c monitor.c utils.c disasm/disasm.c z80/z80emu.c nosdk/startup.S nosdk/nosdk8266.c
OBJS		=  main.o uart.o monitor.o utils.o disasm/disasm.o z80/z80emu.o nosdk/startup.o nosdk/nosdk8266.o


.PHONY: all full disks clean

all: bins flash serial

full: clean bins disks systemdiskA writealldisks flash

bins: $(OBJS)
	@echo [LINK]
	@$(GCC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $(TARGET).elf
	@$(SIZE) $(TARGET).elf
	@nm -S -n $(TARGET).elf > $(TARGET).map
	@$(OBJDUMP) -S $(TARGET).elf > $(TARGET).lst
	@export PATH=$(BINS):$$PATH; $(ESPTOOL) elf2image $(TARGET).elf > /dev/null

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

hex/boot.data: CPM22/BOOT.bin
	@$(XXD) -i < CPM22/BOOT.bin > hex/boot.data

CPM22/BOOT.bin :
	@$(MAKE) --no-print-directory -C CPM22 BOOT.bin

CPM22/BIOS.bin :
	@$(MAKE) --no-print-directory -C CPM22 BIOS.bin

CPM22/BDOS.bin :
	@$(MAKE) --no-print-directory -C CPM22 BDOS.bin

CPM22/CCP.bin :
	@$(MAKE) --no-print-directory -C CPM22 CCP.bin

flash : $(FW1) $(FW2)
	@echo "[UPLOAD]" $(FW1) $(FW2)
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x00000 $(FW2) 0x10000 $(FW1) $(FLASHPARAM) > /dev/null

disks :
	@$(MAKE) --no-print-directory -C disks clean all


systemdiskA : CPM22/BOOT.bin CPM22/BIOS.bin CPM22/BDOS.bin CPM22/CCP.bin
	@$(DD) of=disks/DISK_A.DSK if=CPM22/BOOT.bin seek=0    count=128  $(DDFLAGS)
	@$(DD) of=disks/DISK_A.DSK if=CPM22/CCP.bin  seek=128  count=2048 $(DDFLAGS)
	@$(DD) of=disks/DISK_A.DSK if=CPM22/BDOS.bin seek=2176 count=3584 $(DDFLAGS)
	@$(DD) of=disks/DISK_A.DSK if=CPM22/BIOS.bin seek=5760 count=896  $(DDFLAGS)


serial :
	minicom -b -D $(ESPPORT) $(EMULATIONBAUD)
#	screen $(ESPPORT) $(EMULATIONBAUD)


define WRITEDISK =
writedisk$(1) :
#	address=$(shell n=$$(printf "%d\n" \'$(1)); printf "0x%06x" $$(( ($$n-64)*262144 )); )
	@echo "[UPLOAD]" disks/DISK_$(1).DSK to $(shell n=$$(printf "%d\n" \'$(1)); printf "0x%06x" $$(( ($$n-64)*262144 )); )
	@$(ESPTOOL) \
	-p $(ESPPORT) \
	-b $(FLASHBAUD) \
	write_flash $(shell n=$$(printf "%d\n" \'$(1)); printf "0x%06x" $$(( ($$n-64)*262144 )); ) \
		disks/DISK_$(1).DSK \
	$(FLASHPARAM) > /dev/null
endef


define READDISK =
readdisk$(1) :
#	address=$(shell n=$$(printf "%d\n" \'$(1)); printf "0x%06x" $$(( ($$n-64)*262144 )); )
	@echo "[DOWNLOAD]" disks/DISK_$(1).DSK from $(shell n=$$(printf "%d\n" \'$(1)); printf "0x%06x" $$(( ($$n-64)*262144 )); )
	@$(ESPTOOL) \
	-p $(ESPPORT) \
	-b $(FLASHBAUD) \
	read_flash \
		$(shell n=$$(printf "%d\n" \'$(1)); printf "0x%06x" $$(( ($$n-64)*262144 )); ) \
		262144 \
		disks/DISK_$(1).DSK \
	> /dev/null
endef

$(foreach DSK,$(DISKS),$(eval $(call WRITEDISK,$(DSK))))
$(foreach DSK,$(DISKS),$(eval $(call READDISK,$(DSK))))

writealldisks : writediskA writediskB writediskC writediskC writediskD \
				writediskE writediskF writediskG writediskH writediskI \
				writediskJ writediskK writediskL writediskM writediskN \
				writediskO

readalldisks : 	readdiskA readdiskB readdiskC readdiskC readdiskD \
				readdiskE readdiskF readdiskG readdiskH readdiskI \
				readdiskJ readdiskK readdiskL readdiskM readdiskN \
				readdiskO


clean :
	@rm -rf $(FW1) $(FW2)
	@rm -rf hex/CPM22.data
	@rm -rf {.,nosdk,z80,disasm}/*.{data,s,o,i,elf,res,map,bin,lst,out,bak}
	@rm -rf {.,nosdk,z80,disasm}/*~
	@$(MAKE) --no-print-directory -C CPM22 clean
	@$(MAKE) --no-print-directory -C disks clean
