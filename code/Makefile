SHELL		=  /bin/bash		

FLASHPORT	?= /dev/ttyUSB0
ESP8266SDK	?= /Volumes/case-sensitive/esp-open-sdk/
FLASHBAUD	=  921600
MAIN_MHZ	=  320 #Pick from *52, *80, 104 or *115, 160, *173, *189#, 231, 346, 378#  * = peripheral clock at processor clock. # = Mine won't boot + on ESP8285, Clock Lower and unreliable.  Warning. Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why)

TARGET	 	=  image
FW_1 		=  $(TARGET)-0x10000.bin
FW_2 		=  $(TARGET)-0x00000.bin

#all : $(TARGET).elf

ESPTOOL		=  $(ESP8266SDK)/esptool/esptool.py
BINS		=  $(ESP8266SDK)/xtensa-lx106-elf/bin
SIZE		=  $(BINS)/xtensa-lx106-elf-size
OBJDUMP		=  $(BINS)/xtensa-lx106-elf-objdump
OBJCOPY		=  $(BINS)/xtensa-lx106-elf-objcopy
GCC		=  $(BINS)/xtensa-lx106-elf-gcc

LDFLAGS		=  -T ld/linkerscript.ld -T ld/addresses.ld
CFLAGS		=  -flto -Os -g -Iinclude -nostdlib -DMAIN_MHZ=$(MAIN_MHZ) -mno-serialize-volatile -mlongcalls
SRCS		=  main.c z80/z80emu.c src/startup.S src/nosdk8266.c
OBJS		=  main.o z80/z80emu.o src/startup.o src/nosdk8266.o

all: $(OBJS)
	@echo [LINK] $(OBJS)
	@$(GCC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $(TARGET).elf 
	@$(SIZE) $(TARGET).elf
	@nm -S -n $(TARGET).elf > $(TARGET).map
	@$(OBJDUMP) -S $(TARGET).elf > $(TARGET).lst
	@export PATH=$(BINS):$$PATH; $(ESPTOOL) elf2image $(TARGET).elf

main.o: main.c z80/z80emu.h z80/z80user.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

z80/z80emu.o: z80/z80emu.c z80/z80emu.h
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

src/nosdk8266.o: src/nosdk8266.c
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<

src/startup.o: src/startup.S
	@echo [CC] $<
	@$(GCC) $(CFLAGS) -o $@ -c $<


#$(TARGET).elf : $(SRCS)
#	$(GCC) $(CFLAGS) $^ $(LDFLAGS) -o $@
#	nm -S -n $(TARGET).elf > $(TARGET).map
#	$(SIZE) $@
#	$(OBJDUMP) -S $@ > $(TARGET).lst
#	export PATH=$(BINS):$$PATH; $(ESPTOOL) elf2image $(TARGET).elf 

burn : $(FW_FILE_1) $(FW_FILE_2)
	$(ESPTOOL) -p $(FLASHPORT) -b $(FLASHBAUD) write_flash 0x00000 image.elf-0x00000.bin -ff 80m -fm dout

clean :
	rm -rfv $(FW_1) $(FW_2) {.,src,z80}/*.{s,o,i,elf,res,out,map,lst,out,bak} *~
	

