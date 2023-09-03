
	
	;ei
	;; Hardware is ready now
	
	;; Setup global static variables
	;call gsinit
	
	
	;; Call main code
	
	;call _main
	;jp _main	; VGLDK default entry point
	jp _bios_boot	; CP/M entry point
	;jp _exit
	
init_end:	; Store end of init
init_size .equ (init_end - init)	; Store how big the init code is
;.asciz '[init_end]'


.org 0x7fee
.asciz '[end_of_transient]'


;; ----------------------------------------
;; Also do the VTech cartridge auto-start header


;; Cartridge ROM space
.org 0x8000
cart_sig:

;; ROM signature
	;; cartridge ROM header
	.db #0x55
	.db #0xaa
	
	;; Normal signature (i.e. non-autostart program cartridge)
	;.db #0x47 ; "G"
	;.db #0x41 ; "A"
	
	;; Auto-start signatures (if present: program will start instantly on boot)
	;; Select one of these two:
	
	;; PreComputer1000 auto-start signature
	;.db #0x33  ; 0x33 = autostart jump to 0x8010
	;.db #0x00  ; Dont care
	
	;; GL2000/4000 auto-start signature
	.db #0x59	; "Y"
	.db #0x45	; "E"
	

cart_code4000:
	;; First executed instruction on GL2000/4000 (usually a jump)
	;di
	;im 1
	;jp	init	; Jump to CP/M bootstrap code
	jp _bios_boot	; CP/M entry point

.org 0x8010
cart_code1000:
	;; First executed instruction on PC1000 (usually a jump)
	;di
	;im 1
	;jp	init	; Jump to CP/M bootstrap code
	jp _bios_boot	; CP/M entry point

cart_end:
;.asciz '[cart_end]'


;; back to normal CRT0
;; ----------------------------------------

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
