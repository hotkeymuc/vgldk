;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 4.0.0 #11528 (Linux)
;--------------------------------------------------------
	.module cpm
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _bios_memset
	.globl _gets
	.globl _printf_d
	.globl _printf
	.globl _puts
	.globl _vgldk_init
	.globl _beep
	.globl _sound_note
	.globl _vgl_sound_tone
	.globl _vgl_sound_off
	.globl _keyboard_getchar
	.globl _keyboard_checkkey
	.globl _keyboard_inkey
	.globl _lcd_putchar
	.globl _lcd_putchar_at
	.globl _lcd_scroll
	.globl _lcd_refresh
	.globl _lcd_init
	.globl _lcd_clear
	.globl _lcd_set_cursor
	.globl _lcd_writeData
	.globl _lcd_writeControl
	.globl _lcd_delay_short
	.globl _lcd_delay_long
	.globl _keyboard_getchar_last
	.globl _lcd_cursor
	.globl _lcd_y
	.globl _lcd_x
	.globl _lcd_scroll_cb
	.globl _lcd_buffer
	.globl _bint_timer
	.globl _bios_dummy_dph
	.globl _bios_sec
	.globl _bios_trk
	.globl _bios_dma
	.globl _bios_curdsk
	.globl _bios_iobyte
	.globl _bdos_ret_b
	.globl _bdos_ret_a
	.globl _bdos_param_e
	.globl _bdos_param_d
	.globl _bdos_param_c
	.globl _bdos_fcb
	.globl _bdos_user
	.globl _bdos_delimiter
	.globl _vgl_key_map
	.globl _lcd_map_4rows
	.globl _bdos
	.globl _bdos_putchar
	.globl _bdos_getchar
	.globl _bdos_puts
	.globl _bdos_gets
	.globl _bdos_printf
	.globl _bdos_printf_d
	.globl _bdos_strlen
	.globl _bdos_memset
	.globl _bdos_f_open
	.globl _bdos_f_close
	.globl _bdos_f_read
	.globl _bdos_f_readrand
	.globl _bdos_f_write
	.globl _bdos_f_writerand
	.globl _bdos_f_sfirst
	.globl _bdos_f_snext
	.globl _bdos_init
	.globl _bios_boot
	.globl _bios_wboot
	.globl _bios_const
	.globl _bios_conin
	.globl _bios_conout
	.globl _bios_list
	.globl _bios_punch
	.globl _bios_reader
	.globl _bios_home
	.globl _bios_seldsk
	.globl _bios_settrk
	.globl _bios_setsec
	.globl _bios_setdma
	.globl _bios_read
	.globl _bios_write
	.globl _bios_listst
	.globl _bios_sectran
	.globl _bios
	.globl _bint
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
_lcd_controlPort	=	0x000a
_lcd_dataPort	=	0x000b
_keyboard_port_matrixRowOut	=	0x0010
_keyboard_port_matrixColIn	=	0x0010
_keyboard_port_matrixColIn2	=	0x0011
_keyboard_port_matrixLatch	=	0x0011
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_bdos_delimiter::
	.ds 1
_bdos_user::
	.ds 1
_bdos_fcb	=	0x005c
_bdos_param_c::
	.ds 1
_bdos_param_d::
	.ds 1
_bdos_param_e::
	.ds 1
_bdos_ret_a::
	.ds 1
_bdos_ret_b::
	.ds 1
_bios_iobyte	=	0x0003
_bios_curdsk	=	0x0004
_bios_dma::
	.ds 2
_bios_trk::
	.ds 2
_bios_sec::
	.ds 2
_bios_dummy_dph::
	.ds 1
_bint_timer::
	.ds 2
_lcd_buffer::
	.ds 80
_lcd_scroll_cb::
	.ds 2
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
_lcd_x::
	.ds 1
_lcd_y::
	.ds 1
_lcd_cursor::
	.ds 1
_keyboard_getchar_last::
	.ds 1
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;./bdos.c:25: void bdos() __naked {
;	---------------------------------
; Function bdos
; ---------------------------------
_bdos::
;./bdos.c:118: __endasm;
;.asciz	'BDOS!' ; Mark in binary, so we can check if it is REALLY the first bytes in _CODE area
;	Make registers available in C context
	push	af
;	Store BDOS function number "C"
	ld	a, c
	ld	(_bdos_param_c), a
;	Store 8-bit argument "D"
	ld	a, d
	ld	(_bdos_param_d), a
;	Store 8-bit argument "E"
	ld	a, e
	ld	(_bdos_param_e), a
	pop	af
;./bdos.c:131: bdos_func = bdos_param_c;
	ld	a,(#_bdos_param_c + 0)
	ld	-3 (ix), a
;./bdos.c:132: bdos_param_de = (word)bdos_param_d * 256 + (word)bdos_param_e;
	ld	hl,#_bdos_param_d + 0
	ld	b, (hl)
	ld	c, #0x00
	ld	iy, #_bdos_param_e
	ld	l, 0 (iy)
	ld	h, #0x00
	add	hl, bc
	ld	-2 (ix), l
	ld	-1 (ix), h
;./bdos.c:140: switch(bdos_func) {
	ld	a, #0x28
	sub	a, -3 (ix)
	jp	C, 00151$
	ld	c, -3 (ix)
	ld	b, #0x00
	ld	hl, #00201$
	add	hl, bc
	add	hl, bc
	add	hl, bc
	jp	(hl)
00201$:
	jp	_bdos_init
	jp	00102$
	jp	00103$
	jp	00104$
	jp	00105$
	jp	00106$
	jp	00107$
	jp	00117$
	jp	00118$
	jp	00119$
	jp	00123$
	jp	00124$
	jp	00125$
	jp	00152$
	jp	00127$
	jp	00128$
	jp	00129$
	jp	00130$
	jp	00131$
	jp	00132$
	jp	00133$
	jp	00134$
	jp	00135$
	jp	00136$
	jp	00137$
	jp	00138$
	jp	00139$
	jp	00152$
	jp	00151$
	jp	00151$
	jp	00151$
	jp	00151$
	jp	00141$
	jp	00145$
	jp	00146$
	jp	00147$
	jp	00148$
	jp	00152$
	jp	00151$
	jp	00151$
	jp	00150$
;./bdos.c:142: case BDOS_FUNC_P_TERMCPM:	// 0: System reset
;./bdos.c:160: __endasm;
	jp	_bdos_init
;./bdos.c:161: break;
	ret
;./bdos.c:163: case BDOS_FUNC_C_READ:	// 1: Console input
00102$:
;./bdos.c:164: s = bios_conin();	// Get a char from bios
	call	_bios_conin
	ld	a, l
	ld	b, a
;./bdos.c:165: bios_conout(s);	// Echo it
	push	bc
	push	bc
	inc	sp
	call	_bios_conout
	inc	sp
	pop	bc
;./bdos.c:166: bdos_return1(s);	// Return it
	ld	iy, #_bdos_ret_a
	ld	0 (iy), b
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:167: break;
	ret
;./bdos.c:169: case BDOS_FUNC_C_WRITE:	// 2: Console output
00103$:
;./bdos.c:170: bios_conout(bdos_param_e);
	ld	a, (_bdos_param_e)
	push	af
	inc	sp
	call	_bios_conout
	inc	sp
;./bdos.c:171: break;
	ret
;./bdos.c:173: case BDOS_FUNC_A_READ:	// 3: Reader input
00104$:
;./bdos.c:174: s = bios_reader();	// Get a char from bios
	call	_bios_reader
	ld	a, l
	ld	(_bdos_ret_a+0), a
;./bdos.c:177: bdos_return1(s);	// Return it
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:178: break;
	ret
;./bdos.c:180: case BDOS_FUNC_A_WRITE:	// 4: Punch output
00105$:
;./bdos.c:181: bios_punch(bdos_param_e);
	ld	a, (_bdos_param_e)
	push	af
	inc	sp
	call	_bios_punch
	inc	sp
;./bdos.c:182: break;
	ret
;./bdos.c:184: case BDOS_FUNC_L_WRITE:	// 5: List output
00106$:
;./bdos.c:185: bios_list(bdos_param_e);
	ld	a, (_bdos_param_e)
	push	af
	inc	sp
	call	_bios_list
	inc	sp
;./bdos.c:186: break;
	ret
;./bdos.c:188: case BDOS_FUNC_C_RAWIO:	// 6: Direct console I/O
00107$:
;./bdos.c:190: switch(bdos_param_e) {
	ld	a,(#_bdos_param_e + 0)
	cp	a, #0xfc
	jr	Z,00114$
	cp	a, #0xfd
	jr	Z,00113$
	cp	a, #0xfe
	jr	Z,00112$
	inc	a
	jr	NZ,00115$
;./bdos.c:195: c = bios_const();
	call	_bios_const
	ld	c, l
;./bdos.c:196: if (c == 0xff) {	// Key is pressed
	ld	a, c
	inc	a
	jr	NZ,00110$
;./bdos.c:197: c = bios_conin();	// Get that key
	call	_bios_conin
	ld	a, l
	ld	c, a
	jr	00111$
00110$:
;./bdos.c:201: bdos_return1(0);	// No new key
	ld	iy, #_bdos_ret_a
	ld	0 (iy), #0x00
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
00111$:
;./bdos.c:206: bdos_return1(c);
	ld	iy, #_bdos_ret_a
	ld	0 (iy), c
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:207: break;
	ret
;./bdos.c:209: case 0xfe:
00112$:
;./bdos.c:212: bdos_return1(bios_const());
	call	_bios_const
	ld	a, l
	ld	(_bdos_ret_a+0), a
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:213: break;
	ret
;./bdos.c:214: case 0xfd:
00113$:
;./bdos.c:217: bdos_return1(bios_conin());	// Do NOT echo!
	call	_bios_conin
	ld	a, l
	ld	(_bdos_ret_a+0), a
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:218: case 0xfc:
00114$:
;./bdos.c:220: bdos_return1(0);
	ld	iy, #_bdos_ret_a
	ld	0 (iy), #0x00
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:221: default:
00115$:
;./bdos.c:222: bios_conout(bdos_param_e);
	ld	a, (_bdos_param_e)
	push	af
	inc	sp
	call	_bios_conout
	inc	sp
;./bdos.c:224: break;
	ret
;./bdos.c:226: case BDOS_FUNC_GET_IOBYTE:	// 7: Get I/O Byte
00117$:
;./bdos.c:227: bdos_return1(bios_iobyte);
	ld	a,(#_bios_iobyte + 0)
	ld	iy, #_bdos_ret_a
	ld	0 (iy), a
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:228: break;
	ret
;./bdos.c:230: case BDOS_FUNC_SET_IOBYTE:	// 8: Set I/O Byte
00118$:
;./bdos.c:231: bios_iobyte = bdos_param_e;
	ld	a,(#_bdos_param_e + 0)
	ld	(#_bios_iobyte + 0),a
;./bdos.c:232: break;
	ret
;./bdos.c:234: case BDOS_FUNC_C_WRITESTR:	// 9: Print string (until delimiter "$")
00119$:
;./bdos.c:235: pc = (char *)bdos_param_de;
	ld	c, -2 (ix)
	ld	b, -1 (ix)
;./bdos.c:236: while(*pc != bdos_delimiter) {
00120$:
	ld	a, (bc)
	ld	d, a
	ld	a,(#_bdos_delimiter + 0)
	sub	a, d
	ret	Z
;./bdos.c:237: bios_conout(*pc++);
	inc	bc
	push	bc
	push	de
	inc	sp
	call	_bios_conout
	inc	sp
	pop	bc
	jr	00120$
;./bdos.c:241: case BDOS_FUNC_C_READSTR:	// 10: Read console buffer
00123$:
;./bdos.c:242: pc = (char *)bdos_param_de;
	ld	c, -2 (ix)
	ld	b, -1 (ix)
;./bdos.c:244: bdos_gets(pc);
	push	bc
	push	bc
	call	_bdos_gets
	pop	af
	call	_bdos_strlen
	pop	af
;./bdos.c:248: pc = (char *)(bdos_param_de + s);
	ld	h, #0x00
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	add	hl, bc
;./bdos.c:249: *pc = bdos_delimiter;
	ld	a,(#_bdos_delimiter + 0)
	ld	(hl), a
;./bdos.c:250: break;
	ret
;./bdos.c:252: case BDOS_FUNC_C_STAT:	// 11: Get console status
00124$:
;./bdos.c:255: bdos_return1(bios_const());
	call	_bios_const
	ld	a, l
	ld	(_bdos_ret_a+0), a
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:256: break;
	ret
;./bdos.c:258: case BDOS_FUNC_S_BDOSVER:	// 12: Return version number
00125$:
;./bdos.c:262: bdos_return2(0, 0x22);
	ld	iy, #_bdos_ret_a
	ld	0 (iy), #0x00
	ld	iy, #_bdos_ret_b
	ld	0 (iy), #0x22
	ld a, (_bdos_ret_b) 
	ld b, a 
	ld h, a 
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:263: break;
	ret
;./bdos.c:270: case BDOS_FUNC_DRV_SET:	// 14: Select disk
00127$:
;./bdos.c:274: bios_seldsk(bdos_param_e);
	ld	a, (_bdos_param_e)
	push	af
	inc	sp
	call	_bios_seldsk
	inc	sp
;./bdos.c:276: bdos_return1(0);	// 0 = OK, 0xff = error
	ld	iy, #_bdos_ret_a
	ld	0 (iy), #0x00
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:277: break;
	ret
;./bdos.c:279: case BDOS_FUNC_F_OPEN:	// 15: Open file
00128$:
;./bdos.c:287: s = bdos_f_open((struct FCB *)bdos_param_de);
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	push	bc
	call	_bdos_f_open
	pop	af
	ld	a, l
	ld	(_bdos_ret_a+0), a
;./bdos.c:289: bdos_return2(s, 0);
	ld	iy, #_bdos_ret_b
	ld	0 (iy), #0x00
	ld a, (_bdos_ret_b) 
	ld b, a 
	ld h, a 
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:290: break;
	ret
;./bdos.c:292: case BDOS_FUNC_F_CLOSE:	// 16: Close file
00129$:
;./bdos.c:297: s = bdos_f_close((struct FCB *)bdos_param_de);
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	push	bc
	call	_bdos_f_close
	pop	af
	ld	a, l
	ld	(_bdos_ret_a+0), a
;./bdos.c:298: bdos_return2(s, 0);
	ld	iy, #_bdos_ret_b
	ld	0 (iy), #0x00
	ld a, (_bdos_ret_b) 
	ld b, a 
	ld h, a 
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:299: break;
	ret
;./bdos.c:301: case BDOS_FUNC_F_SFIRST:	// 17: Search for first
00130$:
;./bdos.c:305: s = bdos_f_sfirst((struct FCB *)bdos_param_de);
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	push	bc
	call	_bdos_f_sfirst
	pop	af
	ld	a, l
	ld	(_bdos_ret_a+0), a
;./bdos.c:307: bdos_return2(s, 0);
	ld	iy, #_bdos_ret_b
	ld	0 (iy), #0x00
	ld a, (_bdos_ret_b) 
	ld b, a 
	ld h, a 
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:308: break;
	ret
;./bdos.c:310: case BDOS_FUNC_F_SNEXT:	// 18: Search for next
00131$:
;./bdos.c:312: s = bdos_f_snext((struct FCB *)bdos_param_de);
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	push	bc
	call	_bdos_f_snext
	pop	af
	ld	a, l
	ld	(_bdos_ret_a+0), a
;./bdos.c:313: bdos_return1(s);
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:314: break;
	ret
;./bdos.c:316: case BDOS_FUNC_F_DELETE:	// 19: Delete file
00132$:
;./bdos.c:321: bdos_return1(0xff);
	ld	iy, #_bdos_ret_a
	ld	0 (iy), #0xff
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:322: break;
	ret
;./bdos.c:324: case BDOS_FUNC_F_READ:	// 20: Read sequential
00133$:
;./bdos.c:334: s = bdos_f_read((struct FCB *)bdos_param_de);
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	push	bc
	call	_bdos_f_read
	pop	af
	ld	a, l
	ld	(_bdos_ret_a+0), a
;./bdos.c:335: bdos_return2(s, 0);
	ld	iy, #_bdos_ret_b
	ld	0 (iy), #0x00
	ld a, (_bdos_ret_b) 
	ld b, a 
	ld h, a 
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:336: break;
	ret
;./bdos.c:338: case BDOS_FUNC_F_WRITE:	// 21: Write sequential
00134$:
;./bdos.c:350: bdos_return1(bdos_f_write((struct FCB *)bdos_param_de));
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	push	bc
	call	_bdos_f_write
	pop	af
	ld	a, l
	ld	(_bdos_ret_a+0), a
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:351: break;
	ret
;./bdos.c:353: case BDOS_FUNC_F_MAKE:	// 22: Make file
00135$:
;./bdos.c:357: bdos_return1(0xff);	// 0xff if directory is full
	ld	iy, #_bdos_ret_a
	ld	0 (iy), #0xff
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:358: break;
	ret
;./bdos.c:360: case BDOS_FUNC_F_RENAME:	// 23: Rename file
00136$:
;./bdos.c:364: bdos_return1(0xff);	// Returns A=0-3 if successful; A=0FFh if error. Under CP/M 3, if H is zero then the file could not be found
	ld	iy, #_bdos_ret_a
	ld	0 (iy), #0xff
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:365: break;
	ret
;./bdos.c:367: case BDOS_FUNC_DRV_LOGINVEC:	// 24: Return login vector
00137$:
;./bdos.c:369: bdos_return2(0x00, 0x01);	// 0x00 0x01 = Only drive A
	ld	iy, #_bdos_ret_a
	ld	0 (iy), #0x00
	ld	iy, #_bdos_ret_b
	ld	0 (iy), #0x01
	ld a, (_bdos_ret_b) 
	ld b, a 
	ld h, a 
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:370: break;
	ret
;./bdos.c:372: case BDOS_FUNC_DRV_GET:	// 25: Return current disk
00138$:
;./bdos.c:374: bdos_return1(bios_curdsk);
	ld	a,(#_bios_curdsk + 0)
	ld	iy, #_bdos_ret_a
	ld	0 (iy), a
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:375: break;
	ret
;./bdos.c:377: case BDOS_FUNC_F_DMAOFF:	// 26: Set DMA address
00139$:
;./bdos.c:381: bios_setdma((byte *)bdos_param_de);
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	push	bc
	call	_bios_setdma
	pop	af
;./bdos.c:382: break;
	ret
;./bdos.c:401: case BDOS_FUNC_F_USERNUM:	// 32: Set/Get user code
00141$:
;./bdos.c:410: if (bdos_param_e == 0xff) {
	ld	a,(#_bdos_param_e + 0)
	inc	a
	jr	NZ,00143$
;./bdos.c:411: bdos_return1(bdos_user);
	ld	a,(#_bdos_user + 0)
	ld	iy, #_bdos_ret_a
	ld	0 (iy), a
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
	ret
00143$:
;./bdos.c:413: bdos_user = bdos_param_e;
	ld	a,(#_bdos_param_e + 0)
	ld	iy, #_bdos_user
	ld	0 (iy), a
;./bdos.c:414: bdos_return1(bdos_user);
	ld	a, 0 (iy)
	ld	iy, #_bdos_ret_a
	ld	0 (iy), a
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:416: break;
	ret
;./bdos.c:418: case BDOS_FUNC_F_READRAND:	// 33: Read random
00145$:
;./bdos.c:419: s = bdos_f_readrand((struct FCB *)bdos_param_de);
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	push	bc
	call	_bdos_f_readrand
	pop	af
	ld	a, l
	ld	(_bdos_ret_a+0), a
;./bdos.c:420: bdos_return2(s, 0);
	ld	iy, #_bdos_ret_b
	ld	0 (iy), #0x00
	ld a, (_bdos_ret_b) 
	ld b, a 
	ld h, a 
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:421: break;
	ret
;./bdos.c:423: case BDOS_FUNC_F_WRITERAND:	// 34: Write random
00146$:
;./bdos.c:425: s = bdos_f_writerand((struct FCB *)bdos_param_de);
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	push	bc
	call	_bdos_f_writerand
	pop	af
	ld	a, l
	ld	(_bdos_ret_a+0), a
;./bdos.c:426: bdos_return2(s, 0);
	ld	iy, #_bdos_ret_b
	ld	0 (iy), #0x00
	ld a, (_bdos_ret_b) 
	ld b, a 
	ld h, a 
	ld a, (_bdos_ret_a) 
	ld l, a 
	ret
;./bdos.c:427: break;
	ret
;./bdos.c:429: case BDOS_FUNC_F_SIZE:	// 35: Compute file size
00147$:
;./bdos.c:439: bdos_puts("SIZE");
	ld	hl, #___str_0
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:440: break;
	ret
;./bdos.c:442: case BDOS_FUNC_F_RANDREC:	// 36: Set random record
00148$:
;./bdos.c:444: bdos_puts("RANDREC");
	ld	hl, #___str_1
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:453: break;
	ret
;./bdos.c:462: case BDOS_FUNC_F_WRITEZF:	// 40: Fill random file w/ zeros
00150$:
;./bdos.c:463: bdos_puts("WRITEZF");
	ld	hl, #___str_2
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:465: break;
	ret
;./bdos.c:478: default:
00151$:
;./bdos.c:479: bdos_printf_d("BDOS#", bdos_func);
	ld	a, -3 (ix)
	push	af
	inc	sp
	ld	hl, #___str_3
	push	hl
	call	_bdos_printf_d
	pop	af
	inc	sp
;./bdos.c:480: bdos_getchar();
	call	_bdos_getchar
;./bdos.c:481: }
00152$:
;./bdos.c:485: __endasm;
	ret
;./bdos.c:486: }
;./bdos.c:495: void bdos_putchar(char c) {
;	---------------------------------
; Function bdos_putchar
; ---------------------------------
_bdos_putchar::
;./bdos.c:496: bios_conout(c);
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	call	_bios_conout
	inc	sp
;./bdos.c:497: }
	ret
___str_0:
	.ascii "SIZE"
	.db 0x00
___str_1:
	.ascii "RANDREC"
	.db 0x00
___str_2:
	.ascii "WRITEZF"
	.db 0x00
___str_3:
	.ascii "BDOS#"
	.db 0x00
;./bdos.c:498: byte bdos_getchar() {
;	---------------------------------
; Function bdos_getchar
; ---------------------------------
_bdos_getchar::
;./bdos.c:499: return bios_conin();
;./bdos.c:500: }
	jp	_bios_conin
;./bdos.c:502: void bdos_puts(const char *str) {
;	---------------------------------
; Function bdos_puts
; ---------------------------------
_bdos_puts::
;./bdos.c:503: while(*str) bios_conout(*str++);
	pop	de
	pop	bc
	push	bc
	push	de
00101$:
	ld	a, (bc)
	or	a, a
	ret	Z
	inc	bc
	push	bc
	push	af
	inc	sp
	call	_bios_conout
	inc	sp
	pop	bc
;./bdos.c:504: }
	jr	00101$
;./bdos.c:505: void bdos_gets(char *pc) {
;	---------------------------------
; Function bdos_gets
; ---------------------------------
_bdos_gets::
	push	ix
	ld	ix,#0
	add	ix,sp
;./bdos.c:508: pcs = pc;
	ld	c, 4 (ix)
	ld	b, 5 (ix)
;./bdos.c:510: while(1) {
00111$:
;./bdos.c:511: c = bios_conin();
	push	bc
	call	_bios_conin
	ld	a, l
	pop	bc
;./bdos.c:512: if ( (c == 8) || (c == 127) ) {
	ld	d, a
	sub	a, #0x08
	jr	Z,00103$
	ld	a, d
	sub	a, #0x7f
	jr	NZ,00104$
00103$:
;./bdos.c:514: if (pc > pcs) {
	ld	a, c
	sub	a, 4 (ix)
	ld	a, b
	sbc	a, 5 (ix)
	jr	NC,00111$
;./bdos.c:515: pc--;
	ld	l, 4 (ix)
	ld	h, 5 (ix)
	dec	hl
	ld	4 (ix), l
	ld	5 (ix), h
;./bdos.c:523: continue;
	jr	00111$
00104$:
;./bdos.c:526: bios_conout(c);
	push	bc
	push	de
	push	de
	inc	sp
	call	_bios_conout
	inc	sp
	pop	de
	pop	bc
;./bdos.c:508: pcs = pc;
	ld	l, 4 (ix)
	ld	h, 5 (ix)
;./bdos.c:528: if ((c == '\n') || (c == '\r') || (c == 0)) {
	ld	a,d
	cp	a,#0x0a
	jr	Z,00106$
	cp	a,#0x0d
	jr	Z,00106$
	or	a, a
	jr	NZ,00107$
00106$:
;./bdos.c:532: *pc = 0;
	ld	(hl), #0x00
;./bdos.c:533: return;
	jr	00113$
00107$:
;./bdos.c:537: *pc++ = c;
	ld	(hl), d
	inc	hl
	ld	4 (ix), l
	ld	5 (ix), h
	jr	00111$
00113$:
;./bdos.c:539: }
	pop	ix
	ret
;./bdos.c:542: void bdos_printf(char *pc) {
;	---------------------------------
; Function bdos_printf
; ---------------------------------
_bdos_printf::
;./bdos.c:544: c = *pc;
	pop	bc
	pop	hl
	push	hl
	push	bc
	ld	b, (hl)
;./bdos.c:545: while(c != 0) {
00101$:
	ld	a, b
	or	a, a
	ret	Z
;./bdos.c:546: bios_conout(c);
	push	bc
	push	bc
	inc	sp
	call	_bios_conout
	inc	sp
	pop	bc
;./bdos.c:547: pc++;
;./bdos.c:549: }
	jr	00101$
;./bdos.c:551: void bdos_printf_d(char *pc, byte d) {
;	---------------------------------
; Function bdos_printf_d
; ---------------------------------
_bdos_printf_d::
;./bdos.c:554: bdos_printf(pc);
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	_bdos_printf
	pop	af
;./bdos.c:555: i = 100;
	ld	c, #0x64
;./bdos.c:556: while(i > 0) {
00101$:
	ld	a, c
	or	a, a
	ret	Z
;./bdos.c:557: bios_conout('0' + ((d / i) % 10));
	push	bc
	ld	a, c
	push	af
	inc	sp
	ld	hl, #7+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	call	__divuchar
	pop	af
	pop	bc
	ld	h, #0x00
	push	bc
	ld	de, #0x000a
	push	de
	push	hl
	call	__modsint
	pop	af
	pop	af
	pop	bc
	ld	a, l
	add	a, #0x30
	push	bc
	push	af
	inc	sp
	call	_bios_conout
	inc	sp
	pop	bc
;./bdos.c:558: i /= 10;
	ld	b, #0x00
	ld	hl, #0x000a
	push	hl
	push	bc
	call	__divsint
	pop	af
	pop	af
	ld	c, l
;./bdos.c:561: }
	jr	00101$
;./bdos.c:566: byte bdos_strlen(const char *c) {
;	---------------------------------
; Function bdos_strlen
; ---------------------------------
_bdos_strlen::
;./bdos.c:569: while (*c++ != 0)  {
	ld	c, #0x00
	pop	de
	pop	hl
	push	hl
	push	de
00101$:
	ld	a, (hl)
	inc	hl
	or	a, a
	jr	Z,00103$
;./bdos.c:570: l++;
	inc	c
	jr	00101$
00103$:
;./bdos.c:572: return l;
	ld	l, c
;./bdos.c:573: }
	ret
;./bdos.c:574: void bdos_memset(byte *addr, byte b, word count) {
;	---------------------------------
; Function bdos_memset
; ---------------------------------
_bdos_memset::
;./bdos.c:575: while(count > 0) {
	pop	de
	pop	bc
	push	bc
	push	de
	ld	hl, #5
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
00101$:
	ld	a, d
	or	a, e
	ret	Z
;./bdos.c:576: *addr++ = b;
	ld	hl, #4+0
	add	hl, sp
	ld	a, (hl)
	ld	(bc), a
	inc	bc
;./bdos.c:577: count--;
	dec	de
;./bdos.c:579: }
	jr	00101$
;./bdos.c:853: byte bdos_f_open(struct FCB *fcb) {
;	---------------------------------
; Function bdos_f_open
; ---------------------------------
_bdos_f_open::
;./bdos.c:862: bdos_puts("bdos_f_open n/a");
	ld	hl, #___str_4
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:884: return r;
	ld	l, #0xff
;./bdos.c:885: }
	ret
___str_4:
	.ascii "bdos_f_open n/a"
	.db 0x00
;./bdos.c:887: byte bdos_f_close(struct FCB *fcb) {
;	---------------------------------
; Function bdos_f_close
; ---------------------------------
_bdos_f_close::
;./bdos.c:896: bdos_puts("bdos_f_close n/a!");
	ld	hl, #___str_5
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:903: return r;
	ld	l, #0xff
;./bdos.c:904: }
	ret
___str_5:
	.ascii "bdos_f_close n/a!"
	.db 0x00
;./bdos.c:906: byte bdos_f_read(struct FCB *fcb) {
;	---------------------------------
; Function bdos_f_read
; ---------------------------------
_bdos_f_read::
;./bdos.c:919: bdos_puts("bdos_f_read n/a!");
	ld	hl, #___str_6
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:925: return 1;	// 1 = EOF
	ld	l, #0x01
;./bdos.c:948: return 0x00;
;./bdos.c:949: }
	ret
___str_6:
	.ascii "bdos_f_read n/a!"
	.db 0x00
;./bdos.c:951: byte bdos_f_readrand(struct FCB *fcb) {
;	---------------------------------
; Function bdos_f_readrand
; ---------------------------------
_bdos_f_readrand::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
;./bdos.c:973: rn = (word)fcb->r0 + ((word)fcb->r1 * 256);
	ld	c, 4 (ix)
	ld	b, 5 (ix)
	ld	l, c
	ld	h, b
	ld	de, #0x0021
	add	hl, de
	ld	e, (hl)
	ld	l, c
	ld	h, b
	push	bc
	ld	bc, #0x0022
	add	hl, bc
	pop	bc
	ld	h, (hl)
	ld	l, #0x00
	ld	d, #0x00
	add	hl, de
	ex	de, hl
;./bdos.c:974: ex = rn / 128;
	ld	l, e
	ld	h, d
	srl	h
	rr	l
	srl	h
	rr	l
	srl	h
	rr	l
	srl	h
	rr	l
	srl	h
	rr	l
	srl	h
	rr	l
	srl	h
	rr	l
	inc	sp
	inc	sp
	push	hl
;./bdos.c:975: fcb->cr = rn % 128;
	ld	hl, #0x0020
	add	hl, bc
	res	7, e
	ld	(hl), e
;./bdos.c:976: fcb->ex = ex % 32;
	ld	hl, #0x000c
	add	hl, bc
	pop	de
	push	de
	ld	a, e
	and	a, #0x1f
	ld	-2 (ix), a
	ld	-1 (ix), #0x00
	ld	a, -2 (ix)
	ld	(hl), a
;./bdos.c:977: fcb->s2 = (0x80 | (ex / 32));
	ld	hl, #0x000e
	add	hl, bc
	srl	d
	rr	e
	srl	d
	rr	e
	srl	d
	rr	e
	srl	d
	rr	e
	srl	d
	rr	e
	set	7, e
	ld	(hl), e
;./bdos.c:980: r = bdos_f_read(fcb);
	push	bc
	call	_bdos_f_read
;./bdos.c:981: return r;
;./bdos.c:982: }
	ld	sp,ix
	pop	ix
	ret
;./bdos.c:984: byte bdos_f_write(struct FCB *fcb) {
;	---------------------------------
; Function bdos_f_write
; ---------------------------------
_bdos_f_write::
;./bdos.c:991: bdos_puts("bdos_f_write n/a!");
	ld	hl, #___str_7
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:993: return 0xff;
	ld	l, #0xff
;./bdos.c:996: }
	ret
___str_7:
	.ascii "bdos_f_write n/a!"
	.db 0x00
;./bdos.c:998: byte bdos_f_writerand(struct FCB *fcb) {
;	---------------------------------
; Function bdos_f_writerand
; ---------------------------------
_bdos_f_writerand::
;./bdos.c:1005: bdos_puts("bdos_f_writerand n/a!");
	ld	hl, #___str_8
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:1007: return 0xff;
	ld	l, #0xff
;./bdos.c:1009: }
	ret
___str_8:
	.ascii "bdos_f_writerand n/a!"
	.db 0x00
;./bdos.c:1027: byte bdos_f_sfirst(struct FCB *fcb) {
;	---------------------------------
; Function bdos_f_sfirst
; ---------------------------------
_bdos_f_sfirst::
;./bdos.c:1033: bdos_puts("bdos_f_sfirst n/a!");
	ld	hl, #___str_9
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:1035: return 0xff;
	ld	l, #0xff
;./bdos.c:1037: }
	ret
___str_9:
	.ascii "bdos_f_sfirst n/a!"
	.db 0x00
;./bdos.c:1038: byte bdos_f_snext(struct FCB *fcb) {
;	---------------------------------
; Function bdos_f_snext
; ---------------------------------
_bdos_f_snext::
;./bdos.c:1044: bdos_puts("bdos_f_snext n/a!");
	ld	hl, #___str_10
	push	hl
	call	_bdos_puts
	pop	af
;./bdos.c:1046: return 0xff;
	ld	l, #0xff
;./bdos.c:1048: }
	ret
___str_10:
	.ascii "bdos_f_snext n/a!"
	.db 0x00
;./bdos.c:1052: void bdos_init() __naked {
;	---------------------------------
; Function bdos_init
; ---------------------------------
_bdos_init::
;./bdos.c:1057: bdos_delimiter = '$';
	ld	hl,#_bdos_delimiter + 0
	ld	(hl), #0x24
;./bdos.c:1058: bdos_user = 1;
	ld	hl,#_bdos_user + 0
	ld	(hl), #0x01
;./bdos.c:1060: bdos_memset((byte *)bdos_fcb, 0x00, 36);	//sizeof(FCB));
	ld	bc, #_bdos_fcb+0
	ld	hl, #0x0024
	push	hl
	xor	a, a
	push	af
	inc	sp
	push	bc
	call	_bdos_memset
	pop	af
;./bdos.c:1079: bdos_puts("Must load CCP now (n/a)");
	inc	sp
	ld	hl,#___str_11
	ex	(sp),hl
	call	_bdos_puts
	pop	af
;./bdos.c:1080: bdos_getchar();
	jp	_bdos_getchar
;./bdos.c:1082: }
;../../include/driver/hd44780.h:169: void lcd_delay_long() __naked {
;	---------------------------------
; Function lcd_delay_long
; ---------------------------------
_lcd_delay_long::
;../../include/driver/hd44780.h:181: __endasm;
;	Used for screen functions (after putting stuff to ports 0x0a or 0x0b)
	push	hl
	ld	hl, #0x1fff
	_delay_1fff_loop:
	dec	l
	jr	nz, _delay_1fff_loop
	dec	h
	jr	nz, _delay_1fff_loop
	pop	hl
	ret
;../../include/driver/hd44780.h:182: }
;../../include/driver/hd44780.h:185: void lcd_delay_short() __naked {
;	---------------------------------
; Function lcd_delay_short
; ---------------------------------
_lcd_delay_short::
;../../include/driver/hd44780.h:197: __endasm;
;	Used for screen functions (after putting stuff to ports 0x0a or 0x0b)
	push	hl
	ld	hl, #0x010f
	_delay_010f_loop:
	dec	l
	jr	nz, _delay_010f_loop
	dec	h
	jr	nz, _delay_010f_loop
	pop	hl
	ret
;../../include/driver/hd44780.h:198: }
;../../include/driver/hd44780.h:201: void lcd_writeControl(byte a) {
;	---------------------------------
; Function lcd_writeControl
; ---------------------------------
_lcd_writeControl::
;../../include/driver/hd44780.h:202: lcd_controlPort = a;	// Output value to LCD control port
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	out	(_lcd_controlPort), a
;../../include/driver/hd44780.h:205: lcd_delay_short();
	call	_lcd_delay_short
;../../include/driver/hd44780.h:206: lcd_delay_short();
;../../include/driver/hd44780.h:207: }
	jp	_lcd_delay_short
___str_11:
	.ascii "Must load CCP now (n/a)"
	.db 0x00
_lcd_map_4rows:
	.db #0x00	; 0
	.db #0x01	; 1
	.db #0x02	; 2
	.db #0x03	; 3
	.db #0x08	; 8
	.db #0x09	; 9
	.db #0x0a	; 10
	.db #0x0b	; 11
	.db #0x0c	; 12
	.db #0x0d	; 13
	.db #0x0e	; 14
	.db #0x0f	; 15
	.db #0x18	; 24
	.db #0x19	; 25
	.db #0x1a	; 26
	.db #0x1b	; 27
	.db #0x1c	; 28
	.db #0x1d	; 29
	.db #0x1e	; 30
	.db #0x1f	; 31
	.db #0x40	; 64
	.db #0x41	; 65	'A'
	.db #0x42	; 66	'B'
	.db #0x43	; 67	'C'
	.db #0x48	; 72	'H'
	.db #0x49	; 73	'I'
	.db #0x4a	; 74	'J'
	.db #0x4b	; 75	'K'
	.db #0x4c	; 76	'L'
	.db #0x4d	; 77	'M'
	.db #0x4e	; 78	'N'
	.db #0x4f	; 79	'O'
	.db #0x58	; 88	'X'
	.db #0x59	; 89	'Y'
	.db #0x5a	; 90	'Z'
	.db #0x5b	; 91
	.db #0x5c	; 92
	.db #0x5d	; 93
	.db #0x5e	; 94
	.db #0x5f	; 95
	.db #0x04	; 4
	.db #0x05	; 5
	.db #0x06	; 6
	.db #0x07	; 7
	.db #0x10	; 16
	.db #0x11	; 17
	.db #0x12	; 18
	.db #0x13	; 19
	.db #0x14	; 20
	.db #0x15	; 21
	.db #0x16	; 22
	.db #0x17	; 23
	.db #0x20	; 32
	.db #0x21	; 33
	.db #0x22	; 34
	.db #0x23	; 35
	.db #0x24	; 36
	.db #0x25	; 37
	.db #0x26	; 38
	.db #0x27	; 39
	.db #0x44	; 68	'D'
	.db #0x45	; 69	'E'
	.db #0x46	; 70	'F'
	.db #0x47	; 71	'G'
	.db #0x50	; 80	'P'
	.db #0x51	; 81	'Q'
	.db #0x52	; 82	'R'
	.db #0x53	; 83	'S'
	.db #0x54	; 84	'T'
	.db #0x55	; 85	'U'
	.db #0x56	; 86	'V'
	.db #0x57	; 87	'W'
	.db #0x60	; 96
	.db #0x61	; 97	'a'
	.db #0x62	; 98	'b'
	.db #0x63	; 99	'c'
	.db #0x64	; 100	'd'
	.db #0x65	; 101	'e'
	.db #0x66	; 102	'f'
	.db #0x67	; 103	'g'
	.db #0x67	; 103	'g'
;../../include/driver/hd44780.h:209: void lcd_writeData(byte a) {
;	---------------------------------
; Function lcd_writeData
; ---------------------------------
_lcd_writeData::
;../../include/driver/hd44780.h:210: lcd_dataPort = a;	// Output value to LCD data port
	ld	iy, #2
	add	iy, sp
	ld	a, 0 (iy)
	out	(_lcd_dataPort), a
;../../include/driver/hd44780.h:214: lcd_delay_short();
;../../include/driver/hd44780.h:215: }
	jp	_lcd_delay_short
;../../include/driver/hd44780.h:231: void lcd_set_cursor() {
;	---------------------------------
; Function lcd_set_cursor
; ---------------------------------
_lcd_set_cursor::
;../../include/driver/hd44780.h:235: o = lcd_x + (lcd_y * LCD_COLS);
	ld	a,(#_lcd_y + 0)
	ld	c, a
	add	a, a
	add	a, a
	add	a, c
	add	a, a
	add	a, a
	ld	hl,#_lcd_x + 0
	ld	c, (hl)
	add	a, c
	ld	e, a
;../../include/driver/hd44780.h:236: lcd_writeControl(LCD_SETDDRAMADDR | lcd_map[o]);
	ld	hl, #_lcd_map_4rows+0
	ld	d, #0x00
	add	hl, de
	ld	a, (hl)
	set	7, a
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:238: if (lcd_cursor) lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);
	ld	a,(#_lcd_cursor + 0)
	or	a, a
	ret	Z
	ld	a, #0x0f
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:239: }
	ret
;../../include/driver/hd44780.h:242: void lcd_clear() {
;	---------------------------------
; Function lcd_clear
; ---------------------------------
_lcd_clear::
;../../include/driver/hd44780.h:247: lcd_writeControl(LCD_CLEARDISPLAY);
	ld	a, #0x01
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:249: lcd_writeControl(LCD_RETURNHOME);
	ld	a, #0x02
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:250: lcd_delay_long();
	call	_lcd_delay_long
;../../include/driver/hd44780.h:252: lcd_x = 0;
	ld	hl,#_lcd_x + 0
	ld	(hl), #0x00
;../../include/driver/hd44780.h:253: lcd_y = 0;
	ld	iy, #_lcd_y
	ld	0 (iy), #0x00
;../../include/driver/hd44780.h:257: for(i = 0; i < (LCD_COLS * LCD_ROWS); i++) {
	ld	bc, #_lcd_buffer+0
	ld	e, #0x00
00102$:
;../../include/driver/hd44780.h:258: lcd_buffer[i] = 0x20;
	ld	l, e
	ld	h, #0x00
	add	hl, bc
	ld	(hl), #0x20
;../../include/driver/hd44780.h:257: for(i = 0; i < (LCD_COLS * LCD_ROWS); i++) {
	inc	e
	ld	a, e
	sub	a, #0x50
	jr	C,00102$
;../../include/driver/hd44780.h:261: lcd_set_cursor();
;../../include/driver/hd44780.h:263: }
	jp	_lcd_set_cursor
;../../include/driver/hd44780.h:266: void lcd_init() {
;	---------------------------------
; Function lcd_init
; ---------------------------------
_lcd_init::
;../../include/driver/hd44780.h:273: lcd_writeControl(0x38);	//Function set: 2 Line, 8-bit, 5x7 dots
	ld	a, #0x38
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:274: lcd_writeControl(0x38);
	ld	a, #0x38
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:275: lcd_writeControl(0x38);
	ld	a, #0x38
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:276: lcd_writeControl(0x38);
	ld	a, #0x38
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:279: lcd_writeControl(LCD_CLEARDISPLAY);
	ld	a, #0x01
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:285: lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);
	ld	a, #0x0f
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:287: lcd_writeControl(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT );
	ld	a, #0x06
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:298: lcd_cursor = 1;
	ld	iy, #_lcd_cursor
	ld	0 (iy), #0x01
;../../include/driver/hd44780.h:299: lcd_scroll_cb = 0;
	ld	hl, #0x0000
	ld	(_lcd_scroll_cb), hl
;../../include/driver/hd44780.h:302: lcd_clear();
;../../include/driver/hd44780.h:319: }
	jp	_lcd_clear
;../../include/driver/hd44780.h:323: void lcd_refresh() {
;	---------------------------------
; Function lcd_refresh
; ---------------------------------
_lcd_refresh::
;../../include/driver/hd44780.h:329: lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
	ld	a, #0x0c
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:331: p0 = &lcd_buffer[0];
	ld	bc, #_lcd_buffer+0
;../../include/driver/hd44780.h:333: for(i = 0; i < (LCD_COLS * LCD_ROWS); i++) {
	ld	de, #0x0000
00102$:
;../../include/driver/hd44780.h:334: lcd_writeControl(LCD_SETDDRAMADDR | lcd_map[o]);
	ld	a, #<(_lcd_map_4rows)
	add	a, e
	ld	l, a
	ld	a, #>(_lcd_map_4rows)
	adc	a, #0x00
	ld	h, a
	ld	a, (hl)
	set	7, a
	push	bc
	push	de
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
	pop	de
	pop	bc
;../../include/driver/hd44780.h:335: lcd_writeData(*p0++);
	ld	a, (bc)
	inc	bc
	push	bc
	push	de
	push	af
	inc	sp
	call	_lcd_writeData
	inc	sp
	pop	de
	pop	bc
;../../include/driver/hd44780.h:336: o++;
	inc	e
;../../include/driver/hd44780.h:333: for(i = 0; i < (LCD_COLS * LCD_ROWS); i++) {
	inc	d
	ld	a, d
	sub	a, #0x50
	jr	C,00102$
;../../include/driver/hd44780.h:339: lcd_set_cursor();
	call	_lcd_set_cursor
;../../include/driver/hd44780.h:341: lcd_delay_long();
;../../include/driver/hd44780.h:342: }
	jp	_lcd_delay_long
;../../include/driver/hd44780.h:345: void lcd_scroll() {
;	---------------------------------
; Function lcd_scroll
; ---------------------------------
_lcd_scroll::
	push	ix
	ld	ix,#0
	add	ix,sp
	dec	sp
;../../include/driver/hd44780.h:364: p0 = &lcd_buffer[0];
	ld	bc, #_lcd_buffer
;../../include/driver/hd44780.h:369: p1 = &lcd_buffer[LCD_COLS];
	ld	de, #_lcd_buffer + 20
;../../include/driver/hd44780.h:370: for(i = 0; i < (LCD_COLS * (LCD_ROWS-1)); i++) {
	xor	a, a
	ld	-1 (ix), a
00103$:
;../../include/driver/hd44780.h:371: *p0++ = *p1++;
	ld	a, (de)
	inc	de
	ld	(bc), a
	inc	bc
;../../include/driver/hd44780.h:370: for(i = 0; i < (LCD_COLS * (LCD_ROWS-1)); i++) {
	inc	-1 (ix)
	ld	a, -1 (ix)
	sub	a, #0x3c
	jr	C,00103$
;../../include/driver/hd44780.h:377: for(i = 0; i < LCD_COLS; i++) {
	ld	e, #0x14
00107$:
;../../include/driver/hd44780.h:378: *p0++ = 0x20;	// Fill with spaces
	ld	a, #0x20
	ld	(bc), a
	inc	bc
;../../include/driver/hd44780.h:377: for(i = 0; i < LCD_COLS; i++) {
	dec	e
	jr	NZ,00107$
;../../include/driver/hd44780.h:381: lcd_refresh();
	call	_lcd_refresh
;../../include/driver/hd44780.h:382: }
	inc	sp
	pop	ix
	ret
;../../include/driver/hd44780.h:395: void lcd_putchar_at(byte x, byte y, char c) {
;	---------------------------------
; Function lcd_putchar_at
; ---------------------------------
_lcd_putchar_at::
;../../include/driver/hd44780.h:399: o = x + (y * LCD_COLS);
	ld	iy, #3
	add	iy, sp
	ld	a, 0 (iy)
	ld	c, a
	add	a, a
	add	a, a
	add	a, c
	add	a, a
	add	a, a
	dec	iy
	ld	c, 0 (iy)
	add	a, c
	ld	e, a
;../../include/driver/hd44780.h:402: lcd_writeControl(LCD_SETDDRAMADDR | lcd_map[o]);
	ld	hl, #_lcd_map_4rows+0
	ld	d, #0x00
	add	hl, de
	ld	a, (hl)
	set	7, a
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
;../../include/driver/hd44780.h:405: lcd_writeData(c);
	ld	hl, #4+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	call	_lcd_writeData
	inc	sp
;../../include/driver/hd44780.h:415: }
	ret
;../../include/driver/hd44780.h:417: void lcd_putchar(byte c) {
;	---------------------------------
; Function lcd_putchar
; ---------------------------------
_lcd_putchar::
	push	ix
	ld	ix,#0
	add	ix,sp
;../../include/driver/hd44780.h:420: if (c == '\r') {
	ld	a, 4 (ix)
	sub	a, #0x0d
	jr	NZ,00109$
;../../include/driver/hd44780.h:422: lcd_x = 0;
	ld	hl,#_lcd_x + 0
	ld	(hl), #0x00
;../../include/driver/hd44780.h:423: c = 0;
	xor	a, a
	ld	4 (ix), a
	jr	00110$
00109$:
;../../include/driver/hd44780.h:426: if (c == '\n') {
	ld	a, 4 (ix)
	sub	a, #0x0a
	jr	NZ,00106$
;../../include/driver/hd44780.h:428: lcd_x = 0;
	ld	hl,#_lcd_x + 0
	ld	(hl), #0x00
;../../include/driver/hd44780.h:429: lcd_y++;
	ld	hl, #_lcd_y+0
	inc	(hl)
;../../include/driver/hd44780.h:430: c = 0;
	xor	a, a
	ld	4 (ix), a
	jr	00110$
00106$:
;../../include/driver/hd44780.h:433: if (c == 8) {
	ld	a, 4 (ix)
	sub	a, #0x08
	jr	NZ,00110$
;../../include/driver/hd44780.h:435: if (lcd_x > 0) lcd_x--;
	ld	iy, #_lcd_x
	ld	a, 0 (iy)
	or	a, a
	jr	Z,00102$
	dec	0 (iy)
00102$:
;../../include/driver/hd44780.h:436: c = 0;
	xor	a, a
	ld	4 (ix), a
00110$:
;../../include/driver/hd44780.h:440: if (lcd_x >= LCD_COLS) {
	ld	iy, #_lcd_x
	ld	a, 0 (iy)
	sub	a, #0x14
	jr	C,00112$
;../../include/driver/hd44780.h:441: lcd_x = 0;
	ld	0 (iy), #0x00
;../../include/driver/hd44780.h:442: lcd_y++;
	ld	hl, #_lcd_y+0
	inc	(hl)
00112$:
;../../include/driver/hd44780.h:445: if (lcd_y >= LCD_ROWS) {
	ld	a,(#_lcd_y + 0)
	sub	a, #0x04
	jr	C,00120$
;../../include/driver/hd44780.h:448: if (lcd_scroll_cb != 0)
	ld	iy, #_lcd_scroll_cb
	ld	a, 1 (iy)
	or	a, 0 (iy)
	jr	Z,00113$
;../../include/driver/hd44780.h:449: (*lcd_scroll_cb)();
	ld	hl, (_lcd_scroll_cb)
	call	___sdcc_call_hl
	jr	00120$
;../../include/driver/hd44780.h:451: while(lcd_y >= LCD_ROWS) {
00113$:
	ld	iy, #_lcd_y
	ld	a, 0 (iy)
	sub	a, #0x04
	jr	C,00120$
;../../include/driver/hd44780.h:452: lcd_y--;
	dec	0 (iy)
;../../include/driver/hd44780.h:453: lcd_scroll();
	call	_lcd_scroll
	jr	00113$
00120$:
;../../include/driver/hd44780.h:464: o = lcd_x + (lcd_y * LCD_COLS);
	ld	a,(#_lcd_y + 0)
	ld	c, a
	add	a, a
	add	a, a
	add	a, c
	add	a, a
	add	a, a
	ld	hl,#_lcd_x + 0
	ld	c, (hl)
	add	a, c
	ld	e, a
;../../include/driver/hd44780.h:467: lcd_writeControl(LCD_SETDDRAMADDR | lcd_map[o]);
	ld	bc, #_lcd_map_4rows+0
	ld	l, e
	ld	h, #0x00
	add	hl, bc
	ld	a, (hl)
	set	7, a
	push	de
	push	af
	inc	sp
	call	_lcd_writeControl
	inc	sp
	pop	de
;../../include/driver/hd44780.h:469: if (c > 0) {
	ld	a, 4 (ix)
	or	a, a
	jr	Z,00123$
;../../include/driver/hd44780.h:471: lcd_writeData(c);
	push	de
	ld	a, 4 (ix)
	push	af
	inc	sp
	call	_lcd_writeData
	inc	sp
	pop	de
;../../include/driver/hd44780.h:472: lcd_x++;
	ld	hl, #_lcd_x+0
	inc	(hl)
;../../include/driver/hd44780.h:476: lcd_buffer[o] = c;
	ld	hl, #_lcd_buffer+0
	ld	d, #0x00
	add	hl, de
	ld	a, 4 (ix)
	ld	(hl), a
;../../include/driver/hd44780.h:478: lcd_set_cursor();
	call	_lcd_set_cursor
00123$:
;../../include/driver/hd44780.h:482: }
	pop	ix
	ret
;../../include/arch/gl4000/keyboard.h:72: byte keyboard_inkey() {
;	---------------------------------
; Function keyboard_inkey
; ---------------------------------
_keyboard_inkey::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl, #-7
	add	hl, sp
	ld	sp, hl
;../../include/arch/gl4000/keyboard.h:84: keyboard_port_matrixLatch = 0xff;
	ld	a, #0xff
	out	(_keyboard_port_matrixLatch), a
;../../include/arch/gl4000/keyboard.h:91: m = 0x01;
	ld	-7 (ix), #0x01
;../../include/arch/gl4000/keyboard.h:92: for(row = 0; row < 8; row++) {
	xor	a, a
	ld	-6 (ix), a
	xor	a, a
	ld	-2 (ix), a
00109$:
;../../include/arch/gl4000/keyboard.h:96: keyboard_port_matrixRowOut = m;
	ld	a, -7 (ix)
	out	(_keyboard_port_matrixRowOut), a
;../../include/arch/gl4000/keyboard.h:99: b1 = keyboard_port_matrixColIn;
	in	a, (_keyboard_port_matrixColIn)
	ld	-5 (ix), a
;../../include/arch/gl4000/keyboard.h:106: keyboard_port_matrixRowOut = 0x00;
	ld	a, #0x00
	out	(_keyboard_port_matrixRowOut), a
;../../include/arch/gl4000/keyboard.h:108: if (b1 < 0xff) {
	ld	a, -5 (ix)
	sub	a, #0xff
	jr	NC,00105$
;../../include/arch/gl4000/keyboard.h:109: m2 = 0x01;
	ld	-4 (ix), #0x01
;../../include/arch/gl4000/keyboard.h:110: for(col = 0; col < 8; col++) { 
	xor	a, a
	ld	-3 (ix), a
	xor	a, a
	ld	-1 (ix), a
00107$:
;../../include/arch/gl4000/keyboard.h:111: if ((b1 & m2) == 0) {
	ld	a, -5 (ix)
	and	a, -4 (ix)
	or	a, a
	jr	NZ,00102$
;../../include/arch/gl4000/keyboard.h:113: return vgl_key_map[8 * row + col];
	ld	a, -6 (ix)
	add	a, a
	add	a, a
	add	a, a
	ld	-1 (ix), a
	ld	a, -3 (ix)
	add	a, -1 (ix)
	ld	-1 (ix), a
	ld	-2 (ix), a
	rla
	sbc	a, a
	ld	-1 (ix), a
	ld	a, -2 (ix)
	add	a, #<(_vgl_key_map)
	ld	-4 (ix), a
	ld	a, -1 (ix)
	adc	a, #>(_vgl_key_map)
	ld	-3 (ix), a
	ld	l, -4 (ix)
	ld	h, -3 (ix)
	ld	a, (hl)
	ld	-1 (ix), a
	ld	l, a
	jr	00111$
00102$:
;../../include/arch/gl4000/keyboard.h:116: m2 = m2 << 1;
	ld	a, -4 (ix)
	add	a, a
	ld	-4 (ix), a
;../../include/arch/gl4000/keyboard.h:110: for(col = 0; col < 8; col++) { 
	inc	-1 (ix)
	ld	a, -1 (ix)
	ld	-3 (ix), a
	ld	a, -1 (ix)
	sub	a, #0x08
	jr	C,00107$
00105$:
;../../include/arch/gl4000/keyboard.h:135: m = m << 1;
	ld	a, -7 (ix)
	add	a, a
	ld	-7 (ix), a
;../../include/arch/gl4000/keyboard.h:92: for(row = 0; row < 8; row++) {
	inc	-2 (ix)
	ld	a, -2 (ix)
	ld	-6 (ix), a
	ld	a, -2 (ix)
	sub	a, #0x08
	jp	C, 00109$
;../../include/arch/gl4000/keyboard.h:147: return 0;
	ld	l, #0x00
00111$:
;../../include/arch/gl4000/keyboard.h:148: }
	ld	sp, ix
	pop	ix
	ret
_vgl_key_map:
	.db #0x09	; 9
	.db #0x1b	; 27
	.db #0x65	; 101	'e'
	.db #0x66	; 102	'f'
	.db #0x67	; 103	'g'
	.db #0x69	; 105	'i'
	.db #0x6a	; 106	'j'
	.db #0x6b	; 107	'k'
	.db #0x63	; 99	'c'
	.db #0x31	; 49	'1'
	.db #0x51	; 81	'Q'
	.db #0x41	; 65	'A'
	.db #0x59	; 89	'Y'
	.db #0x61	; 97	'a'
	.db #0x20	; 32
	.db #0x0d	; 13
	.db #0x32	; 50	'2'
	.db #0x33	; 51	'3'
	.db #0x45	; 69	'E'
	.db #0x53	; 83	'S'
	.db #0x44	; 68	'D'
	.db #0x58	; 88	'X'
	.db #0x43	; 67	'C'
	.db #0x57	; 87	'W'
	.db #0x34	; 52	'4'
	.db #0x35	; 53	'5'
	.db #0x54	; 84	'T'
	.db #0x46	; 70	'F'
	.db #0x47	; 71	'G'
	.db #0x56	; 86	'V'
	.db #0x42	; 66	'B'
	.db #0x52	; 82	'R'
	.db #0x36	; 54	'6'
	.db #0x37	; 55	'7'
	.db #0x55	; 85	'U'
	.db #0x48	; 72	'H'
	.db #0x4a	; 74	'J'
	.db #0x4e	; 78	'N'
	.db #0x4d	; 77	'M'
	.db #0x5a	; 90	'Z'
	.db #0x38	; 56	'8'
	.db #0x39	; 57	'9'
	.db #0x4f	; 79	'O'
	.db #0x4b	; 75	'K'
	.db #0x4c	; 76	'L'
	.db #0x2c	; 44
	.db #0x2e	; 46
	.db #0x49	; 73	'I'
	.db #0x30	; 48	'0'
	.db #0x08	; 8
	.db #0x3d	; 61
	.db #0x3a	; 58
	.db #0x27	; 39
	.db #0x2f	; 47
	.db #0x73	; 115	's'
	.db #0x50	; 80	'P'
	.db #0x78	; 120	'x'
	.db #0x79	; 121	'y'
	.db #0x68	; 104	'h'
	.db #0x6c	; 108	'l'
	.db #0x0d	; 13
	.db #0x7f	; 127
	.db #0x1b	; 27
	.db #0x7a	; 122	'z'
;../../include/arch/gl4000/keyboard.h:152: byte keyboard_checkkey() {
;	---------------------------------
; Function keyboard_checkkey
; ---------------------------------
_keyboard_checkkey::
;../../include/arch/gl4000/keyboard.h:159: keyboard_port_matrixLatch = 0xff;
	ld	a, #0xff
	out	(_keyboard_port_matrixLatch), a
;../../include/arch/gl4000/keyboard.h:163: keyboard_port_matrixRowOut = 0xff;
	ld	a, #0xff
	out	(_keyboard_port_matrixRowOut), a
;../../include/arch/gl4000/keyboard.h:166: b1 = keyboard_port_matrixColIn;
	in	a, (_keyboard_port_matrixColIn)
	ld	c, a
;../../include/arch/gl4000/keyboard.h:168: keyboard_port_matrixRowOut = 0x00;
	ld	a, #0x00
	out	(_keyboard_port_matrixRowOut), a
;../../include/arch/gl4000/keyboard.h:170: if (b1 < 0xff) return b1;	// Some key is pressed
	ld	a, c
	sub	a, #0xff
	jr	NC,00102$
	ld	l, c
	ret
00102$:
;../../include/arch/gl4000/keyboard.h:173: return 0;	// No keys are pressed
	ld	l, #0x00
;../../include/arch/gl4000/keyboard.h:174: }
	ret
;../../include/arch/gl4000/keyboard.h:178: char keyboard_getchar() {
;	---------------------------------
; Function keyboard_getchar
; ---------------------------------
_keyboard_getchar::
;../../include/arch/gl4000/keyboard.h:184: while(1) {
00106$:
;../../include/arch/gl4000/keyboard.h:185: c = keyboard_inkey();
	call	_keyboard_inkey
	ld	c, l
;../../include/arch/gl4000/keyboard.h:186: if (c == 0) keyboard_getchar_last = 0;
	ld	a, c
	or	a, a
	jr	NZ,00102$
	ld	hl,#_keyboard_getchar_last + 0
	ld	(hl), #0x00
00102$:
;../../include/arch/gl4000/keyboard.h:187: if (c != keyboard_getchar_last) break;
	ld	iy, #_keyboard_getchar_last
	ld	a, 0 (iy)
	sub	a, c
	jr	Z,00106$
;../../include/arch/gl4000/keyboard.h:190: keyboard_getchar_last = c;
	ld	0 (iy), c
;../../include/arch/gl4000/keyboard.h:191: return c;
	ld	l, c
;../../include/arch/gl4000/keyboard.h:192: }
	ret
;../../include/arch/gl4000/sound.h:9: void vgl_sound_off() {
;	---------------------------------
; Function vgl_sound_off
; ---------------------------------
_vgl_sound_off::
;../../include/arch/gl4000/sound.h:19: __endasm;
;	Speaker off
;ld	a, #0x00 ; +20h
	in	a, (0x12)
	and	#0xf7
	out	(0x12), a
	ret
;../../include/arch/gl4000/sound.h:20: }
	ret
;../../include/arch/gl4000/sound.h:22: void vgl_sound_tone(word frq, word len) {
;	---------------------------------
; Function vgl_sound_tone
; ---------------------------------
_vgl_sound_tone::
;../../include/arch/gl4000/sound.h:109: __endasm;
;di
	push	af
	push	hl
	push	de
	push	bc
;	Get frq param
	ld	hl, #2+0
	add	hl, sp
	ld	e, (hl)
	ld	hl, #2+1
	add	hl, sp
	ld	d, (hl) ; frq is now in DE
;inc	de ; Inc by one
	ld	b, d ; Safe DE for later in BC
	ld	c, e
;len	to DE
	ld	hl, #4+0
	add	hl, sp
	ld	e, (hl)
	ld	hl, #4+1
	add	hl, sp
	ld	d, (hl) ; len is now in DE
;inc	de ; Inc by one
	ld	h, b ; Copy old frq value back from BC to HL
	ld	l, c
;	Actual sound loop
	  _sound_loop:
;	Speaker on
	ld	a, #0x08 ; +20h
	out	(0x12), a
	call	_sound_delay
;	Speaker off
	ld	a, #0x0 ; +20h
	out	(0x12), a
	call	_sound_delay
;djnz	_sound_loop
;dec	e
;jr	nz, _sound_loop
;dec	d
;jr	nz, _sound_loop
	dec	de
	ld	a, d
	or	e
	jr	nz, _sound_loop
	jr	_sound_end
	  _sound_delay:
	push	hl
	push	af
	   _sound_delay_loop:
	dec	hl
	ld	a,h
	or	l
	jr	nz, _sound_delay_loop
	pop	af
	pop	hl
	ret
	 _sound_end:
	pop	bc
	pop	de
	pop	hl
	pop	af
;ei
;../../include/arch/gl4000/sound.h:111: }
	ret
;../../include/arch/gl4000/sound.h:118: void sound_note(word n, word len) {
;	---------------------------------
; Function sound_note
; ---------------------------------
_sound_note::
	push	ix
	ld	ix,#0
	add	ix,sp
;../../include/arch/gl4000/sound.h:121: switch(n % 12) {
	ld	e, 4 (ix)
	ld	d, 5 (ix)
	push	de
	ld	hl, #0x000c
	push	hl
	push	de
	call	__moduint
	pop	af
	pop	af
	ld	c, l
	ld	b, h
	pop	de
	ld	a, #0x0b
	cp	a, c
	ld	a, #0x00
	sbc	a, b
	jr	C,00113$
	ld	b, #0x00
	ld	hl, #00121$
	add	hl, bc
	add	hl, bc
	add	hl, bc
	jp	(hl)
00121$:
	jp	00101$
	jp	00102$
	jp	00103$
	jp	00104$
	jp	00105$
	jp	00106$
	jp	00107$
	jp	00108$
	jp	00109$
	jp	00110$
	jp	00111$
	jp	00112$
;../../include/arch/gl4000/sound.h:122: case 0:	frq = 0x0900;	break;
00101$:
	ld	bc, #0x0900
	jr	00113$
;../../include/arch/gl4000/sound.h:123: case 1:	frq = 0x087e;	break;
00102$:
	ld	bc, #0x087e
	jr	00113$
;../../include/arch/gl4000/sound.h:124: case 2:	frq = 0x0804;	break;
00103$:
	ld	bc, #0x0804
	jr	00113$
;../../include/arch/gl4000/sound.h:125: case 3:	frq = 0x0791;	break;
00104$:
	ld	bc, #0x0791
	jr	00113$
;../../include/arch/gl4000/sound.h:126: case 4:	frq = 0x0724;	break;
00105$:
	ld	bc, #0x0724
	jr	00113$
;../../include/arch/gl4000/sound.h:127: case 5:	frq = 0x06be;	break;
00106$:
	ld	bc, #0x06be
	jr	00113$
;../../include/arch/gl4000/sound.h:128: case 6:	frq = 0x065d;	break;
00107$:
	ld	bc, #0x065d
	jr	00113$
;../../include/arch/gl4000/sound.h:129: case 7:	frq = 0x0601;	break;
00108$:
	ld	bc, #0x0601
	jr	00113$
;../../include/arch/gl4000/sound.h:130: case 8:	frq = 0x05ab;	break;
00109$:
	ld	bc, #0x05ab
	jr	00113$
;../../include/arch/gl4000/sound.h:131: case 9:	frq = 0x0559;	break;
00110$:
	ld	bc, #0x0559
	jr	00113$
;../../include/arch/gl4000/sound.h:132: case 10:	frq = 0x050d;	break;
00111$:
	ld	bc, #0x050d
	jr	00113$
;../../include/arch/gl4000/sound.h:133: case 11:	frq = 0x04c4;	break;
00112$:
	ld	bc, #0x04c4
;../../include/arch/gl4000/sound.h:134: }
00113$:
;../../include/arch/gl4000/sound.h:136: frq = frq >> (n/12);
	push	bc
	ld	hl, #0x000c
	push	hl
	push	de
	call	__divuint
	pop	af
	pop	af
	pop	bc
	inc	l
	jr	00123$
00122$:
	srl	b
	rr	c
00123$:
	dec	l
	jr	NZ, 00122$
;../../include/arch/gl4000/sound.h:137: len = 150 * (len / frq);	// Length to wave length, correcting for rough milliseconds
	push	bc
	push	bc
	ld	l, 6 (ix)
	ld	h, 7 (ix)
	push	hl
	call	__divuint
	pop	af
	pop	af
	pop	bc
	ld	e, l
	ld	d, h
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, de
	add	hl, hl
	add	hl, hl
	add	hl, de
	add	hl, hl
	add	hl, de
	add	hl, hl
;../../include/arch/gl4000/sound.h:138: sound_tone(frq, len);
	ld	6 (ix), l
	ld	7 (ix), h
	push	hl
	push	bc
	call	_vgl_sound_tone
	pop	af
	pop	af
;../../include/arch/gl4000/sound.h:139: }
	pop	ix
	ret
;../../include/arch/gl4000/sound.h:141: void beep() {
;	---------------------------------
; Function beep
; ---------------------------------
_beep::
;../../include/arch/gl4000/sound.h:142: sound_note(12*4+0, 0x0111);
	ld	hl, #0x0111
	push	hl
	ld	hl, #0x0030
	push	hl
	call	_sound_note
	pop	af
	pop	af
;../../include/arch/gl4000/sound.h:143: }
	ret
;../../include/arch/gl4000/system.h:39: void vgldk_init() __naked {
;	---------------------------------
; Function vgldk_init
; ---------------------------------
_vgldk_init::
;../../include/arch/gl4000/system.h:45: __endasm;
	di
;;	Set stack pointer directly above top of memory.
	ld	sp, #0xdff0 ; Load StackPointer to 0xdff0
;../../include/arch/gl4000/system.h:47: lcd_init();
	call	_lcd_init
;../../include/arch/gl4000/system.h:48: sound_off();
	call	_vgl_sound_off
;../../include/arch/gl4000/system.h:49: lcd_clear();
	call	_lcd_clear
;../../include/arch/gl4000/system.h:54: __endasm;
	jp	_main
;../../include/arch/gl4000/system.h:55: }
;../../include/stdiomin.h:73: int puts(const char *str) {
;	---------------------------------
; Function puts
; ---------------------------------
_puts::
;../../include/stdiomin.h:74: while(*str) putchar(*str++);
	pop	de
	pop	bc
	push	bc
	push	de
00101$:
	ld	a, (bc)
	or	a, a
	jr	Z,00103$
	inc	bc
	push	bc
	push	af
	inc	sp
	call	_lcd_putchar
	inc	sp
	pop	bc
	jr	00101$
00103$:
;../../include/stdiomin.h:75: putchar('\n');
	ld	a, #0x0a
	push	af
	inc	sp
	call	_lcd_putchar
	inc	sp
;../../include/stdiomin.h:76: return 1;
	ld	hl, #0x0001
;../../include/stdiomin.h:77: }
	ret
;../../include/stdiomin.h:79: void printf(const char *pc) {
;	---------------------------------
; Function printf
; ---------------------------------
_printf::
;../../include/stdiomin.h:89: while(*pc) putchar(*pc++);
	pop	de
	pop	bc
	push	bc
	push	de
00101$:
	ld	a, (bc)
	or	a, a
	ret	Z
	inc	bc
	push	bc
	push	af
	inc	sp
	call	_lcd_putchar
	inc	sp
	pop	bc
;../../include/stdiomin.h:90: }
	jr	00101$
;../../include/stdiomin.h:94: void printf_d(byte d) {
;	---------------------------------
; Function printf_d
; ---------------------------------
_printf_d::
;../../include/stdiomin.h:99: i = 100;
	ld	c, #0x64
;../../include/stdiomin.h:100: while(i > 0) {
00101$:
	ld	a, c
	or	a, a
	ret	Z
;../../include/stdiomin.h:101: putchar('0' + ((d / i) % 10));
	push	bc
	ld	a, c
	push	af
	inc	sp
	ld	hl, #5+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	call	__divuchar
	pop	af
	pop	bc
	ld	h, #0x00
	push	bc
	ld	de, #0x000a
	push	de
	push	hl
	call	__modsint
	pop	af
	pop	af
	pop	bc
	ld	a, l
	add	a, #0x30
	push	bc
	push	af
	inc	sp
	call	_lcd_putchar
	inc	sp
	pop	bc
;../../include/stdiomin.h:102: i /= 10;
	ld	b, #0x00
	ld	hl, #0x000a
	push	hl
	push	bc
	call	__divsint
	pop	af
	pop	af
	ld	c, l
;../../include/stdiomin.h:112: }
	jr	00101$
;../../include/stdiomin.h:115: char *gets(char *pc) {
;	---------------------------------
; Function gets
; ---------------------------------
_gets::
	push	ix
	ld	ix,#0
	add	ix,sp
;../../include/stdiomin.h:118: pcs = pc;
	ld	c, 4 (ix)
	ld	b, 5 (ix)
;../../include/stdiomin.h:123: while(1) {
00113$:
;../../include/stdiomin.h:124: c = getchar();
	push	bc
	call	_keyboard_getchar
	ld	a, l
	pop	bc
	ld	d, a
;../../include/stdiomin.h:129: putchar(c);
	push	bc
	push	de
	push	de
	inc	sp
	call	_lcd_putchar
	inc	sp
	pop	de
	pop	bc
;../../include/stdiomin.h:131: if ( (c == 8) || (c == 127) ) {
	ld	a,d
	cp	a,#0x08
	jr	Z,00105$
	sub	a, #0x7f
	jr	NZ,00106$
00105$:
;../../include/stdiomin.h:133: if (pc > pcs) {
	ld	a, c
	sub	a, 4 (ix)
	ld	a, b
	sbc	a, 5 (ix)
	jr	NC,00113$
;../../include/stdiomin.h:134: pc--;
	ld	l, 4 (ix)
	ld	h, 5 (ix)
	dec	hl
	ld	4 (ix), l
	ld	5 (ix), h
;../../include/stdiomin.h:142: continue;
	jr	00113$
00106$:
;../../include/stdiomin.h:118: pcs = pc;
	ld	l, 4 (ix)
	ld	h, 5 (ix)
;../../include/stdiomin.h:145: if ((c == '\n') || (c == '\r') || (c == 0)) {
	ld	a,d
	cp	a,#0x0a
	jr	Z,00108$
	cp	a,#0x0d
	jr	Z,00108$
	or	a, a
	jr	NZ,00109$
00108$:
;../../include/stdiomin.h:149: *pc = 0;
	ld	(hl), #0x00
;../../include/stdiomin.h:150: return pcs;
	ld	l, c
	ld	h, b
	jr	00115$
00109$:
;../../include/stdiomin.h:154: *pc++ = c;
	ld	(hl), d
	inc	hl
	ld	4 (ix), l
	ld	5 (ix), h
	jr	00113$
00115$:
;../../include/stdiomin.h:157: }
	pop	ix
	ret
;./bios.c:52: void bios_memset(byte *addr, byte b, word count) {
;	---------------------------------
; Function bios_memset
; ---------------------------------
_bios_memset::
;./bios.c:53: while(count > 0) {
	pop	de
	pop	bc
	push	bc
	push	de
	ld	hl, #5
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
00101$:
	ld	a, d
	or	a, e
	ret	Z
;./bios.c:54: *addr++ = b;
	ld	hl, #4+0
	add	hl, sp
	ld	a, (hl)
	ld	(bc), a
	inc	bc
;./bios.c:55: count--;
	dec	de
;./bios.c:57: }
	jr	00101$
;./bios.c:83: void bios_boot() __naked {
;	---------------------------------
; Function bios_boot
; ---------------------------------
_bios_boot::
;./bios.c:87: bint_timer = 0;
	ld	hl, #0x0000
	ld	(_bint_timer), hl
;./bios.c:97: bios_iobyte = 0;
	ld	hl,#_bios_iobyte + 0
	ld	(hl), #0x00
;./bios.c:100: bios_curdsk = 0;
	ld	hl,#_bios_curdsk + 0
	ld	(hl), #0x00
;./bios.c:104: lcd_init();
	call	_lcd_init
;./bios.c:105: sound_off();
	call	_vgl_sound_off
;./bios.c:113: puts(CPM_VERSION);
	ld	hl, #___str_16
	push	hl
	call	_puts
	pop	af
;./bios.c:115: sound_note(12*4, 250);
	ld	hl, #0x00fa
	push	hl
	ld	l, #0x30
	push	hl
	call	_sound_note
	pop	af
	pop	af
;./bios.c:120: __endasm;
	jp	_bios_wboot
;./bios.c:122: }
;./bios.c:125: void bios_wboot() __naked {
;	---------------------------------
; Function bios_wboot
; ---------------------------------
_bios_wboot::
;./bios.c:151: bios_curdsk = 0;
	ld	hl,#_bios_curdsk + 0
	ld	(hl), #0x00
;./bios.c:152: bios_dma = (byte *)0x0080;
	ld	hl, #0x0080
	ld	(_bios_dma), hl
;./bios.c:154: bios_memset(bios_dma, 0x1a, 0x80);	// Fill DMA area with EOFs
	ld	l, #0x80
	push	hl
	ld	a, #0x1a
	push	af
	inc	sp
	ld	l, #0x80
	push	hl
	call	_bios_memset
	pop	af
	pop	af
	inc	sp
;./bios.c:157: bios_trk = 0;
	ld	hl, #0x0000
	ld	(_bios_trk), hl
;./bios.c:158: bios_sec = 1;
	ld	l, #0x01
	ld	(_bios_sec), hl
;./bios.c:170: __endasm;
	ld	c, #0
	jp	_bdos
;./bios.c:171: }
;./bios.c:174: byte bios_const() {
;	---------------------------------
; Function bios_const
; ---------------------------------
_bios_const::
;./bios.c:179: if (keyboard_checkkey() > 0) return 0xff;	// Key pressed
	call	_keyboard_checkkey
	ld	a, l
	or	a, a
;./bios.c:180: return 0x00;	// No key pressed
	ld	l, #0xff
	ret	NZ
	ld	l, #0x00
;./bios.c:181: }
	ret
___str_16:
	.ascii "32K VGL CP/M 2.0.0"
	.db 0x0a
	.ascii "2023-08-15 by HotKey"
	.db 0x00
;./bios.c:183: byte bios_conin() {
;	---------------------------------
; Function bios_conin
; ---------------------------------
_bios_conin::
;./bios.c:187: return keyboard_getchar();
;./bios.c:188: }
	jp	_keyboard_getchar
;./bios.c:190: void bios_conout(byte c) {
;	---------------------------------
; Function bios_conout
; ---------------------------------
_bios_conout::
;./bios.c:194: putchar(c);
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	call	_lcd_putchar
	inc	sp
;./bios.c:195: }
	ret
;./bios.c:197: void bios_list(byte c) {
;	---------------------------------
; Function bios_list
; ---------------------------------
_bios_list::
;./bios.c:203: putchar(c);
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	call	_lcd_putchar
	inc	sp
;./bios.c:204: }
	ret
;./bios.c:206: void bios_punch(byte c) {
;	---------------------------------
; Function bios_punch
; ---------------------------------
_bios_punch::
;./bios.c:212: putchar(c);
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	call	_lcd_putchar
	inc	sp
;./bios.c:213: }
	ret
;./bios.c:215: byte bios_reader() {
;	---------------------------------
; Function bios_reader
; ---------------------------------
_bios_reader::
;./bios.c:221: return getchar();
;./bios.c:222: }
	jp	_keyboard_getchar
;./bios.c:224: void bios_home() {
;	---------------------------------
; Function bios_home
; ---------------------------------
_bios_home::
;./bios.c:226: bios_trk = 0;
	ld	hl, #0x0000
	ld	(_bios_trk), hl
;./bios.c:227: bios_sec = 1;
	ld	l, #0x01
	ld	(_bios_sec), hl
;./bios.c:228: }
	ret
;./bios.c:230: DPH *bios_seldsk(byte n) {
;	---------------------------------
; Function bios_seldsk
; ---------------------------------
_bios_seldsk::
;./bios.c:237: bios_curdsk = n;
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	ld	(_bios_curdsk+0), a
;./bios.c:238: return &bios_dummy_dph;
	ld	hl, #_bios_dummy_dph
;./bios.c:239: }
	ret
;./bios.c:241: void bios_settrk(word t) {
;	---------------------------------
; Function bios_settrk
; ---------------------------------
_bios_settrk::
;./bios.c:243: bios_trk = t;
	ld	iy, #2
	add	iy, sp
	ld	a, 0 (iy)
	ld	(_bios_trk+0), a
	ld	a, 1 (iy)
	ld	(_bios_trk+1), a
;./bios.c:244: }
	ret
;./bios.c:246: void bios_setsec(word s) {
;	---------------------------------
; Function bios_setsec
; ---------------------------------
_bios_setsec::
;./bios.c:248: bios_sec = s;
	ld	iy, #2
	add	iy, sp
	ld	a, 0 (iy)
	ld	(_bios_sec+0), a
	ld	a, 1 (iy)
	ld	(_bios_sec+1), a
;./bios.c:249: }
	ret
;./bios.c:251: void bios_setdma(byte *a) {
;	---------------------------------
; Function bios_setdma
; ---------------------------------
_bios_setdma::
;./bios.c:253: bios_dma = a;
	ld	iy, #2
	add	iy, sp
	ld	a, 0 (iy)
	ld	(_bios_dma+0), a
	ld	a, 1 (iy)
	ld	(_bios_dma+1), a
;./bios.c:254: }
	ret
;./bios.c:256: byte bios_read() {
;	---------------------------------
; Function bios_read
; ---------------------------------
_bios_read::
;./bios.c:259: return 0;
	ld	l, #0x00
;./bios.c:260: }
	ret
;./bios.c:262: byte bios_write(byte c) {
;	---------------------------------
; Function bios_write
; ---------------------------------
_bios_write::
;./bios.c:267: return 0;
	ld	l, #0x00
;./bios.c:268: }
	ret
;./bios.c:270: byte bios_listst() {
;	---------------------------------
; Function bios_listst
; ---------------------------------
_bios_listst::
;./bios.c:272: return 0xff;
	ld	l, #0xff
;./bios.c:273: }
	ret
;./bios.c:275: word bios_sectran(word n, byte *a) {
;	---------------------------------
; Function bios_sectran
; ---------------------------------
_bios_sectran::
;./bios.c:280: return n;
	pop	bc
	pop	hl
	push	hl
	push	bc
;./bios.c:281: }
	ret
;./bios.c:284: void bios() __naked {
;	---------------------------------
; Function bios
; ---------------------------------
_bios::
;./bios.c:309: __endasm;
	jp	_bios_boot
	jp	_bios_wboot
	jp	_bios_const
	jp	_bios_conin
	jp	_bios_conout
	jp	_bios_list
	jp	_bios_punch
	jp	_bios_reader
	jp	_bios_home
	jp	_bios_seldsk
	jp	_bios_settrk
	jp	_bios_setsec
	jp	_bios_setdma
	jp	_bios_read
	jp	_bios_write
;	CP/M 2
	jp	_bios_listst
	jp	_bios_sectran
;CP/M	3:
;./bios.c:310: }
;./bint.c:11: void bint() __naked {
;	---------------------------------
; Function bint
; ---------------------------------
_bint::
;./bint.c:21: __endasm;
;.asciz	'[BINT]' ; Marker to find segment in binary
	push	af
	push	bc
	push	de
	push	hl
	push	ix
	push	iy
;./bint.c:25: bint_timer ++;
	ld	bc, (_bint_timer)
	inc	bc
	ld	(_bint_timer), bc
;./bint.c:63: __endasm;
	pop	iy
	pop	ix
	pop	hl
	pop	de
	pop	bc
	pop	af
;./bint.c:71: __endasm;
	ei
	reti
;.asciz	'[BINT end]'
;./bint.c:72: }
;./cpm.c:22: void main() __naked {
;	---------------------------------
; Function main
; ---------------------------------
_main::
;./cpm.c:26: __endasm;
	jp	_bios_boot
;./cpm.c:27: }
	.area _CODE
	.area _INITIALIZER
__xinit__lcd_x:
	.db #0x00	; 0
__xinit__lcd_y:
	.db #0x00	; 0
__xinit__lcd_cursor:
	.db #0x01	; 1
__xinit__keyboard_getchar_last:
	.db #0x00	; 0
	.area _CABS (ABS)
