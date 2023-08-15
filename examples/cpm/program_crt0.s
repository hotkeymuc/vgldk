;; crt0 for a CP/M program
	.module crt0
	.globl	_main	; main() function (in C)
	.globl	_exit	; exit() function (in C)
	.area	_HEADER (ABS)
	

;.org 0x0000
;;; First executed instructions (usually "jp 0xf200" on CP/M)
;	;di
;	;im 1
;	jp init


;; Init - actual CRT init/life cycle procedure
.org	0x0100
init:
	;; Call main() function (C entry point)
	;jp	_main
	call _main
	
	;; Invoke BDOS shutdown function
	jp	_exit
	
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
	
;_exit::
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
