#ifndef __LCD_H
#define __LCD_H

#define LCD_W 240
#define LCD_H 100
#define LCD_ADDR 0xe000

const byte lcd_w = LCD_W;
const byte lcd_h = LCD_H;
const word lcd_addr = 0xe000;
#define LCD_FRAMEBUFFER_SIZE ((LCD_W * LCD_H) / 8)

byte lcd_x = 0;
byte lcd_y = 0;

#define clear lcd_clear


#include "font_console_8x8.h"
const byte font_w = 8;
const byte font_h = 8;
#define font_data font_console_8x8


void port_out_0x31(byte a) __naked {(void)a;
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


void lcd_clear() {
	byte x, y;
	byte *p;
	p = (byte *)lcd_addr;
	
	// Clear screen
	for (y = 0; y < lcd_h; y++) {
		//b = (y % 2 == 0) ? 0xaa : 0x55;	// 50% grey pattern
		for(x = 0; x < (lcd_w / 8); x++) {
			//*p++ = b;
			//*p++ = 0x55;
			*p++ = 0x00;	// white
			//*p++ = 0xff;	// black
		}
	}
	lcd_x = 0;
	lcd_y = 0;
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
	// Dump of init port sequence (as captured in custom MAME)
	port_out(0x31, 0x03);
	
	port_out(0x22, 0x00);
	port_out(0x21, 0xe0);
	port_out(0x23, 0x60);
	
	// Configure LCD
	port_out(0x32, 0x1d);
	port_out(0x33, 0xff);
	port_out(0x34, 0x64);
	port_out(0x35, 0x00);
	port_out(0x3d, 0x00);
	port_out(0x36, 0x00);
	port_out(0x37, 0x00);
	port_out(0x3a, 0x64);
	port_out(0x3b, 0x00);
	port_out(0x38, 0xb8);
	port_out(0x39, 0x0b);
	port_out(0x3d, 0x00);
	port_out(0x3e, 0x00);
	port_out(0x3f, 0x00);
	
	port_out(0x23, 0x60);
	port_out(0x21, 0xe0);
	port_out(0x21, 0xff);
	port_out(0x23, 0x20);
	*/
	
	//lcd_clear();
	lcd_x = 0;
	lcd_y = 0;
	
	// LCD on (?)
	//port_out(0x31, 0x83);
	port_out_0x31(0x83);
	
	//port_out(0x31, 0x83);
	port_out_0x31(0x83);

}





void drawGlyph(byte x, byte y, byte code) {
	byte *p;
	byte *f;
	byte iy;
	
	p = (byte *)lcd_addr + (y * lcd_w + x) / 8;
	f = (byte *)&font_data[code][0];
	
	for(iy = 0; iy < font_h; iy++) {
		*p = *f;
		f++;
		p += (lcd_w / 8);
	}
}
void drawString(byte x, byte y, char *s) {
	while (*s != 0) {
		drawGlyph(x, y, *s++);
		x += font_w;
	}
}

//#define LCD_SCROLL_AMOUNT 1
#define LCD_SCROLL_AMOUNT 8
void lcd_scroll(int dy) {
	//byte x;	// Enough for one line
	word x;	// For more than one line
	byte *p;
	
	//while (lcd_y + font_h >= lcd_h) {
	while(dy >= 0) {
		// Minimal
		//lcd_y = 0;
		
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
		lcd_y-= LCD_SCROLL_AMOUNT;
	}
}

void lcd_putchar(byte c) {
	
	if (c == '\r') {
		lcd_x = 0;
		c = 0;	// Stop handling it
	} else
	if (c == '\n') {
		lcd_x = 0;
		lcd_y += font_h;
		c = 0;	// Stop handling it
	} else
	if (c == '\b') {
		if (lcd_x > font_w)
			lcd_x -= font_w;
		drawGlyph(lcd_x, lcd_y, ' ');
		c = 0;	// Stop handling it
	}
	
	if (lcd_x + font_w >= lcd_w) {
		lcd_x = 0;
		lcd_y += font_h;
	}
	
	
	if (lcd_y + font_h >= lcd_h) {
		// We are at end of screen
		lcd_scroll((lcd_y + font_h) - lcd_h);
	}
	
	
	if (c > 0) {
		drawGlyph(lcd_x, lcd_y, c);
		
		lcd_x += font_w;
	}
	
}

//#define putchar lcd_putchar

#endif	// __LCD_H