;; crt0 for a Z80 based V-Tech Genius Leader / PreComputer
;; This is a blue print of the CP/M "Low Storage"

	.module crt0
	.globl	_bdos	; BDOS address
	;.globl	_bios	; BIOS address
	.globl	_bios_boot; BIOS cold boot address
	.globl	_bint; Interrupt handler

	.area	_HEADER (ABS)

;; CP/M Low Storage
	.org 0x0000
	
	;; 0x0000: First executed instruction (usually "jp 0xf200" on CP/M)
	;jp _bios	; We cannot jump to bios directly, since the cartridge ROM is not bank switched, yet
	jp init	; Firs, call a little helper to bank switch the cartridge ROM at 0x8000
	
	
	.org 0x0003	;; 0x0003 = IO Byte
	.db	#0x00
	
	.org 0x0004	;; 0x0004 = DSK Byte
	.db	#0x00
	
	.org 0x0005	;; 0x0005 = BDOS Call
	jp	_bdos
	
	.org 0x0008	;; 0x0008 = Restart Vectors 1..6 (for interrupt mode "im 2")
	jp	_bint
	jp	_bint
	jp	_bint
	jp	_bint
	jp	_bint
	jp	_bint
	
	.org	0x0038	;; 0x38 = Restart Vector 7 (opcode 0xff = "rst 0x38"), also the "single interrupt handler" in "im 1"
	jp	_bint
	
	.org	0x0040	;; 0x40 = BIOS Work Area
	;
	
	.org	0x0050	;; 0x50 = Unused (in CP/M 2.2)
	;
	
	.org	0x0060	;; 0x60 = FCB (Default File Control Block)
	;
	
	.org	0x0080	;; 0x80 = File Buffer
	;
	
	.org	0x0100	;; 0x100 Start of TPA (Transient Program Area)
	;
	

;; Little cold boot patch
;; Ensures the ROMs are ready to be booted
;; Optimally, keep everything between 0x038 (reset vector) and 0x05c (default FCB)
.org	0x003b
init:
	;; Initialise hardware
	di	; BIOS4000 0000
	im	1	; BIOS4000 0001
	
	ld	sp, #0xdff0	; BIOS4000 0eda: Load StackPointer to 0xdff0
	
	ld	a, #0xff	; BIOS4000 0edd
	out	(0x11),a
	
	ld	a, #0xde	; BIOS4000 0ee1
	out	(0x12),a
	
	xor	a			; BIOS4000 0ee5: sets 0x01, 0x02, 0x03, 0x04, 0x05 to 0x00
	;out	(0x00),a	; rombank0
	;out	(0x01),a	; rombank1
	out	(0x02),a	; Unknown
	;out	(0x03),a	; rombank2: This disables cartridge port!
	out	(0x04),a	; Unknown
	out	(0x05),a	; Unknown
	
	;ld	a, #0x40	; BIOS4000 0f15: dunno what this is used for
	;out	(0x06),a
	
	;call vgl_sound_off
	; Speaker off
	;ld	a, #0x00	; +20h
	;out	(0x12), a
	;
	;call vgl_lcd_init
	
	; We need to turn on the cartridge port in order to jump to BIOS
	ld	a, #0x80	; BIOS4000 0f3e: Turn on cartridge
	out	(0x03),a	; Memory region 0x03 = cartridge = 0x8000 - 0xbfff
	
	; Map second half of system ROM to 0x4000-0x7fff
	ld	a,	#0x01	; Offset inside ROM: 0x01 = 1 x 0x4000 = 0x4000
	out	(0x01),a	; Memory region 0x01 = rombank1 = 0x4000 - 0x7fff
	
	; Jump to BIOS
	;jp	_bios
	jp	_bios_boot
	

;; CP/M default FCB area
.org	0x005C

;; CP/M default DMA area
.org	0x0080

;; CP/M transient area
.org	0x0100

;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL
	
	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area   _BSS
	.area   _HEAP
	
	.area   _CODE
	
	
;1$:
;	halt
;	jr	1$

	.area   _GSINIT
gsinit::
;	ld	bc, #l__INITIALIZER
;	ld	a, b
;	or	a, c
;	jr	Z, gsinit_next
;	ld	de, #s__INITIALIZED
;	ld	hl, #s__INITIALIZER
;	ldir
gsinit_next:

;	.area   _GSFINAL
;	ret

