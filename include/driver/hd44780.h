#ifndef __HD44780_H
#define __HD44780_H
/*
LCD code for the HD44780 LCD controller

For more info about the LCD see MAME code:
	https://github.com/mamedev/mame/blob/master/src/devices/video/hd44780.cpp
	https://github.com/mamedev/mame/blob/master/src/.../pc2000.cpp

...or other HDD44780 libraries:
	https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.h


TODO:
	Implement proper "lcd_busy()" check as described in:
	http://www.8051projects.net/lcd-interfacing/commands.php


2020-01-21 Bernhard "HotKey" Slawik
*/


#define byte unsigned char
#define word unsigned short

#define clear lcd_clear	// Make it available as well known "clear()" command

// HDD44780 specific definitions
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

// Define port accesses

//#define LCD_PORT_CTRL 0x0a	// pc1000: 0x20, gl2000/gl4000: 0x0a
#ifndef LCD_PORT_CTRL
	#error "LCD_PORT_CTRL must be defined in order to use HD44780 LCD"
#endif
__sfr __at LCD_PORT_CTRL lcd_controlPort;

//#define LCD_PORT_DATA 0x0b	// pc1000: 0x21, gl2000/gl4000: 0x0b
#ifndef LCD_PORT_DATA
	#error "LCD_PORT_DATA must be defined in order to use HD44780 LCD"
#endif
__sfr __at LCD_PORT_DATA lcd_dataPort;


// Mapping of screen coordinates to HDD44780 DDRAM addresses
//#define LCD_ROWS 4	// pc1000: 1, gl2000: 2, gl4000: 4
//#define LCD_COLS 20
#if LCD_ROWS == 1
	//@FIXME: Not yet working correctly...
	const byte lcd_map_1row[LCD_ROWS * LCD_COLS + 1] = {
		0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,	16,	17,	18,	19,
		19	// Double
	};
	#define lcd_map lcd_map_1row
	
#elif LCD_ROWS == 2
	const byte lcd_map_2rows[LCD_ROWS * LCD_COLS + 1] = {
		0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,	16,	17,	18,	19,
		64,	65,	66,	67,	68,	69,	70,	71,	72,	73,	74,	75,	76,	77,	78,	79,	80,	81,	82,	83,
		83	// Double
	};
	#define lcd_map lcd_map_2rows
	
#elif LCD_ROWS == 4
	const byte lcd_map_4rows[LCD_ROWS * LCD_COLS + 1] = {
		0,	1,	2,	3,	8,	9,	10,	11,	12,	13,	14,	15,	24,	25,	26,	27,	28,	29,	30,	31,
		64,	65,	66,	67,	72,	73,	74,	75,	76,	77,	78,	79,	88,	89,	90,	91,	92,	93,	94,	95,
		4,	5,	6,	7,	16,	17,	18,	19,	20,	21,	22,	23,	32,	33,	34,	35,	36,	37,	38,	39,
		68,	69,	70,	71,	80,	81,	82,	83,	84,	85,	86,	87,	96,	97,	98, 99,	100,	101,	102,	103,
		103	// Double
	};
	#define lcd_map lcd_map_4rows
	
#else
	#error "A sensible value for LCD_ROWS must be defined in order to use HD44780 LCD"
#endif


byte lcd_x = 0;
byte lcd_y = 0;

#ifndef lcd_MINIMAL
	// "Minimal" version does not include a screen buffer and custom scrolling
	byte lcd_cursor = 1;
	byte lcd_buffer[LCD_ROWS * LCD_COLS];
	void (*lcd_scroll_cb)(void);	// Scroll callback. Can be set to a function that is executed when cursor reaches bottom of screen
#endif

void lcd_delay_long() __naked {
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
	ret
__endasm;
}


void lcd_delay_short() __naked {
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
	ret
__endasm;
}


void lcd_writeControl(byte a) {
	lcd_controlPort = a;	// Output value to LCD control port
	
	//@TODO: Tune these!
	lcd_delay_short();
	lcd_delay_short();
}

void lcd_writeData(byte a) {
	lcd_dataPort = a;	// Output value to LCD data port
	
	//@TODO: Tune these!
	//_lcd_delay_long();
	lcd_delay_short();
}

/*
void lcd_cursor_on() {
	lcd_cursor = 1;
	lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);
}
void lcd_cursor_off() {
	lcd_cursor = 0;
	lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
}
*/

#ifndef lcd_MINIMAL
void lcd_set_cursor() {
	// Set cursor to position
	byte o;
	
	o = lcd_x + (lcd_y * LCD_COLS);
	lcd_writeControl(LCD_SETDDRAMADDR | lcd_map[o]);
	
	if (lcd_cursor) lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);
}
#endif

void lcd_clear() {
	#ifndef lcd_MINIMAL
	byte i;
	#endif
	
	lcd_writeControl(LCD_CLEARDISPLAY);
	//lcd_delay_long();
	lcd_writeControl(LCD_RETURNHOME);
	lcd_delay_long();
	
	lcd_x = 0;
	lcd_y = 0;
	
	#ifndef lcd_MINIMAL
	//@TODO: Use fillmem function!
	for(i = 0; i < (LCD_COLS * LCD_ROWS); i++) {
		lcd_buffer[i] = 0x20;
	}
	
	lcd_set_cursor();
	#endif
}


#ifndef lcd_MINIMAL
void lcd_refresh() {
	// Put buffer to screen
	byte *p0;
	byte o;
	byte i;
	
	lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
	
	p0 = &lcd_buffer[0];
	o = 0;
	for(i = 0; i < (LCD_COLS * LCD_ROWS); i++) {
		lcd_writeControl(LCD_SETDDRAMADDR | lcd_map[o]);
		lcd_writeData(*p0++);
		o++;
	}
	
	lcd_set_cursor();
	
	lcd_delay_long();
}


void lcd_scroll() {
	byte i;
	byte *p0;
	#if LCD_ROWS > 1
	byte *p1;
	#endif
	
	// Move buffer up one line
	
	/*
	@TODO: Use "ldir" opcode to do the copy!:
		__asm
		ld	bc, (_screen_scrollSize)
		ld	hl, __lcd_buffer_row1
		ld	de, __lcd_buffer_row0
		ldir	; Copy BC chars from (HL) to (DE)
		__endasm;
	*/
	
	p0 = &lcd_buffer[0];
	
	#if LCD_ROWS > 1
	// Copy from row 1 to row 0
	//@TODO: memcpy(dst, src, l)
	p1 = &lcd_buffer[LCD_COLS];
	for(i = 0; i < (LCD_COLS * (LCD_ROWS-1)); i++) {
		*p0++ = *p1++;
	}
	#endif
	
	// Fill last row
	//@TODO: memset(dst, what, count)
	for(i = 0; i < LCD_COLS; i++) {
		*p0++ = 0x20;	// Fill with spaces
	}
	
	lcd_refresh();
}
#endif


void lcd_init() {
	// This performs the LCD initialization
	// as seen in several ROMs and in the HD44780 data sheet itself.
	// Most important fact is that a delay after each port access is MANDATORY.
	// GL4000 05b4
	
	// Enter 8 bit mode
	lcd_writeControl(0x38);	//Function set: 2 Line, 8-bit, 5x7 dots
	lcd_writeControl(0x38);
	lcd_writeControl(0x38);
	lcd_writeControl(0x38);
	
	// CLS
	lcd_writeControl(LCD_CLEARDISPLAY);
	
	//lcd_cursor = 0;
	//lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
	
	//lcd_cursor = 1;
	lcd_writeControl(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);
	
	lcd_writeControl(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT );
	
	
	// First byte is missing if we do not delay enough
	//lcd_delay_long();
	
	
	//lcd_x = 0;
	//lcd_y = 0;
	
	#ifndef lcd_MINIMAL
	lcd_cursor = 1;
	lcd_scroll_cb = 0;
	#endif
	
	lcd_clear();
	
	/*
	// CLS
	lcd_writeControl(LCD_CLEARDISPLAY);
	
	// Home
	lcd_writeControl(LCD_RETURNHOME);
	
	lcd_x = 0;
	lcd_y = 0;
	for(i = 0; i < (LCD_COLS * LCD_ROWS); i++) {
		lcd_buffer[i] = 0x20;
	}
	//lcd_update();
	*/
	//lcd_delay_long();
}

/*
void lcd_scroll_cb_scroll() {
	while(lcd_y >= LCD_ROWS) {
		lcd_y--;
		lcd_scroll();
	}
}
*/

void lcd_putchar_at(byte x, byte y, char c) {
	byte o;
	
	// Calculate DDRAM offset
	o = x + (y * LCD_COLS);
	
	// Set DDRAM insert point for data
	lcd_writeControl(LCD_SETDDRAMADDR | lcd_map[o]);
	
	// Actually display
	lcd_writeData(c);
	
	/*
	#ifndef lcd_MINIMAL
	// Store in buffer (for scrolling)
	lcd_buffer[o] = c;
	
	lcd_set_cursor();
	#endif
	*/
}

void lcd_putchar(byte c) {
	byte o;
	
	if (c == '\r') {
		// Carriage return
		lcd_x = 0;
		c = 0;
	}
	else
	if (c == '\n') {
		// New line
		lcd_x = 0;
		lcd_y++;
		c = 0;
	}
	else
	if (c == 8) {
		// Backspace
		if (lcd_x > 0) lcd_x--;
		c = 0;
	}
	
	
	if (lcd_x >= LCD_COLS) {
		lcd_x = 0;
		lcd_y++;
	}
	
	if (lcd_y >= LCD_ROWS) {
		#ifndef lcd_MINIMAL
		// Invoke scroll callback
		if (lcd_scroll_cb != 0)
			(*lcd_scroll_cb)();
		else
			while(lcd_y >= LCD_ROWS) {
				lcd_y--;
				lcd_scroll();
			}
		#else
			// Minimal
			lcd_y = 0;
		#endif
	}
	
	//lcd_putchar_at(lcd_x, lcd_y, c);
	
	// Calculate DDRAM offset
	o = lcd_x + (lcd_y * LCD_COLS);
	
	// Set DDRAM insert point for data
	lcd_writeControl(LCD_SETDDRAMADDR | lcd_map[o]);
	
	if (c > 0) {
		// Actually display
		lcd_writeData(c);
		lcd_x++;
		
		#ifndef lcd_MINIMAL
		// Store in buffer (for scrolling)
		lcd_buffer[o] = c;
		
		lcd_set_cursor();
		#endif
	}
	
}

#endif //__HD44780_H