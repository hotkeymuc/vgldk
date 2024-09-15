/*
	VTech AGI (Adventure Game Interpreter)
	======================================
	
	Trying to run old Sierra AGI adventure games on a GL6000SL/GL7007SL.
	The tricky part: We only have 32 kilobytes of RAM, which also contains VRAM (3000 bytes).
	The RAM is bank-switchable to mount one of the four 8 kilobyte segments to 0xC000 - 0xDFFF.
	
	So, just in order to parse the PIC resources, we have to be inventive to fit the data into RAM:
		* Process each frame (visual / priority) separately, because only one can fit in full res 4 bpp
		* Crop an area out of the full frame, so the result fits into one RAM bank (8192 bytes)
		* Scroll & redraw to allow viewing the whole picture
		* PIC resources must be kept at 4bpp (or more), while the resulting LCD pixel will be 1bpp. Dither!
	
	2024-09-11 Bernhard "HotKey" Slawik
*/


//#define VAGI_MINIMAL	// Squeeze all as much space as possible. No stdio!

#ifdef VAGI_MINIMAL
	#include <vgldk.h>
	#define printf(s) ((void)s)
	#define putchar(c) ((void)c)
	#define getchar() 0
	//#define unit8_t char
	//#define byte unit8_t
	//#define word int
	//#define true 1
	//#define false 0
#else
	// VGLDK_SERIES=0 defines VGLDK_VARIABLE_STDIO to use run-time provided putchar/getchar
	#include <vgldk.h>
	#include <stdiomin.h>	// for gets/puts/printf/printf_d. Will be auto-included on VGLDK_SERIES=0
	
	//#define HEX_USE_DUMP	// For dump(addr, count)
	//#define HEX_DUMP_WIDTH 16	// Usually 4 for 20-character screens
	//#include <hex.h>	// provides printf_x2
#endif

#include <stringmin.h>	// for memcpy()


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


//	0x55: (usually 0x1C = 0b00011100)
//		lowest bit set: Map cartridge at 0x0000 (e.g. "out 55 1d" maps cart to 0x0000 AND 0x8000)
//		2nd    bit set: Map cartridge at 0x4000 (e.g. "out 55 1e" maps cart to 0x4000 AND 0x8000)
//		3rd    bit unset: Reboot loop (e.g. "out 55 18")
//		4th    bit unset: Reboot loop (e.g. "out 55 14")
//		5th    bit unset: nothing, doesn't even gets retained (turns back to 0x1c)
//		!! out 55 0c -> Crash sometimes
//		!! out 55 3c -> Crash with Capslock LED on
//		!! out 55 9c -> nothing, doesn't even gets retained (turns back to 0x1c)
//		!! out 55 fc -> System powers off!
__sfr __at 0x55 bank_type_port;	// This controls whether a bank is internal ROM or cartridge ROM


#define VAGI_STEP_VIS 0
#define VAGI_STEP_PRI 1
byte vagi_drawing_step = VAGI_STEP_VIS;	// Current rendering step (which type of frame to process)

#include "vagi_lcd.h"

#include "vagi_frame.h"	// this handles the full-size AGI frame

#include "vagi_buffer.h"	// this handles the working buffer(s)

// Draw something to the frame buffer
// This is where the original AGI engine should hook into!
void render_frame_spirals_small() {
	byte x;
	byte y;
	
	frame_clear();
	
	// Draw a nice pattern across the whole frame buffer
	for(y = 0; y < AGI_FRAME_HEIGHT; y++) {
		for(x = 0; x < AGI_FRAME_WIDTH; x++) {
			
			// Draw something in 4 bit color (0..15)
			//c = (x * 3) & 0x0f;	// Garbage
			//c = (x / 10) & 0x0f;	// Horizontal gradient (with weird aliasing...)
			//c = ((x >> 2) * (y >> 2)) & 0x0f;	// Spirals
			//c = ((x >> 1) * (y >> 1)) & 0x0f;	// Small Spirals
			//frame_set_pixel_4bit(x, y, ((x >> 1) * (y >> 1)) & 0x0f);		// Small Spirals
			frame_set_pixel_4bit(x, y, ((x * y) >> 1) & 0x0f);		// Small Spirals
		}
	}
}
void render_frame_spirals_large() {
	byte x;
	byte y;
	
	frame_clear();
	
	// Draw a nice pattern across the whole frame buffer
	for(y = 0; y < AGI_FRAME_HEIGHT; y++) {
		for(x = 0; x < AGI_FRAME_WIDTH; x++) {
			// Draw something in 4 bit color (0..15)
			//c = (x * 3) & 0x0f;	// Garbage
			//c = (x / 10) & 0x0f;	// Horizontal gradient (with weird aliasing...)
			//c = ((x >> 2) * (y >> 2)) & 0x0f;	// Spirals
			//c = ((x >> 1) * (y >> 1)) & 0x0f;	// Small Spirals
			//frame_set_pixel_4bit(x, y, ((x >> 2) * (y >> 2)) & 0x0f);		// Large Spirals
			//frame_set_pixel_4bit(x, y, ((x * y) >> 4) & 0x0f);		// Large Spirals
			frame_set_pixel_4bit(x, y, ((x * y) >> 6) & 0x0f);		// Large Spirals
		}
	}
}




#include "agi_pic.h"

void render_frame_agi(byte drawing_step) {
	byte i;
	
	
	// Mount our cartridge ROM to address 0x4000 (data must be copied to ROM binary at position 0x4000 * n!)
	bank_type_port = bank_type_port | 0x02;	// Switch address region 0x4000-0x7FFF to use cartridge ROM (instead of internal ROM)
	bank_0x4000_port = 0x20 | 1;	// Mount ROM segment n=1 (offset 0x4000 * n) to address 0x4000
	//dump(0x4000, 16);
	
	// Tell AGI renderer where the data is located
	_data = (const byte *)0x4000;
	_dataSize = 3306;	// Size of SQ2_PIC_5
	_dataOffset = 0;
	_dataOffsetNibble = 0;
	
	// Prepare drawing state...
	//vagi_drawing_step = VAGI_STEP_VIS;	// Only perform drawing operations for visual (screen) frame
	//vagi_drawing_step = VAGI_STEP_PRI;	// Only perform drawing operations for priority frame
	vagi_drawing_step = drawing_step;	// Only perform drawing operations for either screen OR priority
	_width = 160;
	_height = 168;
	//_patCode = 0;
	//_patNum = 0;
	_scrOn = 1;
	_scrColor = 0xf;
	
	//_pictureVersion = AGIPIC_V1;
	//_pictureVersion = AGIPIC_V15;
	_pictureVersion = AGIPIC_V2;
	_minCommand = 0xf0;
	
	// Actually call AGI drawing routine...
	frame_clear();
	//for(i = 0; i < 10; i++) { draw_Line(0,i*4, 159,167); }	// Test pattern
	
	//drawPictureV1();
	//drawPictureV15();
	drawPictureV2();
	
}

void test_draw_agi() {
	byte bank_vis = 3;
	byte bank_pri = 1;
	byte x_src = 0;
	byte y_src = 0;
	byte x_ofs = 0;
	byte y_ofs = 0;
	byte i;
	
	for(;;) {
		// Render both frames
		//printf("VIS...");
		render_frame_agi(VAGI_STEP_VIS);
		process_frame_to_buffer(bank_vis, x_src, y_src);	// Crop (upper or lower part)
		draw_buffer(bank_vis, 0,0, false);
		
		//printf("PRIO...");
		render_frame_agi(VAGI_STEP_PRI);
		process_frame_to_buffer(bank_pri, x_src, y_src);	// Crop (upper or lower part)
		draw_buffer(bank_pri, 0,0, false);
		
		// Zoom and scroll horizontally (no re-rendering needed)
		for(i = 0; i < 2; i++) {
			//printf("VIS");
			//draw_buffer(bank_vis, 0,0, true);
			draw_buffer(bank_vis, x_ofs,y_ofs, true);
			
			//printf("PRIO");
			draw_buffer(bank_pri, x_ofs,y_ofs, true);
			
			// Next time: Scroll to the other side (left / right)
			if (x_ofs == 0) x_ofs = (160 - (240/2));
			else x_ofs = 0;
		}
		
		// Next time: Crop to the other slice of the frame (upper / lower)
		if (y_src == 0) y_src = (168 - 100);
		else y_src = 0;
	}
}

void test_draw_combined() {
	// Test the drawing pipeline
	byte x;
	byte y;
	byte i;
	
	/*
	byte bank;	// Which bank to use for buffer
	bank = 3;	// Easy: Read from frame banks 1 and 2, write to 3rd bank
	//bank = 1;	// Advanced: Read from frame banks 1 and 2, write BACK to 1st bank (overwriting the frame!)
	
	// Render one full frame in full internal AGI resolution (160x168) at 4 bpp
	// One 4 bit frame (either visual or priority) at internal AGI resolution (160x168) is:
	// 160x168 x 4bit = 26880 / 2 = 13440 bytes = 8192 + 5248 bytes across 2 banks
	printf("Rendering...");
	//render_frame();
	//render_frame_spirals_large();
	render_frame_spirals_small();
	printf("OK\n");
	
	// Now crop and scroll that full frame
	//x = 0;
	for (y = 0; y < 68; y++) {
		
		// Clear destination buffer
		//buffer_switch(bank);	// Map destination working buffer to 0xc000
		//buffer_clear();	// Caution! If re-using the same region for frame and buffer, this will clear the rendered frame!
		
		// Crop and scale from frame to working buffer
		//printf("Processing...");
		process_frame_to_buffer(bank, 0, y);	// Cropy a slice, scrolling vertically
		//printf("OK\n");
		
		// Clear screen (which might be "dirty" because of contiguous buffer)
		//lcd_clear();
		
		// Draw from working buffer (4 bpp) to screen (1 bpp)
		// Draw the buffer 1:1
		//draw_buffer(bank, 0, false);
		
		// Draw the buffer scaled horizontally (like the original games)
		// Scroll the 40 pixels that are missing (120 pixels of frame are shown at 2x scale)
		for (x = 0; x < 40; x+=1) {
			draw_buffer(bank, x, true);	// X-stretch and scroll horizontally
		}
	}
	*/
	// Double draw test (keeping visual and prio in RAM)
	byte bank_vis = 3;
	byte bank_prio = 1;
	
	printf("Rendering...");
	printf("1");
	render_frame_spirals_small();
	//render_frame_agi();
	printf("...");
	process_frame_to_buffer(bank_vis, 0, 0);
	
	printf("2");
	render_frame_spirals_large();
	printf("...");
	process_frame_to_buffer(bank_prio, 0, 0);
	printf("OK");
	
	y = 0;
	for (x = 0; x < 40; x+=1) {
		// Draw buffers sequencially
		draw_buffer(bank_vis, x,y, true);
		draw_buffer(bank_prio, x,y, true);
		
		// Draw combined picture
		for(i = 0; i < 15; i++) {
			draw_buffer_combined(bank_vis, bank_prio, i, x,y, true);
		}
	}
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
	test_draw_agi();
	//test_draw_combined();
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
			
			case 'a':
				test_draw_agi();
				break;
			case 'c':
				test_draw_combined();
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
