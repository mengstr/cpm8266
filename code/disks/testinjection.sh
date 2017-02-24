#!/bin/bash

#         DISK FILE                 MEMORY       LENGTH
#         STRT  END                STRT END     DEC/  HEX
# -------------------------------------------------------
# BOOT2     0..127   / 0000..007F  0000-007F    128 / 080 
# CCP     128..2175  / 0080..087F  DB00-E2FF   2048 / 800
# BDOS    2176..5759 / 0880..167F  E300-F0FF   3584 / E00
# BIOS    5760..6655 / 1680..19FF  F100-F47F    896 / 380



for i in {1..256256}; do echo -ne '\xE5'; done > DISK_A.DSK
for i in {1..128};    do echo -ne '@';    done > BOOT.bin
for i in {1..2048};   do echo -ne 'A';    done > CCP.bin
for i in {1..3584};   do echo -ne 'B';    done > BDOS.bin
for i in {1..896};    do echo -ne 'C';    done > BIOS.bin

dd of=DISK_A.DSK if=BOOT.bin seek=0    obs=1 conv=notrunc
dd of=DISK_A.DSK if=CCP.bin  seek=128  obs=1 conv=notrunc
dd of=DISK_A.DSK if=BDOS.bin seek=2176 obs=1 conv=notrunc
dd of=DISK_A.DSK if=BIOS.bin seek=5760 obs=1 conv=notrunc

hexdump -C DISK_A.DSK
