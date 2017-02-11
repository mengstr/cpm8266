TARGET_OUT = image.elf
all : $(TARGET_OUT)
MAIN_MHZ:=320 #Pick from *52, *80, 104 or *115, 160, *173, *189#, 231, 346, 378#  * = peripheral clock at processor clock. # = Mine won't boot + on ESP8285, Clock Lower and unreliable.  Warning. Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why)
USE_PRINT:=YES

#ESP_OPEN_SDK:=../esp-open-sdk
ESP_OPEN_SDK:=/Volumes/case-sensitive/esp-open-sdk/

FW_1 = image.elf-0x10000.bin
FW_2 = image.elf-0x00000.bin
GCC_FOLDER:=$(ESP_OPEN_SDK)/xtensa-lx106-elf
ESPTOOL:=$(ESP_OPEN_SDK)/esptool/esptool.py
ESPTOOLOPTS:=-b 921600
PREFIX:=$(GCC_FOLDER)/bin/xtensa-lx106-elf-
SIZE:=$(PREFIX)size
OBJDUMP:=$(PREFIX)objdump
OBJCOPY:=$(PREFIX)objcopy
GCC:=$(PREFIX)gcc

#-mno-serialize-volatile will prevent extra memw things from being generated.

LDFLAGS:=-T ld/linkerscript.ld -T ld/addresses.ld
FOLDERPREFIX:=$(GCC_FOLDER)/bin
PORT:=/dev/ttyUSB0

SRCS:=$(SRCS) main.c z80/z80emu.c

CFLAGS:=$(CFLAGS) -flto
CFLAGS:=$(CFLAGS) -Os -g
#CFLAGS:=$(CFLAGS) -Og -ggdb --save-temps
CFLAGS:=$(CFLAGS) -Iinclude -nostdlib  -DMAIN_MHZ=$(MAIN_MHZ)  -mno-serialize-volatile -mlongcalls

SRCS:=$(SRCS) src/startup.S src/nosdk8266.c

$(TARGET_OUT) : $(SRCS)
	$(GCC) $(CFLAGS) $^  $(LDFLAGS) -o $@
	nm -S -n $(TARGET_OUT) > image.map
	$(SIZE) $@
	$(PREFIX)objdump -S $@ > image.lst
	PATH=$(FOLDERPREFIX):$$PATH;$(ESPTOOL) elf2image $(TARGET_OUT) 

burn : $(FW_FILE_1) $(FW_FILE_2)
	($(ESPTOOL)  --port $(PORT) write_flash 0x00000 image.elf-0x00000.bin -ff 80m -fm dout)||(true)

clean :
	rm -rf $(TARGET_OUT) $(FW_1) $(FW_2) *.s *.o *.i *.res *.out *.map *.lst *.out *.bak *~
	

