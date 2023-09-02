.include "cpm_crt0_lowstorage.inc"

;init:
	;; Bare minimum VTech hardware init
	;; Note: This depends on the model...
	;; GL6000SL
	
	;; Set up interrupt mode
	di
	im 1	; Only call 0x38
	;ei
	
	
	ld sp, #0xff87	; GL6000: ld sp, #0xff87 - leave just a little bit more
	ld a, #0
	ld i, a
	
	; LCD
	;ld a, #3
	;out (0x31), a
	
	; Bank switching (0x50...0x56, e.g. 0x50=0x0000-0x4000, 0x51=0x4000-0x8000, ...)
	;; e.g. OUT 0x51, 0x1B	-> maps ROM:0x6C000 to CPU:0x4000
	;; e.g. OUT 0x52, 0x20	-> maps CART:0x0000 to CPU:0x8000
	ld a, #0
	out (0x55), a
	
	out (0x50), a
	ld a, #1
	out (0x51), a
	
	;ld a, #2
	;out (0x52), a
	; Map cart to 0x8000 (found by trial and error)
	ld a, #0x0e
	out (0x55), a
	ld a, #0x20
	out (0x52), a
	
	ld a, #1
	out (0x53), a
	; 0x54 = 0xe000 = vram
	ld a, #0
	out (0x54), a
	
	; ?
	;ld a, #0
	;out (0x22), a
	;ld a, #0xe0
	;out (0x21), a
	;ld a, #0x60
	;out (0x23), a
	
	
	;	; Trial and error... Trying to find the port-combination to mount cart at 0x8000
	;	ld h, #0x80
	;	ld l, #0x00
	;	
	;	ld b, #0x00
	;	ld c, #0xff
	;loop_outer_start:
	;	inc c
	;	ld a, c
	;	;out(0x13), a
	;	out(0x55), a
	;loop_start:
	;	ld a, b
	;	out(0x13), a
	;	out(0x52), a
	;	
	;	; Check for cartridge header
	;	ld a, (hl)
	;	cp a, #0x55
	;	jr z, found
	;	inc b
	;	jr c, loop_outer_start
	;	jr loop_start
	;found:
	
.include "cpm_crt0_outro.inc"