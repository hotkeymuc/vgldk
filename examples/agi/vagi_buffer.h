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
#define BUFFER_BANK_VIS 3
#define BUFFER_BANK_PRI 1

// Image/Frame processing options

// Chose one draw scaling option:
//#define BUFFER_DRAW_W160	// Draw width 1:1 (does not fill the full screen width)
//#define BUFFER_DRAW_W192	// Stretch width to 192 (slow, but nice; combine with BUFFER_PROCESS_HCRUSH to get full-screen image)
#define BUFFER_DRAW_W240	// Stretch width to 240 (slow, but nice; combine with BUFFER_PROCESS_HCRUSH to get full-screen image)
//#define BUFFER_DRAW_W320	// Draw width X2 with crop (fast, but requires scrolling)

// Chose one frame processing option:
//#define BUFFER_PROCESS_HCROP	// Just extract 100 pixels in height (and employ scrolling with re-rendering)
#define BUFFER_PROCESS_HCRUSH	// Crush the 168 frame height down to 100

// Chose one pixel drawing option:
//#define BUFFER_DRAW_MONO	// Use 1 bit on/off
#define BUFFER_DRAW_PATTERN	// Use 4 bit patterns
//#define BUFFER_DRAW_DITHER	// Use simple error dithering. Looks great, but is problematic for partial redraws!


//#define RGB_TO_LCD_4BPP(r,g,b) (0xf - ((r+g+b)/(3*0x11)))
//#define RGB_TO_LUMA_4BPP(r,g,b) ((r+g+b)/(3*0x11))
//#define RGB_TO_LUMA(r,g,b) ((r+g+b) / 3)
#define RGB_TO_LUMA(r,g,b) (0.299*r + 0.587*g + 0.114*b)


void inline buffer_switch(byte bank) {
	// Mount a different RAM segment to BUFFER_ADDR (0xc000)
	bank_0xc000_port = bank;
}
void buffer_clear(byte c) {
	//memset((byte *)BUFFER_ADDR, 0x00, ((BUFFER_WIDTH * BUFFER_HEIGHT) >> 1));
	memset((byte *)BUFFER_ADDR, c * 0x11, ((BUFFER_WIDTH * BUFFER_HEIGHT) >> 1));
}
void buffer_add_pixel_4bit(byte x, byte y, byte c) {
	// Add 4 bit color value of the working buffer at 0xc000
	// Like set_pixel, but only does a single OR operation, hence: faster.
	/*
	word a;
	a = BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1);
	if ((x & 1) == 0)	*(byte *)a |= c;
	else				*(byte *)a |= c << 4;
	*/
	/*
	if ((x & 1) == 0)	*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c;
	else				*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c << 4;
	*/
	if (x & 1)	*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c << 4;
	else		*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c;
}

//void inline buffer_set_pixel_4bit(byte x, byte y, byte c) {
void buffer_set_pixel_4bit(byte x, byte y, byte c) {
	// Set 4 bit color value of the working buffer at 0xc000
	byte *a = (byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1));
	if (x & 1)	*a = (*a & 0x0f) | (c << 4);
	else		*a = (*a & 0xf0) | c;
}

//byte inline buffer_get_pixel_4bit(byte x, byte y) {
byte buffer_get_pixel_4bit(byte x, byte y) {
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


//void draw_buffer(byte bank, byte x_ofs, byte y_ofs, byte x_scale) {	//, byte y_scale) {
void draw_buffer(
		byte bank,
		byte area_x1, byte area_x2, byte area_y1, byte area_y2,
		byte x_ofs, byte y_ofs, byte x_scale	//, byte y_scale
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
	
	// Map working buffer to 0xc000
	//bank_0xc000_port = bank;
	buffer_switch(bank);
	
	// Transfer (and optionally scale) all pixels to screen
	for(y = area_y1; y < area_y2; y++) {
		// Transform source y-coordinate here!
		// 1:1
		//y2 = y;
		
		// 1:1 with crop/transform
		y2 = y + y_ofs;
		
		// scaling
		//if (y_scale) y2 = (y >> 1) + y_ofs;
		//else y2 = y + y_ofs;
		
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		#ifdef BUFFER_DRAW_DITHER
		v_err = 0;
		//v_err = (y * 0x77) & 0x1f;	// Add some noise
		#endif
		
		for(x = area_x1; x < area_x2; x++) {
			
			// Transform source x-coordinate here!
			#ifdef BUFFER_DRAW_W160
				x2 = x;	// 1:1
				(void)x_ofs;
				(void)x_scale;
			#endif
			#ifdef BUFFER_DRAW_W192
				x2 = (x * 5) / 6;	// stretch 160 to 192
				(void)x_ofs;
				(void)x_scale;
			#endif
			#ifdef BUFFER_DRAW_W240
				x2 = (x * 2) / 3;	// stretch 160 to 240
				(void)x_ofs;
				(void)x_scale;
			#endif
			#ifdef BUFFER_DRAW_W320
				if (x_scale) x2 = (x >> 1) + x_ofs;	// stretch 160 to 320 if specified
				else x2 = x + x_ofs;
			#endif
			
			if (x2 >= BUFFER_WIDTH) break;	// Beyond buffer
			
			// Get pixel from working buffer
			c = buffer_get_pixel_4bit(x2, y2);
			
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
					lcd_set_pixel_1bit(x, y, 1);	// B/W monochrome (1=black)
					v_err = (v - 0x00);
				} else {
					lcd_set_pixel_1bit(x, y, 0);	// B/W monochrome (0=white)
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
		// Transform source y-coordinate here!
		// 1:1
		//y2 = y;
		
		// 1:1 with crop/transform
		y2 = y + y_ofs;
		
		// scaling
		//if (y_scale) y2 = (y >> 1) + y_ofs;
		//else y2 = y + y_ofs;
		
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		#ifdef BUFFER_DRAW_DITHER
		v_err = 0;
		//v_err = (y * 0x77) & 0x1f;	// Add some noise
		#endif
		
		for(x = area_x1; x < area_x2; x++) {
			
			// Transform source x-coordinate here!
			#ifdef BUFFER_DRAW_W160
				x2 = x;	// 1:1
				(void)x_ofs;
				(void)x_scale;
			#endif
			#ifdef BUFFER_DRAW_W192
				x2 = (x * 5) / 6;	// stretch 160 to 192
				(void)x_ofs;
				(void)x_scale;
			#endif
			#ifdef BUFFER_DRAW_W240
				x2 = (x * 2) / 3;	// stretch 160 to 240
				(void)x_ofs;
				(void)x_scale;
			#endif
			#ifdef BUFFER_DRAW_W320
				if (x_scale) x2 = (x >> 1) + x_ofs;	// stretch 160 to 320 if specified
				else x2 = x + x_ofs;
			#endif
			
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
					lcd_set_pixel_1bit(x, y, 1);	// B/W monochrome (1=black)
					v_err = (v - 0x00);
				} else {
					lcd_set_pixel_1bit(x, y, 0);	// B/W monochrome (0=white)
					v_err = (v - 0xff);
				}
			#endif
		}
	}
}
*/

//void draw_buffer_combined(byte bank_vis, byte bank_pri, byte thresh, byte x_ofs, byte y_ofs, byte x_scale) {	//, byte y_scale) {
void draw_buffer_sprite_priority(
		//byte bank_vis,
		byte bank_pri,
		
		byte *sprite_data,
		byte sprite_w, byte sprite_h,
		byte sprite_trans,
		
		byte sprite_x, byte sprite_y,
		byte sprite_prio,
		
		byte x_ofs, byte y_ofs, byte x_scale //, byte y_scale
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
	
	byte ssx;
	byte ssy;
	byte ssw;
	byte ssh;
	word syo;
	#ifdef BUFFER_DRAW_W160
		ssx = sprite_x;	// 1:1
		ssw = sprite_w;
	#endif
	#ifdef BUFFER_DRAW_W192
		//x2 = (x * 5) / 6;	// stretch 160 to 192
		ssx = (sprite_x * 6) / 5;
		ssw = (sprite_w * 6) / 5;
	#endif
	#ifdef BUFFER_DRAW_W240
		//x2 = (x * 2) / 3;	// stretch 160 to 240
		ssx = (sprite_x * 3) / 2;
		ssw = (sprite_w * 3) / 2;
	#endif
	#ifdef BUFFER_DRAW_W320
		ssx = sprite_x * (x_scale ? 2 : 1);
		ssw = sprite_w * (x_scale ? 2 : 1);
	#endif
	
	
	#ifdef BUFFER_PROCESS_HCROP
		// 1:1 with crop/transform
		//y2 = y + y_src;
		ssy = sprite_y;
		ssh = sprite_h;
	#endif
	#ifdef BUFFER_PROCESS_HCRUSH
		// Scale (crush) 168 down to 100
		//y2 = (y * 5) / 3;
		ssy = (sprite_y * 3) / 5;
		ssh = (sprite_h * 3) / 5;
	#endif
	
	
	
	
	// Transfer (and optionally scale) all pixels to screen
	for(byte iy = 0; iy < ssh; iy++) {
		// Transform source y-coordinate here!
		//y = sprite_y + iy;
		y = ssy + iy;
		// 1:1
		//y2 = y;
		
		// 1:1 with crop/transform
		y2 = y + y_ofs;
		
		// scaling
		//if (y_scale) y2 = (y >> 1) + y_ofs;
		//else y2 = y + y_ofs;
		
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		#ifdef BUFFER_PROCESS_HCROP
			syo = (iy * sprite_w);
		#endif
		#ifdef BUFFER_PROCESS_HCRUSH
			syo = (((iy*5)/3) * sprite_w);
		#endif
		
		
		#ifdef BUFFER_DRAW_DITHER
		//v_err = 0;
		//v_err = ((sprite_x * y) * 0x77) & 0x1f;	// Add some noise
		v_err = ((ssx * y) * 0x77) & 0x1f;	// Add some noise
		#endif
		
		for(byte ix = 0; ix < ssw; ix++) {
			x = ssx + ix;
			
			// Transform source x-coordinate here!
			#ifdef BUFFER_DRAW_W160
				x2 = x;	// 1:1
				(void)x_ofs;
				(void)x_scale;
			#endif
			#ifdef BUFFER_DRAW_W192
				x2 = (x * 5) / 6;	// stretch 160 to 192
				(void)x_ofs;
				(void)x_scale;
			#endif
			#ifdef BUFFER_DRAW_W240
				x2 = (x * 2) / 3;	// stretch 160 to 240
				(void)x_ofs;
				(void)x_scale;
			#endif
			#ifdef BUFFER_DRAW_W320
				if (x_scale) x2 = (x >> 1) + x_ofs;	// stretch 160 to 320 if specified
				else x2 = x + x_ofs;
			#endif
			
			if (x2 >= BUFFER_WIDTH) break;	// Beyond buffer
			
			// Get value from working buffers
			buffer_switch(bank_pri);
			c_prio = buffer_get_pixel_4bit(x2, y2);
			
			// Optional: Skip drawing foreground pixels (faster, but requires visual pixel to be already there)
			if (c_prio >= sprite_prio) continue;	// Just skip (and hope background is correct)
			
			//syo = (iy * sprite_w);
			#ifdef BUFFER_DRAW_W160
				c = sprite_data[syo + ix];
			#endif
			#ifdef BUFFER_DRAW_W192
				c = sprite_data[syo + (ix * 5) / 6];
			#endif
			#ifdef BUFFER_DRAW_W240
				c = sprite_data[syo + (ix * 2) / 3];
			#endif
			#ifdef BUFFER_DRAW_W320
				if (x_scale)	c = sprite_data[syo + (ix >> 1)];
				else			c = sprite_data[syo + ix];
			#endif
			
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
					lcd_set_pixel_1bit(x, y, 1);	// B/W monochrome (1=black)
					v_err = (v - 0x00);
				} else {
					lcd_set_pixel_1bit(x, y, 0);	// B/W monochrome (0=white)
					v_err = (v - 0xff);
				}
			#endif
		}
	}
}


#endif