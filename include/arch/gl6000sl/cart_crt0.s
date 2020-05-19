;; crt0 for a Z80 based VTech Genius LEADER / PreComputer

;; SDCC asm syntax: https://github.com/darconeous/sdcc/blob/master/sdas/doc/asmlnk.txt
;; vgl info: /z/data/_code/_c/z88dk.git/libsrc/_DEVELOPMENT/target/vgl

	.module crt0
	.globl	_main
	.globl	_vgldk_init
	
	.area	_HEADER (ABS)
	
;; Zero page ROM start
	.org 0x8000	; cartridges need "0x8000" here, internal ROMs need "0x0000" here

;; ROM signature
	;; cartridge ROM header
	.db	#0x55
	.db	#0xaa
	
	;; Auto-start signature for ROM cartridges (if found: will start instantly on power-on)
	;GL6000SL: Check code is at ROM 0x2737
	.db	#0x59	; "Y"	GL6000SL: 0x11->2792, 0xee->27a8, 0x47->2779, 0x42->27a8, 0x59->2784, 0x47->27a6
	.db	#0x45	; "E"	GL6000SL: 0x45->27a8, 0x41->27a8 -- else 27a6
	
	;2792: out(0x52),0x24, check 0x8000 for 0x88, set b accordingly -> 27a8
	;
	;
	
	;; Normal signature (i.e. non-autostart program cartridge)
	;.db	#0x47 ; "G"
	;.db	#0x41 ; "A"

	;; PreComputer1000 autostart
	;.db #0x33  ; 0x33 = autostart jump to 0x8010
	;.db #0x00  ; Dont care
	
	;; First executed instruction (usually a jump)
	; Jump to actual code
	
	;di
	;im 1
	
	jp	init
	


init:
	;di ; disable interrupts
	
	;; Set stack pointer directly above top of memory.
	;ld	sp, #0xdff0	; Load StackPointer to 0xdff0
	
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


