;; crt0 for a Z80 based V-Tech Genius Leader / PreComputer
;; Based on SDCC's z80 crt0 and stuff I have implemented for z88dk's "vgl" target

;; SDCC asm syntax: https://github.com/darconeous/sdcc/blob/master/sdas/doc/asmlnk.txt
;; vgl info: /z/data/_code/_c/z88dk.git/libsrc/_DEVELOPMENT/target/vgl

	.module crt0
	.globl	_main	; main() function (in C)
	.area	_HEADER (ABS)
	

.org 0x0000
	;; Cartridge header start
	;.db	#0x55
	;.db	#0xaa

	;; Auto-start signature for ROM cartridges (if found: will start instantly on power-on)
	;.db	#0x59	; "Y"
	;.db	#0x45	; "E"

	;; Normal signature (i.e. non-autostart program cartridge)
	;.db #0x47 ; "G"
	;.db #0x41 ; "A"

	;; PreComputer1000 autostart signature
	;.db #0x33  ; 0x33 = autostart jump to 0x8010
	;.db #0x00  ; Dont care


;; First executed instructions (usually "jp 0xf200" on CP/M)
	di
	im 1
	jp init

;; Spare space to have some fun
;	.ascii	'prepare_crt0 2019-11-12 Bernhard "HotKey" Slawik'

;; Restart vectors for "im2"
	.org 0x0008	;; 0x0008 = Restart Vectors 1..6 (for interrupt mode "im 2")
;	jp	_inthandler
;	jp	_inthandler
;	jp	_inthandler
;	jp	_inthandler
;	jp	_inthandler
;	jp	_inthandler
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	
	;.org 0x0010
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	
	;.org 0x0020
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	
	;.org 0x0030
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38
	rst	0x38

;; Interrupt vector
	.org	0x0038	;; 0x38 = Restart Vector 7 (opcode 0xff = "rst 0x38"), also the "single interrupt handler" in "im 1"
	jp	_inthandler


;; Init - actual CRT init/life cycle procedure
;	.org	0x0040
init:
	;di ; disable interrupts
	;im 1	; GL original uses "im 1": only ONE single interrupt handler at 0x0038
	
	ld sp, #0xdff0	; GL4000 0eda: Load StackPointer to 0xdff0 (top of internal RAM)
	
	;call	gsinit	; Initialise global variables
	
	ld	a, #0xff	; GL4000 0edd: output 0xff to port 0x11
	out	(0x11),a
	
	ld	a, #0xde	; GL4000 0ee1: output 0xde to port 0x12 (default mode)
	;ld	a, #0xfe	; All on, except bit 0 which powers off the system
	;ld	a, #0xf6	; All on, except PowerOff and Beep
	out	(0x12),a	
	
	xor	a			; GL4000 0ee5: output 0x00 to all mappers (1, 2, 3, 4, 5)
	;out	(0x00),a	;	rombank0 (0x0000 - 0x3fff)
	out	(0x01),a	;	rombank1 (0x4000 - 0x7fff)
	out	(0x02),a	; ?
	;out	(0x03),a	; rombank2 (0x8000 - 0xffff): Sending "0x00" disables the cartridge port!
	out	(0x04),a	; ?
	out	(0x05),a	; ?
	
	ld	(0xdff1),a	; GL4000 0ef0: Put zeros above stack
	
	ld	a, #0x40	; GL4000 0f15: Output 0x40 to port 0x06 - dunno what this does
	out	(0x06),a
	
	;ei
	
	;; System is ready
	
	;; Call main() function (C entry point)
	jp	_main
	;; End of main()
	
	;; Jump to shutdown code
	;jp	_exit


;; Dummy interrupt handler
;	.org	0x7800
_inthandler:
	;push 	af
	;push 	bc
	;push 	de
	;push 	hl
	;push 	ix
	;push 	iy
	
	; acquire data
	
	
	; main sub-routine
	
	;pop 	iy
	;pop 	ix
	;pop 	hl
	;pop 	de
	;pop 	bc
	;pop 	af
	ei
	reti


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
	
_exit::
	;; Exit - special code to the emulator
	;ld	a,#0
	;rst	0x08
	
	;; V-Tech power down:
	;	ld	a, 1
	;	out	(0ah), a
	;	
	;	ld	a, 1
	;	out	(12h), a
	;	halt
	
1$:
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
