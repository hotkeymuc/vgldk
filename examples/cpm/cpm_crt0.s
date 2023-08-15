; CP/M CRT0 file for VTech Genius Leader 4000
; 
; Memory layout of a running CP/M system
;
; 2023-08-15 Bernhard "HotKey" Slawik
;

;; Definitions
	.module cpm_crt0
	
	; Addresses
	stack .equ 0xdff0	; Stack top (GL4000 bios uses 0xdff0)
	
	; Global C functions
	.globl _main	; cpm.c:main()
	.globl _bint	; bint.c:bint()
	;.globl _bios_boot
	.globl _bdos	; bdos.c:bdos()
	

;; Start of memory layout
.area _HEADER (ABS)

;; Reset vector
.org 0
	jp init

;; Well-known bytes
;; 0x0003 = IO Byte
.org 0x0003
bios_io_byte:
	.db	#0x00
	
;; 0x0004 = DSK Byte
.org 0x0004
bios_curdsk:
	.db	#0x00
	
;; 0x0005 = BDOS Call
; Byte at 0x0006 (High byte of BDOS address) is used by user programs to determine free usable RAM
.org 0x0005
bdos_call:
	jp _bdos	; Jump to first byte in _CODE segment which does the "jp _bdos"


;; IM2 vectors
.org 0x08
	;reti
	jp	_bint

.org 0x10
	;reti
	jp	_bint

.org 0x18
	;reti
	jp	_bint

.org 0x20
	;reti
	jp	_bint

.org 0x28
	;reti
	jp	_bint

.org 0x30
	;reti
	jp	_bint

;; IM1 vector
.org 0x38
	;reti
	jp	_bint

;; BIOS workarea
.org 0x40

;; Default FCB
.org 0x5c
;default_fcb:

;; Workarea / DMA
.org 0x80
;bios_dma:
	.asciz '[bios_dma]'


;; CP/M transient area
.org 0x100
;transient_area:
	.asciz '[cpm_transient area]'
	jp init



;; First run code
; Should stay in lower memory, so that a cold boot can be performed at all times
;.org 0x150	; Fit it somewhere at the transient area
;.area _CODE
.org 0x40	; Fit it between vector table and working area
.asciz '[init:]'	; Just a marker inside the binary
init:
	
	;; Set up interrupt mode
	di
	im 1	; Only call 0x38
	
	
	;; Stack at the top of memory.
	ld sp, #stack	; GL4000: ld sp, #0xdff0 - leave just a little bit more
	
	
	;; Bare minimum VTech hardware init
	
	; Parallel port reset
	ld	a, #0x00				; Set all D0-D7 to HIGH
	out	(0x10), a
	
	ld a, #0xff	; GL4000 0edd: output 0xff to port 0x11
	out (0x11), a
	
	; Periphery reset (speaker etc.)
	ld a, #0xde	; GL4000 0ee1: output 0xde to port 0x12 (default mode)
	;ld a, #0xfe	; All on, except bit 0 which powers off the system
	;ld a, #0xf6	; All on, except PowerOff and Beep
	out (0x12), a	
	
	
	;; Prepare bank switching
	xor a
	out (0x00), a	; rombank0 (0x0000 - 0x3fff) to internal ROM 0x0000
	;out (0x01), a	; rombank1 (0x4000 - 0x7fff)
	out (0x02), a	; ?
	;out (0x03), a	; rombank2 (0x8000 - 0xffff): Sending "0x00" would disable the cartridge port!
	out (0x04), a	; ?
	out (0x05), a	; ?
	
	ld a, #0x01	; Offset inside ROM: 0x01 = 1 x 0x4000 = 0x4000
	out (0x01), a	; rombank1 (0x4000 - 0x7fff) to internal ROM 0x4000
	
	ld a, #0x80	; BIOS4000 0f3e: Turn on cartridge
	out (0x03), a	; cartridge (0x8000 - 0xbfff) to cartridge ROM 0x0000
	
	ld a, #0x40	; GL4000 0f15: Output 0x40 to port 0x06 - dunno what this does
	out (0x06), a
	
	
	;; Setup global static variables
	;call gsinit
	
	
	;; Call main code
	;call _main
	jp _main
	;jp _bios_boot
	;jp _exit
init_end:	; Store end of init
init_size .equ (init_end - init)	; Store how big the init code is
;.asciz '[init_end]'


;; Ordering of segments for the linker.
	.area _CODE
	.area _INITIALIZER
	.area _HOME
	
	.area _GSINIT
	.area _GSFINAL
	
	.area _DATA
	.area _INITIALIZED
	.area _BSEG
	.area _BSS
	.area _HEAP
	
	; Anything declared after this is relatively addressed



;; Clock (where is this used?)
;.area _CODE
;__clock::
;	ld a, #2
;	rst 0x08
;	ret


;; Exit
;_exit:
;	; Send special code to the emulator
;	ld a, #0
;	rst 0x08
;1$:
;	halt
;	jr 1$


;; Global state initializer
.area   _GSINIT
gsinit:
	;ld bc, #l__INITIALIZER
	;ld a, b
	;or a, c
	;jr Z, gsinit_next
	;ld de, #s__INITIALIZED
	;ld hl, #s__INITIALIZER
	;ldir
;gsinit_next:
;
.area _GSFINAL
	;ret

