#!/bin/bash

zasm --z80 -u -w -x CPM22.Z80	# Generate HEX
zasm --z80 -u -w -b CPM22.Z80	# Generate BIN
