#ifndef __SOFTIO_H
#define __SOFTIO_H
/*
VTech Genius Leader "Soft I/O"

Access to display and keyboard using the onboard ROM functions / MMIO

It is dependent on the model it is actually run.

2020-01-24 Bernhard "HotKey" Slawik
*/


/*
// System-mapped values
extern char SYSTEM_TYPE_TEXT[4] @ 0x7be8;	// May contain "TEXT" after out(01h),07h
extern byte SYSTEM_ARCHITECTURE_0006 @ 0x0006;	// Used to determine the system architecture
extern byte SYSTEM_ARCHITECTURE_0038 @ 0x0038;	// Used to determine the system architecture (and the 30 bytes after that, in 3 byte increments). Checked for being 0xc3 or not
extern byte SYSTEM_ARCHITECTURE_4000 @ 0x4000;	// Used to determine the system architecture
extern byte SYSTEM_ARCHITECTURE_801E @ 0x801e;	// Used to determine the ROM type
*/



#if (VGLDK_SERIES == 2000) || (VGLDK_SERIES == 1000)
	// MODEL2000
	#define LCD_COLS 20
	#define LCD_ROWS 2
	volatile __at (0xdca0) byte LCD_VRAM[20*2];
	volatile __at (0xdca0) byte LCD_VRAM_ROW0[20];	// ... 0xdcb3
	volatile __at (0xdcb4) byte LCD_VRAM_ROW1[20];	// ... 0xdcc7

	//volatile __at (0xdce0) byte KEY_STATUS;	// Controls reading from the keyboard on 2000 (put 0xc0 into it, wait for it to become 0xd0)
	//volatile __at (0xdce4) byte KEY_CURRENT;	// Holds the current key code on 2000
	volatile __at (0xdceb) byte LCD_REFRESH_ROWS[2];
	volatile __at (0xdceb) byte LCD_REFRESH_ROW0;	// MODEL2000: Refreshes display of VRAM_ROW0 when 1 is put there
	volatile __at (0xdcec) byte LCD_REFRESH_ROW1;	// MODEL2000: Refreshes display of VRAM_ROW1 when 1 is put there
	volatile __at (0xdced) byte LCD_CURSOR_MODE;	// Show cursor (0=off, 1=block 2=line)
	volatile __at (0xdcef) byte LCD_CURSOR_COL;	// Current column + 64*row
#endif

#if (VGLDK_SERIES == 4000)
	// MODEL4000
	#define LCD_COLS 20
	#define LCD_ROWS 4

	volatile __at (0xdca0) byte LCD_VRAM[20*4];
	volatile __at (0xdca0) byte LCD_VRAM_ROW0[20];	// ... 0xdcb3
	volatile __at (0xdcb4) byte LCD_VRAM_ROW1[20];	// ... 0xdcc7
	volatile __at (0xdcc8) byte LCD_VRAM_ROW2[20];	// ... 0xdcdb	// MODEL4000 only (MODEL2000 stores cursor data there)
	volatile __at (0xdcdc) byte LCD_VRAM_ROW3[20];	// ... 0xdcef	// MODEL4000 only (MODEL2000 stores cursor data there)

	//volatile __at (0xdb00) byte KEY_STATUS;	// Controls reading from the keyboard on 4000 (put 0xc0 into it, wait for it to become 0xd0)
	//volatile __at (0xdb01) byte KEY_CURRENT;	// Holds the current key code on 4000
	volatile __at (0xdcf0) byte LCD_REFRESH_ROWS[4];
	volatile __at (0xdcf0) byte LCD_REFRESH_ROW0;	// MODEL4000: Refreshes display of VRAM_ROW0 when 0 is put there
	volatile __at (0xdcf1) byte LCD_REFRESH_ROW1;	// MODEL4000: Refreshes display of VRAM_ROW0 when 1 is put there
	volatile __at (0xdcf2) byte LCD_REFRESH_ROW2;	// MODEL4000: Refreshes display of VRAM_ROW0 when 2 is put there
	volatile __at (0xdcf3) byte LCD_REFRESH_ROW3;	// MODEL4000: Refreshes display of VRAM_ROW0 when 3 is put there
	volatile __at (0xdcf4) byte LCD_CURSOR_COL;
	volatile __at (0xdcf5) byte LCD_CURSOR_ROW;
	volatile __at (0xdcf6) byte LCD_CURSOR_MODE;	// Show cursor (0=off, 1=block 2=line)
#endif




// Key codes
#define KEY_NONE	0
#define KEY_ASTERISK	0x2a	// Asterisk (top left key) without modifiers
#define KEY_BREAK	0x60	// Asterisk (top left key) + Shift

#define KEY_HELP	0x83	// HELP/TAB without modifiers
#define KEY_TAB		0x82	// HELP/TAB + Shift

#define KEY_PLAYER_LEFT	0x7b	// LEFT PLAYER without modifiers
#define KEY_PLAYER_RIGHT	0x7f	// RIGHT PLAYER without modifiers
//#define KEY_PLAYER_LEFT	0x5b	// LEFT PLAYER + Shift
//#define KEY_PLAYER_RIGHT	0x5f	// RIGHT PLAYER + Shift
//#define KEY_INS	0x7b
//#define KEY_DEL	0x7f

#define KEY_CURSOR_RIGHT	0xf0
#define KEY_CURSOR_LEFT	0xf1
#define KEY_CURSOR_UP	0xf2	// uUml + Alt
#define KEY_CURSOR_DOWN	0xf3	// aUml + Alt
#define KEY_BACKSPACE_X	0xf4	// oUml + Alt

#define KEY_ANSWER	0x2b	// +/ANSWER (does not change with Shift)
#define KEY_ENTER	0x7c

#define KEY_REPEAT	0x7e	// REPEAT/DELETE without modifiers
#define KEY_DELETE	0x5C	// REPEAT/DELETE + Shift

#define KEY_ACTIVITY_1	0x01	// First activity selection foil button
#define KEY_ACTIVITY_2	0x02
#define KEY_ACTIVITY_3	0x03
#define KEY_ACTIVITY_4	0x04
#define KEY_ACTIVITY_5	0x05
#define KEY_ACTIVITY_6	0x06
#define KEY_ACTIVITY_7	0x07
#define KEY_ACTIVITY_8	0x08
#define KEY_ACTIVITY_9	0x09
#define KEY_ACTIVITY_10	0x0a
#define KEY_ACTIVITY_11	0x0b
#define KEY_ACTIVITY_12	0x0c
#define KEY_ACTIVITY_13	0x0d
#define KEY_ACTIVITY_14	0x0e
#define KEY_ACTIVITY_15	0x0f
#define KEY_ACTIVITY_16	0x10
#define KEY_ACTIVITY_17	0x11
#define KEY_ACTIVITY_18	0x12
#define KEY_ACTIVITY_19	0x13
#define KEY_ACTIVITY_20	0x14
#define KEY_ACTIVITY_21	0x15
#define KEY_ACTIVITY_22	0x16
#define KEY_ACTIVITY_23	0x17
#define KEY_ACTIVITY_24	0x18
#define KEY_ACTIVITY_25	0x19
#define KEY_ACTIVITY_26	0x1a
#define KEY_ACTIVITY_27	0x1b
#define KEY_ACTIVITY_28	0x1c
#define KEY_ACTIVITY_29	0x1d
#define KEY_ACTIVITY_30	0x1e
#define KEY_ACTIVITY_31	0x1f	// This is the last one they could fit below valid ASCII codes
#define KEY_ACTIVITY_32	0xB0	// This is where they continue (0xb0)
#define KEY_ACTIVITY_33	0xB1
#define KEY_ACTIVITY_34	0xB2
#define KEY_ACTIVITY_35	0xB3	// Last activity selection foil button

#define KEY_POWER_OFF	0xB4	// OFF foil button
#define KEY_LEVEL	0xB5	// LEVEL foil button
#define KEY_PLAYERS	0xB6	// PLAYERS foil button
#define KEY_DISKETTE	0xB7	// DISKETTE foil button

/*
void delay(word cycles) {
	(void)cycles;	// Suppress warning "unreferenced function argument"
	
__asm
	push	hl
	push	af
	
	; Get parameter from stack into a
	ld hl,#0x0002
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l, a
	
	_delay_loop:
		dec	hl
		ld	a,h
		or	l
		jr	nz, _delay_loop
	pop	af
	pop	hl
__endasm;
}
*/


// Mandatory stdio defines
char getchar() {
	KEY_STATUS = 0xc0;
	while (KEY_STATUS != 0xd0) {}
	
	// Translate keys
	switch(KEY_CURRENT) {
		case KEY_ENTER: return 10;
		case KEY_BACKSPACE_X: return 8;
		case KEY_DELETE: return 8;
	}
	
	return KEY_CURRENT;
}


//////////////////////////////////////////

// HDD44780 definitions
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define VGL_LCD_CONTROL_PORT 0x0a
#define VGL_LCD_DATA_PORT 0x0b

byte vgl_cursorX = 0;
byte vgl_cursorY = 0;

void vgl_delay_1fff() {
__asm
	; Used for screen functions (after putting stuff to ports 0x0a or 0x0b)
	push	hl
	ld	hl, #0x1fff
_delay_1fff_loop:
	dec	l
	jr	nz, _delay_1fff_loop
	dec	h
	jr	nz, _delay_1fff_loop
	pop	hl
__endasm;
}


void vgl_delay_010f() {
__asm
	; Used for screen functions (after putting stuff to ports 0x0a or 0x0b)
	push	hl
	ld	hl, #0x010f
_delay_010f_loop:
	dec	l
	jr	nz, _delay_010f_loop
	dec	h
	jr	nz, _delay_010f_loop
	pop	hl
__endasm;
}




void vgl_lcd_writeControl(byte a) __naked {	// aka "_out_0x0a"
	(void)a;	// suppress warning "unreferenced function argument"
	
	__asm
		push hl
		
		; Get parameter from stack into a
		ld hl,#0x0004
		add hl,sp
		ld a,(hl)
		
		; Put it to port
		out	(VGL_LCD_CONTROL_PORT), a
		
		call _vgl_delay_010f
		call _vgl_delay_010f
		
		pop hl
		ret
	__endasm;
}

void vgl_lcd_writeData(byte a) __naked {	// aka "_out_0x0b"
	(void)a;	// suppress warning "unreferenced function argument"
	
	__asm
		push hl
		
		; Get parameter from stack into a
		ld hl,#0x0004
		add hl,sp
		ld a,(hl)
		
		; Put it to port
		out	(VGL_LCD_DATA_PORT), a
		
		;call _vgl_delay_1fff
		call _vgl_delay_010f
		
		pop hl
		ret
	__endasm;
}



void vgl_lcd_scroll() {
	//vgl_lcd_writeControl(0x01);	// Clear
	//vgl_lcd_writeControl(0x02);	// Home
	//memset(LCD_VRAM, 0, LCD_COLS*LCD_ROWS);
	byte x, y;
	byte *cUp = LCD_VRAM_ROW0;
	byte *cDown = LCD_VRAM_ROW1;
	for (y = 0; y < LCD_ROWS-1; y++) {
		for (x = 0; x < LCD_COLS; x++) {
			*cUp = *cDown;
			cUp++;
			cDown++;
		}
		LCD_REFRESH_ROWS[y] = 1;
	}
	for (x = 0; x < LCD_COLS; x++) {
		*cUp++ = 0x20;
	}
	LCD_REFRESH_ROWS[LCD_ROWS-1] = 1;
}
void vgl_lcd_clear() {
	
	byte x, y;
	byte *c;
	
	vgl_lcd_writeControl(LCD_CLEARDISPLAY);
	//vgl_delay_1fff();
	vgl_lcd_writeControl(LCD_RETURNHOME);
	vgl_delay_1fff();
	
	//memset(LCD_VRAM, 0, LCD_COLS*LCD_ROWS);
	
	
	c = LCD_VRAM;
	for (y = 0; y < LCD_ROWS; y++) {
		for (x = 0; x < LCD_COLS; x++) {
			*c++ = 0x20;
		}
	}
	for (y = 0; y < LCD_ROWS; y++) {
		LCD_REFRESH_ROWS[y] = 1;
	}
	
	vgl_cursorX = 0;
	vgl_cursorY = 0;
}



void vgl_lcd_init() {
	// This performs the LCD initialization
	// as seen in several ROMs and in the HD44780 data sheet itself
	
	// Enter 8 bit mode
	vgl_lcd_writeControl(0x38);
	vgl_lcd_writeControl(0x38);
	vgl_lcd_writeControl(0x38);
	vgl_lcd_writeControl(0x38);
	
	// CLS
	vgl_lcd_writeControl(LCD_CLEARDISPLAY);
	
	//vgl_lcd_cursor = 0;
	//vgl_lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
	
	//vgl_lcd_cursor = 1;
	vgl_lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);
	
	vgl_lcd_writeControl(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT );
	
	vgl_lcd_clear();
	
	// First byte is missing if we do not delay enough
	vgl_delay_1fff();
	
	//vgl_cursorX = 0;
	//vgl_cursorY = 0;
}



void vgl_lcd_putchar(char c) {
	
	// Directly to RAM
	LCD_VRAM[(vgl_cursorY * LCD_COLS) + vgl_cursorX] = c;
	LCD_REFRESH_ROWS[vgl_cursorY] = 1;
	
	vgl_cursorX++;
	if (vgl_cursorX >= LCD_COLS) {
		vgl_cursorX -= LCD_COLS;
		vgl_cursorY++;
		
		while (vgl_cursorY >= LCD_ROWS) {
			// Clear
			//vgl_cursorY = 0;
			//vgl_lcd_clear();
			
			// Scroll
			vgl_lcd_scroll();
			vgl_cursorY--;
		}
	}
	
	// Update cursor
	vgl_lcd_writeControl(0x08 + 0x04 + 0x02 + 0x01);
	
}



void putchar(char c) {
	vgl_lcd_putchar(c);
}


/*
byte sys_getType() {
	
	// Returns 2 for MODEL2000, 3 for MODEL4000
	
	
	// checkArchitecture1(): Check 10x byte at [0038 + x*3]. If all are 0xc3 return A=3, else return A=2
	#asm
		push	bc
		push	de
		push	hl
	
		ld	de, 3h
		ld	b, 0ah
		ld	hl, _SYSTEM_ARCHITECTURE_0038
		
		_ca1_loop:
			ld	a, (hl)
			cp	0c3h	; check if [HL] == 0xc3	(This is the case for MODEL2000 and MODEL4000)
			jr	nz, _ca1_ret2	; if not: set a:=2 and return
			add	hl, de	; increment by 3
		djnz	_ca1_loop
		
		; They have all evaluated to 0xc3
		ld	a, 03h	; set a:=3
		jr	_ca1_retA
		
		_ca1_ret2:
		ld	a, 02h
		
		_ca1_retA:
		pop	hl
		pop	de
		pop	bc
		
		;ret
		
		; Store result
		xor	h
		ld	l, a
	#endasm
	
	// checkArchitecture2(A): call checkArchitecture3_A2/3 accordingly
	#asm
		push	af
		push	hl
		; The next check depends on the previous result (A=2 or A=3)
		cp	02h
		jr	z, _ca3A2	; if a == 2: checkArchitecture3_A2(): Probe CPU and ROM, continue at checkArchitecture5()
		call	_ca3A3	; checkArchitecture3A3(): Set A to ([0x0006] & 0x7f >> 4)
		jr	_ca5
	#endasm
	
	// checkArchitecture3_A2(): Probe CPU and ROM, continue at checkArchitecture5()
	#asm
		_ca3A2:
			call	_ca4A2	; checkArchitecture4_A2(): Probe CPU (out (0x01), 0x01), check [0x4003]: Return A=1 for 0x02 and A=2 for 0x01 else A=that value
			jr	_ca5	; checkArchitecture5(): Probe ROM header, reset if 0x801e <> A
	#endasm

	// checkArchitecture3_A3(): Set A to ([0x0006] & 0x7f >> 4)
	#asm
		_ca3A3:
			ld	a, (_SYSTEM_ARCHITECTURE_0006)
			and	7fh
			srl	a
			srl	a
			srl	a
			srl	a
		ret
	#endasm
	
	
	// checkArchitecture4_A2(): Probe CPU (out (0x01), 0x01), check [0x4003]: Return A=1 for 0x02 and A=2 for 0x01 else A=that value
	#asm
		_ca4A2:
			push	hl
			ld	hl, _SYSTEM_ARCHITECTURE_4000
			ld	a, 01h	; out 0x01, 0x01: Get model? language?
			out	(01h), a
			push	bc
			ld	bc, 0003h
			add	hl, bc
			pop	bc
			ld	a, (hl)	; get value at 0x4003
			cp	02h
			jr	z, _ca4A2_ret1	; return 1
			cp	01h
			jr	z, _ca4A2_ret2	; return 2
			jr	_ca4A2_retA	; break
			
		_ca4A2_ret1:
			ld	a,01h
			jr	_ca4A2_retA	; break
			
		_ca4A2_ret2:
			ld	a,02h
			
		_ca4A2_retA:
			pop	hl
		ret
	#endasm
	
	// checkArchitecture5(): Probe ROM header, reset if 0x801e <> A
	#asm
		_ca5:
			ld	hl, _SYSTEM_ARCHITECTURE_801E	; This is inside the header at the beginning of the ROM, Right after the PC-PROGCARD-text
			cp	(hl)
			jp	nz,0000h	; reset if [801e] > 0 (it is in fact 01h)
			pop	hl
			pop	af
		
		_ca_end:
			; Store result
			xor	h
			ld	l, a
	#endasm
}
*/

//#include <hardware/lcd.h>
void softio_init() {
	
	//lcd_init();
	//vgl_lcd_init();
	
	
	/*
	vgl_lcd_init();
	
	
	sys_type = 0;
	
	screen_cols = 20;
	screen_rows = 1;
	
	// Determine the system architecture so we know how to output stuff
	sys_type = sys_getType();
	
	switch(sys_type) {
		case 2:	// MODEL2000
			screen_rows = 2;
			break;
		
		case 3:	// MODEL4000
			screen_rows = 4;
			break;
	}
	
	screen_size = screen_cols * screen_rows;
	screen_scrollSize = screen_cols * (screen_rows-1);
	
	cursor_col = 0;
	cursor_row = 0;
	cursor_ofs = 0;
	
	//screen_reset();
	*/
	
	
	__asm
		ei	; We need interrupts so the keyboard gets polled
	__endasm;

}

#endif // __SOFTIO_H