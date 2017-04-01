TL;DR - Quick Start
-------------------
  esptool.py write_flash 0 cpm8266_VERSION.bin -ff 80m -fm dout
  Connect to serial with any standard speed between 300 and 115200 baud
  Press <enter> twice to autobaud

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
esptool.py -p COM2 write_flash 0 cpm8266_04.bin -ff 80m -fm dout

On Linux:
esptool.py -p /dev/ttyUSB0 write_flash 0 cpm8266_04.bin -ff 80m -fm dout

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

Connect to the ESP with any standard speed between 300 and 115200  baud having
8 bits, 1 stopbit, no parity. 

After pressing Enter twice to let the emulator detect your baudrate 
youy will be greeted by the a> -prompt of CP/M.

There are 15 pcs of 256KB 8" drives emulated named from A: to O:, currently
some of them have pre-populated files.

"SCREENSHOT"
------------
cpm8266 - Z80 Emulator and CP/M 2.2 system version 0.4

62K CP/M v2.2 [cpm8266 v0.4 - SmallRoomLabs]

a>dir
A: ASM      COM : CRC      COM : CRC      MAC : DDT      COM
A: DDTZ     COM : DUMP     COM : ED       COM : FILES    TXT
A: LOAD     COM : PIP      COM : STAT     COM : SUBMIT   COM
A: XR       COM : XS       COM : XSUB     COM : FREE     SUB
A: ZDE      COM
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
