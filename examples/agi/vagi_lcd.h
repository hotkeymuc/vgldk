#ifndef __VAGI_LCD_H__
#define __VAGI_LCD_H__
// LCD functions

#define LCD_WIDTH 240	// aka. lcd.h:LCD_W
#define LCD_HEIGHT 100	// aka. lcd.h:LCD_H
#define LCD_ADDR 0xe000	// aka. lcd.h:LCD_ADDR

//#define LCD_PIXEL_USE_MASK	// Use look-up masks for bitwise pixel setting?

#define LCD_PIXEL_INLINE inline // Inline the pixel set func (to potentially gain speed?)
//#define LCD_PIXEL_INLINE  // Do not inline the pixel set func (to potentially gain speed?)

// Chose one pixel drawing algorithm:
//#define VAGI_DRAW_MONO	// Just hard-cut black or white
//#define VAGI_DRAW_PATTERN	// Diagonal hatch line patterns
//#define VAGI_DRAW_DITHER	// Error dithering (bad for partial redraws)
#define VAGI_DRAW_BAYER	// Bayer pattern dithering

#ifdef LCD_PIXEL_USE_MASK
	// Look-up quickly
	const byte lcd_pixel_mask_set[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	const byte lcd_pixel_mask_clear[8] = {0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe};

	void LCD_PIXEL_INLINE lcd_set_pixel_1bit(byte x, byte y, byte c) {
		// Draw to LCD VRAM (1bpp):
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

	// Optimized versions that only turn on or off (to make the optimizer happy)
	void LCD_PIXEL_INLINE lcd_set_pixel_1bit_on(byte x, byte y) {
		*(byte *)(LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3)) |= lcd_pixel_mask_set[x & 0x07];
	}
	void LCD_PIXEL_INLINE lcd_set_pixel_1bit_off(byte x, byte y) {
		*(byte *)(LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3)) &= lcd_pixel_mask_clear[x & 0x07];
	}
#else
	void LCD_PIXEL_INLINE lcd_set_pixel_1bit(byte x, byte y, byte c) {
		// Draw to LCD VRAM (1bpp):
		//	c == 0 = pixel CLEAR = WHITE
		//	c > 0 = pixel SET = BLACK
		word a = LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3);
		if (c)	*(byte *)a |= 1 << (7 - x & 0x07);
		else	*(byte *)a &= ~(1 << (7 - x & 0x07));
	}
	void LCD_PIXEL_INLINE lcd_set_pixel_1bit_on(byte x, byte y) {
		// Draw to LCD VRAM (1bpp):
		//	c == 0 = pixel CLEAR = WHITE
		//	c > 0 = pixel SET = BLACK
		word a = LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3);
		*(byte *)a |= 1 << (7 - x & 0x07);
	}
	void LCD_PIXEL_INLINE lcd_set_pixel_1bit_off(byte x, byte y) {
		// Draw to LCD VRAM (1bpp):
		//	c == 0 = pixel CLEAR = WHITE
		//	c > 0 = pixel SET = BLACK
		word a = LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3);
		*(byte *)a &= ~(1 << (7 - x & 0x07));
	}
#endif


#ifdef VAGI_DRAW_BAYER
// From http://www.edenwaith.com/blog/index.php?p=157
/*
	#define BAYER_SIZE 4 
const byte BAYER_PATTERN[4][4] = {	//	4x4 Bayer Dithering Matrix. Color levels: 17
	{	 15, 195,  60, 240	},
	{	135,  75, 180, 120	},
	{	 45, 225,  30, 210	},
	{	165, 105, 150,  90	}
};
*/
#define BAYER_SIZE 16
const byte BAYER_PATTERN[16][16] = {	//	16x16 Bayer Dithering Matrix.  Color levels: 256
	{	  0, 191,  48, 239,  12, 203,  60, 251,   3, 194,  51, 242,  15, 206,  63, 254	}, 
	{	127,  64, 175, 112, 139,  76, 187, 124, 130,  67, 178, 115, 142,  79, 190, 127	},
	{	 32, 223,  16, 207,  44, 235,  28, 219,  35, 226,  19, 210,  47, 238,  31, 222	},
	{	159,  96, 143,  80, 171, 108, 155,  92, 162,  99, 146,  83, 174, 111, 158,  95	},
	{	  8, 199,  56, 247,   4, 195,  52, 243,  11, 202,  59, 250,   7, 198,  55, 246	},
	{	135,  72, 183, 120, 131,  68, 179, 116, 138,  75, 186, 123, 134,  71, 182, 119	},
	{	 40, 231,  24, 215,  36, 227,  20, 211,  43, 234,  27, 218,  39, 230,  23, 214	},
	{	167, 104, 151,  88, 163, 100, 147,  84, 170, 107, 154,  91, 166, 103, 150,  87	},
	{	  2, 193,  50, 241,  14, 205,  62, 253,   1, 192,  49, 240,  13, 204,  61, 252	},
	{	129,  66, 177, 114, 141,  78, 189, 126, 128,  65, 176, 113, 140,  77, 188, 125	},
	{	 34, 225,  18, 209,  46, 237,  30, 221,  33, 224,  17, 208,  45, 236,  29, 220	},
	{	161,  98, 145,  82, 173, 110, 157,  94, 160,  97, 144,  81, 172, 109, 156,  93	},
	{	 10, 201,  58, 249,   6, 197,  54, 245,   9, 200,  57, 248,   5, 196,  53, 244	},
	{	137,  74, 185, 122, 133,  70, 181, 118, 136,  73, 184, 121, 132,  69, 180, 117	},
	{	 42, 233,  26, 217,  38, 229,  22, 213,  41, 232,  25, 216,  37, 228,  21, 212	},
	{	169, 106, 153,  90, 165, 102, 149,  86, 168, 105, 152,  89, 164, 101, 148,  85	}
};

#endif


#ifdef VAGI_DRAW_DITHER
int v_err;
#endif

void vagi_set_pixel_8bit(byte x, byte y, byte v) {
	// Draw pixel to VRAM
	#ifdef VAGI_DRAW_MONO
		//lcd_set_pixel_1bit(x, y, c);	// B/W monochrome
		lcd_set_pixel_1bit(x, y, (v < 0x80) ? 1 : 0);	// B/W monochrome
	#endif
	#ifdef VAGI_DRAW_PATTERN
		//lcd_set_pixel_4bit(x, y, v);	// Use patterns
		/*
		// 3 shades:
		if (c < 6) c = 0;
		else if (c < 12) c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) );
		else c = 1;
		*/
	
		/*
		// 5 shades:
			 if (c <  3) c = 0;	// 0=white (unset)
		else if (c <  7) c = ((x + y) & 3) ? 0 : 1;	// Mostly white
		else if (c < 11) c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) );	// 50% gray
		else if (c < 14) c = ((x + y) & 3) ? 1 : 0;	// Mostly black
		else             c = 1;	// 1=black (set)
		lcd_set_pixel_1bit(x, y, c);
		*/
		
		/*
		// 7-8 shades
		switch(c >> 5) {
			case 0: c = 1; break;	// 1=black (set)
			case 1: c = ((x + y) & 7) ? 1 : 0; break;	// Mostly 1 (black)
			case 2: c = ((x + y) & 3) ? 1 : 0;	// Mostly 1 (black)
			case 3: c = ((x + y) & 1); break;
			case 4: c = ((x + y) & 1); break;
			case 5: c = ((x + y) & 3) ? 0 : 1; break;	// Mostly 0 (white)
			case 6: c = ((x + y) & 7) ? 0 : 1; break;	// Mostly 0 (white)
			case 7: c = 0; break;
		}
		*/
		/*
		// 7 shades
			 if (c <  36) c = 1;	// 1=black (set)
		else if (c <  72) c = ((x + y) & 7) ? 1 : 0;	// Mostly 1 (black)
		else if (c < 108) c = ((x + y) & 3) ? 1 : 0;	// Mostly 1 (black)
		else if (c < 141) c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) );	// 50% gray
		else if (c < 182) c = ((x + y) & 3) ? 0 : 1;	// Mostly 0 (white)
		else if (c < 218) c = ((x + y) & 7) ? 0 : 1;	// Mostly 0 (white)
		else              c = 0;	// 0=white (unset)
		lcd_set_pixel_1bit(x, y, c);
		*/
		
		
		// 16 shades
		byte xy = x + y;
		switch(c >> 4) {
			case  0: c = 1; break;	// 1=black (set)
			case  1: c = (xy & 15) ? 1 : 0; break;			// Mostly 1 (black)	0111111111111111	15/16
			case  2: c = (xy &  7) ? 1 : 0; break;			// Mostly 1 (black)	01111111	7/8
			case  3: c = (xy &  7) ? 1 : 0; break;			// Mostly 1 (black)	01111111	7/8
			case  4: c = (xy &  3) ? 1 : 0;	break;			// Mostly 1 (black)	01110111	6/8
			case  5: c = (xy &  3) ? 1 : 0;	break;			// Mostly 1 (black)	01110111	6/8
			case  6: c = (xy &  3) ? 1 : 0;	break;			// More 1 (black)	11011101	6/8
			case  7: c = (xy &  1); break;					// 50%			01010101	4/8
			case  8: c = (xy &  1); break;					// 50%			01010101	4/8
			case  9: c = (xy &  3) ? 0 : 1; break;			// Mostly 0 (white)	10001000	2/8
			case 10: c = (xy &  3) ? 0 : 1; break;			// Mostly 0 (white)	10001000	2/8
			case 11: c = (xy &  3) ? 0 : 1; break;			// Mostly 0 (white)	10001000	2/8
			case 12: c = (xy &  7) ? 0 : 1; break;			// Mostly 0 (white)	10000000	1/8
			case 13: c = (xy &  7) ? 0 : 1; break;			// Mostly 0 (white)	10000000	1/8
			case 14: c = (xy & 15) ? 0 : 1; break;			// Mostly 0 (white)	1000000000000000	1/15
			case 15: c = 0; break;
		}
		lcd_set_pixel_1bit(x, y, c);
		
		
	#endif
	#ifdef VAGI_DRAW_DITHER
		// Dithering with error
		v += v_err;
		if (v < 0x80) {
			//lcd_set_pixel_1bit(x, y, 1);	// B/W monochrome (1=black)
			lcd_set_pixel_1bit_on(x, y);	// B/W monochrome (1=black)
			v_err = (v - 0x00);
		} else {
			//lcd_set_pixel_1bit(x, y, 0);	// B/W monochrome (0=white)
			lcd_set_pixel_1bit_off(x, y);	// B/W monochrome (0=white)
			v_err = (v - 0xff);
		}
	#endif
	
	#ifdef VAGI_DRAW_BAYER
		// BAYER dithering
		if (v == 0) {
			lcd_set_pixel_1bit_on(x, y);
		} else
		if (v == 255) {
			lcd_set_pixel_1bit_off(x, y);
		} else {
			if (v < BAYER_PATTERN[y % BAYER_SIZE][x % BAYER_SIZE])
				lcd_set_pixel_1bit_on(x, y);
			else
				lcd_set_pixel_1bit_off(x, y);
		}
	#endif
}


#endif