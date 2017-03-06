TL;DR - Quick Start
-------------------
  esptool.py write_flash 0 cpm8266_VERSION.bin -ff 80m -fm dout
  Connect to serial at 9600 baud
  Press <enter> 
  Start CP/M with B <enter>

INTRODUCTION
------------
This is the binary distribution of the cpm8266 project.  More information
and all the sources required for building the project yourself is available
at https://github.com/SmallRoomLabs/cpm8266

Please note that an ESP8266 with 4MB memory is required.

INSTALLING
----------
To install the binary onto your ESP8266 give the following command:

esptool.py -p PORT write_flash 0 cpm8266_VERSION.bin -ff 80m -fm dout

Change PORT to the serial/USB port your ESP is connected to and also change
VERSION to whatever version is indicated in the binary file.

Example for Windows:
esptool.py -p COM2 write_flash 0 cpm8266_01.bin -ff 80m -fm dout

On Linux:
esptool.py -p /dev/ttyUSB0 write_flash 0 cpm8266_01.bin -ff 80m -fm dout

You might want to add "-b 921600" to the command to increase the speed of 
the upload or it will take about 5 minutes to upload all of the 4MB using
the version 1.2 esptool. With the 2.0 tool at 921600 it only takes 35 
seconds.

USING
-----
Connect to the ESP8266 by a terminal emulator that can connect via the
serial port. 

On windows that might be HyperTerminal or putty.

On Linux I personally use Putty, screen or minicom from the command prompt.
Screen is not really a terminal emulator, but is easy to use as one. Start
it as "screen /dev/ttyUSB0 9600" and you can type away. It can be exited by
ctrl-A backslash.

Connect to the ESP with 9600 baud, 8 bits, 1 stopbit, no parity. 

After pressing Enter once you should be greeted by the EMON: -prompt
indicating that you have contact with a monitor in the Z80 emulator. To see
available commands type ? and press Enter.

To boot the CP/M emulator type B and press Enter.  The machine will boot and
you will be in the simulated A-drive of a CP/M 2.2 maching with 64K RAM and
a Z80 processor.

There are 15 pcs of 256KB 8" drives emulated named from A: to O:, currently
some of them have pre-populate files.

"SCREENSHOT"
------------
EMON:B
Starting excution at 0x0000

62K CP/M v2.2 [cpm8266 v0.1 - SmallRoomLabs]

a>dir
A: ASM      COM : DDT      COM : DDTZ     COM : DUMP     COM
A: ED       COM : LOAD     COM : PIP      COM : STAT     COM
A: SUBMIT   COM : XSUB     COM
a>

FINAL WORDS
-----------
This project uses code from a few other projects:

Lin Ke-Fong @anotherlin made the Z80 Emulator. 
https://github.com/anotherlin

Charles Lohr @CNLohr created the NoSDK for the ESP8266. 
https://github.com/cnlohr

Marcel de Kogel made the Z80 disassembler (to be used) in the Z80 monitor

If you have any questions, need help with using/installing this project, 
have suggestions, ideas or want to help develop this further please contact
me at mats.engstrom@gmail.com

Happy CP/M'ing. ^__^
