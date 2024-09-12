/*
	VTech AGI (Adventure Game Interpreter)
	======================================
	
	Trying to run old Sierra AGI adventure games on a GL6000SL/GL7007SL.
	The tricky part: We only have 32 kilobytes of RAM, which also contains VRAM (3000 bytes).
	The RAM is bank-switchable to mount one of the four 8 kilobyte segments to 0xC000 - 0xDFFF.
	
	So, just in order to parse the PIC resources, we have to be inventive to fit the data into RAM:
		
	
	2024-09-11 Bernhard "HotKey" Slawik
*/

// VGLDK_SERIES=0 defines VGLDK_VARIABLE_STDIO to use run-time provided putchar/getchar
#include <vgldk.h>
#include <stdiomin.h>	// for gets/puts/printf/printf_d. Will be auto-included on VGLDK_SERIES=0

#include <stringmin.h>	// for memcpy()
#include <hex.h>	// provides printf_x2


// Define GL6000SL bank switching ports:
//	0x50 = 0x0000 - 0x3fff
//	0x51 = 0x4000 - 0x7fff
//	0x52 = 0x8000 - 0xbfff
//	0x53 = 0xc000 - 0xdfff
//	0x54 = 0xe000 - 0xffff (VRAM at 0xe000 - 0xebb7)
__sfr __at 0x50 bank_0x0000_port;
__sfr __at 0x51 bank_0x4000_port;
__sfr __at 0x52 bank_0x8000_port;
__sfr __at 0x53 bank_0xc000_port;
__sfr __at 0x54 bank_0xe000_port;

// RAM segments (values to write to the bank ports):
//	bank 0	0x0000 - 0x1fff	8KB VRAM (3000 bytes) at 0xe000 - 0xebb7 and RAM (5192 bytes) at 0xebb8 - 0xffff
//	bank 1	0x2000 - 0x3fff	8KB default RAM at 0xc000 - 0xdfff
//	bank 2	0x4000 - 0x5fff	8KB extended RAM at 0xc000 - 0xdfff
//	bank 3	0x6000 - 0x7fff	8KB extended RAM at 0xc000 - 0xdfff

#define LCD_WIDTH 240
#define LCD_HEIGHT 100
#define LCD_ADDR 0xe000

// Size of AGI internal data (at 4 bit)
#define AGI_WIDTH 160
#define AGI_HEIGHT 168
#define AGI_FRAME_ADDR 0xc000	// Will be mapped there
//#define AGI_FRAME_CONTIGUOUS	// Use one contiguous buffer (which WILL cross VRAM and maybe heap!)

// Size of our internal working buffers (at 4 bit)
#define BUFFER_WIDTH 160
#define BUFFER_HEIGHT 100
#define BUFFER_ADDR 0xc000	// Will always be mapped there

#ifdef AGI_FRAME_CONTIGUOUS
void frame_contiguous_set_pixel_4bit(byte x, byte y, byte c) {
	// Set 4 bit color value of contiguous buffer at 0xc000
	word a;
	// Calculate the contiguous address
	a = AGI_FRAME_ADDR + y * (AGI_WIDTH >> 1) + (x >> 1);
	// Add (OR) the 4bpp nibble (Note: the buffer must have ben null'ed beforehand!)
	if ((x & 1) == 0)	*(byte *)a |= c;
	else				*(byte *)a |= c << 4;
}
byte frame_contiguous_get_pixel_4bit(byte x, byte y) {
	// Get 4 bit color value from contiguous buffer at 0xc000
	word a;
	// Calculate the contiguous address
	a = AGI_FRAME_ADDR + y * (AGI_WIDTH >> 1) + (x >> 1);
	// Return desired 4bpp nibble
	if ((x & 1) == 0)	return (*(byte *)a) & 0x0f;
	else				return (*(byte *)a) >> 4;
}
#else
void frame_banked_set_pixel_4bit(byte x, byte y, byte c) {
	// Set 4 bit color value of banked buffer at 0xc000
	//word a = y * (AGI_WIDTH >> 1) + (x >> 1);
	word a = ((y * AGI_WIDTH) + x) >> 1;
	// Do the bank switching
	if (a < 0x2000) {
		bank_0xc000_port = 1;	// Map lower bank to 0xc000
	} else {
		bank_0xc000_port = 2;	// Map upper bank to 0xc000
		a &= (0xFFFF - 0x2000);	// Clear high bit
	}
	// Map to RAM address
	a |= AGI_FRAME_ADDR;
	
	// Add (OR) the 4bpp nibble (Note: the buffer must have ben null'ed beforehand!)
	if ((x & 1) == 0)	*(byte *)a |= c;
	else				*(byte *)a |= c << 4;
}
byte frame_banked_get_pixel_4bit(byte x, byte y) {
	// Get 4 bit color value from banked buffer at 0xc000
	//word a = y * (AGI_WIDTH >> 1) + (x >> 1);
	word a = ((y * AGI_WIDTH) + x) >> 1;
	
	// Do the bank switching
	if (a < 0x2000) {
		bank_0xc000_port = 1;	// Map lower bank to 0xc000
	} else {
		bank_0xc000_port = 2;	// Map upper bank to 0xc000
		a &= (0xFFFF - 0x2000);	// Clear high bit
	}
	
	// Map to RAM address
	a |= AGI_FRAME_ADDR;
	
	// Return desired 4bpp nibble
	if ((x & 1) == 0)	return (*(byte *)a) & 0x0f;
	else				return (*(byte *)a) >> 4;
}
#endif


void buffer_set_pixel_4bit(byte x, byte y, byte c) {
	// Set 4 bit color value of working buffer at 0xc000
	/*
	word a;
	a = BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1);
	if ((x & 1) == 0)	*(byte *)a |= c;
	else				*(byte *)a |= c << 4;
	*/
	if ((x & 1) == 0)	*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c;
	else				*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1)) |= c << 4;
}
byte buffer_get_pixel_4bit(byte x, byte y) {
	// Set 4 bit color value of working buffer at 0xc000
	/*
	word a;
	a = BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1);
	if ((x & 1) == 0)	return (*(byte *)a) & 0x0f;
	else				return (*(byte *)a) >> 4;
	*/
	if ((x & 1) == 0)	return (*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1))) & 0x0f;
	else				return (*(byte *)(BUFFER_ADDR + y * (BUFFER_WIDTH >> 1) + (x >> 1))) >> 4;
}

void lcd_set_pixel_1bit(byte x, byte y, byte c) {
	// Draw to LCD VRAM (1bpp)
	word a;
	a = LCD_ADDR + y * (LCD_WIDTH >> 3) + (x >> 3);
	if (c)	*(byte *)a |= 1 << (x & 0x07);
	else	*(byte *)a &= 0xff - (1 << (x & 0x07));
}

void lcd_set_pixel_4bit(byte x, byte y, byte c) {
	// Draw pixel with tone mapping / dithering
	// 3 scales:
	//if (c < 6) c = 0;
	//else if (c < 12) c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) );
	//else c = 1;
	
	// 5 scales
	     if (c <  3) c = 0;
	else if (c <  7) c = ((x + y) & 3) ? 0 : 1;
	else if (c < 11) c = ((y & 1) ? ((x & 1) ? 1 : 0) : ((x & 1) ? 0 : 1) );
	else if (c < 14) c = ((x + y) & 3) ? 1 : 0;
	else             c = 1;
	lcd_set_pixel_1bit(x, y, c);
}

void render() {
	byte x;
	byte y;
	byte x2;
	byte y2;
	
	//word a_row;
	//word a;
	
	byte c;
	//byte *p;
	
	// One 4 bit frame (either visual or priority) at internal AGI resolution (160x168) is:
	// 160x168 x 4bit = 26880 / 2 = 13440 bytes = 8192 + 5248 bytes across 2 banks
	
	printf("Clearing...");
	#ifdef AGI_FRAME_CONTIGUOUS
		// For performance reasons we should create a contiguous area of RAM, so that we do not need to switch banks during intense geometric operations (flood fills / lines).
		// Since VRAM is always at 0xe000 we will HAVE to cross it up until address 0xe000 + 5248 = 0xf480
		// !!! This leaves almost no RAM for the vagi runtime! Must set LOC_CODE/LOC_DATA in Makefile carefully!
		// Create contiguous area of RAM at 0xc000 - 0xffff, crossing VRAM!
		bank_0xc000_port = 1;	// Mount bank 1 to 0xc000
		bank_0xe000_port = 0;	// Mount bank 0 to 0xe000 (default case)
		
		// Clear contiguous buffer
		memset((byte *)0xc000, 0x00, (AGI_HEIGHT * (AGI_WIDTH >> 1)));
	#else
		// Alternatively we could work completely banked, but ceck for bank switches at each single pixel operation...
		// Clear banked frame buffers
		bank_0xc000_port = 1;	// Mount bank 1 to 0xc000
		memset((byte *)0xc000, 0x00, 0x2000);	// Clear whole first buffer
		bank_0xc000_port = 2;	// Mount bank 2 to 0xc000
		memset((byte *)0xc000, 0x00, (AGI_HEIGHT * (AGI_WIDTH >> 1)) - 0x2000);	// Clear remainder inside second buffer
	#endif
	printf("OK\n");
	
	// Draw to contiguous area in AGI resolution at 4 bpp
	printf("Drawing...");
	for(y = 0; y < AGI_HEIGHT; y++) {
		#ifdef AGI_FRAME_CONTIGUOUS
			// Pre-calculate row address (only once for each row)
			word a_row = 0xc000 + y * (AGI_WIDTH >> 1);
		#endif
		//c = ((y & 1) == 0) ? 0x05 : 0x0a;	// 50% grey
		
		for(x = 0; x < AGI_WIDTH; x++) {
			// Draw something in 4 bit color (0..15)
			//c = (x * 3) & 0x0f;
			//c = (x / 10) & 0x0f;	// Horizontal gradient
			c = ((x >> 2) * (y >> 2)) & 0x0f;	// Spirals
			//c = ((x >> 1) * (y >> 1)) & 0x0f;	// Small Spirals
			
			#ifdef AGI_FRAME_CONTIGUOUS
				// Naive contiguous mode:
				//a = 0xc000 + y * (AGI_WIDTH >> 1) + (x >> 1);
				
				// Faster: Use pre-calculated row address
				//word a = a_row + (x >> 1);
				//if ((x & 1) == 0)	*(byte *)a |= c;
				//else				*(byte *)a |= c << 4;
				
				// Clean via function:
				frame_contiguous_set_pixel_4bit(x, y, c);
			#else
				frame_banked_set_pixel_4bit(x, y, c);
			#endif
		}
	}
	printf("OK\n");
	
	printf("Cropping...");
	// Crop and scale from frame to working buffer
	for(y = 0; y < BUFFER_HEIGHT; y++) {
		y2 = y;
		for(x = 0; x < BUFFER_WIDTH; x++) {
			// Get pixel from contiguous frame
			x2 = x;
			#ifdef AGI_FRAME_CONTIGUOUS
				bank_0xc000_port = 1;	// Map contiguous lower bank to 0xc000
				c = frame_contiguous_get_pixel_4bit(x2, y2);
			#else
				c = frame_banked_get_pixel_4bit(x2, y2);
			#endif
			
			// Write to final working buffer
			bank_0xc000_port = 3;	// Map working buffer to 0xc000
			buffer_set_pixel_4bit(x, y, c);
		}
	}
	printf("OK\n");
	
	// Clear screen (which is "dirty" because of contiguous buffer)
	memset((byte *)LCD_ADDR, 0x00, (LCD_HEIGHT * (LCD_WIDTH >> 3)));
	
	// Render from working buffer (4 bpp) to screen (1 bpp)
	//printf("Rendering...");
	bank_0xc000_port = 3;	// Map working buffer to 0xc000
	for(y = 0; y < LCD_HEIGHT; y++) {
		y2 = y;
		if (y2 >= BUFFER_HEIGHT) break;	// Beyond buffer
		
		for(x = 0; x < LCD_WIDTH; x++) {
			// Get pixel from working buffer
			x2 = x;
			
			if (x2 >= BUFFER_WIDTH) break;	// Beyond buffer
			c = buffer_get_pixel_4bit(x2, y2);
			
			// Draw to VRAM
			//lcd_set_pixel_1bit(x, y, c);	// With 4-to-1 bpp tone mapping
			lcd_set_pixel_4bit(x, y, c);	// With 4-to-1 bpp tone mapping
		}
	}
	//printf("OK\n");
}


#if VGLDK_SERIES == 0
// app
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
#else
// regular cart ROM does not provide command line args
void main() __naked {
#endif
	
	byte running = 1;
	char c;
	//word i;
	
	printf("VAGI\n");
	
	//printf("rendering...");
	//render();
	//printf("done.\n");
	
	
	while(running) {
		
		putchar('?');
		c = getchar();
		
		switch(c) {
			case 13:
			case 10:
				break;
			
			case 'q':
			case 'Q':
				// Soft reset
				__asm
					;rst0
					call #0x0000
				__endasm;
				break;
			
			case 'x':
			case 'X':
				// Exit
				__asm
					;rst0
					;call #0x8002
					;return
				__endasm;
				running = 0;
				break;
			
			case 'h':
				// Help
				break;
			
			case 'r':
				render();
				break;
			
			case 0x1b:	// LEFT
				break;
			case 0x1a:	// RIGHT
				break;
			
			//	default:
			//		//printf_x2(c); putchar('?');
			//		printf_d(c); putchar('?');
			//		break;
		}
	}
	
	#if VGLDK_SERIES == 0
	return 42;	// Apps can return stuff
	#endif
}
