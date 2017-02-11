SHELL		=  /bin/bash		

FLASHPORT	?= /dev/ttyUSB0
ESP8266SDK	?= /Volumes/case-sensitive/esp-open-sdk/
FLASHBAUD	=  921600
MAIN_MHZ	=  320 #Pick from *52, *80, 104 or *115, 160, *173, *189#, 231, 346, 378#  * = peripheral clock at processor clock. # = Mine won't boot + on ESP8285, Clock Lower and unreliable.  Warning. Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why)

TARGET	 	=  image
FW_1 		=  $(TARGET).elf-0x10000.bin
FW_2 		=  $(TARGET).elf-0x00000.bin

all : $(TARGET).elf

GCC_FOLDER	=  $(ESP8266SDK)/xtensa-lx106-elf
ESPTOOL		=  $(ESP8266SDK)/esptool/esptool.py
PREFIX		=  $(GCC_FOLDER)/bin/xtensa-lx106-elf-
BINS		=  $(ESP8266SDK)/xtensa-lx106-elf/bin
SIZE		=  $(BINS)/xtensa-lx106-elf-size
OBJDUMP		=  $(BINS)/xtensa-lx106-elf-objdump
OBJCOPY		=  $(BINS)/xtensa-lx106-elf-objcopy
GCC		=  $(BINS)/xtensa-lx106-elf-gcc

LDFLAGS		=  -T ld/linkerscript.ld -T ld/addresses.ld
CFLAGS		=  -flto -Os -g -Iinclude -nostdlib -DMAIN_MHZ=$(MAIN_MHZ) -mno-serialize-volatile -mlongcalls
SRCS		=  main.c z80/z80emu.c src/startup.S src/nosdk8266.c

$(TARGET).elf : $(SRCS)
	$(GCC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	nm -S -n $(TARGET).elf > $(TARGET).map
	$(SIZE) $@
	$(OBJDUMP) -S $@ > $(TARGET).lst
	export PATH=$(BINS):$$PATH; $(ESPTOOL) elf2image $(TARGET).elf 

burn : $(FW_FILE_1) $(FW_FILE_2)
	$(ESPTOOL) -p $(PORT) -b $(FLASHBAUD) write_flash 0x00000 image.elf-0x00000.bin -ff 80m -fm dout

clean :
	rm -rf $(FW_1) $(FW_2) *.{s,o,i,elf,res,out,map,lst,out,bak} *~
	

