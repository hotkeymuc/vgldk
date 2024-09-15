#ifndef __VAGI_BUFFER_H__
#define __VAGI_BUFFER_H__

// Working buffer functions (holding one reduced frame in a single RAM bank, ready for the engine to access it)
// Size of our internal working buffers (at 4 bit)
#define BUFFER_WIDTH 160
#define BUFFER_HEIGHT 100
#define BUFFER_ADDR 0xc000	// Will always be banked there

//#define BUFFER_DITHER	// Use simple error dithering. Looks great, but is problematic for partial redraws!

//#define RGB_TO_LCD_4BPP(r,g,b) (0xf - ((r+g+b)/(3*0x11)))
//#define RGB_TO_LUMA_4BPP(r,g,b) ((r+g+b)/(3*0x11))
//#define RGB_TO_LUMA(r,g,b) ((r+g+b) / 3)
#define RGB_TO_LUMA(r,g,b) (0.299*r + 0.587*g + 0.114*b)


void inline buffer_switch(byte bank) {
	// Mount a different RAM segment to BUFFER_ADDR (0xc000)
	bank_0xc000_port = bank;
}
void buffer_clear() {
	memset((byte *)BUFFER_ADDR, 0x00, ((BUFFER_WIDTH * BUFFER_HEIGHT) >> 1));
}
void inline buffer_add_pixel_4bit(byte x, byte y, byte c) {
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
void inline buffer_set_pixel_4bit(byte x, byte y, byte c) {
	// Set 4 bit color value of the working buffer at 0xc000
	byte *a = (byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1));
	if (x & 1)	*a = (*a & 0x0f) | (c << 4);
	else		*a = (*a & 0xf0) | c;
}
byte inline buffer_get_pixel_4bit(byte x, byte y) {
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


void process_frame_to_buffer(byte dest_bank, byte x_src, byte y_src) {
	// Crop/scale/scroll full frame into working buffer (can re-use source bank as destination!)
	byte x;
	byte y;
	byte x2;
	byte y2;
	byte c;
	
	// Clear destination buffer (Cuation! When re-using the same bank for frame and buffer, this will clear the frame!)
	//bank_0xc000_port = dest_bank;	// Map destination working buffer to 0xc000
	//buffer_switch(dest_bank);
	//buffer_clear();
	
	// Copy (and transform) pixels from full frame to reduced buffer
	for(y = 0; y < BUFFER_HEIGHT; y++) {
		// Transform y coordinate here!
		//y2 = y;
		y2 = y + y_src;
		for(x = 0; x < BUFFER_WIDTH; x++) {
			// Transform x coordinate here!
			//x2 = x;
			x2 = x + x_src;
			
			// Get pixel from frame
			/*
			#ifdef AGI_FRAME_CONTIGUOUS
				// Get pixel from contiguous frame
				//bank_0xc000_port = AGI_FRAME_CONTIGUOUS_BANK;	// Map contiguous lower bank to 0xc000
				buffer_switch(AGI_FRAME_CONTIGUOUS_BANK);
				c = frame_contiguous_get_pixel_4bit(x2, y2);
			#else
				// Get pixel from banked frame
				c = frame_banked_get_pixel_4bit(x2, y2);
			#endif
			*/
			c = frame_get_pixel_4bit(x2, y2);
			
			// Write to final working buffer
			buffer_switch(dest_bank);	// Map destination working buffer to 0xc000
			buffer_set_pixel_4bit(x, y, c);
		}
	}
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

void draw_pixel_luma(byte x, byte y, byte c) {
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
	
	#ifdef BUFFER_DITHER
	// Dithering
	int v;
	int v_err;
	#endif
	
	// Map working buffer to 0xc000
	//bank_0xc000_port = bank;
	buffer_switch(bank);
	
	// Transfer (and optionally scale) all pixels to screen
	//for(y = 0; y < LCD_HEIGHT; y++) {
	for(y = area_y1; y < area_y2; y++) {
		// Transform source y-coordinate here!
		//y2 = y;
		y2 = y + y_ofs;
		//if (y_scale) y2 = (y >> 1) + y_ofs;
		//else y2 = y + y_ofs;
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		#ifdef BUFFER_DITHER
		v_err = 0;
		#endif
		
		//for(x = 0; x < LCD_WIDTH; x++) {
		for(x = area_x1; x < area_x2; x++) {
			// Transform source x-coordinate here!
			//x2 = x;
			if (x_scale) x2 = (x >> 1) + x_ofs;
			else x2 = x + x_ofs;
			if (x2 >= BUFFER_WIDTH) break;	// Beyond buffer
			
			// Get pixel from working buffer
			c = buffer_get_pixel_4bit(x2, y2);
			
			// Draw pixel to VRAM
			#ifdef BUFFER_DITHER
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
			#else
				//lcd_set_pixel_1bit(x, y, c);	// B/W monochrome
				//lcd_set_pixel_4bit(x, y, c);	// With 4-to-1 bpp tone mapping
				draw_pixel_luma(x, y, AGI_PALETTE_TO_LUMA[c]);	// Do palette luma tone mapping
			#endif
			
		}
	}
}


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
	byte c_vis;
	byte c_prio;
	
	#ifdef BUFFER_DITHER
	// Dithering
	int v;
	int v_err;
	#endif
	
	// Transfer (and optionally scale) all pixels to screen
	//for(y = 0; y < LCD_HEIGHT; y++) {
	for(y = area_y1; y < area_y2; y++) {
		// Transform source y-coordinate here!
		//y2 = y;
		y2 = y + y_ofs;
		//if (y_scale) y2 = (y >> 1) + y_ofs;
		//else y2 = y + y_ofs;
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		#ifdef BUFFER_DITHER
		v_err = 0;
		#endif
		//for(x = 0; x < LCD_WIDTH; x++) {
		for(x = area_x1; x < area_x2; x++) {
			// Transform source x-coordinate here!
			//x2 = x;
			if (x_scale) x2 = (x >> 1) + x_ofs;
			else x2 = x + x_ofs;
			if (x2 >= BUFFER_WIDTH) break;	// Beyond buffer
			
			// Get value from working buffers
			buffer_switch(bank_pri);
			c_prio = buffer_get_pixel_4bit(x2, y2);
			
			// Do some simple threshold combination
			if (thresh > c_prio) continue;	// Behind! Skip!
			
			buffer_switch(bank_vis);
			c_vis = buffer_get_pixel_4bit(x2, y2);
			
			// Draw to VRAM
			#ifdef BUFFER_DITHER
				// Dithering with luma
				v = AGI_PALETTE_TO_LUMA[c_vis];
				v += v_err;
				if (v < 0x80) {
					lcd_set_pixel_1bit(x, y, 1);	// B/W monochrome (1=black)
					v_err = (v - 0x00);
				} else {
					lcd_set_pixel_1bit(x, y, 0);	// B/W monochrome (0=white)
					v_err = (v - 0xff);
				}
			#else
				//lcd_set_pixel_1bit(x, y, c_vis);	// B/W monochrome
				//lcd_set_pixel_4bit(x, y, c_vis);	// With 4-to-1 bpp tone mapping
				draw_pixel_luma(x, y, AGI_PALETTE_TO_LUMA[c_vis]);	// Do palette luma tone mapping
			#endif
		}
	}
}

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
	
	#ifdef BUFFER_DITHER
	// Dithering
	int v;
	int v_err;
	#endif
	
	// Transfer (and optionally scale) all pixels to screen
	for(byte iy = 0; iy < sprite_h; iy++) {
		// Transform source y-coordinate here!
		y = sprite_y + iy;
		//y2 = y;
		y2 = y + y_ofs;
		//if (y_scale) y2 = (y >> 1) + y_ofs;
		//else y2 = y + y_ofs;
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		#ifdef BUFFER_DITHER
		v_err = 0;
		#endif
		for(byte ix = 0; ix < (sprite_w * (x_scale ? 2 : 1)); ix++) {
			// Transform source x-coordinate here!
			x = sprite_x + ix;
			//x2 = x;
			if (x_scale) x2 = (x >> 1) + x_ofs;
			else x2 = x + x_ofs;
			if (x2 >= BUFFER_WIDTH) break;	// Beyond buffer
			
			// Get value from working buffers
			buffer_switch(bank_pri);
			c_prio = buffer_get_pixel_4bit(x2, y2);
			
			// Optional: Skip drawing foreground pixels (faster, but requires visual pixel to be already there)
			if (c_prio >= sprite_prio) continue;	// Just skip (and hope background is correct)
			
			if (x_scale)	c = sprite_data[(iy * sprite_w) + (ix >> 1)];
			else			c = sprite_data[(iy * sprite_w) + ix];
			
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
			#ifdef BUFFER_DITHER
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
			#else
				//lcd_set_pixel_1bit(x, y, c);	// B/W monochrome
				//lcd_set_pixel_4bit(x, y, c);	// With 4-to-1 bpp tone mapping
				draw_pixel_luma(x, y, AGI_PALETTE_TO_LUMA[c]);	// Do palette luma tone mapping
			#endif
		}
	}
}


#endif