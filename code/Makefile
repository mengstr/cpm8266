ESPPORT		?= /dev/ttyUSB0
ESP8266SDK	?= /Volumes/case-sensitive/esp-open-sdk/

# Select WIFI for small memory CP/M machine with telnet over wifi access
# or     NOSDK for 64K CP/M with serial access

#BUILD		?= WIFI
BUILD		?= NOSDK

#  Pick from *52 *80 104 *115 160 *173 *189 231 320 346 378
#  * = peripheral clock at processor clock.
MAIN_MHZ	=  160

FLASHPARAM 	=  --flash_freq 80m --flash_mode dout
FLASHBAUD	=  921600
EMULATIONBAUD	=  9600
PORT 		=  23

DISKSIZE	=  0x3E900  # 256256 bytes (IMB 8-inch 26sec, 77trk, SS)

ifeq (NOSDK, $(BUILD))
 DISKS		=  A B C D E F G H I J K L M N O
 DISKADDR	=  3932160  # 0x3c0000
 export CPMMEMORY = 1024*64
endif

ifeq (WIFI, $(BUILD))
 DISKS		=  A B C D E F G H I J K L M N
 DISKADDR	=  3915776  # 0x3bc000
 export CPMMEMORY =  1024*38
endif

export VERSION	 =  02

TARGET	 	=  image
FW0 		=  $(TARGET).elf-0x00000.bin
FW1 		=  $(TARGET).elf-0x10000.bin

SHELL		=  /bin/bash
CPMCP		=  /usr/bin/cpmcp
XXD		=  /usr/bin/xxd
ZIP		=  /usr/bin/zip
DD		=  /bin/dd
ESPTOOL		?= /opt/esptool/esptool.py
BINS		=  $(ESP8266SDK)/xtensa-lx106-elf/bin
SIZE		=  $(BINS)/xtensa-lx106-elf-size
READELF		=  $(BINS)/xtensa-lx106-elf-readelf
OBJDUMP		=  $(BINS)/xtensa-lx106-elf-objdump
OBJCOPY		=  $(BINS)/xtensa-lx106-elf-objcopy
GCC		=  $(BINS)/xtensa-lx106-elf-gcc
AS		=  $(BINS)/xtensa-lx106-elf-as

DDFLAGS		= status=none bs=1 conv=notrunc

CFLAGS		=  -std=c11 -DMAIN_MHZ=$(MAIN_MHZ) -DVERSION=$(VERSION) -DEMULATIONBAUD=$(EMULATIONBAUD) -DCPMMEMORY=$(CPMMEMORY) -DPORT=$(PORT)
CFLAGS 		:= $(CFLAGS) -Iinclude -I./ -Os -mlongcalls

ifeq (NOSDK, $(BUILD))
 CFLAGS		:= $(CFLAGS) -DNOSDK  -flto -mno-serialize-volatile
 LDLIBS 	= -nostdlib
 LDFLAGS	=  -T ld/linkerscript.ld -T ld/addresses.ld
endif

ifeq (WIFI, $(BUILD))
 CFLAGS 	:= $(CFLAGS) -DWIFI -DICACHE_FLASH
 LDLIBS 	= -nostdlib -Wl,--start-group -lmain -lupgrade -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lcirom -lgcc
 LDFLAGS 	= -Teagle.app.v6.ld
endif

SRCS		=  main.c uart.c conio.c flash.c monitor.c utils.c gpio16.c z80/z80emu.c
OBJS		=  main.o uart.o conio.o flash.o monitor.o utils.o gpio16.o z80/z80emu.o

ifeq (NOSDK, $(BUILD))
 SRCS		:= $(SRCS) nosdk/startup.S nosdk/nosdk8266.c
 OBJS		:= $(OBJS) nosdk/startup.o nosdk/nosdk8266.o
endif

ifeq (WIFI, $(BUILD))
 SRCS		:= $(SRCS) wifi.c
 OBJS		:= $(OBJS) wifi.o
endif


.PHONY: all full disks clean flash flashinit dist ram size

all: bins flash

full: clean bins flashinit flash disks systemdiskA writealldisks

erase:
	@echo "[ERASING]"
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) erase_flash > /dev/null

dist: erase full
	@echo "[READING IMAGE]"
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash 0x00000 0x400000 dist/cpm8266_$(VERSION).bin > /dev/null
	@$(ZIP) -j -9 dist/cpm8266_$(VERSION).zip dist/cpm8266_$(VERSION).bin dist/readme.txt > /dev/null

bins: $(OBJS)
	@echo [LINK]
	@$(GCC) $(OBJS) $(LDLIBS) $(CFLAGS) $(LDFLAGS) -o $(TARGET).elf
	@$(SIZE) $(TARGET).elf
	@nm -S -n $(TARGET).elf > $(TARGET).map
	@$(OBJDUMP) -S $(TARGET).elf > $(TARGET).lst
	@export PATH=$(BINS):$$PATH; $(ESPTOOL) elf2image $(TARGET).elf $(FLASHPARAM) > /dev/null

main.o: main.c z80/z80emu.h z80/z80user.h CPM22/boot.data
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

uart.o: uart.c uart.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

conio.o: conio.c conio.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

flash.o: flash.c flash.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

utils.o: utils.c utils.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

monitor.o: monitor.c monitor.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

gpio16.o: gpio16.c gpio16.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

z80/z80emu.o: z80/z80emu.c z80/z80emu.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

ifeq (NOSDK, $(BUILD))
nosdk/nosdk8266.o: nosdk/nosdk8266.c
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

nosdk/startup.o: nosdk/startup.S
	@echo [AS] $<
	@$(AS) -o $@ $<
endif

ifeq (WIFI, $(BUILD))
wifi.o: wifi.c
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<
endif

CPM22/boot.data: CPM22/BOOT.bin
	@$(XXD) -i < CPM22/BOOT.bin > CPM22/boot.data

CPM22/BOOT.bin :
	@$(MAKE) --no-print-directory -C CPM22 BOOT.bin

CPM22/BIOS.bin :
	@$(MAKE) --no-print-directory -C CPM22 BIOS.bin

CPM22/BDOS.bin :
	@$(MAKE) --no-print-directory -C CPM22 BDOS.bin

CPM22/CCP.bin :
	@$(MAKE) --no-print-directory -C CPM22 CCP.bin

flash : $(FW0) $(FW1)
	@echo "[UPLOAD]" $(FW0) $(FW1)
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash \
		0x00000 $(FW0) \
		0x10000 $(FW1) \
		$(FLASHPARAM) > /dev/null

flashinit:
	@echo "[ERASING FLASH]"
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) erase_flash > /dev/null
ifeq (WIFI, $(BUILD))
	@echo "[INITIALIZING FLASH]"
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash \
		0x3fc000 espbin/esp_init_data_default.binary \
		0x3fe000 espbin/blank.binary \
		$(FLASHPARAM) > /dev/null
endif

disks :
	@$(MAKE) --no-print-directory -C disks clean all

systemdiskA : CPM22/BOOT.bin CPM22/BIOS.bin CPM22/BDOS.bin CPM22/CCP.bin
	@$(DD) of=disks/DISK_A.DSK if=CPM22/BOOT.bin seek=0    count=128  $(DDFLAGS)
	@$(DD) of=disks/DISK_A.DSK if=CPM22/CCP.bin  seek=128  count=2048 $(DDFLAGS)
	@$(DD) of=disks/DISK_A.DSK if=CPM22/BDOS.bin seek=2176 count=3584 $(DDFLAGS)
	@$(DD) of=disks/DISK_A.DSK if=CPM22/BIOS.bin seek=5760 count=896  $(DDFLAGS)

undiskA :
	@$(MAKE) --no-print-directory -C disks undiskA

serialp :
	putty -load cpm8266 -sercfg $(EMULATIONBAUD)

serialm :
	minicom -b -D $(ESPPORT) $(EMULATIONBAUD)

serials :
	screen $(ESPPORT) $(EMULATIONBAUD)

define WRITEDISK =
writedisk$(1) :
	@$(eval address := $(shell n=$$(printf "%d\n" \'$(1)); printf "0x%06x" $$(( $(4)-(($$n-65)*$(2)) )); ) )
	@echo "[UPLOAD]" disks/DISK_$(1).DSK to $(address)
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) write_flash $(address) disks/DISK_$(1).DSK > /dev/null
endef

define READDISK =
readdisk$(1) :
	@$(eval address := $(shell n=$$(printf "%d\n" \'$(1)); printf "0x%06x" $$(( $(4)-(($$n-65)*$(2)) )); ) )
	@echo "[DOWNLOAD]" disks/DISK_$(1).DSK from $(address)
	@$(ESPTOOL) -p $(ESPPORT) -b $(FLASHBAUD) read_flash $(address) $(3) disks/DISK_$(1).DSK > /dev/null
endef

$(foreach DSK,$(DISKS), \
	$(eval $(call WRITEDISK,$(DSK), $(DISKSIZE), $(DISKSIZE), $(DISKADDR) )); \
	$(eval $(call  READDISK,$(DSK), $(DISKSIZE), $(DISKSIZE), $(DISKADDR) )); \
)

WRITEDISKTARGETS=$(foreach DSK,$(DISKS), writedisk$(DSK) )
READDISKTARGETS=$(foreach DSK,$(DISKS), readdisk$(DSK) )

writealldisks : $(WRITEDISKTARGETS)
readalldisks  : $(READDISKTARGETS)

size :
	$(SIZE) *.o

ram : image.elf
	$(READELF) -s $(TARGET).elf | sort -k2 | grep ": 401"

-include Makefile.tmp

Makefile.tmp: Makefile
	@touch $@
	@$(MAKE) -s clean

clean :
	@rm -rf $(FW1) $(FW2)
	@rm -rf {.,nosdk,z80,dist}/*.{data,s,o,i,elf,res,map,bin,lst,out,bak,zip,cap,log}
	@rm -rf {..,.,nosdk,z80,dist}/*~
	@$(MAKE) --no-print-directory -C CPM22 clean
	@$(MAKE) --no-print-directory -C disks clean
