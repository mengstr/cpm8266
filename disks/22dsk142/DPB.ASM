;*	Program to display DPB information on CP/M 2.2
;
;	For those systems without MS BASIC
;

bdos	equ	0005h		; BDOS call vector
wboot	equ	0000h		; Warm boot vector
cdisk	equ	0004h		; current disk
tpa	equ	0100h		; TPA start
cr	equ	13		; return
lf	equ	10		; line feed
bel	equ	7		; bell

	org	tpa

GetDPB:
	call	Strout
	db	cr,lf,'       Parameters for Drive ',0

	lda	cdisk
	ani	15
	adi	'A'
	call	chout		; give a drive letter

	call	Strout
	db	':',cr,lf,lf,0  ; skip a line

	lda	cdisk
	mov	c,a
	lhld	wboot+1
	mvi	l,27		; point to seldisk
	call	Indcall		; go call (seldisk)

	mov	a,h
	ora	l
	jnz	GetDPB2		; if okay
	call	Strout
	db	cr,lf,bel,'Select error',cr,lf,0
	jmp	wboot		; exit...

GetDPB2:
	shld	DPHaddr		; save DPH address
	lxi	d,10
	dad	d
	mov	e,m
	inx	h
	mov	d,m		; get DPB address
	xchg
	shld	DPBaddr		; save DPB address

;	Show sectors/track.

	call	Strout
	db	'Sectors per Track............',0
	mvi	a,0
	call	GetField	; get a field from the DPB
	shld	SectTrk		; save it
	call	ShowDec		; show it

;	If there's a translate table, show it.

	lhld	DPHaddr
	mov	e,m
	inx	h
	mov	d,m
	mov	a,d
	ora	e
	jz	GetDPB6		; if none..
	call	Strout
	db	cr,lf,'Sectors interleaved as follows:',0

	lhld	SectTrk		; get Sectors/track
	mov	b,h
	mov	c,l
GetDPB4:
	call	Strout
	db	cr,lf,'    ',0  ; indent
	ldax	d		; get a sector
	push	h
	mov	l,a
	mvi	h,0
	call	ShowDec		; show it
	pop	h
	dcx	b
	mov	a,b
	ora	c
	jnz	GetDPB4		; loop...
	jmp	GetDPB8		; do the rest...

GetDPB6:
	call	Strout
	db	cr,lf,'Sectors are not interleaved',0

;	Show the rest of the fields.

GetDPB8:
	call	Strout
	db	cr,lf,'Block Shift (BSH)............',0
	mvi	a,2
	call	GetField
	mvi	h,0
	call	ShowDec
	call	Strout
	db	cr,lf,'Block Mask (BLM).............',0
	mvi	a,3
	call	GetField
	mvi	h,0
	call	ShowDec
	call	Strout
	db	cr,lf,'Extent Mask (EXM)............',0
	mvi	a,4
	call	GetField
	mvi	h,0
	call	ShowDec
	call	Strout
	db	cr,lf,'Total Blocks (DSM)...........',0
	mvi	a,5
	call	GetField
	call	ShowDec
	call	Strout
	db	cr,lf,'Directory Entries (DRM)......',0
	mvi	a,7
	call	GetField
	call	ShowDec
	call	Strout
	db	cr,lf,'Allocation 0 (AL0)...........',0
	mvi	a,9
	call	GetField
	mvi	h,0
	call	ShowHex
	call	Strout
	db	cr,lf,'Allocation 1 (AL1)...........',0
	mvi	a,10
	call	GetField
	mvi	h,0
	call	ShowHex
	call	Strout
	db	cr,lf,'Cylinder Offset (OFS)........',0
	mvi	a,13
	call	GetField
	call	ShowDec
	call	Strout
	db	cr,lf,lf,0		; done...
	jmp	wboot			; exit...

;*	Indirect call to (hl).
;
;	Simple, yet effective.
;

Indcall:
	pchl			; jmp to (hl)

;*	GetField - Get a DPB field into (hl)
;
;	(a) = offset on entry.
;

GetField:
	push	d
	lhld	DPBaddr
	mov	e,a
	mvi	d,0
	dad	d
	mov	e,m
	inx	h
	mov	d,m
	xchg
	pop	d
	ret


;*	Chout - Character in (a) to console output.
;
;	Preserves all but (a)
;

Chout:
	push	h
	push	b
	push	d
	mov	e,a
	mvi	c,2
	call	bdos			; issue a console-out
	pop	d
	pop	b
	pop	h
	ret

;*	Strout - Output a string following the call to this routine.
;
;	preserves all.
;
;	String is null-terminated.
;

Strout:
	xthl				; get return in (hl)
	push	psw			; save (a)
Strout2:
	mov	a,m
	inx	h
	ora	a
	jz	Strout4			; if done...
	push	d
	push	b
	push	h
	mov	e,a
	mvi	c,2			; console out
	call	bdos
	pop	h
	pop	b
	pop	d
	jmp	Strout2

Strout4:
	pop	psw
	xthl
	ret				; exit...all regs preserved

;*	ShowDec - Show (HL) as a decimal number.
;
;	Output to the console; blank leading zeroes.
;

ShowDec:
	push	h
	push	d
	push	b
	push	psw
	lxi	d,DecTable	; get powers table
	xchg
	mvi	c,0		; say blanking
ShowDec2:
	mov	a,m
	ora	a		; see if last digit
	jz	ShowDec8	; if so
	mvi	b,0
ShowDec4:
	inr	b
	mov	a,e
	sub	m
	mov	e,a
	inx	h
	mov	a,d
	sbb	m
	mov	d,a
	dcx	h
	jnc	ShowDec4	; if oversubtract

;	correct oversubtract.

	mov	a,e
	add	m
	mov	e,a
	inx	h
	mov	a,d
	adc	m
	inx	h
	mov	d,a
	dcr	b
	jz	ShowDec6	; if a zero
	mvi	c,1
ShowDec6:
	inr	c
	dcr	c		; check (c)
	jz	ShowDec2	; loop to next digit
	mov	a,b
	adi	'0'
	call	chout		; character out
	jmp	ShowDec2	; to next digit

;	Do the last digit - do not blank.

ShowDec8:
	mov	a,e
	adi	'0'
	call	chout
	pop	psw
	pop	b
	pop	d
	pop	h
	ret

DecTable:
	dw	10000,1000,100,10,0	; powers of 10


;*	ShowHex - Show a string of hex digits.
;
;	On entry (HL) = value.
;
;	Output is to console, leading zeroes blanked.
;

ShowHex:
	push	h
	push	d
	push	b
	push	psw
	lxi	b,0400h			; 4 digits + blank indicator
ShowHex2:
	xra	a
	dad	h
	ral
	dad	h
	ral
	dad	h
	ral
	dad	h
	ral				; isolate a digit in (a)
	ora	a
	jz	ShowHex4		; if zero
	mvi	c,1			; say nonzero
ShowHex4:
	adi	'0'                     ; bias for ASCII
	cpi	'9'+1
	jc	ShowHex6		; if 0-9
	adi	'A'-'9'-1               ; make digits
ShowHex6:
	dcr	b
	jz	ShowHex8		; if last digit
	inr	c
	dcr	c
	jz	ShowHex10		; skip the leading zero
ShowHex8:
	call	chout
ShowHex10:
	inr	b
	dcr	b
	jnz	ShowHex2		; if not done yet
	pop	psw
	pop	b
	pop	d
	pop	h
	ret				; exit...

DPBaddr dw	0		; address of DPB
DPHaddr dw	0		; address of DPH
SectTrk dw	0		; sectors/track

	end	GetDPB
