#ifndef __LCD_H
#define __LCD_H


const byte lcd_w = 240;
const byte lcd_h = 100;
const word lcd_addr = 0xe000;

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
}

#include "font_console_8x8.h"
const byte font_w = 8;
const byte font_h = 8;
#define font_data font_console_8x8

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

void lcd_init() {
	
}

#endif	// __LCD_H