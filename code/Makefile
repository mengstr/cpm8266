SHELL		=  /bin/bash


ESPPORT	    ?= /dev/ttyUSB0
ESP8266SDK	?= /Volumes/case-sensitive/esp-open-sdk/
FLASHBAUD	=  921600
MAIN_MHZ	=  80 #Pick from *52, *80, 104 or *115, 160, *173, *189#, 231, 346, 378#  * = peripheral clock at processor clock. # = Mine won't boot + on ESP8285, Clock Lower and unreliable.  Warning. Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why)

TERMSPEED   = 9600

TARGET	 	=  image
FW_1 		=  $(TARGET).elf-0x10000.bin
FW_2 		=  $(TARGET).elf-0x00000.bin

#all : $(TARGET).elf

ZASM		=  /usr/local/bin/zasm
XXD		=  /usr/bin/xxd
ESPTOOL		=  $(ESP8266SDK)/esptool/esptool.py
BINS		=  $(ESP8266SDK)/xtensa-lx106-elf/bin
SIZE		=  $(BINS)/xtensa-lx106-elf-size
OBJDUMP		=  $(BINS)/xtensa-lx106-elf-objdump
OBJCOPY		=  $(BINS)/xtensa-lx106-elf-objcopy
GCC		    =  $(BINS)/xtensa-lx106-elf-gcc
AS		    =  $(BINS)/xtensa-lx106-elf-as

LDFLAGS		=  -T ld/linkerscript.ld -T ld/addresses.ld
CFLAGS		=  -std=c11 -flto -Os -g -Iinclude -nostdlib -DMAIN_MHZ=$(MAIN_MHZ) -mno-serialize-volatile -mlongcalls
SRCS		=  main.c uart.c z80/z80emu.c nosdk/startup.S nosdk/nosdk8266.c
OBJS		=  main.o uart.o z80/z80emu.o nosdk/startup.o nosdk/nosdk8266.o

all: $(OBJS)
	@echo [LINK] $(OBJS)
	@$(GCC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $(TARGET).elf
	@$(SIZE) $(TARGET).elf
	@nm -S -n $(TARGET).elf > $(TARGET).map
	@$(OBJDUMP) -S $(TARGET).elf > $(TARGET).lst
	@export PATH=$(BINS):$$PATH; $(ESPTOOL) elf2image $(TARGET).elf

main.o: main.c z80/z80emu.h z80/z80user.h hex/CPM22.data
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

uart.o: uart.c uart.h
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
	@$(ZASM) --z80 -u -w -b CPM22/CPM22.Z80 CPM22/CPM22.bin
	@$(XXD) -i < CPM22/CPM22.bin > hex/CPM22.data


flash : $(FW_FILE_1) $(FW_FILE_2)
	$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x00000 image.elf-0x00000.bin -ff 80m -fm dout

disk:
	$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash 0x40000 disks/CPM22RED.DSK -ff 80m -fm dout

serial :
#	minicom -b $(TERMSPEED) -D $(ESPPORT)
	screen $(ESPPORT) $(TERMSPEED)

clean :
	rm -rfv $(FW_1) $(FW_2) {.,CPM22,nosdk,z80}/*.{s,o,i,elf,res,out,map,bin,lst,out,bak} {.,nosdk,z80}/*~


