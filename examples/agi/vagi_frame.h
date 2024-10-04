#ifndef __VAGI_FRAME_H__
#define __VAGI_FRAME_H__

// Size of AGI "frame" data: 160x168 at 4 bit
#define AGI_FRAME_WIDTH 160
#define AGI_FRAME_HEIGHT 168

#define VAGI_FRAME_ADDR 0xc000	// Will be mapped there
//#define VAGI_FRAME_CONTIGUOUS	// Use one contiguous buffer (which WILL cross VRAM and maybe heap!)
//#define VAGI_FRAME_CONTIGUOUS_BANK 1	// Bank number to use for lower part of contiguous RAM segment


// Frame functions (handling one full resolution AGI frame)
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
	// Alternatively: Work across two different banks
	// Note: We need to ceck for bank switches at EACH SINGLE pixel operation...
	#define VAGI_FRAME_BANK_LO 1
	#define VAGI_FRAME_BANK_HI 2
	#define VAGI_FRAME_BANK_SIZE 0x2000	// Size of one memory bank
	
	void frame_clear(byte b) {
		// Clear banked frame buffers
		bank_0xc000_port = VAGI_FRAME_BANK_LO;	// Mount bank 1 to 0xc000
		memset((byte *)VAGI_FRAME_ADDR, b, VAGI_FRAME_BANK_SIZE);	// Clear whole first buffer (0x2000)
		bank_0xc000_port = VAGI_FRAME_BANK_HI;	// Mount bank 2 to 0xc000
		memset((byte *)VAGI_FRAME_ADDR, b, (AGI_FRAME_HEIGHT * (AGI_FRAME_WIDTH >> 1)) - VAGI_FRAME_BANK_SIZE);	// Clear remainder inside second buffer
	}
	
	//void inline frame_banked_set_pixel_4bit(byte x, byte y, byte c) {
	void frame_banked_set_pixel_4bit(byte x, byte y, byte c) {
		// Set 4 bit color value of banked buffer at 0xc000
		
		//word a = y * (AGI_FRAME_WIDTH >> 1) + (x >> 1);
		word a = ((y * AGI_FRAME_WIDTH) + x) >> 1;
		
		// Do the bank switching
		if (a < VAGI_FRAME_BANK_SIZE) {
			bank_0xc000_port = VAGI_FRAME_BANK_LO;	// Map lower bank to 0xc000
		} else {
			bank_0xc000_port = VAGI_FRAME_BANK_HI;	// Map upper bank to 0xc000
			//a &= 0xdfff;	//(0xFFFF - 0x2000);	// Clear high bit (it's the bank number)
			a &= ~VAGI_FRAME_BANK_SIZE;	// Clear high bit
		}
		
		// Map to RAM address
		a |= VAGI_FRAME_ADDR;
		
		// Add (OR) the 4bpp nibble (Note: the buffer must have ben null'ed beforehand!)
		//if ((x & 1) == 0)	*(byte *)a |= c;
		//else				*(byte *)a |= c << 4;
		
		// Set the 4bpp nibble (Note: the buffer must have ben null'ed beforehand!)
		if ((x & 1) == 0)	*(byte *)a = (*(byte *)a & 0xf0) | c;
		else				*(byte *)a = (*(byte *)a & 0x0f) | (c << 4);
	}
	
	//byte inline frame_banked_get_pixel_4bit(byte x, byte y) {
	byte frame_banked_get_pixel_4bit(byte x, byte y) {
		// Get 4 bit color value from banked buffer at 0xc000
		
		//word a = y * (AGI_FRAME_WIDTH >> 1) + (x >> 1);
		word a = ((y * AGI_FRAME_WIDTH) + x) >> 1;
		
		// Do the bank switch
		if (a < VAGI_FRAME_BANK_SIZE) {
			bank_0xc000_port = VAGI_FRAME_BANK_LO;	// Map lower bank to 0xc000
		} else {
			bank_0xc000_port = VAGI_FRAME_BANK_HI;	// Map upper bank to 0xc000
			//a &= 0xdfff;	//(0xFFFF - 0x2000);	// Clear high bit (it's the bank number)
			a &= ~VAGI_FRAME_BANK_SIZE;	// Clear high bit
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



#endif