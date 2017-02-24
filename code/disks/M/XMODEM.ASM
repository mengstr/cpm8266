;==============================================================
;              CP/M XMODEM by Martin Eberhard 
;==============================================================
; XMODEM file transfer program for CP/M 1.4 and 2.2
;
; A compact command line and/or configuration file driven
; program for transferring files to and from a CP/M system
; using the XMODEM protocol, supporting both the original
; XMODEM checksum protocol and the newer XMODEM-CRC protocol.
;
; To use XMODEM, type:
;   XMODEM <filename> {option list}
;
; A file name is madatory, and can be any legal CP/M file
; name. If you are receiving with XMODEM and the file already
; exists, then you will be asked if it should be overwritten.
; If you are sending, then the file must exist.
;
; XMODEM first looks for a file called XMODEM.CFG on CP/M's
; default drive. If found, then this file is parsed for
; options, the same as the command line. XMODEM.CFG is parsed
; first, so that options that are set on the command line will
; override those set in XMOODEM.CFG.
;
; XMODEM.CFG and the command line both support the following
; options (though some are less useful on the command line.)
;
;  /R Specifies Receive mode
;
;  /S Specifies Send mode
;
;  Either /R or /S must be specified.
;
;  /C Selects checksum error checking when receiving;
;     otherwise receiving uses CRC error checking. When
;     sending, the error-checking mode is set by the receiver.
;
;  /E Specifies an enhanced RDR: routine that returns with the
;     Z flag set if no character is waiting. Note that this
;     option does not actually select the RDR: device as the
;     transfer port. (/X2 does.)
;
;  /In h0 h1 h2... (max h7) Defines assembly code for the custom
;      I/O port (used by the /X3 option), using Intel 8080
;      machine language.
;
;        n = 0 specifies initialization code, to be run when
;              command line and config file parsing are done.
;              All registers may be trashed. This is useful
;              for setting the baud rate, etc.
;
;        n = 1 installs a transmit byte routine.
;              on entry to this routine, the character to
;              send is in c. do not trash bc or de. Sample
;              custom transmit routine (for SOLOS):
;              48        mov   b,c    ;SOLOS wants chr in b
;              3e 01     mvi   a,1    ;serial pseudoport
;              cd 1c c0  call  AOUT   ;output character
;            Encode as follows:
;              /I1 48 3E 01 CD 1C C0
;
;        n = 2 installs a receive status subroutine,
;              which should return with the Z flag set if
;              no character is waiting. Do not trash any
;              registers exept psw. Sample routine:
;              3e 01     mvi   a,1    ;serial pseudoport
;              cd 22 c0  call  AINP   ;input character,
;                                     ;Z set if none
;            Encode as follows:
;              /I2 3E 01 CD 22 C0
;
;        n = 3 installs a receive character routine,
;              assuming a character is waiting. Returns
;              the character in a. Trashes no registers
;              except psw. If no routine is required
;              (e.g.  for SOLOS), then no /I3 option
;              is required.
;
;  /M Print message on console. This lets you tell the user
;     e.g. what port is set up for direct I/O in XMODEM.CFG
;
;  /O Specifies an output sequence for an I/O port, intended to
;     initialize the direct I/O port. The form of this
;     option is:
;       /O pp h1 h2 ... hn
;     where pp is the port address, and all the subsequent
;     bytes are sent to that port. You can have more than
;     one /O option, allowing you e.g. to initialize the
;     UART and also to set the baud rate.
;
;  /P Defines the direct I/O transfer port (used by the /X2
;     option). The form of this command is:
;       /P ss dd qq rr tt
;       where:
;          ss is the status port (for Rx and Tx)
;          dd is the data port (for both Rx and tx)
;          qq is 00 if the ready bits are true when low
;            and 01 if the ready bits are true when high
;          rr is the receive-ready bit mask
;          tt is the transmit-ready bit mask
;
;     XMODEM assumes that the receive port works like this:
;       RXWAIT: in       <status port>
;               ani      <Rx Mask>
;               jz/jnz   RXWAIT
;               nop
;               in       <data port>
;
;     ..and the transmit port works like this:
;               push     psw
;       TXWAIT: in       <status port>
;               ani      <Tx Mask>
;               jz/jnz   TXWAIT
;               pop      psw
;               out      <data port>
;
;     Any port that can work with these templates will work
;     with XMODEM's /X2 option.
;
;  All variables for the /O and /P commands are in hexidecimal,
;  and must be exactly two characters long. Legal characters
;  are: {0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F}
;
;  /Q Specifies quiet mode, preventing messages and pacifiers
;     from being printed on the console during transfer. This
;     is particularly useful if the port you are using is also
;     the port used for CP/M's console. Otherwise, a '+' will
;     be printed on CON: for every succesful block, and a '-'
;     will be printed for every block retry.
;
;  /Xn Specifies the transfer port:
;      n=0 uses CP/M's CON: device. Contrary to CP/M specs,
;          the CON: input port must not strip parity.
;
;      n=1 uses CP/M'd RDR: and PUN: devices. Contrary to CP/M
;          specs, the RDR: input port must not strip parity.
;          Also use /E if the RDR: driver has been enhanced
;          to return with the Z flag set when no character is
;          waiting. (Otherwise, no timeout is possible when
;          waiting for a RDR: character.)
;
;      n=2 uses direct I/O, which can be set up using the /P
;          option. If no /P option is entered, then /X2 will
;          select a MITS 88-SIO port.
;
;      n=3 uses custom-patched I/O routines, set with the
;          /I option. If no /I option is entered then
;          transfers with /X3 will just cause errors.
;
;  /Zn Specifies CPU speed in MHz - n is between 1 and 9.
;      The default is 2 MHz. This is used for timeouts while
;      sending or receiving. The default is 2 MHz for an 8080,
;      and 4 MHz for a Z80. (Xmodem can tell the difference.)
;
;   A semicolon begins a comment on a new line. All characters
;   from the ';' until the end of the line will be ignored.
;
; Here is a sample XMODEM.CFG file:
;
;    /MDirect I/O is configured for 88-2SIO port B
;    /P 12 13 01 01 02	;set up for an 88-2SIO Port B
;    /O 12 03 15	;8 bits, 1 stop, no parity
;    /X2		;use direct I/O
;
; You can usually abort a transfer with ^C on the console.
;
; The /P option modifies code in the direct I/O transmit and
; receive routines. This is because the 8080 has no command to
; IN or OUT to a port that is specified by a register value -
; so self-modifying code is the only way.
;
; XMODEM uses all available RAM, up to a maximum of 32K, for
; buffering received and transmitted data, to speed up
; transfers by minimizing disk thrashing.
;
; Code that is only used during initialization is at the end,
; and gets overwritten by the sector buffer. Also, XMODEM can
; overwrite CP/M's CCP, which gets reloaded when XMODEM
; terminates. This allows XMODEM to run with as little as
; (for example) 10K of user memory in a 16K-byte CP/M system,
; even with a large 8K data buffer and reasonably robust
; options and messages.
;
; This program will display correctly on screens that are 16X64
; or larger.
;
; Assemble with Digital Research's ASM Assembler
;
;==============================================================
; Thanks to Ward Christensen for inventing XMODEM and keeping
;   it simple.
; Thanks to John Byrns for the XMODEM-CRC extension, which was
;   adopted in Windows Hyperterm.
; Thanks to Keith Petersen, W8SDZ, for a few ideas I borrowed
;   from his XMODEM.ASM V3.2
;==============================================================
; Revision History:
;
; 1.0x  06 APR 2013 through 27 SEP 2014 M. Eberhard
;  Command-line driven versions Based on XMODEM for CDOS
;  (Z-80), version 1.03 by M. Eberhard
;
; 2.0  1 OCT 2014   M. Eberhard
;  New major release:
;   + Supports a configuration file (XMODEM.CFG), with same
;     options as on the command line
;   + combine features of all 1.0x assembly options
;   + Define direct I/O port in XMODEM.CFG (or on command line)
;   + User-set CPU speed (/Z), overrides 8080/Z80 defaults
;   + Option to delete file on /R, if it already exists
;   + Include which port we are using in announcement prior to
;     Xmodem file transfer
;   + A few new timeouts, to prevent hanging in odd situations
;   + Several other minor enhancements
;
; 2.1  3 Oct 2014  M. Eberhard
;  Fix bug in reporting the source of an error
;  Speed up RDR/PUN
;  require CR after "Y" on overwrite question
;
; 2.2  7 Oct 2014  M. Eberhard
;  fix error-reporting bug, increase stack size for BDOS
;
; 2.3  9 Oct 2014 M. Eberhard
;  Eliminate intermediate data buffer. Support CP/M 1.4
;
; 2.4  4 August 2016  M. Eberhard
;  Fix bug in TXLOOP that would cause sending with checksums
;  to fail. Fix bug causing a spurrious block after the 1st
;  buffer-full of received data. (Thanks to Bob Bell for
;  helping find and fix these bugs.) Add /I cmd, and add /X3
;  option for custom port routine patching. (This makes it
;  possible to call external I/O routines, such as SOLOS or
;  POLEX.) Also cleaned up comments.
;
; To Do (maybe in some future version):
;  Terminal mode for port testing/modem setup
;  Support for S-100 internal modem
;  Support Y-Modem extension (file name in block 0)
; (If any of these would be useful to you, let me know...)
;==============================================================
FALSE	equ	0
TRUE	equ	not FALSE

VERBOS	equ	FALSE	;true enables several progress messages
			;(Intended mainy for debugging)

ERRLIM	equ	10	;Max error-retries. 10 is standard.

;Timeout values in seconds. Values in parenthesis are
;XMODEM standard values.

SOHTO	equ	10	;(10)sender to send SOH 
NAKTO	equ	90	;(90)receiver to send init NAK
ACKTO	equ	60	;(60)receiver to ACK (or NAK)
			;(time to write to disk)

BLKSIZ	equ	128	;bytes per XMODEM block
			;DO NOT CHANGE. BLKSIZ must be 128

SECSIZ	equ	128	;CP/M sector size must be 128

BUFBLK	equ	16	;SECBUF will be a multiple of this
			;..many XMODEM blocks, which MUST BE
			; a power of 2. Some versions of CP/M
			;..work best with 16.

CIDEL	equ	10	;seconds before receiving when /X0

;Progress pacifiers printed on the console

PACACK	equ	'+'	;Sent/Received a good block
PACNAK	equ	'-'	;Sent/Received a NAK
PACLIN	equ	60	;pacifiers per line

;The following cycle values are used in the timing loops for
;timeouts when transferring via the CON: or the RDR: and PUN:.
;It is ok if they are imperfect - the XMODEM protocol is quite
;tolerant of timing variations. The example BIOS Code below was
;used to estimate these cycle counts for CSTIME and CRTIME.

CSTIME	equ	85	;number of CPU cycles that BIOS uses to
			;..return CON: status
CRTIME	equ	95	;number of cpu cycles that BIOS uses to
			;..return with no chr ready for custom
			;..RDR: driver

EXTIME	equ	135	;Number of cycles an external receive
			;routine (e.g. SOLOS) will use for testing
			;status, when a chr is not ready.

;===Example BIOS Code==========================================
;Timing est. for getting reader status via custom RDR: driver.
;Assume the IOBYTE is implemented, and assume RDR:=UR1:
;(the desired RDR: port)
;This takes 95 cycles.

;	jmp	RDRIN		;(10) BIOS jump vector

;	...

;RDRIN:	lda	IOBYTE		;(13) which reader port?
;	ani	0Ch		;(7)
;	jz	<not taken>	;(10) not RDR:=TTY:
;	cpi	8		;(7)
;	jc	<not taken>	;(10) not RDR:=HSR:
;	jz	UR1ST		;(10) RDR:=UR1:

;	...
	
;UR1ST:	in	<port>		;(10) get reader stat 
;	ani	<mask>		;(7) test, set Z
;	rz			;(11) return from BIOS

;===Example BIOS Code==========================================
;Timing estimate for getting console status.
;Assume the IOBYTE is implemented, and assume CON:=CRT:
;This takes 85 cycles.

;	jmp	CONST		;(10) BIOS jump vector

;	...

;RDRIN:	lda	IOBYTE		;(13) which CON: port?
;	ani	03h		;(7)
;	jz	<not taken>	;(10) not CON:=TTY:
;	cpi	2		;(7)
;	jc	CRTST		;(10) CON:=CRT:

;	...
	
;CRTST:	in	<port>		;(10) get reader stat 
;	ani	<mask>		;(7) test, set Z
;	rz			;(11) return from BIOS
;==============================================================

;**************
; CP/M Equates
;**************
;------------------------------------------
;BDOS Entry Points and low-memory locations
;------------------------------------------
WBOOT	equ	0000H		;Jump to BIOS warm boot
WBOOTA	equ	WBOOT+1		;Address of Warm Boot
IOBYTE	equ	WBOOT+3
;CDISK	equ	WBOOT+4		;Login drive
BDOS	equ	WBOOT+5		;BDOS Entry Point
BDOSA	equ	WBOOT+6		; First address of BDOS
				;(can overlay up to here)

FCB	equ	WBOOT+5CH	;CP/M file control blk
FCBDR	equ	FCB		;Drive Descriptor
FCBFN	equ	FCB+1		;File name (8 chrs)
FCBFT	equ	FCB+9		;File Type (3 chrs)
FCBEXT	equ	FCB+12		;File extent within FCB
FCBCLR	equ	24		;# of bytes to clear,
				;starting at FCBEXT
COMBUF	equ	WBOOT+80H	;disk & cmd line buffer
USAREA	equ	WBOOT+100H	;User program area

;------------------------------------------
;BDOS Function Codes, passed in register C
;Note: CON:, RDR:, and PUN: I/O is done via
;direct BIOS calls, not BDOS calls.
;------------------------------------------
;BRESET	equ	0	;System Reset
BCONIN	equ	1	;Read Console Chr
;BCONOT	equ	2	;Type Chr on Console
;BRDRIN	equ	3	;Read Reader Chr
;BPUNOT	equ	4	;Write Punch Chr
BPRINT	equ	9	;Print $-terminated String
BRDCON	equ	10	;Get Line from Console
;BCONST	equ	11	;Console Status (<>0 IF CHR)
;BDRST	equ	13	;Reset Disk
BSDISK	equ	14	;select disk
BOPEN	equ	15	;Disk File Open
BCLOSE	equ	16	;Close disk file, FCB at de
BSERCH	equ	17	;Search dir for file, FCB at de
BDELET	equ	19	;delete file, FCB at (de)
BREAD	equ	20	;Read from Disk, 0=OK, <>0=EOF
BWRITE	equ	21	;Write next record, 0=OK, <>0=ERR
BMAKE	equ	22	;Make new file, 0FFH=BAD
BCDISK	equ	25	;get current disk
BSTDMA	equ	26	;Set disk buffer to (de)

;--------------------------------------------------------
;BIOS Entry Points, relative to the base address in WBOOT
;--------------------------------------------------------
CONST	equ	06h	;Console Status
CONIN	equ	09h	;Console Input
CONOUT	equ	0Ch	;Console output
PUNCH	equ	12h	;punch output
READER	equ	15h	;reader input

;----------------------------
;88-SIO Registers and Equates
;----------------------------
SIOSTA	equ	00h		;status port
SIODAT	equ	01h		;data port

SIORDF	equ	00000001b	;-RX Data register full
SIOTDE	equ	10000000b	;-TX Data register empty

;----------------
;ASCII Characters
;----------------
SOH	equ	1		;Start of XMODEM block
CTRLC	equ	3		;Control-C for user-abort
EOT	equ	4		;End XMODEM session
ACK	equ	6		;XMODEM block acknowledge
TAB	equ	9		;horizontal tab
LF	equ	0AH		;Linefeed
CR	equ	0DH		;Carriage return
NAK	equ	15H		;XMODEM block negative ACK
EOF	equ	1Ah		;^Z end of XMODEM.CFG file
SELCRC	equ	'C'		;selects CRC mode at initiation

;*********************
;* Beginning of Code *
;*********************
	org	USAREA		;normal place for CP/M programs

;********************************
; Run-time stack is in the COMBUF
;********************************
RSTACK:

;-----------------------------------------------------
;Initialize, using code that gets wiped out by SECBUF,
;returns with b=0 for receive, 1 for send. During the
;transfer, the stack is located in the COMBUF. But
;until then, the COMBUD contains the command line
;options.
;-----------------------------------------------------
	lxi	SP,ISTACK	;initialization stack
	call	INIT
	lxi	SP,RSTACK	;run-time stack in COMBUF

;-------------------------------------------------
;Send or receive, based on XMODE,  set by /S or /C
;-------------------------------------------------
	dcr	b		;0 means receive	
	jnz	RXFILE

;	fall into TXFILE

;***Function*******************************************
; Send a CP/M file in XMODEM format
; On Entry:
;      FCB is valid
;******************************************************
TXFILE:	call	FOPEN		;Open file specified in FCB
				;& print message on console
				;sets SECCNT to 1

	call	GTMODE		;wait for NAK or SELCRC to
				;..determine cksum or CRC mode

;---------------------------------------------
;Transmit all sectors of the file:
;Read a sector from the disk, and test for EOF
;---------------------------------------------
TXLOOP:	xra	a		;clear carry
	sta	ERRCNT		;and error count

	lxi	h,SECCNT	;decrement SECCNT
	dcr	m
	cz	FILBUF		;Empty? go read disk
	jc	TXEOF		;C set means EOF

	lhld	CURBLK		;inc 16-bit block count
	inx	h
	shld	CURBLK

;---------------------------------
;Send block header: SOH, Block
;number, Complimented block number
;---------------------------------
TXRPT:	mvi	a,SOH		;SOH first
	call	TXBYTE

	lda	CURBLK		;8-bit block number
	call	TXBYTE		;(preserves a)

	cma			;complimented block no
	call	TXBYTE

;-------------------------------------------
;Send the next BLKSIZ-byte block from SECBUF
; On Entry:
;   BLKBUF has a sector full of data
; On Exit:
;   Data checksum is in c
;   16-bit data CRC is in CRC16
;-------------------------------------------
	lxi	h,0		;clear CRC for new block
	shld	CRC16

	lxi	b,BLKSIZ*256+0	;b=bytes/block,
				;...clear checksum in c

	lhld	SECPTR		;(hl) = data in SECBUF

TXBLUP:	mov	a,m		;Get a data byte
	call	TXBYTE		;Send it, do checksum

	call	CALCRC		;combine a into the CRC
				;and do checksum too

	inx	h		;Next byte
	dcr	b
	jnz	TXBLUP		;loop through block

;--------------------------------------------
;Send checksum or 16-bit CRC, based on CRCFLG
; c= 8-bit checksum
; CRCFLG <>0 if CRC mode enabled
;--------------------------------------------
	lda	CRCFLG		;Checksum or CRC?
	ora	a		;clear Z if CRCFLG
	jz	TXBDON		;jump to send checksum

	push	h		;save SECBUF pointer

	lhld	CRC16		;get calculated CRC
	mov	a,h
	call	TXBYTE		;send byte in a
	mov	c,l		;now the 2nd CRC byte

	pop	h		;recover SECBUF pointer

TXBDON:	mov	a,c		;a=cksum or CRC 2nd byte 
	call	TXBYTE		;send byte in a

;-------------------------------------------
;Wait for the ACK. If none arrives by the
;end of the timeout, or if a NAK is received
;instead of an ACK, then resend block.
;-------------------------------------------
	call	GETACK		;Wait for the ACK
				;Z flag set if ACK

	jnz	TXRPT		;NZ: timeout or NAK

	shld	SECPTR		;next sector in the buffer

;---------------------------------------------------
;Ack received. Print pacifier, and go for next block
;---------------------------------------------------
	call	PACOK		;..if allowed
	jmp	TXLOOP

;---------------------------------------------------
;File send completed. Send EOT'S intil we get an ACK
;---------------------------------------------------
TXEOF:	mvi	a,EOT		;Send an EOT
	call	TXBYTE

	call	GETACK		;Wait for an ACK
	jnz	TXEOF		;Loop until an ACK

;--------------------------------------------------
;Report count of successful blocks & return to CP/M
;--------------------------------------------------
TXCNT:	call	CILPRT
	db	'OK',CR,LF
	db	'Sent',' '+80h

	jmp	REPCNT		;print block count, goto CP/M

;***Function*******************************************
; Receive XMODEM file & save it to disk 
; On Entry:
;       FCB is valid
;******************************************************
RXFILE:	call	CREATE		;create & open file on disk

;----------------------------------------------------
;Receive & validate a block, and see if we got an EOT
;----------------------------------------------------
RXLOOP:	call	GETBLK		;Receive an XMODEM block
	jc	RXEOT		;C set means EOT received

	lhld	CURBLK		;inc 16-bit block count
	inx	h
	shld	CURBLK

;------------------------------------------------
;Good block received. Print pacifier on console,
;then send ACK and loop back to get another block
;------------------------------------------------
	call	PACOK		;..if allowed
	call	TXACK		;Send XMODEM ACK
	jmp	RXLOOP		;LOOP until EOF

;----------------------------------
;Received EOT. Flush SECBUF and end
;----------------------------------
RXEOT:	call	WFLUSH		;Write all blocks in SECBUF
	call	TXACK		;ACK this sector
	call	FCLOSE		;Close CP/M file

;-------------------------------------------------
;Send happy termination message and return to CP/M
;-------------------------------------------------
	call	CILPRT
	db	'O','K'+80h

;	fall into RRXCNT

;***Exit********************************
;Report the number of blocks succesfully
;received, and then return to CP/M
;***************************************
RRXCNT:	call	CILPRT
	db	'Received',' '+80h

;	fall into REPCNT

;***Exit*******************************************
;Report 16-bit block count, and then return to CP/M
;**************************************************
REPCNT:	lhld	CURBLK
	call	PDEC16		;print hl in decimal

	call	MSGXIT
	db	' blocks$'

;***Subroutine***************
; Get an XMODEM block
; On Entry:
; On Exit:
;   Carry set if EOT received
;   Trashes all registers
;****************************
GETBLK:

;-----------------------------------------
;Wait for SOH from sender to start
;reception, checking for EOT while we wait
;-----------------------------------------
	xra	a
	sta	ERRCNT		;Clear error count

RXRPT:	mvi	e,SOHTO*2	;Timeout for SOH
	call	RXBYTE
	jc	RXSERR		;Carry means timeout

	cpi	SOH		;Did we get an SOH?
	jz	RXSOH		;If so, get the block

;-------------------------------------
;Set carry and return if we get an EOT
;-------------------------------------
	cpi	EOT
	stc
	rz

;------------------------------------------
;No SOH or EOT - this is an invalid header.
;Eat the rest of this block, up to 255
;received chrs, until a 1-sec timeout.
;------------------------------------------
PURGE:	mvi	b,0		;count 255 chrs

PRGLUP:	lxi	d,SYNMSG	;Sync error message
	dcr	b
	jz	ABORT		;too many bogus chrs

	call	RXBYT1		;Receive w/ 1-sec t/o
	jnc	PRGLUP		;Carry means timeout


; fall into RXSERR

;-----------------------------------------------
;Send a NAK to indicate receive error. If we are
;waiting to start and we are in CRC mode (NAKCHR
;=SELCRC), then send SELCRC instead of NAK
;-----------------------------------------------
RXSERR:	call	CCTRLC		;user abort?

	call	PACERR		;..if allowed

	lda	NAKCHR		;current NAK chr
	call	TXBYTE

;----------------------------------------------
;Bump error count, and abort if too many errors
;----------------------------------------------
	lxi	h,ERRCNT	;Clear error count
	inr	m		;bump error count

	mov	a,m		;Too many errors?
	cpi	ERRLIM
	jc	RXRPT		;No: try again

;-----------------------------------
;Too many errors: abort with message
;-----------------------------------
	lxi	d,ELEMSG	;error limit exceeded
	jmp	ABORT

;--------------------------------------------------
;Got an SOH, at beginning of block. Now get header:
;Block number, Complemented block number
;--------------------------------------------------
RXSOH:	mvi	a,NAK		;we have received
	sta	NAKCHR		;..at least one SOH

	call	RXBYT1		;Get block number
	jc	RXSERR		;Carry means timeout

	mov	d,a		;Save block number

	call	RXBYT1		;complimented block number
	jc	RXSERR		;Carry means timeout

	cma			;compliment to compare
	cmp	d
	jnz	PURGE		;No match: error

	sta	RXBLK		;Save block number

;----------------------------------------------------------
;Loop to receive BLKSIZ bytes and store them in the next
;slot in SECBUF, computing the checksum & CRC along the way
;----------------------------------------------------------
	lxi	h,0		;clear CRC
	shld	CRC16

	lxi	b,BLKSIZ*256+0	;b=bytes, c=0 cksum
	lhld	SECPTR		;next sector in the buffer

RXCHR:	call	RXBYT1		;Get one byte of data
	jc	RXSERR		;Carry means timeout

	mov	m,a		;Store byte in buffer
	call	CALCRC		;calculate CRC & checksum

	inx	h		;next byte
	dcr	b
	jnz	RXCHR

;------------------------------------------------------
;Verify checksum in c, or CRC in CRC16, based on CRCFLG
;------------------------------------------------------
	lda	CRCFLG		;CRC mode?
	ora	a		;0 means cksum
	jz	CKCKSM

	call	RXBYT1		;Get 1st byte of CRC
	jc	RXSERR		;Carry means timeout

	push	h		;save pointer

	lhld	CRC16
	mov	c,l		;put 2nd CRC byte in c
	cmp	h

	pop	h		;recover pointer

	jnz	PURGE		;no: try again, but 1st
				;purge rest of CRC

CKCKSM:	call	RXBYT1		;2nd CRC byte or cksum
	jc	RXSERR		;Carry means timeout

	cmp	c		;Does it match?
	jnz	RXSERR		;No: error

;-----------------------------------------------
;Got a good block. See if we've already received
;this block. (It might be a retransmission.) If
;it's the most recently received block, then try
;again - otherwise it's an error.
;-----------------------------------------------
	lda	CURBLK		;8-bit block number
	mov	b,a		;b=recent Rx block

	lda	RXBLK		;a=this block's number
	sub	b		;calc the difference
	cz	TXACK		;same as last block:
	jz	GETBLK		;..send ACK & try again

	dcr	a		;should be next block
	jnz	SYNCER		;if not, sync error

;--------------------------------------------------
;Good block. Bump pointers and see if we must flush
;--------------------------------------------------
	shld	SECPTR		;next slot in SECBUF
	
	lxi	h,SECCNT	;next sector
	inr	m
	lda	SBUFSZ
	cmp	m		;is SECBUF full?
	rnz			;N: return for more
				;carry is clear
	
; SECBUF is full. fall into WFLUSH to write it to disk

;***Subroutine************************************
; Write all data in SECBUF to disk
; On Entry:
;   SECCNT has count of blocks currently in SECBUF
; On Exit:
;   SECCNT=0
;   carry is clear
;   hl=(SECPTR)=SECBUF
;   Trashes all other registers
;*************************************************
WFLUSH:	lda	SECCNT		;# of sectors in SECBUF
	ora	a		;End of file already?
	rz			;Return w/ Z set if so

	mov	b,a		;Sector count in b
	lxi	d,SECBUF	;de=start of sect data

WFLOOP:	mvi	c,BSTDMA	;CP/M SET DMA function
	call	GOBDOS		;de = DMA address

	xchg			;pointer to hl, free de

	lxi	d,FCB		;Write from buf to disk
	mvi	c,BWRITE
	call	GOBDOS

	lxi	d,EWFMSG
	ora	a		;return 0 if okay
				;this clears carry too
	jnz	ABORT		;OOPS, write error

				;hl = address in SECBUF
	lxi	d,BLKSIZ	;de=block size
	dad	d		;(hl)=next sector data
	xchg			;addr to de for BSTDMA

	dcr	b
	jnz	WFLOOP		;until all sectors sent

;-------------------------------
;Reset pointers for empty SECBUF
;(a=0 and carry is clear here)
;-------------------------------
;	xra	a
	sta	SECCNT		;SECCNT = 0
	lxi	h,SECBUF	;reset SECPTR
	shld	SECPTR

	ret

;--------------------------
;Error: Blocks out of order
;--------------------------
SYNCER:	lxi	d,SEMSG		;sync error
	jmp	ABORT

;***Subroutine****************************************
; Get an ACK from the receiver. If we get a NAK, print
; the NAK pacifier on the console.
; On Exit:
;   Z set and Carry clear if ACK received
;   Z clear and Carry clear if NAK received
;   Z clear, Carry set and ERRCNT bumped if timeout
;      or too many bogus chrs received
;   If too mant errors, abort
;   Trashes a,bc,e,hl
;*****************************************************
GETACK:	

;-------------------------------
;Get a received byte, or timeout
;-------------------------------
	mvi	e,ACKTO*2	;ACK-wait timeout value
	call	RXBYTE		;go get a character
	jc	ACKERR		;Carry means timeout

;------------------------------------------------
;Return form subroutine with Z set if it's an ACK
;------------------------------------------------
	cpi	ACK		;Did we get an ACK?
	rz			;Y: return w/ carry cleared

;----------------------------------------
;If NAK, print pacifier, and return with
;Carry & Z cleared unless the user aborts
;----------------------------------------
	mov	e,a		;save received chr
	call	CCTRLC		;user abort?
	mov	a,e		;recover received chr

	cpi	NAK		;NAK?
	jnz	ACKERR		;NZ: bad byte received

	call	PACERR		;..if allowed

	ora	a		;NAK: Clear Z & carry
	ret	

;----------------------------------------------------
;Timeout or bogus chr while waiting for ACK/NAK
;Bump error count & check limit. Set carry for return
;----------------------------------------------------
ACKERR:	call	CCTRLC		;user abort?

	lxi	h,ERRCNT	;bump error count
	inr	m

	mov	a,m		;too many errors?
	cpi	ERRLIM
	rc			;No: Return w/ carry set
				;..and Z cleared for timeout

;--------------------------------------
;Abort waiting for ACK: Too many errors
;--------------------------------------
	lxi	d,TAEMSG	;too many ack errors
	jmp	ABORT

;***Subroutine*****************************
; Close CP/M disk file 
; This is required after writing to a file.
; On Exit:
;   de = FCB
;   Trashes psw
;******************************************
FCLOSE:	lxi	d,FCB		;FCB describes the file
	mvi	c,BCLOSE	;CP/M CLOSE FILE funct.
	call	GOBDOS
	inr	a		;-1 meant close error
	rnz

;--------------------------------------
;Error closing file: abort with message
;--------------------------------------
	call	CMSGXT
	db	'FILE CLOSE ERROR! May be corrupt.$'

;***Subroutine************************************
; Read up to SBUFSZ more sectors from the disk and
; put them in SECBUF
;
; On Entry:
; On Exit:
;   hl=(SECPTR)=SECBUF
;   SECCNT= number of sectors in the buffer
;   Carry set if no more sector data
;   EOFLAG set if EOF encountered
;   Trashes psw,bc,de
;*************************************************

;---------------------------------------
;SECBUF is empty: read up to SBUFSZ more
;sectors from the disk into SECBUF
;---------------------------------------
FILBUF:	lda	EOFLAG		;Have we seen the EOF?
	ora	a
	stc			;Return w/ carry if so
	rnz

	lda	SBUFSZ
	mov	b,a		;b=free secs in SECBUF
	lxi	d,SECBUF	;de=address in SECBUF

RSECLP:	mvi	c,BSTDMA	;Set CP/M DMA address
	call	GOBDOS		;trashes no registers

	xchg			;pointer to hl, free de

	lxi	d,FCB		;Disk sect. into SECBUF
	mvi	c,BREAD
	call	GOBDOS		;trashes no registers

	sta	EOFLAG		;Set EOF flag if EOF
	ora	a		;Read ok?
	jnz	FBDONE		;No: no more data

	lxi	d,BLKSIZ 	;next block
	dad	d
	xchg			;Result goes in de

	dcr	b
	jnz	RSECLP		;go until all space used

;---------------------------------------------------
;Receive buffers are all full, or we got an EOF from
;CP/M. compute & save block count, point SECPTR to
;the 1st block. If we received 0 sectors, the return
;with carry set.
; On Entry:
;   b = remaining space in SECBUF
;---------------------------------------------------
FBDONE:	lxi	h,SECBUF	;Point SECPTR to start
	shld	SECPTR		;..of SECBUF

	lda	SBUFSZ		;compute # of secs read
	sub	b		;b=remaining space
	sta	SECCNT		;Store sector count

	rnz			;ret with carry clear

	stc			;zero sectors:
	ret			;ret with carry set

;***Subroutine****************************
; Update the 16-bit CRC with one more byte
; speed matters here.
; On Entry:
;   a has the new byte
;   c has checksum so far
;   CRC16 is current except this byte
; On Exit:
;   c = c + new byte
;   CRC16 has been updated
;   Trashes psw,de
;*****************************************
CALCRC:	mov	d,a		;save chr
	add	c		;update checksum
	mov	c,a
	mov	a,d		;recover chr

	push	b
	push	h
	lhld	CRC16		;get CRC so far

	xra	h		;XOR into CRC top byte
	mov	h,a

	lxi	d,1021h		;CRC16's magic number
	mvi	b,8		;prepare to rot 8 bits

CROTLP:	dad	h		;16-bit shift
	jnc	CCLR		;skip if bit 15 was 0

	mov	a,h		;CRC=CRC xor 1021H
	xra	d
	mov	h,a
	mov	a,l
	xra	e
	mov	l,a

CCLR:	dcr	b
	jnz	CROTLP		;rotate 8 times

	shld	CRC16		;save CRC so far
	pop	h
	pop	b
	ret

;***Subroutine**********************
; Receive a byte, with 1-sec timeout
; On Entry:
; On Exit:
;   Carry set for timeout error
;   a = received byte if no timeout
;   trashes e
;***********************************
RXBYT1:	mvi	e,2		;1-second timeout

;	fall into RXBYTE

;***Subroutine********************************************
; Receive a byte from the transfer port - the CON: or RDR:
; device or a direct I/O port, based on the state of XPORT
;
; This routine gets modified by /P option
; file.
; On Entry:
;   e = timeout value in half-seconds
;   XPORT specifies the transfer port:
;     0 for CON:
;     1 for RDR:
;     2 for direct port access
;     3 for custom I/O routine
; On Exit:
;   Carry set for timeout error
;   a = received byte if no timeout
;   e trashed
;
; Note: reading with an enhanced RDR device is just barely
; fast enough at 38.4K baud with a Z80 CPU. Some effort
; was taken here to keep that path quick.
;*********************************************************
RXBYTE:	push	h

	lhld	XPORT		;which kind of transfer port?
	dcr	l		;Make RDR: fast
	jz	RXRDR

	mov	a,l
	lhld	TIMRLD		;start timeout timer

	dcr	a
	jz	DIRRX		;Direct port access
	jp	CUSTRX		;receive via external routine

;Receive via CON:

;--------------------------------------------
;Receive a transfer byte from CON:
;RXBCON loop: 195+CRTIME cycles, and round up
CONTO:	equ	50000/((195+CRTIME+9)/10)
;--------------------------------------------
RXBCON:	call	RXTIMR		;(17+24) Timeout?

	mvi	a,CONST		;(7)get console status
	call	GOBIOS		;(116+17+CRTIME)
	ora	a		;(4)nz means chr ready
	jz	RXBCON		;(10)Go get the chr

	mvi	a,CONIN		;get console chr
	jmp	CRDONE

;----------------------------------------------------
;Receive a transfer byte from RDR:
;If ENHRDR<>0 then read from a RDR: port that returns
;with Z set if no chr ready. Timeout if no character
;in e x 2 seconds. If ENHRDR=0 then just read from
;the RDR: device, which will hang in the BIOS until a
;character arrives - no timeout is possible.
;On Entry:
;  l=0
;  h=ENHRDR
;----------------------------------------------------
RXRDR:	dcr	h		;test ENHRDR

	lhld	TIMRLD		;start timeout timer
	jz	RXERDR

	mvi	a,READER	;BIOS routine offset

;----------------------------------
;Get character from BIOS and return
;On Entry:
;   a = BIOS routine offset
;----------------------------------
CRDONE:	pop	h
	call	GOBIOS

	ora	a		;clear carry		
	ret			;success

;---------------------------------------------------------
;Receiver from enhanced RDR routine, which returns with Z
;set if no character is waiting - allowing a timeout here.
;RXERDR loop: 191+CRTIME cycles, and round up
RDRTO:	equ	50000/((191+CRTIME+9)/10)
;--> Entry is at RXERDR <--
;---------------------------------------------------------
RXERLP:	call	RXTIMR		;(17+24)

RXERDR:	mvi	a,READER	;(7)BIOS routine offset
	call	GOBIOS		;(116+17+BIOS time)
	jz	RXERLP		;(10)nz means chr ready

	pop	h
	ora	a		;clear carry
	ret

;------------------------------------------------
;Generic transfer port Input Routine - gets
;modified by INIT based on selected transfer port
;Timeout if no chr in e/2 seconds.
;--> Entry is it DIRRX <--
; WAITRX loop = 69 cycles. 0.5S / 69 uS = 7247
DIRTO	equ	7247
;------------------------------------------------
WAITRX:	call	RXTIMR		;(17+24)

DIRRX:
IMODFY:	in	SIOSTA		;(10+1)status port (modified)
	ani	SIORDF		;(7)test ready (clears carry) (modified)
	jnz	WAITRX		;(10)high when chr ready (modified)

DTIMOT:	pop	h		;here for IMODFY
	in	SIODAT		;data port (modified)
	ret

;--------------------------------------------------
;Custom Receive Subroutine
;--> Entry is at CUSTRX <---
;Assume WATCRX loop time is 96 cycles, and round up
CUSTO:	equ	50000/((96+EXTIME+9)/10)
;--------------------------------------------------
WATCRX:	call	RXTIMR		;(17+24)

CUSTRX:
;Wait for data to be ready
;(8 bytes will be written here by /I2)

CRSTAT:	xra	a		;set z flag, so that default
	db	nop		;..routine will timeout
	db	nop
	db	nop
	db	nop
	db	nop
	db	nop
	db	nop

	jz	WATCRX
	pop	h

;Get the received data byte
;(8 bytes will be written here by /I3)

CRDAT:	ora	a		;just make default faster
	ret
	db	nop
	db	nop
	db	nop
	db	nop
	db	nop
	db	nop

	ora	a		;clear carry
	ret

;---Local Subroutine----------------------
; bump timer, test for abort every 1/2 sec
; On Entry:
;  hl = timer current value
;   e = remaining timeout in 0.5 sec units
;  top-of-stack = our return address
;  next-on-stack = hl save value
;  next-on-stack = RXBYTE return address
; On Exit:
;  hl decremented, and reloaded if zero
; On Timeout:
;    Return from RXBYTE
;    carry set
; trashes psw
; (24 cycles for normal case)
;-----------------------------------------
RXTIMR:	dcx	h		;(5)timeout timer?
	mov	a,l		;(5)Test for 16-bit 0
	ora	h		;(4) (clears carry)
	rnz			;(10)

;low word Overflow. Test for timeout and user abort

	call	CCTRLC		;user abort?

	lhld	TIMRLD		;reload timer
	dcr	e		;bump timer high byte
	rnz			;return unless timeout

;Timeout: fix stack, return from RXBYTE with carry set

	pop	h		;chuck our return address
	pop	h		;restore original hl
	stc			;indicate error
	ret			;from RXBYTE

;***Subroutine*********************************
; Send ACK
; On Exit:
;   a trashed
;   All flags and all other registers preserved
;**********************************************
TXACK:	mvi	a,ACK

;	fall into TXBYTE

;***Subroutine***********************************
; Send a to the transfer port, based on XPORT and
; the assembly options.
; This routine gets modified by /P option
;
; On Entry:
;   a = byte to send
;   XPORT specifies the transfer port:
;     0 for CON:
;     1 for PUN:
;     2 for direct port access
;     3 for custom subroutine
; On Exit:
;   All registers preserved
;************************************************
TXBYTE:	push	b
	mov	c,a		;chr to c

	lda	XPORT		;which port?
	dcr	a
	jm	TXCON		;0:transmit via console

	dcr	a
	mvi	a,PUNCH		;1:BIOS send c to punch
	jm	TXBA
	jz	TXDRCT		;2:direct port access
				;3:custom subroutine
;-------------------------------------
;Custom Output Subroutine
;(8 bytes will be written here by /I1)
;-------------------------------------
CWDAT:	lxi	d,TTOMSG	;default causes error
	jmp	ABORT
	nop
	nop

	mov	a,c		;restore registers
	pop	b
	ret	

;----------------------
;Transmit via CP/M CON:
;----------------------
TXCON:	mvi	a,CONOUT	;BIOS send c to console

;------------------------------
;Transmit via CP/M CON: or PUN:
;------------------------------
TXBA:	call	GOBIOS		;chr in c, routine in a

	mov	a,c		;restore character
	pop	b
	ret

;----------------------------------------
;Transmit via direct I/O, with timeout
;the timeout value doesn't really matter:
;we just shouldn't hang forever here
;----------------------------------------
TXDRCT:	push	h
	lxi	h,0		;about 1.7 second timeout
				;..at 2 MHz

TXWAIT:	dcx	h		;(5)timeout?
	mov	a,h		;(5)
	ora	l		;(4)
	jz	TXBTO		;(10)y: abort

OMODFY:	in	SIOSTA		;(10+1)status port (modified)
	ani	SIOTDE		;(7)mask (modified)
	jnz	TXWAIT		;(10)may become jnz (modified)

;52 cycles = 26 uS per pass at 2 MHz

	mov	a,c		;recover chr
	out	SIODAT		;data port (modified)

	pop	h
	pop	b
	ret

;---------------------------------
;Transmitter timeout: the UART CTS
;signal is probably not true.
;Rudely abort program.
;---------------------------------
TXBTO:	lxi	d,UTOMSG	;exit message
	jmp	ABORT
	
;***Subroutine***********************************
; Print hl in decimal on the console with leading
; zeros suppressed
; Trashes all registers
;************************************************
PDEC16:	mvi	d,0		;Suppress leading 0's

	lxi	b,-10000
	call	DECDIG
	lxi	b,-1000
	call	DECDIG
	lxi	b,-100
	call	DECDIG
	lxi	b,-10
	call	DECDIG

	mov	a,l		;last digit is simple
	jmp	DECDG0		;with leading 0's

;---Local Subroutine------------------------------
; Divide hl by power of 10 in bc and print result,
; unless it's a leading 0.
; On Entry:
;   hl=Dividend
;   bc=divisor (a negative power of 10)
;   d=0 if all prior digits were 0
; On Exit:
;   Quotent is printed, unless it's a leading 0
;   hl=remainder
;   d=0 iff this and all prior digits are 0
;-------------------------------------------------
DECDIG:	mvi	a,0FFh		;will go 1 too many times
	push	d		;leading zero state

DIGLP:	mov	d,h		;de gets prev value
	mov	e,l
	inr	a
	dad	b		;subtract power of 10
	jc	DIGLP

	xchg			;hl has remainder
	pop	d		;leading 0 state

	mov	e,a		;e has digit to print
	ora	d		;leading 0 to suppress?
	rz			;yes: digit is done

	mov	d,a		;don't suppress next digit
 
	mov	a,e	

DECDG0:	adi	'0'		;make digit ASCII

; fall into PRINTA

;***Subroutine*******************
; Print character in a on console
; trashes psw,c
;********************************
PRINTA:	mov	c,a		;value to c for PRINTC

; fall into PRINTC

;***Subroutine*******************
; Print character in c on console
; trashes psw
;********************************
PRINTC:	mvi	a,CONOUT

; fall into GOBIOS

;***Subroutine**********************
; Go call a BIOS driver directly
; On Entry:
;   c=value for BIOS routine, if any
;   a = BIOS call address offset
; On Return:
;   psw as BIOS left it
;   all other regs preserved
;   (116 cycles + BIOS time)
;***********************************
GOBIOS:	push	h		;(11)
	push	d		;(11)
	push	b		;(11)

	call	DOBIOS		;(17+26+BIOS time)

	pop	b		;(10)
	pop	d		;(10)
	pop	h		;(10)
	ret			;(10)done

;***Subroutine**********************
; Go call a BIOS driver directly
; On Entry:
;   c=value for BIOS routine, if any
;   a = BIOS call address offset
; On Return:
;   all regs as BIOS left them
;   (26 cycles + BIOS time)
;***********************************
DOBIOS:	lhld	WBOOTA		;(16)get BIOS base address
	mov	l,a		;(5)a has jump vector

	pchl			;(5) 'call' BIOS routine

;***Subroutine***************************************
; Print error pacifier on the console unless disabled
; On Entry:
;   PACCNT =FFh to disable pacifier printing
;   otherwise, PACCNT = column position
; On Exit:
;   PACCNT incremented mod 64, unless it is FFh
; trashes psw,c
;****************************************************
PACERR:	mvi	c,PACNAK
	db	LDA		;lda opcode skips 2 bytes

;Hop into PACIFY

;***Subroutine**************************************
; Print good pacifier on the console unless disabled
; On Entry:
;   PACCNT =FFh to disable pacifier printing
;   otherwise, PACCNT = column position
; On Exit:
;   PACCNT incremented mod 64, unless it is FFh
; trashes psw,c
;***************************************************
PACOK:	mvi	c,PACACK

;Fall into PACIFY

;***Subroutine*************************************
; Print pacifier on the console unless disabled.
; Print a CR/LF at the end of every 64 columns.
; On Entry:
;   C=pacify chr
;   PACCNT =FFh to disable pacifier printing
;   otherwise, PACCNT = column position
; On Exit:
;   PACCNT incremented mod PACLIN, unless it is FFh
; trashes psw,c
;**************************************************
PACIFY:	lda	PACCNT		;pacifiers enabled?
	inr	a
	rz			;n: no pacifier printed

	sta	PACCNT
	cpi	PACLIN		;line full?
	jnz	PRINTC

	xra	a		;new line
	sta	PACCNT
	call	PCRLF		;need a CR?

	jmp	PRINTC		;print pacifier

;***Subroutine*****
;Delete file at FCB
;******************
FDELET:	mvi	c,BDELET	;delete existing file
	lxi	d,FCB
	db	LDA		;lda opcode skips 2 bytes

;Hop into GOBDOS

;***Subroutine******************
;Print $-terminated string at de
;trashes psw,c
;*******************************
PRINTF:	mvi	c,BPRINT
; fall into GOBDOS

;***Subroutine*********************************
;Call BDOS while preserving all regs except psw
;**********************************************
GOBDOS:	push	h
	push	d
	push	b
	call	BDOS
	pop	b
	pop	d
	pop	h
	ret

;***Subroutine***
; Print CR, LF
; Trashes psw
;****************
PCRLF:	call	ILPRNT
	db	CR,LF+80H
	ret
		
;***Subroutine****************************************
; Print CR, LF, then In-line Message
;  The call to ILPRNT is followed by a message string.
;  The last message chr has its msb set.
; Trashes psw
;*****************************************************
CILPRT:	call	PCRLF

; fall into ILPRNT

;***Subroutine****************************************
; Print In-line Message
;  The call to ILPRNT is followed by a message string.
;  The last message chr has its msb set.
; On Exit:
;  Z cleared
; Trashes psw
;*****************************************************
ILPRNT:	xthl			;Save hl, get msg addr
	push	b

IPLOOP:	mov	a,m
	ani	7Fh		;strip end marker
	call	PRINTA		;print byte
	mov	a,m		;end?
	inx	h		;Next byte
	ora	a		;msb set?
	jp	IPLOOP		;Do all bytes of msg

	pop	b
	xthl			;Restore hl,
				;..get return address
	ret

;***Subroutine*************************************
;Check for Control-C on the console, and quit if so
; On Exit:
;   Z set if no chr was waiting
;   Z clear if anything but ^C was waiting
; trashes a
;**************************************************	
CCTRLC:	mvi	a,CONST		;anything on console?
	call	GOBIOS		;(about 200 cycles)
	ora	a		;Z means no chr waiting
	rz

; chr waiting: fall into GETCON to take a look

;***Subroutine*********************************
;Get console character, abort if it's control-C
; On Exit:
;   chr in a
;    Z cleared
; trashes a
;**********************************************	
GETCON:	mvi	a,CONIN		;read the typed chr
	call	GOBIOS
	cpi	CTRLC
	rnz			;ignore everything else

	lxi	d,CCMSG		;control C

;	fall into ABORT to close file and report

;***Exit**********************************************
;Abort - close file if writing, delete it if no blocks
 received
; ON Entry:
;  de = abort message to print
;  XMODE = 0 for receiving, <>0 for sending
;*****************************************************
ABORT:	call	CILPRT
	db	'ABORT:',' '+80h

	call	PRINTF		;print string at de

	lda	XMODE		;need to close the file?
	ora	a		;0 means receiving
	jnz	EXIT

	call	FCLOSE		;Close file neatly

	lhld	CURBLK		;any disk blks written?
	mov	a,h
	ora	l		;check 16-bit blk count
	jnz	RRXCNT		;y: report blks written

	call	FDELET		;n: delete empty file

	inr	a		;successful delete?
	jz	EXIT		;no: done

	call	CMSGXT		;Exit w/ this message
	db	'Empty file deleted$'

;***************************
;$-terminated abort messages
;***************************
CCMSG:	db	'^C$'			;User typed ^C

ELEMSG:	db	(ERRLIM/10)+'0'		;too many block retries
	db	(ERRLIM-((ERRLIM/10)*10))+'0'
	db	' block errors$'

TAEMSG:	db	(ERRLIM/10)+'0'		;too many bad ACKs
	db	(ERRLIM-((ERRLIM/10)*10))+'0'
	db	' ACK errors$'

SEMSG:	db	'lost blocks$'		;out of sequence
EWFMSG:	db	'disk write fail$'	;CP/M error
UTOMSG:	db	'UART '			;fall into TTOMSG
TTOMSG:	db	'Tx fail$'		;Tx not ready
SYNMSG:	db	'sync fail$'		;can't find SOH

;***Exit********************************************
; Print CRLF, then $-terminated string following the
; call. Fix everything for CP/M, and return to CP/M
;***************************************************
CMSGXT:	call	PCRLF

; fall into MSGXIT

;***Exit*******************************************
; Print $-terminated string following the call, fix
; everything for CP/M, and return to CP/M
;**************************************************
MSGXIT:	pop	d		;Get message address
	call	PRINTF

;	fall into EXIT

;***Exit************************************
; Return to CP/M. All exits go through here.
;*******************************************
EXIT:	jmp	WBOOT		;go to CP/M

;******************************************************
;RAM Variables and Storage, all initialized during load
;******************************************************
;------------------------------
;XMODEM file transfer variables
;------------------------------
RXBLK:	db	0	;Received block number
CURBLK:	dw	0	;16-bit Current block number 
ERRCNT:	db	0	;Error count
CRC16:	dw	0	;calculated CRC so far
NAKCHR:	db	NAK	;current NAK chr
TIMRLD:	dw	0	;receive timeout value

;------------------------
;Disk buffering variables
;------------------------
BYTCNT:			;cmd buff bytes (reuse SBUFSZ)
SBUFSZ:	db	0	;max sectors in SECBUF
SECPTR:	DW	SECBUF	;Points to next sect in SECBUF
SECCNT:	db	1	;Count of sectors in SECBUF
			;..init to 1 for send
CLFLAG:			;1 means reading .CFG,
			;..0 means command line
EOFLAG:	db	1	;EOF flag (<>0 means true)

;----------------------
;Command line variables
;----------------------
XMODE:	db	0FFH	;1 for send, 0 for receive
			;FFh for uninitialized
CRCFLG:	db	SELCRC	;0 for cksum, SELCRC for CRC

XPORT:	db	1	;Transfer port defaults to RDR/PUN
ENHRDR:	db	0	;01 for RDR: that returns with
			;..Z set if chr not ready
			;MUST follow XPORT

PACCNT:	db	PACLIN-1 ;Current column position for
			;pacifiers. Init to start new line
			;FF disables pacifiers.
CPUMHZ:	db	2	;CPU speed in MHz (for timeouts)

;******************************************************
;Buffer for SBUFSZ disk sectors (same as XMODEM blocks)
; This buffer over-writes the following initialization
; code.
;******************************************************
SECBUF:	equ	$		;Sector buffer

;******************************************************
; The following subroutines are used only during the  *
; initial command line processing, and get wiped out  *
; by the SECBUF, once we start transfering data.      *
;******************************************************

;***Subroutine*****************************************
;Open CP/M disk file (for reading), and reports success
; or failure to console.
; On Entry:
;   FCB has file name
; On successful Exit:
;   de = FCB
;   File is open
;   File-open message has been rinted on the console
;   TIMRLD is set for chosen port and CPU speed
; On failure:
;   Relevent error msg has been printed on the console
;   jump to CP/M
; trashes psw,bc
;******************************************************
FOPEN:	lxi	d,FCB		;FCB describes file to open
	mvi	c,BOPEN		;CP/M FILE OPEN function
	call	GOBDOS
	inr	a		;-1 means open failure
	jz	FOFAIL

;Compute receive timer timeout value
;based on XPORT and cpu speed

	call	XPRTBC		;set bc for selected port
	call	TSETUP		;set TIMRLD for CPU speed

;Start announcing

	call	CILPRT
	db	'File open',CR,LF
	db	'Sen','d'+80h

	; fall into ANNCTP

;***Subroutine**********************
;Announce transfer port. Disable
; pacifiers if transfer port is CON:
; trashes psw,c
;***********************************
ANNCTP:	call	ILPRNT
	db	'ing via',' '+80h

	lda	XPORT
	dcr	a
	jm	TVC

	dcr	a
	jm	TVR
	jz	TVD

	call	ILPRNT
	db	'external cod','e'+80h
	ret

TVD:	call	ILPRNT
	db	'direct I/','O'+80h
	ret

TVR:	call	ILPRNT
	db	'RDR/PU','N'+80h
	ret

TVC:	sta	PACCNT		;CON: turn off pacifiers
	call	ILPRNT
	db	'CO','N'+80h
	ret

;--------------------------------------
;Error opening file: Abort with message
;--------------------------------------
FOFAIL:	call	CMSGXT	;Exit w/ this message
	db	'File not found$'

;***Subroutine*****************************************
;Get the error-checking mode
; Wait for initial NAK or a SELCRC from receiver to get
; going. (NAK means we use checksums, SELCRC means use
; CRC-16.) Ignore all other characters, w/ long timeout
; Abort if user types Control-C
; On Entry:
;   CRCFLG = 0, defaulting to checksum mode
;   XPORT=0 if using console for transfers
;     (so don't print messages on console)
; On Succesful Exit:
;   CRCFLG = 0 if NAK received
;   CRCFLG = SELCRC if SELCRC received
;   Message printed if CRC mode
; Trashes all registers
;******************************************************
GTMODE:	mvi	b,NAKTO		;Long timeout
	lxi	h,CRCFLG	;assume cksum for now
	mvi	m,0

WAITNK:	lxi	d,NAMSG
	dcr	b		;Timeout?
	jz	ABORT		;yes: abort

	call	RXBYT1		;trashes e
	cpi	NAK		;NAK for checksum?
	jz	PCSNT		;yes:message, done

	cpi	SELCRC		;'C' for CRC?
	jnz	WAITNK		;No: Keep looking

	mov	m,a		;remember CRC mode

	lda	PACCNT		;Quiet mode?
	inr	a
	rz			;y: no message

; fall into PCRC

;***Subroutine*******
; Print 'with CRCs '
; On Exit:
;   Z flag cleared
; trashes a,c
;********************
PCRC:	call	ILPRNT
	db	' with CRC','s'+80h
	ret

;***Subroutine************************
; Print 'with checksums ' unless quiet
; On Exit:
;   Z flag cleared
; trashes a,c
;**************************************
PCSNT:	lda	PACCNT		;quiet mode?
	inr	a
	rz			;y: no message

;fall into PCKSUM

;***Subroutine***********
; Print 'with checksums '
; On Exit:
;   Z flag cleared
; trashes a,c
;************************
PCKSUM:	call	ILPRNT
	db	' with checksum','s'+80h
	ret

;*********************
;$-terminated Messages
;*********************
NAMSG:	db	'no init from receiver$'

;***Subroutine*****************************************
;Create file on disk and report
; On Entry:
;   FCB has file name
; On successful Exit:
;   File is created and open
;   File-created message has been rinted on the console
;   Initial NAK or C (cksum or CRC mode) has been sent
;   SECCNT=0
; On failure:
;   Relevent error msg has been printed on the console
;   jump to CP/M
; trashes all registers
;******************************************************
CREATE:
;------------------------------------------------------
;See if file already exists, and ask to overwrite if so
;------------------------------------------------------
	lxi	d,FCB

	mvi	c,BSERCH	;Search directory for file
	call	GOBDOS
	inr	a		;-1 means not there
	jz	FILNEX		;no file: ok

	call	CILPRT
	db	'File exists. Overwrite (Y/N)','?'+80h

	lxi	h,COMBUF
	mvi	m,16		;max chrs the user may type
	xchg			;save d...
	mvi	c,BRDCON	;get a line of user input
	call	GOBDOS
	xchg			;de still points to FCB

	inx	h		;count of chrs typed
	dcr	m
	jnz	EXIT		;must be just 1 chr there

	inx	h		;get the only chr
	mov	a,m
	ori	'y'-'Y'		;either case
	cpi	'y'
	jnz	EXIT

	call	FDELET		;delete existing file

FILNEX:	xra	a		;no sectors yet
	sta	SECCNT

;------------------------
;Create file on CP/M disk
; de still points to FCB
;------------------------
	call	CILPRT		;either 'File created'
				;or 'File create error'
	db	'File creat','e'+80h

	mvi	c,BMAKE		;CP/M CREATE FILE func
	call	GOBDOS
	inr	a		;-1 means create error

	jz	FCERR

;---------------------------
;Tell user that we are ready
;---------------------------
	call	ILPRNT		;finish message

	db	'd'		;end of 'File created'
	db	CR,LF,'Recei','v'+80h

	call	ANNCTP		;announce port

;-----------------------------------------------
;Delay for a few seconds if transfering via the
;console, to give the user time to start sending
;Use RXTIMER, before it gets set up with XPORT.
;-----------------------------------------------
	lda	XPORT		;console?
	ora	a
	jnz	RXIND

	lxi	b,5208		;delay loop time
	call	TSETUP		;adjust for CPU speed
	mvi	e,CIDEL*8	;initial delay in 1/8 secs

RIDEL:	call	RXTIMR		;(24+17=41)use Rx timer
	jnc	RIDEL		;(10)

; 125 mS at 1MHz is 250000/24=5208

RXIND:

;-----------------------------------
;Compute receive timer timeout value
;based on XPORT and cpu speed
;-----------------------------------
	call	XPRTBC		;set bc for selected port
	call	TSETUP		;adjust for CPU speed

;--------------------------------------------
;Set initial character to NAK or SELCRC, and
;report error checking mode (checksum or CRC)
;--------------------------------------------
	lda	CRCFLG		;CRC or checksum?
	ora	a		;0 means checksum
				;SELCRC means CRC

	jz	RXCSM
	sta	NAKCHR		;set CRC initial ACK

	call	PCRC		;print ' with CRCs'
				;..returns with Z cleared
RXCSM:	cz	PCKSUM		;print ' with checksums'

;----------------------------------------------
;Send initial NAK or SELCRC to get things going
;----------------------------------------------
	lda	NAKCHR		;send the initial ACK
	jmp	TXBYTE		;return via TXBYTE

;---------------------------------------
;Error: File create failed
; 'File create' has already been printed
;---------------------------------------
FCERR:	call	MSGXIT
	db	' fail. Write protect? Dir full?$'

;***Subroutine*********************************
;Initialization: parse command line, set up FCB
; return with b=0 for receive, 1 for send 
;**********************************************

;Set default CPU speed to 4MHZ if a Z80 is detected.
;The user can later change this with /Z

INIT:	sub	a		;test for 8080 or Z80
	jpe	IS8080
	mvi	a,4		;Assume Z80s run 4 MHz
	sta	CPUMHZ
IS8080:

;----------------------------------------
;Copy the command buffer so that CP/M 1.4
;won't destroy it during the BDOS call
;----------------------------------------
	lxi	h,COMBUF
	lxi	d,ICOMBF
	mvi	b,80

LDIR:	mov	a,m
	stax	d
	inx	h
	inx	d
	dcr	b
	jnz	LDIR

;-----------------------------
;look for a configuration file
;and parse it, if it exists
;-----------------------------

	lxi	d,CFGFCB	;FCB describes file to open
	mvi	c,BOPEN		;CP/M FILE OPEN function
	call	GOBDOS
	inr	a		;-1 means open failure

;BYTCNT = 0 here

 if not VERBOS
	cnz	PARSE
 endif
 if VERBOS
	jz	NOCFG
	call	CILPRT
	db	'Reading XMODEM.CF','G'+80h
	call	PARSE
NOCFG:
 endif

;----------------------------------------
;Next, parse commands on the command line
;----------------------------------------
 if VERBOS
	call	CILPRT
	db	'Reading command lin','e'+80h
 endif

	xra	a		;command line next
	sta	CLFLAG		;also clears EOFLAG

	lxi	d,ICOMBF	;copy of CP/M cmd line
	ldax	d		;1st byte is the byte count
	sta	BYTCNT
	inx	d

	call	WSKIP		;skip initial whitespace
	jc	HLPEXT		;no parameters: help

;Skip past the file name, which CP/M
;already put in the FCB for us

SKPFIL:	call	CMDCHR
	jc	CMDONE		;end of command line?

	cpi	'/'		;option crammed
	jnz	SF1		;..against file name?	

	dcx	d		;y: back up
	inr	m		;hl=BYTCNT from CMDCHR
	mvi	a,' '		;..and pass the next test	

SF1:	cpi	' '		;hunt for a space
	jnz	SKPFIL

	call	PARSE
CMDONE:

;------------------------------------------------
;Initialize File Control Block for disk transfers
;------------------------------------------------
	xra	a
	lxi	h,FCBEXT
	mvi	b,FCBCLR
FCBLUP:	mov	m,a
	inx	h
	dcr	b
	jnz	FCBLUP

;-------------------------------------------------------
;Compute available memory for SECBUF, and save it in
;SBUFSZ. (This will allow CP/M's CCP to be overwritten.)
;Limitations:
; 1. The buffer size is a multiple of BUFBLK, to
;    optimize transfers.
; 2. Since SBUFSZ is an 8-bit value, the max size of
;    SECBUF is FFh, rounded to nearest BUFBLK.
;-------------------------------------------------------

; compute number of available 256-byte pages

	lhld	BDOSA		;BDOS address
	lxi	d,0-SECBUF	;-SECBUF address
	dad	d		;16-bit subtract

	mov	a,h		;result high byte is the
				;..# of 256-byte pages

; a=number of 256-byte pages available. Compute number
; of 128-byte (sector-sized) pages, with an 8-bit max.

	rlc			;*2
	jnc	SB1		;overflow?
	mvi	a,0FFh		;y: use max value		

SB1:	ani	100h-BUFBLK	;round to nearest BUFBLK
	sta	SBUFSZ		;save result

 if VERBOS
; announce buffer size

	mov	l,a
	mvi	h,0

	call	CILPRT
	db	'Buffer:',' '+80h

	call	PDEC16

	call	ILPRNT
	db	' sector','s'+80h
 endif

;-----------------------------------------
; Run any user-defined initialization code
; (This gets overwritten by /I0 option)
;-----------------------------------------
CINIT:	nop			;8 bytes space
	nop
	nop
	nop
	nop
	nop
	nop
	nop

;--------------------------------
;Did we get a direction? Return
;with b=XMODE if so, error if not
;--------------------------------
	lda	XMODE		;did /R or /S get set?
	mov	b,a		;for return
	inr	a		;-1 meant uninitialized
	rnz			;go send or receive

	call	CMSGXT		;Exit with this message
	db	'Must specify /R or /S$'

;***Subroutine**********************************
; Parse command line or CFG file options
; On Entry:
;   CLFLAG=1 for config file, 0 for command line
;   de = address of command string
;        terminated by EOF
;***********************************************
PARSE:
;-----------------------------------------------------------
;Parse all command line options & set variables accordingly. 
;Each option must be preceeded by a '/' Options may be
;preceeded by any reasonable number of spaces, tabs,
;carriage returns and/or line feeds.
;-----------------------------------------------------------
OPTLUP:	call	WSKIP		;skip whitespace
	rc			;end of input input?

	lxi	b,OPTLUP	;create return address
	push	b

	cpi	';'		;comment?
	jz	COMMNT		;y: ignore until CR or LF

	cpi	'/'		;all start with /
	jnz	BADINP		;error:no slash

	call	CMDCHR		;Get an option chr
	jc	BADINP		;Error: nothing after /

;-----------------------------------------------------------
;Got a command line option in a. Loop through table of
;options, looking for a match. Update the appropriate option
;variable with the table value. Error exit if not in table.
; a=option character
; Trashes c,hl
;-----------------------------------------------------------
	lxi	h,OPTTAB

CHKLUP:	cmp	m		;Match? (alpha order)
	inx	h
	mov	c,m		;get routine address offset
	inx	h

	jc	OPTERR		;illegal option
	jnz	CHKLUP		;No match: keep looking

;--------------------------------------------
;Option match. Go execute option routine
; On Entry:
;    c = option routine adress offset
;    de  points to next cmd byte
;    top-of-stack = return address to OPTLUP
; Command routines preserve/advance de
;--------------------------------------------
	xra	a
	mov	b,a		;high byte
	lxi	h,CMDBAS
	dad	b		;hl=address of routine

	pchl			;go to routine

;*****************
; Option Commands
;*****************
CMDBAS:

;******-----------------------
;* /C * Set Rx Checksum Mode
;******
; On Entry:
;    a=0
;   (de)=next command line chr
; On Exit:
;   CRCFLG = 0
;-----------------------------
CCKSUM:	sta	CRCFLG
	ret

;******-----------------------------
;* /E * Specify Enhanced Reader
;****** (RDR: returns Z when no chr)
; On Entry:
;    a=0
;   (de)=next command line chr
; On Exit:
;   ENHRDR = 1
;-----------------------------------
CMODR:	inr	a
	sta	ENHRDR
	ret

;*****---------------------------------------------------
; /I * Patch Custom I/O Routine
;*****
; /I0 hh hh hh... defines init code
; /I1 hh hh hh... defines transmit port routine
; /I2 hh hh hh... defines receive status routine
; /I3 hh hh hh... defines receive data routine
; Max 8 hh digits. (The original intention is to use these
; patches to call some ROM I/O routines, perhaps with a
; couple of registers set up prior to the calls.)
; On Entry:
;   (de)=next command line chr
; On Exit:
;   A Custom Transfer port routine has been written
;   de incremented past /I data
;--------------------------------------------------------
CCIO:	call	CMDCHR		;get next command line chr

;Get the address of the routine to define, based on a

	lxi	h,CINIT
	sui	'0'		;un-ASCII, test 0
	jz	CIOGET		;Init code?

	lxi	h,CWDAT
	dcr	a		;Tx port code?
	jz	CIOGET

	lxi	h,CRSTAT
	dcr	a		;Rx stat code?
	jz	CIOGET

	lxi	h,CRDAT
	dcr	a		;Rx data code?
	jnz	BADVAL		;n: bogus

;Get & install all routine bytes, padding with nops at the end

CIOGET:	mvi	c,8		;max bytes for a routine

CIOG0:	push	h
	call	GETHEX
	pop	h
	jnc	GIOG1		;any character?
	xra	a		;n: install nop
GIOG1:	mov	m,a

	inx	h
	dcr	c
	jnz	CIOG0

	ret			;note: any more hex values
				;will cause an error in PARSE

;******-------------------------
;* /M * print message on console
;******
; On Entry:
;   (de)=next command line chr
; On Exit:
;   CRCFLG = 0
;-------------------------------
CMESSG:	call	PCRLF		;initial new line

CMSGLP:	call	CMDCHR		;get next chr
	rc			;end of file?
	rz			;end of message string?
	call	PRINTA		;to console
	jmp	CMSGLP

;******------------------------------------------
;* /O * Output to Port
;******
; On Entry:
;   (de)=next command line chr
;        subsequent bytes are init sequence
; On Exit:
;   Data sequence has been sent to specified port
;   de incremented past /O data
;------------------------------------------------
COUTP:	call	GTHEXM		;get port number
	sta	IPORT+1

CILOOP:	call	GETHEX		;get an init value
	rc			;done?

IPORT:	out	0		;port address gets modified
	jmp	CILOOP

;******--------------------------------------
;* /P * Define Transfer Port
;******
; On Entry:
;   (de)=next command line chr
; On Exit:
;   Transfer port routines have been modified
;   de incremented past /P data
;--------------------------------------------
CPORT:	call	GTHEXM		;get status port
	mov	l,a
	call	GTHEXM		;get data port
	mov	h,a
	call	GTHEXM		;get jz/jnz flag
	mov	c,a
	call	GTHEXM		;get Rx ready mask

	push	psw		;save Rx ready on stack
	call	GTHEXM		;get Tx ready mask

	xchg			;ports to de,pointer to hl
	xthl			;save pointer on stack
	push	h		;put Rx Ready back on stack

	lxi	h,OMODFY+1	;a=Tx ready mask
	call	MODIO		;modify input routine

	pop	psw		;a=Rx ready mask
	lxi	h,IMODFY+1
	call	MODIO		;modify input routine

	pop	d		;restore cmd pointer

	ret

;******-----------------------
;* /Q * Enables quiet mode
;******
; On Entry:
;    a=0
;   (de)=next command line chr
; On Exit:
;   PACCNT=FFh
;-----------------------------
CQUIET:	dcr	a		;a=FFh
	sta	PACCNT
	ret

;******-----------------------
;* /S * Select send mode
;******
; On Entry:
;    a=0
;   (de)=next command line chr
; On Exit:
;   XMODE = 1
;-----------------------------
CSETS:	inr	a	;a=1

; fall into CSETR tpo save XMODE

;******-----------------------
;* /R * Select receive mode
;******
; On Entry:
;    a=0
;   (de)=next command line chr
; On Exit:
;   XMODE = 0
;-----------------------------
CSETR:	sta	XMODE
	ret

;******-----------------------
;* /X * Select transfer port
;******
; On Entry:
;   (de)=next command line chr
; On Exit:
;   XPORT set as specified
;-----------------------------
CSETX:	call	CMDCHR

	sui	'0'		;un-ASCII
	cpi	4		;0-3 allowed
	jnc	BADVAL
	sta	XPORT
	ret

;******--------------------------------
;* /Z * Specify CPU speed, in MHz (1-9)
;******
; On Entry:
;   (de)=next command line chr
; On Exit:
;   CPUMHZ updated
;--------------------------------------
CMHZ:	call	CMDCHR

	sui	'1'		;un-ASCII
	cpi	9		;1-9 allowed
	jnc	BADVAL

	inr	a		;make it 1-9
	sta	CPUMHZ
	ret

;***Subroutine***************
; Ignore a comment
; On Entry:
;   de=next command line chr
;****************************
COMMNT:	call	CMDCHR
	rc			;end of file?
	jnz	COMMNT		;Z means CR or LF
	ret
	
;***Exit********************************************
; Print help screen, and then exit. Break up the
; help screen so that it even fits on a 16x64 screen
;***************************************************
HLPEXT:	call	CILPRT		;print this message

;    1234567890123456789012345678901234567890123456789012345678901234
 db '=========================',CR,LF
 db 'XMODEM 2.4 By M. Eberhard',CR,LF
 db '=========================',CR,LF,LF
 db 'Usage: XMODEM <filename> <option list>',CR,LF
 db '^C aborts',CR,LF,LF
 db 'Command line and XMODEM.CFG options:',CR,LF
 db ' /R to receive or /S to send',CR,LF
 db ' /C receive with checksums, otherwise CRC checking',CR,LF
 db '    (Receiver sets error-check mode when sending)',CR,LF
 db ' /E if CP/M RDR returns with Z set when not ready',CR,LF,LF
 db '--More-','-'+80h

	call	GETCON		;wait for user input

	call	CILPRT

 db ' /I options patch I/O routines with 8080 code for /X3:',CR,LF
 db '   /I0 h0 h1 ...(up to h7) for initialization',CR,LF
 db '   /I1 h0 h1 ...(up to h7) for Tx data (chr is in reg c)',CR,LF
 db '   /I2 h0 h1 ...(up to h7) for Rx status (Z set if no chr)',CR,LF
 db '   /I3 h0 h1 ...(up to h7) for Rx data (chr in reg a)',CR,LF
 db ' /M console message',CR,LF
 db ' /O pp h0 h1 ... hn outputs bytes h1-hn to port pp',CR,LF
 db ' /P ss dd qq rr tt defines direct I/O port',CR,LF
 db '   ss = status port',CR,LF
 db '   dd = data port',CR,LF
 db '   qq = 00 if ready bits are true low, 01 if true high',CR,LF
 db '   rr = Rx ready bit mask',CR,LF
 db '   tt = Tx ready bit mask',CR,LF,LF
 db '--More-','-'+80h

	call	GETCON		;wait for user input

	call	CMSGXT		;print message and exit to CP/M

 db ' /Q for Quiet; else + means good block, - means retry',CR,LF
 db ' /X commands select the transfer port:',CR,LF
 db '   /X0 CP/M CON',CR,LF
 db '   /X1 CP/M RDR/PUN (default)',CR,LF
 db '   /X2 Direct I/O, defined by /P option',CR,LF
 db '   /X3 8080 I/O code, patched with /I options',CR,LF
 db ' /Zm for m MHz CPU. 0<m<10, default m=2',CR,LF,LF
 db 'CP/M CON and RDR must not strip parity.',CR,LF
 db 'Values for /I, /O and /P are 2-digit hex.',CR,LF
 db '$'
	
;***Exit*****************************************
;Illegal option. Print message and return to CP/M
; ON Entry: a=illegal option found
;************************************************
OPTERR:	sta	PAR1		;put it in error msg

	call	CILPRT		;Exit with this message
	db	'/'
PAR1:	db	'&'		;parameter goes here
	db	' unrecognize','d'+80h

	jmp	ERRSRC		;command line or .CFG file

;***Exit********************************************
;Input error exits. Print message and return to CP/M
;***************************************************
BADINP:		call	CILPRT
	db	'Jun','k'+80h

; fall into ERRSRC

;***Exit*********************************************
; Bad input of some sort. Print source of error
; and quit to CP/M
; On Entry:
;  CLFLAG = 1 if reading .CFG file, 0 if command line
;****************************************************
ERRSRC:	lda	CLFLAG		;command line or XMODEM.CFG?
	ora	a
	jz	BADCLN

	call	MSGXIT
	db	' in XMODEM.CFG$'

BADCLN:	call	MSGXIT
	db	' in command line$'


;***Subroutine******************************
; Set bc to the correct value for 1/2 second
; receive timouts based on XPORT
; On Exit:
;  bc is set
;*******************************************
XPRTBC:	lda	XPORT
	lxi	b,CONTO
	dcr	a
	rm		;was it 0: CON:?

	lxi	b,RDRTO
	rz		;was it 1: RDR:?
		
	lxi	b,DIRTO
	dcr	a	;was it 2: direct?
	rz	

	lxi	b,CUSTO
	ret		;it was 3: custom

;***Subroutine*************************************
; Adjust hl based on CPU speed
; On Entry:
;   bc = CPU cycles for 0.5 sec loop
;        assuming 1 MHz CPU
;   CPUMHZ = CPU speed, in MHZ
; On Exit:
;   hl=TIMRLD=CPU cycles in loop for this CPU speed
; trashes psw,hl
;**************************************************
TSETUP:	lda	CPUMHZ
	lxi	h,0

ADJMHZ:	dad	b
	dcr	a
	jnz	ADJMHZ

	shld	TIMRLD		;timer reload value
	ret

;***Subroutine***********************************
; Modify either the transfer input port routine
; or output port routine. This assumes that both
; routines look like this:
;
; WAIT:	...
; IMODFY or OMODFY:
;	in	<status port>
;	ani	<port ready mask>
;	jnz	WAIT	(may get converted to jz)
;
;	pop psw (or other 1-byte cmd)
;	in/out	<data port>
;	...
;	ret
;
; On Entry:
;     a = port-ready mask byte
;     c = 0 if jnz needs to be installed
;     d = data port address
;     e = status port address
;    hl = IMODFY+1 or OMODFY+1
; On Exit:
;    z flag unchanged
; trashes a,hl
;**********************************************
MODIO:	mov	m,e		;install status port

	inx	h		;point to mask location
	inx	h
	mov	m,a		;install mask

	inx	h		;point to jnz location
	mov	a,c
	ora	a
	jz	MODIO1		;need a jz instead?
	mvi	m,JZ		;y: install jz opcode
MODIO1:

	inx	h		;point to data port loc
	inx	h
	inx	h
	inx	h
	inx	h
	mov	m,d		;install data port
	ret

;***Subroutine*************************************
; get a mandatory 2-digit hex value from LINBUF
; On Entry:
;   de points to first hex digit
; On Exit:
;  a=value
;  de advanced 2
;  trashes b
;  Rude jump to BADVAL if no chr or bogus hex found
;**************************************************
GTHEXM:	push	h
	call	GETHEX
	pop	h
	rnc

; Fall into BADVAL

;***Exit*******************************************
; Bad Value, bad hex character
; Fix everything for CP/M, and return to CP/M
;**************************************************
BADVAL:	call	CILPRT
	db	'Bad valu','e'+80h

	jmp	ERRSRC		;command line or .CFG file

;***Subroutine****************************************
; get an exactly 2-digit hex value from LINBUF
; On Entry:
;   de points to first hex digit
; On Exit:
;  carry set if no value found, either due to
;    end of input or non-hex chr found on 1st digit
;  a=value
;  de advanced past hex if hex found
;  de pointing to non-hex chr if found on 1st digit
;  hl=BYTCNT
;  trashes b
;  Rude jump to BADVAL if bogus hex found on 2nd digit
;*****************************************************
GETHEX:	call	WSKIP		;skip whitespace, get a chr
				;also sets hl=BYTCNT
	rc			;eof?

	call	HEX2BN		;convert a=1st digit
	jc	GHBACK		;bogus?

	add	a		;shift into place
	add	a
	add	a
	add	a
	mov	b,a		;save digit

	call	GHNIB
	jc	BADVAL		;no digit found or bogus?

	add	b		;combine w/ high digit
	ret			;carry is clear for ret

;non-hex 1st chr found, so backup

GHBACK:	dcx	d		;back up
	inr	m		;(does not affect carry)
	ret			;with carry set
		
;---Local Subroutine-----------------
;convert (de) to binary
; On Exit:
;   a=chr
;  de advanced 1
;  carry set if bad hex chr or no chr
;------------------------------------
GHNIB:	call	CMDCHR		;get a character
	rc			;carry:no more chrs

; Fall into HEX2BN

;---Local Subroutine-------
;convert a to binary
; On Exit:
;   a=chr
;  carry set if bad hex chr
;--------------------------
HEX2BN:	cpi	'9'+1		;below ASCII 9?
	jc	HC1		;Yes: deal with digit

	cpi	'A'		;between 9 & A?
	rc			;yes: bogus

	sui	'A'-'9'-1	;no: subtract offset	

HC1:	sui	'0'
	cpi	10H		;above 0Fh?
	cmc			;so carry means error
	ret			;carry clear means ok

;***Subroutine*************************************
;Skip over spaces, tabs, CRs, and LFs in command
; line buffer until any other character is found 
; On Entry:
;    BYTCNT has remaining byte count
;    hl points to the next chr in buffer
; On Exit:
;    a = chr from buffer
;    BYTCNT has been decremented
;    de has been advanced
;    Carry means end of buffer (and a is not valid)
;**************************************************
WSKIP:	call	CMDCHR		;sets Z if CR or LF
	rc			;carry set if nothing left
	jz	WSKIP		;skip CR or LF
	cpi	' '
	jz	WSKIP		;skip space
	cpi	TAB
	jz	WSKIP		;skip tab
	ret			;chr in a, carry clear

;***Subroutine******************************************
;Get more command bytes
;If we are reading from ICOMBF then we are done
;Otherwise, try to get another XMODEM.CFG sector
; On Entry:
;    CLFLAG = 1 if reading from XMODEM.CFG
;             0 if reading from command line in ICOMBF
;    de points to the next chr
;    hl=BYTCNT
; On Exit:
;    a = chr from ICOMBF or XMODEM.CFG, parity stripped
;    de has been reset and advanced
;    BYTCNT has bee reset and decremented
;       unless at end
;    Carry means end of buffer
;    Z means CR or LF found
;    hl trashed
;*******************************************************
RDCMD:	lda	CLFLAG		;reading command buffer?
	sui	1
	rc			;y: ret w/carry: done	

	push	b
	lxi	d,CFGBUF
	push	d
	mvi	c,BSTDMA	;Set CP/M DMA address
	call	GOBDOS		;trashes no registers
	lxi	d,CFGFCB
	mvi	c,BREAD		;read another sector
	call	GOBDOS		;a=0 if not eof
	pop	d		;buffer address
	pop	b		;d=CFGBUF

	ora	a		;end of input?
	stc			;nz if end if file found
	rnz

	mvi	m,BLKSIZ	;another XMODEM.CFG sector

; Fall into CMDCHR

;***Subroutine******************************************
;Get next character from command line buffer
; On Entry:
;    CLFLAG = 1 if reading from XMODEM.CFG
;             0 if reading from command line in ICOMBF
;    BYTCNT has remaining buffer byte count
;    de points to the next chr
; On Exit:
;    a = chr from ICOMBF or XMODEM.CFG, parity stripped
;    de has been advanced and BYTCNT decremented
;       unless at end
;    hl =BYTCNT
;    Carry means end of buffer
;    Z means CR or LF found
;*******************************************************
CMDCHR:	lxi	h,BYTCNT
	mov	a,m
	ora	a		;any bytes left?
	jz	RDCMD		;n:try to get more
	
	ldax	d		;get buffer chr
	ani	7Fh		;Strip parity

	cpi	EOF		;file end?
	stc			;y: ret with carry set
	rz

	inx	d		;bump buffer pointer
	dcr	m		;dec BYTCNT

	cpi	CR
	rz
	cpi	LF
	stc
	cmc			;clear carry
	ret
	
;*********************************************
;Command Line Options Table
; Table entries must be in alphabetical order,
; and terminated with 0FFh
;
; 2 bytes per entry:
;  Byte 0 = Uppercase legal option letter
;  Byte 1 = offset to address of parse routine
;*********************************************
OPTTAB:	db	'C',CCKSUM-CMDBAS	;Select checksum mode
	db	'E',CMODR-CMDBAS	;Enhanced RDR port
	db	'I',CCIO-CMDBAS		;Custom I/O definition	
	db	'M',CMESSG-CMDBAS	;console message
	db	'O',COUTP-CMDBAS	;output to port
	db	'P',CPORT-CMDBAS	;define transfer port
	db	'Q',CQUIET-CMDBAS	;quiet mode
	db	'R',CSETR-CMDBAS	;select receive mode
	db	'S',CSETS-CMDBAS	;select receive mode
	db	'X',CSETX-CMDBAS	;select transfer port
	db	'Z',CMHZ-CMDBAS		;specify CPU MHz
	db	0FFh			;end of table

;*********************************
; Configuration File Control Block
;*********************************
CFGFCB:	db	0		;(dr) use default drive
	db	'XMODEM  '	;(f1-f8)
	db	'CFG'		;(t1-t3)
	db	0,0,0,0		;(ex,s1,s2,rc)
	dw	0,0,0,0,0,0,0,0	;(d0-d15)
	db	0,0,0,0		;(cr,r0,r1,r2)

;**************************
; Configuration file buffer
;**************************
CFGBUF:	ds	BLKSIZ

;****************************************************
;Command line buffer, used only during initialization
;****************************************************
ICOMBF:	ds	80

;********************************************
; Stack space used only during initialization
;********************************************
	ds	64		;plenty of room
ISTACK:
	END
