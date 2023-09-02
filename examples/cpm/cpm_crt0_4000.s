.include "cpm_crt0_lowstorage.inc"

;init:
	;; Bare minimum VTech hardware init
	;; Note: This depends on the model...
	;; GL4000
	
	;; Set up interrupt mode
	di
	im 1	; Only call 0x38
	;ei
	
	
	;; Bare minimum VTech hardware init
	;; Note: This depends on the model...
	;; GL4000
	
	ld sp, #0xdff0	; GL4000: ld sp, #0xdff0 - leave just a little bit more
	
	; Parallel port reset
	;ld	a, #0x00				; Set all D0-D7 to HIGH
	;out	(0x10), a
	
	;ld a, #0xff	; GL4000 0edd: output 0xff to port 0x11
	;out (0x11), a
	
	; Periphery reset (speaker etc.)
	;ld a, #0xde	; GL4000 0ee1: output 0xde to port 0x12 (default mode)
	;;ld a, #0xfe	; All on, except bit 0 which powers off the system
	;;ld a, #0xf6	; All on, except PowerOff and Beep
	;out (0x12), a	
	
	
	;; Prepare bank switching
	xor a
	out (0x00), a	; rombank0 (0x0000 - 0x3fff) to internal ROM 0x0000
	;out (0x01), a	; rombank1 (0x4000 - 0x7fff)
	;out (0x02), a	; ?
	;out (0x03), a	; rombank2 (0x8000 - 0xffff): Sending "0x00" would disable the cartridge port!
	;out (0x04), a	; ?
	;out (0x05), a	; ?
	
	ld a, #0x01	; Offset inside ROM: 0x01 = 1 x 0x4000 = 0x4000
	out (0x01), a	; rombank1 (0x4000 - 0x7fff) to internal ROM 0x4000
	
	ld a, #0x80	; BIOS4000 0f3e: Turn on cartridge
	out (0x03), a	; cartridge (0x8000 - 0xbfff) to cartridge ROM 0x0000
	
	;ld a, #0x40	; GL4000 0f15: Output 0x40 to port 0x06 - dunno what this does
	;out (0x06), a
	

.include "cpm_crt0_outro.inc"