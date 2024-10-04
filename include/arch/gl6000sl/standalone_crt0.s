;; crt0 for a Z80 based V-Tech Genius Leader 6000SL / Prestige?
;; Based on SDCC's z80 crt0 and stuff I have implemented for z88dk's "vgl" target

;; SDCC asm syntax: https://github.com/darconeous/sdcc/blob/master/sdas/doc/asmlnk.txt
;; vgl info: /z/data/_code/_c/z88dk.git/libsrc/_DEVELOPMENT/target/vgl

	.module crt0
	;.globl	_main	; main() function (in C)
	.globl	_vgldk_init	; Will invoke main()
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
	di ; disable interrupts
	im 1	; GL original uses "im 1": only ONE single interrupt handler at 0x0038
	
	;call	gsinit	; Initialise global variables
	
	;; GL6000SL
	;ld sp, #0xff87	; GL6000: ld sp, #0xff87 - leave just a little bit more
	ld sp, #0xfffd	; GL6000: ld sp, #0xff87 - leave just a little bit more
	;ld a, #0
	;ld i, a
	
	; LCD
	;ld a, #3
	;out (0x31), a
	
	; Bank switching (0x50...0x56, e.g. 0x50=0x0000-0x4000, 0x51=0x4000-0x8000, ...)
	;	0x50 = 0x0000 - 0x3fff
	;	0x51 = 0x4000 - 0x7fff
	;	0x52 = 0x8000 - 0xbfff
	;	0x53 = 0xc000 - 0xdfff
	;	0x54 = 0xe000 - 0xffff
	;; e.g. OUT 0x51, 0x1B	-> maps ROM:0x6C000 to CPU:0x4000
	;; e.g. OUT 0x52, 0x20	-> maps CART:0x0000 to CPU:0x8000
	ld a, #0
	out (0x55), a
	out (0x50), a
	
	ld a, #1
	out (0x51), a
	
	ld a, #2
	out (0x52), a
	
	
	; Map cart to 0x8000 (found by trial and error)
	ld a, #0x0e
	out (0x55), a
	ld a, #0x20
	out (0x52), a
	
	
	; Next one is needed for VRAM to function properly (first scan lines)
	ld a, #1
	out (0x53), a
	
	; 0x54 = 0xe000 = vram
	ld a, #0
	out (0x54), a
	
;	; ?
;	;ld a, #0
;	;out (0x22), a
;	;ld a, #0xe0
;	;out (0x21), a
;	;ld a, #0x60
;	;out (0x23), a
	
	
	
	;ei
	
	;; System is ready
	
	;; Call main() function (C entry point)
	;jp	_main
	jp	_vgldk_init	; Will invoke main()
	
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
