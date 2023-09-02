; CP/M CRT0 file for VTech Genius Leader 4000
; 
; Memory layout of a running CP/M system
;
; 2023-08-15 Bernhard "HotKey" Slawik
;

;; Definitions
	.module cpm_crt0
	
	; Global C functions
	.globl _main	; cpm.c:main() = jumps to _bios_boot
	.globl _bint	; bint.c:bint()
	.globl _bios_boot	; bios.c:bios_boot() = initializes VGL hardware and continues to bios_wboot(), bdos_init() and finally ccp()
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
	jp _bdos	; Jump to first byte in _CODE segment (which should be the BDOS entry function)


;; IM2 vectors
.org 0x08
	;jp	_bint
	reti

.org 0x10
	;jp	_bint
	reti

.org 0x18
	;jp	_bint
	reti

.org 0x20
	;jp	_bint
	reti

.org 0x28
	;jp	_bint
	reti

.org 0x30
	;jp	_bint
	reti

;; IM1 vector
.org 0x38
	;jp	_bint
	reti

;; BIOS workarea
.org 0x40

;; Default FCB
.org 0x5c
;default_fcb:
	.asciz '[fcb]'

;; Workarea / DMA
.org 0x80
;bios_dma:
	.asciz '[bios_dma]'


;; CP/M transient area
.org 0x100
;transient_area:
	.asciz '[transient_area]'
	jp init


;; First run code
; Should stay in lower memory, so that a cold boot can be performed at all times
.org 0x40	; Fit it between vector table and working area (Must keep it short or it will interfere with default_fcb at 0x5c!)
;.org 0x150	; Fit it somewhere in the transient area
;.area _CODE	; Fit it in _CODE area
;.asciz '[init:]'	; Just a marker inside the binary
init:
	;; Bare minimum VTech hardware init
	;; Note: This depends on the model..