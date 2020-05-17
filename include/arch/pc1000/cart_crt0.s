;; crt0 for a Z80 based VTech Genius LEADER / PreComputer

;; SDCC asm syntax: https://github.com/darconeous/sdcc/blob/master/sdas/doc/asmlnk.txt
;; vgl info: /z/data/_code/_c/z88dk.git/libsrc/_DEVELOPMENT/target/vgl

	.module crt0
	;.globl	_main
	.globl	_vgldk_init
	
	.area	_HEADER (ABS)
	
;; ROM start
	.org 0x8000	; cartridges need "0x8000" here, internal ROMs need "0x0000" here

;; ROM signature
	;; cartridge ROM header
	.db	#0x55
	.db	#0xaa
	
	;; PreComputer1000 auto-start signature
	.db #0x33  ; 0x33 = autostart jump to 0x8010
	.db #0x00  ; Dont care
	

;; PC1000 entry point at 0x8010 (fixed!)
.org 0x8010
	;; First executed instruction (usually a jump)
	;jp	init



init:
	di ; disable interrupts
	;ld a, #0xb0	; value at 0x8002
	
	; from ROM1000 0x021e:
	ld c, #0x0f
	ld b, #0x40
	out (c), a
	ld a, #0x02
	out (c), a
	ld hl, #0x4000
	xor a
	
	; from ROM1000 0x022e:
	_rom1000_022e:
	ld (hl), a
	inc hl
	bit 0x03, h
	jr z, _rom1000_022e
	
	;; Set stack pointer directly above top of memory.
	ld hl, #0x47ff
	ld sp, hl
	
	call	0x0290	; LCD init (sequence of outputs to ports 0x20, 0x21)
	call	0x0244
	
	;ld a, #0x04
	;ld (0x412e), a
	;call	0x0241
	
	
	; Show text at HL
	;ld	hl, #0x3e80
	;call	0x03ab
	
	
	;ld a, #0x40
	;call 0x048e	; Output A to LCD
	;ld a, #0x41
	;call 0x048e	; Output A to LCD
	;ld a, #0x80
	;call 0x048e	; Output A to LCD
	
	; Some internal resets (see ROM4000, 0x0eda)
	;ld a,#0xff		;0edd	3e ff 	> . 
	;out (0x11),a		;0edf	d3 11 	. . 
	;ld a,#0xde		;0ee1	3e de 	> . 
	;out (0x12),a		;0ee3	d3 12 	. . 
	;xor a			;0ee5	af 	. 
	;out (0x01),a		;0ee6	d3 01 	. . 
	;out (0x02),a		;0ee8	d3 02 	. . 
	;out (0x03),a		;0eea	d3 03 	. . 
	;out (0x04),a		;0eec	d3 04 	. . 
	;out (0x05),a		;0eee	d3 05 	. . 
	
	
	;; Initialise global variables
	;call	gsinit
	
	;; Initialise hardware
	;call vgl_sound_off
	;call vgl_lcd_init
	
	;;@TODO: Set the Z80 interrupt mode?
	;; "IM 2" may be used to hijack the hardware interrupts to our own code!
	
	;ei
	
	;; System is ready
	
	;; Call main() function (C entry point)
	;call	_main
	
	jp	_vgldk_init
	
	;; End of main()
	
	;; Jump to shutdown code
	;jp	_exit
	
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
	
;__clock::
;	;;@FIXME: No idea if this is applicable to V-Tech!
;	ld	a,#2
;	rst	0x08
;	ret

_exit::
	;call 0x0164 ; V-Tech 4000 Shutdown animation (German)
	
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


