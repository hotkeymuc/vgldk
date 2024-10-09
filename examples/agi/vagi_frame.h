#ifndef __VAGI_FRAME_H__
#define __VAGI_FRAME_H__
/*

VAGI Frame functions (handling one full resolution AGI frame)

*/

// Size of AGI "frame" data: 160x168 at 4 bit
#define AGI_FRAME_WIDTH 160	// aka. agi.h:PIC_WIDTH
#define AGI_FRAME_HEIGHT 168	// aka. agi.h:PIC_HEIGHT

#define VAGI_FRAME_ADDR 0xc000	// Frame data will be mapped to this address
//#define VAGI_FRAME_CONTIGUOUS	// Use one contiguous buffer (which WILL cross VRAM and even heap/stack!)
//#define VAGI_FRAME_CONTIGUOUS_BANK 1	// Bank number to use for lower part of contiguous RAM segment

#ifdef VAGI_FRAME_CONTIGUOUS
	// For performance reasons we should create a contiguous area of RAM, so that we do not need to switch banks during intense geometric operations (flood fills / lines).
	// Since VRAM is always at 0xe000 we will HAVE to cross it up until address 0xe000 + 5248 = 0xf480
	// !!! This leaves almost no RAM for the vagi runtime! Must set LOC_CODE/LOC_DATA in Makefile carefully!
	
	void frame_clear(byte b) {
		// Create contiguous area of RAM at 0xc000 - 0xffff, crossing VRAM!
		bank_0xc000_port = VAGI_FRAME_CONTIGUOUS_BANK;	// Mount low bank to 0xc000
		bank_0xe000_port = 0;	// Mount high bank 0 to 0xe000 (default case)
		
		// Clear contiguous buffer
		memset((byte *)VAGI_FRAME_ADDR, b, (AGI_FRAME_HEIGHT * (AGI_FRAME_WIDTH >> 1)));
	}
	
	void frame_contiguous_set_pixel_4bit(byte x, byte y, byte c) {
		// Set 4 bit color value of contiguous buffer at 0xc000
		word a;
		// Calculate the contiguous address
		a = VAGI_FRAME_ADDR + y * (AGI_FRAME_WIDTH >> 1) + (x >> 1);
		// Add (OR) the 4bpp nibble (Note: the buffer must have ben null'ed beforehand!)
		if ((x & 1) == 0)	*(byte *)a |= c;
		else				*(byte *)a |= c << 4;
	}
	
	byte frame_contiguous_get_pixel_4bit(byte x, byte y) {
		// Get 4 bit color value from contiguous buffer at 0xc000
		word a;
		// Calculate the contiguous address
		a = VAGI_FRAME_ADDR + y * (AGI_FRAME_WIDTH >> 1) + (x >> 1);
		// Return desired 4bpp nibble
		if ((x & 1) == 0)	return (*(byte *)a) & 0x0f;
		else				return (*(byte *)a) >> 4;
	}
	#define frame_set_pixel_4bit(x, y, c) frame_contiguous_set_pixel_4bit(x, y, c)
	#define frame_get_pixel_4bit(x, y) frame_contiguous_get_pixel_4bit(x, y)
#else
	// Work across two different banks
	// Note: We need to check for bank switches at EACH SINGLE pixel operation... This is very slow.
	#define VAGI_FRAME_BANK_LO 1	// Bank number to use for the lower part
	#define VAGI_FRAME_BANK_HI 2	// Bank number to use for the upper part
	#define VAGI_FRAME_BANK_SIZE 0x2000	// Size of one memory bank (e.g. 0xC000...0xDFFF = 0x2000)
	
	void frame_clear(byte b) {
		// Clear banked frame buffers
		bank_0xc000_port = VAGI_FRAME_BANK_LO;	// Mount bank 1 to 0xc000
		memset((byte *)VAGI_FRAME_ADDR, b*0x11, VAGI_FRAME_BANK_SIZE);	// Clear whole first buffer (0x2000)
		bank_0xc000_port = VAGI_FRAME_BANK_HI;	// Mount bank 2 to 0xc000
		memset((byte *)VAGI_FRAME_ADDR, b*0x11, (AGI_FRAME_HEIGHT * (AGI_FRAME_WIDTH >> 1)) - VAGI_FRAME_BANK_SIZE);	// Clear remainder inside second buffer
	}
	
	//void inline frame_banked_set_pixel_4bit(byte x, byte y, byte c) {
	void frame_banked_set_pixel_4bit(byte x, byte y, byte c) {
		// Set 4 bit color value of banked buffer at 0xc000
		
		// Range check...
		//if (x >= AGI_FRAME_WIDTH) return;
		//if (y >= AGI_FRAME_HEIGHT) return;
		
		// Convert to address
		word a = ((y * AGI_FRAME_WIDTH) + x) >> 1;
		
		// Do the bank switching
		if (a < VAGI_FRAME_BANK_SIZE) {
			bank_0xc000_port = VAGI_FRAME_BANK_LO;	// Map lower bank to 0xc000
		} else {
			bank_0xc000_port = VAGI_FRAME_BANK_HI;	// Map upper bank to 0xc000
			a &= ~VAGI_FRAME_BANK_SIZE;	//a &= 0xdfff;	//(0xFFFF - 0x2000);	// Clear high bit (it's the bank number)
		}
		
		// Map to RAM address
		a |= VAGI_FRAME_ADDR;
		
		// "Add" (bitwise OR) the color to the 4bpp nibble (Note: the buffer must have ben null'ed beforehand!)
		//if ((x & 1) == 0)	*(byte *)a |= c;
		//else				*(byte *)a |= c << 4;
		
		// Set the 4bpp nibble (Note: the buffer must have ben null'ed beforehand!)
		if ((x & 1) == 0)	*(byte *)a = (*(byte *)a & 0xf0) | c;
		else				*(byte *)a = (*(byte *)a & 0x0f) | (c << 4);
	}
	
	//byte inline frame_banked_get_pixel_4bit(byte x, byte y) {
	byte frame_banked_get_pixel_4bit(byte x, byte y) {
		// Get 4 bit color value from banked buffer at 0xc000
		
		word a = ((y * AGI_FRAME_WIDTH) + x) >> 1;
		
		// Do the bank switch
		if (a < VAGI_FRAME_BANK_SIZE) {
			bank_0xc000_port = VAGI_FRAME_BANK_LO;	// Map lower bank to 0xc000
		} else {
			bank_0xc000_port = VAGI_FRAME_BANK_HI;	// Map upper bank to 0xc000
			a &= ~VAGI_FRAME_BANK_SIZE;	//a &= 0xdfff;	//(0xFFFF - 0x2000);	// Clear high bit (it's the bank number)
		}
		
		// Map to RAM address
		a |= VAGI_FRAME_ADDR;
		
		// Return desired 4bpp nibble
		if ((x & 1) == 0)	return (*(byte *)a) & 0x0f;
		else				return (*(byte *)a) >> 4;
	}
	#define frame_set_pixel_4bit(x, y, c) frame_banked_set_pixel_4bit(x, y, c)
	#define frame_get_pixel_4bit(x, y) frame_banked_get_pixel_4bit(x, y)
	
#endif


// We need to access the buffer
#include "vagi_buffer.h"

/*
byte inline buffer_to_frame_x(byte x) {
	byte x2;
	// 1:1
	//x2 = x;
	
	// 1:1 with crop/transform
	x2 = x;	// + x_src;
	return x2;
}
byte inline buffer_to_frame_y(byte y) {
	byte y2;
	#ifdef BUFFER_PROCESS_HCROP
		// 1:1 with crop/transform
		y2 = y;	// + y_src;
	#endif
	#ifdef BUFFER_PROCESS_HCRUSH
		// Scale (crush) 168 down to 100
		y2 = (y * 5) / 3;
	#endif
	return y2;
}
*/

/*
// X
#ifdef BUFFER_DRAW_W160
	#define buffer_to_frame_x(x) (x)	// 1:1
#endif
#ifdef BUFFER_DRAW_W192
	#define buffer_to_frame_x(x) (x)	// 1:1
#endif
#ifdef BUFFER_DRAW_W240
	#define buffer_to_frame_x(x) (x)	// 1:1
#endif
#ifdef BUFFER_DRAW_W320
	#define buffer_to_frame_x(x) (x)	// 1:1
#endif

// Y
#ifdef BUFFER_PROCESS_HCROP
	#define buffer_to_frame_y (y)	// 1:1 with crop/transform
#endif
#ifdef BUFFER_PROCESS_HCRUSH
	#define buffer_to_frame_y ((y * 5) / 3)	// ~Scale (crush) 168 down to 100
#endif
*/


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
		y2 = buffer_to_frame_y(y) + y_src;
		
		for(x = 0; x < BUFFER_WIDTH; x++) {
			// Transform x coordinate here!
			x2 = buffer_to_frame_x(x) + x_src;
			
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




#endif