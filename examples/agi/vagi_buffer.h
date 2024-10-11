#ifndef __VAGI_BUFFER_H__
#define __VAGI_BUFFER_H__

// Working buffer functions (holding one reduced frame in a single RAM bank, ready for the engine to access it)
// Size of our internal working buffers (at 4 bit)
/*
	Must fit one RAM page (0xc000 - 0xdfff = 0x2000 = 8192 bytes)
	* e.g. 160 x 100 x 4bpp = 8000 bytes (with vertical cropping)
	* e.g. 200 x 80 x 4bpp = 8000 bytes
	* e.g. 96 x 168 x 4bpp = 8064 bytes
	* Optimal aspect ratio (no crop/scroll) would be:
		* 240 x 126 (full width, vertical scroll) -> too big (or has smooshed X-resolution of 120)
		* 190 x 100 / 200 x 100 (reduced Y-resolution; 40-50 pixel side bar for inventory?)
			* store 160 x 100 full frame with reduced Y-resolution:
				X: 100%
				Y: 168 * 3 / 5 = 100.8; 100 * 5 / 3 = 166.67
			* display as 200 x 100:
				X: 160 * 5 / 4 = 200; 200 * 4 / 5 = 160
				Y: 100%
			* display as 192 x 100:
				X: 160 * 6 / 5 = 192; 192 * 5 / 6 = 160
				Y: 100%
			* display as 256 x 100 (16 horizontal pixels not visible):
				X: 160 * 8 / 5 = 256; 256 * 5 / 8 = 160
				Y: 100%
*/
#define BUFFER_WIDTH 160
#define BUFFER_HEIGHT 100
#define BUFFER_ADDR 0xc000	// Will always be banked there
#define BUFFER_BANK_PRI 1	// shared with VAGI_FRAME_BANK_LO=1
#define BUFFER_BANK_TMP 2	// shared with VAGI_FRAME_BANK_HI=2 
#define BUFFER_BANK_VIS 3	// exclusively vor VIS data

// Image/Frame processing options
	// Chose one draw scaling option:
	#define BUFFER_DRAW_W160	// FASTEST: Draw width 1:1 (does not fill the full screen width)
	//#define BUFFER_DRAW_W192	// Stretch width to 192 (slow, but nice; combine with BUFFER_PROCESS_HCRUSH to get full-screen image)
	//#define BUFFER_DRAW_W240	// RECOMMENDED: Stretch width to 240 (slow, but nice; combine with BUFFER_PROCESS_HCRUSH to get full-screen image)
	//#define BUFFER_DRAW_W320	// Draw width X2 with crop (fast, but requires scrolling)

	// Chose one frame processing option:
	//#define BUFFER_PROCESS_HCROP	// Just extract 100 pixels in height (and employ scrolling with re-rendering)
	#define BUFFER_PROCESS_HCRUSH	// RECOMMENDED: Crush the 168 frame height down to 100

	// Chose one pixel drawing option:
	//#define BUFFER_DRAW_MONO	// Use 1 bit on/off
	#define BUFFER_DRAW_PATTERN	// Use 4 bit patterns
	//#define BUFFER_DRAW_DITHER	// RECOMMENDED: Use simple error dithering. Looks great, but is problematic for partial redraws!

	// Chose one inlining option:
	#define BUFFER_SWITCH_INLINE inline	// Inline buffer switch calls
	//#define BUFFER_SWITCH_INLINE 	// Do not inline buffer switch calls
	//#define BUFFER_PIXEL_INLINE inline	// Inline buffer set/get pixel
	#define BUFFER_PIXEL_INLINE 	// Do not inline buffer set/get pixel



// Helpers for converting coordinates
/*
byte inline screen_to_buffer_x(byte x) {
	byte x2;
	// Transform source x-coordinate to buffer coordinate
	#ifdef BUFFER_DRAW_W160
		x2 = x;	// 1:1
	#endif
	#ifdef BUFFER_DRAW_W192
		x2 = (x * 5) / 6;	// stretch 160 to 192
	#endif
	#ifdef BUFFER_DRAW_W240
		x2 = (x * 2) / 3;	// stretch 160 to 240
	#endif
	#ifdef BUFFER_DRAW_W320
		if (x_scale) x2 = (x >> 1);	// stretch 160 to 320 if specified
		else x2 = x;
	#endif
	return x2;
}
byte inline screen_to_buffer_y(byte y) {
	byte y2;
	// 1:1
	//y2 = y;
	
	// 1:1 with crop/transform
	y2 = y;	// + y_ofs;
	
	// scaling
	//if (y_scale) y2 = (y >> 1) + y_ofs;
	//else y2 = y + y_ofs;
	return y2;
}

byte inline screen_to_game_x(byte x) {
	// Convert screen coordinates to game coordinates
	byte x2;
	#ifdef BUFFER_DRAW_W160
		x2 = x;	// 1:1
	#endif
	#ifdef BUFFER_DRAW_W192
		x2 = (x * 5) / 6;	// stretch 160 to 192
	#endif
	#ifdef BUFFER_DRAW_W240
		x2 = (x * 2) / 3;	// stretch 160 to 240
	#endif
	#ifdef BUFFER_DRAW_W320
		x2 = x / (x_scale ? 2 : 1);
	#endif
	return x2;
}
byte inline screen_to_game_y(byte y) {
	// Convert screen coordinates to game coordinates
	byte y2;
	#ifdef BUFFER_PROCESS_HCROP
		y2 = y;	// 1:1 with crop/transform
	#endif
	#ifdef BUFFER_PROCESS_HCRUSH
		y2 = (y * 5) / 3;	// Screen is (crushed) 168 down to 100
	#endif
	return y2;
}

byte inline game_to_screen_x(byte x) {
	// Convert game coordinates to screen coordinates
	byte sx;
	#ifdef BUFFER_DRAW_W160
		sx = x;	// 1:1
	#endif
	#ifdef BUFFER_DRAW_W192
		//x2 = (x * 5) / 6;	// stretch 160 to 192
		sx = (x * 6) / 5;
	#endif
	#ifdef BUFFER_DRAW_W240
		//x2 = (x * 2) / 3;	// stretch 160 to 240
		sx = (x * 3) / 2;
	#endif
	#ifdef BUFFER_DRAW_W320
		sx = x * (x_scale ? 2 : 1);
	#endif
	return sx;
}
byte inline game_to_screen_y(byte y) {
	// Convert game coordinates to screen coordinates
	byte sy;
	#ifdef BUFFER_PROCESS_HCROP
		//y2 = y + y_src;	// 1:1 with crop/transform
		sy = y;	// 1:1 with crop/transform
	#endif
	#ifdef BUFFER_PROCESS_HCRUSH
		sy = (y * 3) / 5;	// Scale (crush) 168 down to 100
	#endif
	return sy;
}


*/



// X
#ifdef BUFFER_DRAW_W160
	#define VAGI_SCREEN_W 160
	#define buffer_to_frame_x(x) (x)	// 1:1
	#define screen_to_buffer_x(x) (x)	// ~1:1
	#define screen_to_game_x(x) (x)	// ~1:1
	#define game_to_screen_x(x) (x)	// 1:1
#endif
#ifdef BUFFER_DRAW_W192
	#define VAGI_SCREEN_W 192
	#define buffer_to_frame_x(x) (x)	// 1:1
	#define screen_to_buffer_x(x) ((x * 5) / 6)	// ~stretch 160 to 192
	#define screen_to_game_x(x) ((x * 5) / 6)	// ~stretch 160 to 192
	#define game_to_screen_x(x) ((x * 6) / 5)	// stretch 160 to 192
#endif
#ifdef BUFFER_DRAW_W240
	#define VAGI_SCREEN_W 240
	#define buffer_to_frame_x(x) (x)	// 1:1
	#define screen_to_buffer_x(x) ((x * 2) / 3)	// ~stretch 160 to 240
	#define screen_to_game_x(x) ((x * 2) / 3)	// ~stretch 160 to 240
	#define game_to_screen_x(x) ((x * 3) / 2)	// stretch 160 to 240
#endif
#ifdef BUFFER_DRAW_W320
	#define VAGI_SCREEN_W 320
	#define buffer_to_frame_x(x) (x)	// 1:1
	#define screen_to_buffer_x(x) ((x_scale) ? (x >> 1) : x)	// stretch 160 to 320 if specified
	#define screen_to_game_x(x) (x / (x_scale ? 2 : 1))
	#define game_to_screen_x(x) (x * (x_scale ? 2 : 1))
#endif

// Y
#ifdef BUFFER_PROCESS_HCROP
	#define VAGI_SCREEN_H 168	//100
	#define buffer_to_frame_y(y) (y)	// 1:1 with crop/transform
	#define screen_to_buffer_y(y) (y)	//if (y_scale) y2 = (y >> 1) + y_ofs;
	#define screen_to_game_y(y) (y)	// 1:1 with crop/transform
	#define game_to_screen_y(y) (y)	// 1:1 with crop/transform
#endif
#ifdef BUFFER_PROCESS_HCRUSH
	#define VAGI_SCREEN_H 100
	#define buffer_to_frame_y(y) ((y * 5) / 3)	// ~Scale (crush) 168 down to 100
	#define screen_to_buffer_y(y) (y)	//if (y_scale) y2 = (y >> 1) + y_ofs;
	#define screen_to_game_y(y) ((y * 5) / 3)	// Screen is (crushed) 168 down to 100
	#define game_to_screen_y(y) ((y * 3) / 5)	// Scale (crush) 168 down to 100
#endif



void BUFFER_SWITCH_INLINE buffer_switch(byte bank) {
	// Mount a different RAM segment to BUFFER_ADDR (0xc000)
	bank_0xc000_port = bank;
}
void buffer_clear(byte c) {
	//memset((byte *)BUFFER_ADDR, 0x00, ((BUFFER_WIDTH * BUFFER_HEIGHT) >> 1));
	memset((byte *)BUFFER_ADDR, c * 0x11, ((BUFFER_WIDTH * BUFFER_HEIGHT) >> 1));
}
//	void buffer_add_pixel_4bit(byte x, byte y, byte c) {
//		// Add 4 bit color value of the working buffer at 0xc000
//		// Like set_pixel, but only does a single OR operation, hence: faster.
//		/*
//		word a;
//		a = BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1);
//		if ((x & 1) == 0)	*(byte *)a |= c;
//		else				*(byte *)a |= c << 4;
//		*/
//		/*
//		if ((x & 1) == 0)	*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c;
//		else				*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c << 4;
//		*/
//		if (x & 1)	*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c << 4;
//		else		*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c;
//	}

void BUFFER_PIXEL_INLINE buffer_set_pixel_4bit(byte x, byte y, byte c) {
	// Set 4 bit color value of the working buffer at 0xc000
	byte *a = (byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1));
	if (x & 1)	*a = (*a & 0x0f) | (c << 4);
	else		*a = (*a & 0xf0) | c;
}

byte BUFFER_PIXEL_INLINE buffer_get_pixel_4bit(byte x, byte y) {
	// Get the 4 bit color value of the working buffer at 0xc000
	/*
	word a;
	a = BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1);
	if ((x & 1) == 0)	return (*(byte *)a) & 0x0f;
	else				return (*(byte *)a) >> 4;
	*/
	/*
	if ((x & 1) == 0)	return (*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1))) & 0x0f;
	else				return (*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1))) >> 4;
	*/
	if (x & 1)	return (*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1))) >> 4;
	else		return (*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1))) & 0x0f;
}



//#define RGB_TO_LCD_4BPP(r,g,b) (0xf - ((r+g+b)/(3*0x11)))
//#define RGB_TO_LUMA_4BPP(r,g,b) ((r+g+b)/(3*0x11))
//#define RGB_TO_LUMA(r,g,b) ((r+g+b) / 3)
#define RGB_TO_LUMA(r,g,b) (0.299*r + 0.587*g + 0.114*b)

static const byte AGI_PALETTE_TO_LUMA[16] = {
	RGB_TO_LUMA(0x00, 0x00, 0x00),
	RGB_TO_LUMA(0x00, 0x00, 0xa0),
	RGB_TO_LUMA(0x00, 0x80, 0x00),
	RGB_TO_LUMA(0x00, 0xa0, 0xa0),
	RGB_TO_LUMA(0xa0, 0x00, 0x00),
	RGB_TO_LUMA(0x80, 0x00, 0xa0),
	RGB_TO_LUMA(0xa0, 0x50, 0x00),
	RGB_TO_LUMA(0xa0, 0xa0, 0xa0),
	RGB_TO_LUMA(0x50, 0x50, 0x50),
	RGB_TO_LUMA(0x50, 0x50, 0xff),
	RGB_TO_LUMA(0x00, 0xff, 0x50),
	RGB_TO_LUMA(0x50, 0xff, 0xff),
	RGB_TO_LUMA(0xff, 0x50, 0x50),
	RGB_TO_LUMA(0xff, 0x50, 0xff),
	RGB_TO_LUMA(0xff, 0xff, 0x50),
	RGB_TO_LUMA(0xff, 0xff, 0xff)
};




void draw_buffer(
		byte bank,
		byte area_x1, byte area_x2, byte area_y1, byte area_y2,
		byte x_ofs, byte y_ofs	//, byte x_scale	//, byte y_scale
	) {
	// Transfer a final buffer to the screen, applying optional scale/translate
	byte x;
	byte y;
	byte x2;
	byte y2;
	byte c;
	
	#ifdef BUFFER_DRAW_DITHER
	// Dithering
	int v;
	int v_err;
	#endif
	
	// Clip area
	if (area_x1 >= LCD_WIDTH) return;
	if (area_x2 > LCD_WIDTH) area_x2 = LCD_WIDTH;	//-1;
	if (area_y1 >= LCD_HEIGHT) return;
	if (area_y2 > LCD_HEIGHT) area_y2 = LCD_HEIGHT;	//-1;
	
	// Map working buffer to 0xc000
	//bank_0xc000_port = bank;
	buffer_switch(bank);
	
	// Transfer (and optionally scale) all pixels to screen
	for(y = area_y1; y < area_y2; y++) {
		//if (y >= LCD_HEIGHT) break;	// Beyond screen (already checked before loop)
		
		y2 = screen_to_buffer_y(y) + y_ofs;
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		#ifdef BUFFER_DRAW_DITHER
		//v_err = 0;
		//v_err = (y * 0x77) & 0x1f;	// Add some noise
		v_err = (int)(rand() & 0x7f) - 64;	// Add some noise
		#endif
		
		for(x = area_x1; x < area_x2; x++) {
			//if (x >= LCD_WIDTH) break;	// Beyond screen (already checked before loop)
			
			x2 = screen_to_buffer_x(x) + x_ofs;
			if (x2 >= BUFFER_WIDTH) break;	// Beyond buffer
			
			// Get pixel from working buffer
			c = buffer_get_pixel_4bit(x2, y2);
			
			// Draw pixel to VRAM
			#ifdef BUFFER_DRAW_MONO
				lcd_set_pixel_1bit(x, y, (AGI_PALETTE_TO_LUMA[c] < 0x80) ? 1 : 0);	// B/W monochrome
			#endif
			#ifdef BUFFER_DRAW_PATTERN
				lcd_set_pixel_4bit(x, y, AGI_PALETTE_TO_LUMA[c]);	// Use patterns
			#endif
			#ifdef BUFFER_DRAW_DITHER
				// Dithering with luma
				v = AGI_PALETTE_TO_LUMA[c];
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
			
		}
	}
}


/*
//void draw_buffer_combined(byte bank_vis, byte bank_pri, byte thresh, byte x_ofs, byte y_ofs, byte x_scale) {	//, byte y_scale) {
void draw_buffer_combined(
		byte bank_vis, byte bank_pri,
		byte thresh,
		byte area_x1, byte area_x2, byte area_y1, byte area_y2,
		byte x_ofs, byte y_ofs, byte x_scale //, byte y_scale
	) {
	
	// Combine two buffers (visual and priority) into one image and display it!
	byte x;
	byte y;
	byte x2;
	byte y2;
	byte c_prio;
	byte c;
	//byte c_vis;
	
	#ifdef BUFFER_DRAW_DITHER
	// Dithering
	int v;
	int v_err;
	#endif
	
	// Transfer (and optionally scale) all pixels to screen
	for(y = area_y1; y < area_y2; y++) {
		y2 = screen_to_buffer_y(y) + y_ofs;
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		#ifdef BUFFER_DRAW_DITHER
		v_err = 0;
		//v_err = (y * 0x77) & 0x1f;	// Add some noise
		#endif
		
		for(x = area_x1; x < area_x2; x++) {
			x2 = screen_to_buffer_x(x) + x_ofs;
			if (x2 >= BUFFER_WIDTH) break;	// Beyond buffer
			
			// Get value from working buffers
			buffer_switch(bank_pri);
			c_prio = buffer_get_pixel_4bit(x2, y2);
			
			// Do some simple threshold combination
			if (thresh > c_prio) continue;	// Behind! Skip!
			
			buffer_switch(bank_vis);
			c = buffer_get_pixel_4bit(x2, y2);
			
			// Draw to VRAM
			#ifdef BUFFER_DRAW_MONO
				//lcd_set_pixel_1bit(x, y, c);	// B/W monochrome
				lcd_set_pixel_1bit(x, y, (AGI_PALETTE_TO_LUMA[c] < 0x80) ? 1 : 0);	// B/W monochrome
			#endif
			#ifdef BUFFER_DRAW_PATTERN
				lcd_set_pixel_4bit(x, y, AGI_PALETTE_TO_LUMA[c]);	// Use patterns
			#endif
			#ifdef BUFFER_DRAW_DITHER
				// Dithering with luma
				v = AGI_PALETTE_TO_LUMA[c];
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
		}
	}
}
*/


void draw_buffer_sprite_priority(
		//byte bank_vis,
		byte bank_pri,
		
		byte *sprite_data,
		byte sprite_w, byte sprite_h,
		byte sprite_trans,
		
		byte sprite_x, byte sprite_y,
		byte sprite_prio,
		
		byte x_ofs, byte y_ofs	//, byte x_scale //, byte y_scale
	) {
	
	// Combine two buffers (visual and priority) into one image and display it!
	byte x;
	byte y;
	byte x2;
	byte y2;
	byte c;
	//byte c_sprite;
	//byte c_vis;
	byte c_prio;
	
	#ifdef BUFFER_DRAW_DITHER
	// Dithering
	int v;
	int v_err;
	#endif
	
	word syo;
	byte ssx = game_to_screen_x(sprite_x);
	byte ssy = game_to_screen_y(sprite_y);
	byte ssw = game_to_screen_x(sprite_w);
	byte ssh = game_to_screen_y(sprite_h);
	
	// Clip area
	if (ssx >= LCD_WIDTH) return;
	if (ssx+ssw > LCD_WIDTH) ssw = LCD_WIDTH - ssx;
	if (ssy >= LCD_HEIGHT) return;
	if (ssy+ssh > LCD_HEIGHT) ssh = LCD_HEIGHT - ssy;
	
	// We need priority now
	buffer_switch(bank_pri);
	
	// Transfer (and optionally scale) all pixels to screen
	for(byte iy = 0; iy < ssh; iy++) {
		y = ssy + iy;
		if (y >= LCD_HEIGHT) break;	// Beyond screen
		
		y2 = screen_to_buffer_y((word)y) + y_ofs;
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		syo = (word)sprite_w * screen_to_game_y((word)iy);	// Sprite offset
		
		
		#ifdef BUFFER_DRAW_DITHER
		//v_err = 0;
		//v_err = ((sprite_x * y) * 0x77) & 0x1f;	// Add some noise
		v_err = ((ssx * y) * 0x77) & 0x1f;	// Add some noise
		//v_err = rand() & 0x1f;	// Add some noise
		#endif
		
		for(byte ix = 0; ix < ssw; ix++) {
			x = ssx + ix;
			//if (x >= LCD_WIDTH) break;	// Beyond screen (already checked before loop)
			
			x2 = screen_to_buffer_x((word)x) + x_ofs;
			if (x2 >= BUFFER_WIDTH) break;	// Beyond buffer
			
			// Get value from priority buffer
			//buffer_switch(bank_pri);	// Only do this once per call?
			c_prio = buffer_get_pixel_4bit(x2, y2);
			
			// Optional: Skip drawing foreground pixels (faster, but requires visual pixel to be already there)
			if (c_prio >= sprite_prio) continue;	// Just skip (and hope background is correct)
			
			// Get sprite pixel
			c = sprite_data[syo + screen_to_game_x((word)ix)];
			
			// Optional: Skip drawing transparent pixels
			if (c == sprite_trans) continue;	// Just skip (and hope background is correct)
			
			/*
			// Alternative: Show sprite or background
			if ((c == sprite_trans) || (c_prio >= sprite_prio)) {
				// Show background
				buffer_switch(bank_vis);
				c = buffer_get_pixel_4bit(x2, y2);
			} else {
				// Show sprite
				//c_vis = c_sprite;
			}
			*/
			
			
			//@TODO: Wrap into "vagi_set_pixel(x, y, c);"
			
			// Draw pixel to VRAM
			#ifdef BUFFER_DRAW_MONO
				//lcd_set_pixel_1bit(x, y, c);	// B/W monochrome
				lcd_set_pixel_1bit(x, y, (AGI_PALETTE_TO_LUMA[c] < 0x80) ? 1 : 0);	// B/W monochrome
			#endif
			#ifdef BUFFER_DRAW_PATTERN
				lcd_set_pixel_4bit(x, y, AGI_PALETTE_TO_LUMA[c]);	// Use patterns
			#endif
			#ifdef BUFFER_DRAW_DITHER
				// Dithering with luma
				v = AGI_PALETTE_TO_LUMA[c];
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
		}
	}
}


#endif