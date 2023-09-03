#ifndef __LCD_H
#define __LCD_H

/*
from MAME:src/mame/drivers/prestige.cpp:
ports 0x30...0x3f
		case 0x02:
			m_lcdc.lcd_w = data;
			break;
		case 0x04:
			m_lcdc.lcd_h = data;
			break;
		case 0x06:
			m_lcdc.addr1 = (m_lcdc.addr1 & 0xff00) | data;
			break;
		case 0x07:
			m_lcdc.addr1 = (m_lcdc.addr1 & 0x00ff) | (data << 8);
			break;
		case 0x08:
			m_lcdc.addr2 = (m_lcdc.addr2 & 0xff00) | data;
			break;
		case 0x09:
			m_lcdc.addr2 = (m_lcdc.addr2 & 0x00ff) | (data << 8);
			break;
		case 0x0a:
			m_lcdc.split_pos = data;
			break;
		case 0x0d:
			m_lcdc.fb_width = data;
*/

// Hardware parameters
#define LCD_W 240
#define LCD_H 100
#define LCD_ADDR 0xe000

const byte lcd_w = LCD_W;
const byte lcd_h = LCD_H;
const word lcd_addr = LCD_ADDR;
#define LCD_FRAMEBUFFER_SIZE ((LCD_W * LCD_H) / 8)
#define LCD_SCANLINE_SIZE (LCD_W / 8)

// Select a font
//#define LCD_FONT_4x6
//#define LCD_FONT_4x7
//#define LCD_FONT_5x8
#define LCD_FONT_6x8
//#define LCD_FONT_8x8

// Font
#ifdef LCD_FONT_4x6
	#include "font-4x6.c"
	#define font_bitmap      console_font_4x6_font_bitmap
	#define font_char_width  console_font_4x6_char_width
	#define font_char_height console_font_4x6_char_height
	#define font_first_char  console_font_4x6_first_char
	#define font_last_char   console_font_4x6_last_char
#endif
#ifdef LCD_FONT_4x7
	#include "font-4x7.c"
	#define font_bitmap      console_font_4x7_font_bitmap
	#define font_char_width  console_font_4x7_char_width
	#define font_char_height console_font_4x7_char_height
	#define font_first_char  console_font_4x7_first_char
	#define font_last_char   console_font_4x7_last_char
#endif
#ifdef LCD_FONT_5x8
	#include "font-5x8.c"
	#define font_bitmap      console_font_5x8_font_bitmap
	#define font_char_width  console_font_5x8_char_width
	#define font_char_height console_font_5x8_char_height
	#define font_first_char  console_font_5x8_first_char
	#define font_last_char   console_font_5x8_last_char
#endif
#ifdef LCD_FONT_6x8
	#include "font-6x8.c"
	#define font_bitmap      console_font_6x8_font_bitmap
	#define font_char_width  console_font_6x8_char_width
	#define font_char_height console_font_6x8_char_height
	#define font_first_char  console_font_6x8_first_char
	#define font_last_char   console_font_6x8_last_char
#endif
#ifdef LCD_FONT_8x8
	#include "font-8x8.c"
	#define font_bitmap      console_font_8x8_font_bitmap
	#define font_char_width  console_font_8x8_char_width
	#define font_char_height console_font_8x8_char_height
	#define font_first_char  console_font_8x8_first_char
	#define font_last_char   console_font_8x8_last_char
#endif

// Text mode cursor
byte lcd_text_col = 0;
byte lcd_text_row = 0;
//#define lcd_text_cols (LCD_W/font_char_width)
//#define lcd_text_rows (LCD_H/font_char_height)


__sfr __at 0x31 lcd_port;
#define lcd_put_data(v) lcd_port=v

/*
void lcd_put_data(byte a) __naked {(void)a;
__asm
	; Get parameter from stack into a
	ld hl,#0x0002
	add hl,sp
	ld a,(hl)
	
	; Put it to port
	out	(0x31), a
	ret
__endasm;
}
*/

#define clear lcd_clear
void lcd_clear() {
	byte x, y;
	byte *p;
	p = (byte *)lcd_addr;
	
	// Clear screen
	for (y = 0; y < lcd_h; y++) {
		//b = (y % 2 == 0) ? 0xaa : 0x55;	// 50% grey pattern
		for(x = 0; x < (lcd_w / 8); x++) {
			//*p++ = b;
			//*p++ = 0x55;	// dithered grey?
			*p++ = 0x00;	// white
			//*p++ = 0x80;	// stripes
			//*p++ = 0xff;	// black
		}
	}
	lcd_text_col = 0;
	lcd_text_row = 0;
}


void lcd_init() {
	/*
	__asm
	di
	
	// Continue initialization (see GL6000SL ROM at 0x0b61)
	// It is needed for initializing the LCD controller
	ld hl, #0x00
	ld de, #0x05
	call 0x2900	// ?
	
	// Call function 0x4108 (in ROM segment 0x0A = 0x28000-0x2BFFF mapped at BANK2 at 0x4000-0x7FFF)
	ld hl, #0x37f1	// 0x37f1: Call to L=08 H=41 OUT51=0A -> 0x4108 = physical ROM address 0x28108
	call 0x26eb
	
	// This clears the LCD
	// Call function at 0x4092 (in ROM segment 0x0A = 0x28000-0x2BFFF mapped at BANK2 at 0x4000-0x7FFF)
	ld hl, #0x37dd	// 0x37dd: Call to L=92 H=40 OUT51=0A -> 0x4092 = physical ROM address 0x2800A
	call 0x26eb
	
	//ld hl, #0x37dd
	
	__endasm;
	*/
	
	/*
	// Init sequence traced out through MAME on GL6000SL
	
	draw routine in MAME:
		int width = m_lcdc.fb_width + m_lcdc.lcd_w + 1;
		for (int y = 0; y <= m_lcdc.lcd_h; y++)
			if (y <= m_lcdc.split_pos)	line_start = m_lcdc.addr1 + y * width;
			else						line_start = m_lcdc.addr2 + (y - m_lcdc.split_pos - 1) * width;
			
			for (int sx = 0; sx <= m_lcdc.lcd_w; sx++)
				uint8_t data = m_vram[(line_start + sx) & 0x1fff];
				for (int x = 0; x < 8 / bpp; x++)
				...
	*/
	__asm
		// ?
		ld a, #0x03
		out (0x31), a
		
		// LCD_WIDTH := 240 (last x = 232 = 0x1D * 8)
		// port_out(0x32, 0x1d);	// 0x32 = lcd_w
		ld a, #0x1d
		out (0x32), a
		
		// port_out(0x33, 0xff);
		ld a, #0xff
		out (0x33), a
		
		// LCD_HEIGHT := 100 (0x64)
		// port_out(0x34, 0x64);	// 0x34 = lcd_h
		ld a, #0x64
		out (0x34), a
		
		// port_out(0x35, 0x00);
		ld a, #0x00
		out (0x35), a
		
		// port_out(0x3d, 0x00);
		//ld a, #0x00
		out (0x3d), a
		
		// LCD: addr1 := 0x0000
		// port_out(0x36, 0x00);	// 0x36 = addr1 L
		//ld a, #0x00
		out (0x36), a
		
		// port_out(0x37, 0x00);	// 0x37 = addr1 H
		//ld a, #0x00
		out (0x37), a
		
		// port_out(0x3a, 0x64);	// 0x3a = split_pos
		ld a, #0x64
		out (0x3a), a
		
		// port_out(0x3b, 0x00);
		ld a, #0x00
		out (0x3b), a
		
		// LCD: addr2 := 0x0bb8
		// port_out(0x38, 0xb8);	// 0x38 = addr2 L
		ld a, #0xb8
		out (0x38), a
		
		// port_out(0x39, 0x0b);	// 0x39 = addr2 H
		ld a, #0x0b
		out (0x39), a
		
		// port_out(0x3d, 0x00);	// 0x3d = fb_width
		ld a, #0x00
		out (0x3d), a
		
		// port_out(0x3e, 0x00);
		//ld a, #0x00
		out (0x3e), a
		
		// port_out(0x3f, 0x00);
		//ld a, #0x00
		out (0x3f), a
		
	__endasm;
	
	
	//lcd_clear();
	lcd_text_col = 0;
	lcd_text_row = 0;
	
	// LCD on (?)
	lcd_put_data(0x83);
	
	lcd_put_data(0x83);
	
	//lcd_clear();
}


/*
// Draw 8-bit wide font on 8-pixel grid (simple!)
void lcd_draw_glypth_at(byte x, byte y, byte code) {
	byte *p;
	byte *f;
	byte iy;
	
	p = (byte *)lcd_addr + (y * lcd_w + x) >> 3;	// Framebuffer pointer (full bytes / 8 bits)
	f = (byte *)&font_bitmap[code][0];	// Font pointer
	
	for(iy = 0; iy < font_char_height; iy++) {
		*p = *f;
		f++;
		p += (lcd_w / 8);	// Next scanline
	}
}
*/

// Draw arbitrary font size on screen
void lcd_draw_glypth_at(byte x, byte y, byte code) {
	byte *p;
	byte *f;
	byte iy;
	
	byte d;
	unsigned char over;
	byte shift_next;
	
	//byte screen_cx = (x >> 3);	// Byte x offset on screen
	byte screen_bx = (x & 0x07);	// Bit x offset on screen
	
	p = (byte *)lcd_addr + (y * LCD_SCANLINE_SIZE) + (x >> 3);	// Framebuffer pointer
	f = (byte *)&font_bitmap[code][0];	// Font pointer
	
	over = screen_bx + font_char_width - 8;
	shift_next = font_char_width - over;
	for(iy = 0; iy < font_char_height; iy++) {
		d = *f;
		*p |= d >> screen_bx;
		if (over > 0) {
			*(p+1) = d << shift_next;
		}
		
		f++;	// Next font line
		p += LCD_SCANLINE_SIZE;	//(lcd_w >> 3);	// Next scanline
	}
}

/*
// Helper to draw a whole string
void lcd_draw_string_at(byte x, byte y, char *s) {
	while (*s != 0) {
		lcd_draw_glypth_at(x, y, *s++);
		x += font_char_width;
	}
}
*/

//#define LCD_SCROLL_AMOUNT 1
#define LCD_SCROLL_AMOUNT 8	// font_char_height
void lcd_scroll(int dy) {
	//byte x;	// Enough for one line
	word x;	// For more than one line
	byte *p;
	
	//while (lcd_text_row + font_char_height >= lcd_h) {
	while(dy >= 0) {
		// Minimal
		//lcd_text_row = 0;
		
		// Scroll up
		__asm
		push bc
		push de
		push hl
		
		// Scroll one line
		ld bc, #((LCD_W * (LCD_H - LCD_SCROLL_AMOUNT)) / 8)	// Number of bytes to scroll (i.e. whole screen minus x lines)
		ld hl, #(LCD_ADDR + (LCD_W / 8) * LCD_SCROLL_AMOUNT)	//#0xE01E	// Offset of 2nd line, i.e. LCD_ADDR + bytes-per-line * x
		ld de, #LCD_ADDR	//#0xE000	// Offset of 1st line
		ldir	// Copy BC bytes from HL to DE
		
		pop hl
		pop de
		pop bc
		__endasm;
		
		
		// Clear last line
		p = (byte *)(LCD_ADDR + (LCD_W * (LCD_H - LCD_SCROLL_AMOUNT)) / 8);
		for(x = 0; x < (LCD_W / 8) * LCD_SCROLL_AMOUNT; x++) {
			*p++ = 0x00;	// white
			//*p++ = 0xff;	// black
		}
		
		dy -= LCD_SCROLL_AMOUNT;
		lcd_text_row -= (LCD_SCROLL_AMOUNT / font_char_height);	// or just ONE LINE
	}
}

void lcd_putchar_at(byte x, byte y, char c) {
	lcd_draw_glypth_at(x*font_char_width, y*font_char_height, c);
}

void lcd_putchar(byte c) {
	
	if (c == '\r') {
		lcd_text_col = 0;
		c = 0;	// Stop handling it
	} else
	if (c == '\n') {
		lcd_text_col = 0;
		lcd_text_row ++;
		c = 0;	// Stop handling it
	} else
	if (c == '\b') {
		if (lcd_text_col > 0)
			lcd_text_col --;
		// Cross out old character
		//@TODO: Draw a square
		//lcd_draw_glypth_at(lcd_text_col*font_char_width, lcd_text_row*font_char_height, ' ');
		lcd_putchar_at(lcd_text_col, lcd_text_row, ' ');
		c = 0;	// Stop handling it
	}
	
	if (((lcd_text_col+1)*font_char_width) > lcd_w) {
		lcd_text_col = 0;
		lcd_text_row ++;
	}
	
	if (((lcd_text_row+1)*font_char_height) > lcd_h) {
		// We are at end of screen
		lcd_scroll(((lcd_text_row+1) * font_char_height) - lcd_h);
	}
	
	if (c > 0) {
		//lcd_draw_glypth_at(lcd_text_col*font_char_width, lcd_text_row*font_char_height, c);
		lcd_putchar_at(lcd_text_col, lcd_text_row, c);
		lcd_text_col ++;
	}
	
}

//#define putchar lcd_putchar

#endif	// __LCD_H