#ifndef __VAGI_LCD_H__
#define __VAGI_LCD_H__

#define LCD_WIDTH 240
#define LCD_HEIGHT 100
#define LCD_ADDR 0xe000

// LCD functions
/*
void lcd_clear() {
	memset((byte *)LCD_ADDR, 0x00, (LCD_HEIGHT * (LCD_WIDTH >> 3)));
}
*/

static const byte lcd_pixel_mask_set[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
static const byte lcd_pixel_mask_clear[8] = {0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe};
void inline lcd_set_pixel_1bit(byte x, byte y, byte c) {
	// Draw to LCD VRAM (1bpp):
	//	c == 0 = pixel CLEAR = WHITE
	//	c > 0 = pixel SET = BLACK
	/*
	word a = LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3);
	if (c)	*(byte *)a |= 1 << (7 - x & 0x07);
	else	*(byte *)a &= 0xff - (1 << (7 - x & 0x07));
	*/
	
	// Use mask look-up for more speed
	/*
	word a = LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3);
	if (c > 0) {
		*(byte *)a |= lcd_pixel_mask_set[x & 0x07];
	} else {
		*(byte *)a &= lcd_pixel_mask_clear[x & 0x07];
	}
	*/
	if (c)	*(byte *)(LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3)) |= lcd_pixel_mask_set[x & 0x07];
	else	*(byte *)(LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3)) &= lcd_pixel_mask_clear[x & 0x07];
}


void lcd_set_pixel_4bit(byte x, byte y, byte c) {
	// Draw pixel with tone mapping / dithering
	
	//	// 3 shades:
	//	if (c < 6) c = 0;
	//	else if (c < 12) c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) );
	//	else c = 1;
	
	
	//	// 5 shades:
	//	     if (c <  3) c = 0;	// 0=white (unset)
	//	else if (c <  7) c = ((x + y) & 3) ? 0 : 1;	// Mostly white
	//	else if (c < 11) c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) );	// 50% gray
	//	else if (c < 14) c = ((x + y) & 3) ? 1 : 0;	// Mostly black
	//	else             c = 1;	// 1=black (set)
	//	lcd_set_pixel_1bit(x, y, c);
	
	//	switch(c >> 5) {
	//		case 0: c = 1; break;
	//		case 1: c = ((x + y) & 7) ? 1 : 0; break;	// Mostly 1 (black)
	//		case 2: c = ((x + y) & 3) ? 1 : 0;	// Mostly 1 (black)
	//		case 3: c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) ); break;
	//		case 4: c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) ); break;
	//		case 5: c = ((x + y) & 3) ? 0 : 1; break;	// Mostly 0 (white)
	//		case 6: c = ((x + y) & 7) ? 0 : 1; break;	// Mostly 0 (white)
	//		case 7: c = 0; break;
	//	}
	
	// 7 shades
	     if (c <  36) c = 1;	// 1=black (set)
	else if (c <  72) c = ((x + y) & 7) ? 1 : 0;	// Mostly 1 (black)
	else if (c < 108) c = ((x + y) & 3) ? 1 : 0;	// Mostly 1 (black)
	else if (c < 141) c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) );	// 50% gray
	else if (c < 182) c = ((x + y) & 3) ? 0 : 1;	// Mostly 0 (white)
	else if (c < 218) c = ((x + y) & 7) ? 0 : 1;	// Mostly 0 (white)
	else              c = 0;	// 0=white (unset)
	
	lcd_set_pixel_1bit(x, y, c);
}

#endif