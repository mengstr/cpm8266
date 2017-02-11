#PORT = /dev/tty.SLAB_USBtoUART
PORT = /dev/ttyUSB0
BAUD = 921600

NAME = cpm8266

CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -mlongcalls -DICACHE_FLASH
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc -lc -Wl,-Map,$(NAME).map
LDFLAGS = -Teagle.app.v6.ld

$(NAME)-0x00000.bin: $(NAME)
	esptool.py elf2image $^

$(NAME): $(NAME).o z80emu.o uart.o

$(NAME).o: $(NAME).c
	$(CC) $(CFLAGS) -c $<

uart.o: driver/uart.c driver/uart.h driver/uart_register.h
	$(CC) $(CFLAGS) -c $<

z80emu.o: z80emu.c z80emu.h z80config.h z80user.h instructions.h macros.h tables.h
	$(CC) $(CFLAGS) -c $<

#--------------------------------------------------------

clean:
	rm -f $(NAME) *.o *~ *.bin *.map

flash: $(NAME)-0x00000.bin
	esptool.py -p $(PORT) -b $(BAUD) write_flash 0 $(NAME)-0x00000.bin 0x10000 $(NAME)-0x10000.bin

	 
serial:
	screen /dev/ttyUSB0 115200

size:
	xtensa-lx106-elf-size $(NAME) *.o

dump:
	xtensa-lx106-elf-objdump -x $(NAME) -d

erase:
	esptool.py -p $(PORT) -b $(BAUD) erase_flash
	esptool.py -p $(PORT) -b $(BAUD) write_flash 0x3fc000 esp/esp_init_data_default.bin 
#
#../private/esp-open-sdk/ESP8266_NONOS_SDK_V2.0.0_16_08_10/bin/esp_init_data_default.bin
#    0x07c000 for 512 kB (ESP-01, -03, -07 etc.)
#    0x0fc000 for 1 MB (ESP8285, PSF-A85, some ESP-01, -03 etc.)
#    0x1fc000 for 2 MB
#    0x3fc000 for 4 MB (ESP-12E, NodeMCU, WeMos)
#
    
