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
#define VAGI_MOUSE	// Support mouse

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

#ifdef VAGI_MOUSE
	#include <mouse.h>
#endif


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
	
	frame_clear(0x00);
	
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
	
	frame_clear(0x00);
	
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
		//draw_buffer(bank, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
		
		// Draw the buffer scaled horizontally (like the original games)
		// Scroll the 40 pixels that are missing (120 pixels of frame are shown at 2x scale)
		for (x = 0; x < 40; x+=1) {
			draw_buffer(bank, 0,LCD_WIDTH, 0,LCD_HEIGHT, x,0, false);	// X-stretch and scroll horizontally
		}
	}
	*/
	// Double draw test (keeping visual and prio in RAM)
	byte bank_vis = 3;
	byte bank_pri = 1;
	
	printf("Rendering...");
	printf("1");
	render_frame_spirals_small();
	printf("...");
	process_frame_to_buffer(bank_vis, 0, 0);
	
	printf("2");
	render_frame_spirals_large();
	printf("...");
	process_frame_to_buffer(bank_pri, 0, 0);
	printf("OK");
	
	y = 0;
	for (x = 0; x < 40; x+=1) {
		// Draw buffers sequencially
		//draw_buffer(bank_vis, x,y, true);
		draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, x,y, true);
		//draw_buffer(bank_pri, x,y, true);
		draw_buffer(bank_pri, 0,LCD_WIDTH, 0,LCD_HEIGHT, x,y, true);
		
		// Draw combined picture
		for(i = 0; i < 15; i++) {
			//draw_buffer_combined(bank_vis, bank_pri, thresh, area_x1, area_x2, area_y1, area_y2, x_ofs,y_ofs, true);
			draw_buffer_combined(bank_vis, bank_pri, i, 0,LCD_WIDTH, 0,LCD_HEIGHT, x,y, true);
		}
	}
}


// The fun starts here!

#include "agi_pic.h"

void render_frame_agi(byte drawing_step) {
	// Draw one AGI PIC (either its visual or priority data)
	
	// Mount our cartridge ROM to address 0x4000 (data must be inside the ROM binary at position 0x4000 * n)
	bank_type_port = bank_type_port | 0x02;	// Switch address region 0x4000-0x7FFF to use cartridge ROM (instead of internal ROM)
	bank_0x4000_port = 0x20 | 1;	// Mount ROM segment n=1 (offset 0x4000 * n) to address 0x4000
	//dump(0x4000, 16);
	
	// Tell AGI renderer where the data is located
	_data = (const byte *)0x4000;
	
	//_dataSize = 2978;	// Size of SQ2_PIC_1 (intro space station)
	//_dataSize = 3400;	// Size of SQ2_PIC_2 (first screen) - YAY!
	//_dataSize = 2592;	// Size of SQ2_PIC_3 (change room)
	//_dataSize = 2396;	// Size of SQ2_PIC_4 (control room)
	_dataSize = 3306;	// Size of SQ2_PIC_5 (space ship hangar)
	//_dataSize = 3447;	// Size of SQ2_PIC_6 (vohaul without head)
	//_dataSize = 2533;	// Size of SQ2_PIC_7 (vohaul base in orbit) - NICE!
	//_dataSize = 3266;	// Size of SQ2_PIC_8 (landing pad) - NIICE with dithering!
	//_dataSize = 46;	// Size of SQ2_PIC_9 (empty)
	//_dataSize = 3104;	// Size of SQ2_PIC_10 (jungle)
	//_dataSize = 4180;	// Size of SQ2_PIC_11 (jungle2)
	//_dataSize = 3751;	// Size of SQ2_PIC_12 (jungle3 tree)
	//_dataSize = 3880;	// Size of SQ2_PIC_13 (swamp bay left)
	
	_dataOffset = 0;
	_dataOffsetNibble = 0;
	
	// Prepare drawing state...
	//vagi_drawing_step = VAGI_STEP_VIS;	// Only perform drawing operations for visual (screen) frame
	//vagi_drawing_step = VAGI_STEP_PRI;	// Only perform drawing operations for priority frame
	vagi_drawing_step = drawing_step;	// Only perform drawing operations for either screen OR priority
	_width = 160;
	_height = 168;
	
	_patCode = 0;
	_patNum = 0;
	_priOn = false;
	_scrOn = false;
	_scrColor = 15;
	_priColor = 4;
	
	//_pictureVersion = AGIPIC_V1;
	//_pictureVersion = AGIPIC_V15;
	_pictureVersion = AGIPIC_V2;
	_minCommand = 0xf0;
	
	// Actually call AGI drawing routine...
	frame_clear(0x00);
	if (drawing_step == VAGI_STEP_VIS) frame_clear(_scrColor * 0x11);	// VIS: bg=0xf (on both nibbles)
	if (drawing_step == VAGI_STEP_PRI) frame_clear(_priColor * 0x11);	// PRI: bg=0x4 (on both nibbles)
	//for(i = 0; i < 10; i++) { draw_Line(0,i*4, 159,167); }	// Test pattern
	
	//drawPictureV1();
	//drawPictureV15();
	drawPictureV2();
	
}

void test_draw_agi_scroll() {
	byte bank_vis = 3;
	//byte bank_pri = 1;
	byte x_src = 0;
	byte y_src = 0;
	byte x_ofs = 0;
	byte y_ofs = 0;
	byte i;
	
	y_src = (AGI_FRAME_HEIGHT - LCD_HEIGHT) / 2;	// Start in the middle
	
	for(;;) {
		// Render frame(s)
		//lcd_text_col = 0; lcd_text_row = 0; printf("VIS...");
		render_frame_agi(VAGI_STEP_VIS);
		process_frame_to_buffer(bank_vis, x_src, y_src);	// Crop (upper or lower part)
		//draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);	// Show visual buffer while priority is being rendered
		
		//lcd_text_col = 0; lcd_text_row = 0; printf("PRIO...");
		//render_frame_agi(VAGI_STEP_PRI);
		//process_frame_to_buffer(bank_pri, x_src, y_src);	// Crop (upper or lower part)
		//draw_buffer(bank_pri, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
		
		// Scroll horizontally (no re-rendering needed)
		const byte step = 8;	//(AGI_FRAME_WIDTH - (LCD_WIDTH/2)) / steps;	// Must be multiple of 8 for hardware-scroll to work
		const byte steps = (AGI_FRAME_WIDTH - (LCD_WIDTH/2)) / step;
		
		for(i = 0; i < steps; i++) {
			//lcd_clear();
			x_ofs = (i * (AGI_FRAME_WIDTH - (LCD_WIDTH/2))) / steps;
			
			//printf("VIS");
			//draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
			if (x_ofs == 0) {
				// Draw full frame
				draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
			} else {
				// Scroll VRAM to the left
				memcpy((byte *)LCD_ADDR, (byte *)(LCD_ADDR + step/8), (LCD_HEIGHT*LCD_WIDTH - step)/8);
				//@TODO: Use LDIR (copy BC bytes from HL to DE)!
				/*
				__asm
				push bc
				push de
				push hl
				
				// Scroll one line up
				ld bc, #((LCD_W * (LCD_H - LCD_SCROLL_AMOUNT)) / 8)	// Number of bytes to scroll (i.e. whole screen minus x lines)
				ld hl, #(LCD_ADDR + (LCD_W / 8) * LCD_SCROLL_AMOUNT)	//#0xE01E	// Offset of 2nd line, i.e. LCD_ADDR + bytes-per-line * x
				ld de, #LCD_ADDR	//#0xE000	// Offset of 1st line
				ldir	// Copy BC bytes from HL to DE
				
				pop hl
				pop de
				pop bc
				__endasm;
				*/
				
				// Only re-draw right region
				//draw_buffer(bank_vis, area_x1, area_x2, area_y1, area_y2, x_ofs,y_ofs, true);
				draw_buffer(bank_vis, LCD_WIDTH-step,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
				
			}
			
			
			//printf("PRIO");
			//draw_buffer(bank_pri, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
			
			// Next time: Scroll to the other side (left / right)
			//if (x_ofs == 0) x_ofs = (AGI_FRAME_WIDTH - (LCD_WIDTH/2));
			//else x_ofs = 0;
		}
		
		// Next time: Crop to the other slice of the frame (upper / lower)
		if (y_src == 0) y_src = (AGI_FRAME_HEIGHT - LCD_HEIGHT);
		else y_src = 0;
	}
}

#define sprite_width 8
#define sprite_height 12
#define sprite_transparency 3
static const byte sprite_data[8*12] = {
	  3,   3, 0xf, 0xf, 0xf, 0xf,   3,   3, 
	  3,   3, 0xf, 0xf, 0xf, 0xf,   3,   3, 
	  3, 0xf, 0x0, 0x0, 0x0, 0x0, 0xf,   3, 
	0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 
	0xf, 0x0, 0xf, 0x0, 0x0, 0xf, 0x0, 0xf, 
	0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 
	0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 
	0xf, 0x0, 0xf, 0x0, 0x0, 0xf, 0x0, 0xf, 
	0xf, 0x0, 0x0, 0xf, 0xf, 0x0, 0x0, 0xf, 
	  3, 0xf, 0x0, 0x0, 0x0, 0x0, 0xf,   3, 
	  3,   3, 0xf, 0x0, 0x0, 0xf,   3,   3, 
	  3,   3,   3, 0xf, 0xf,   3,   3,   3, 
};


void test_draw_agi_combined() {
	byte bank_vis = 3;
	byte bank_pri = 1;
	byte x_src = 0;
	byte y_src = 0;
	byte x_ofs = 0;
	byte y_ofs = 0;
	
	y_src = (AGI_FRAME_HEIGHT - LCD_HEIGHT) / 2;	// Start in the middle
	byte x = AGI_FRAME_WIDTH / 2;
	byte y = AGI_FRAME_HEIGHT / 2;
	byte prio = 5;
	byte spd = 5;
	
	bool render = true;
	bool redraw = true;
	
	for(;;) {
		if (render) {
			// Render both frames
			lcd_text_col = 0; lcd_text_row = 0; printf("VIS...");
			render_frame_agi(VAGI_STEP_VIS);
			process_frame_to_buffer(bank_vis, x_src, y_src);	// Crop (upper or lower part)
			draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);	// Show visual buffer while priority is being rendered
			
			lcd_text_col = 0; lcd_text_row = 0; printf("PRIO...");
			render_frame_agi(VAGI_STEP_PRI);
			process_frame_to_buffer(bank_pri, x_src, y_src);	// Crop (upper or lower part)
			//draw_buffer(bank_pri, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
			
			// Draw full background
			draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
			
			render = false;
			redraw = true;
		}
		
		if (redraw) {
			// Redraw the full rendered frame
			draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
			redraw = false;
		}
		
		lcd_text_col = 0; lcd_text_row = 0;	//(LCD_HEIGHT/font_char_height) - 1;
		printf("x="); printf_d(x);
		printf(", y="); printf_d(y);
		printf(", prio="); printf_d(prio);
		putchar('\n');
		
		// Draw ego
		draw_buffer_sprite_priority(
			//bank_vis,
			bank_pri,
			
			&sprite_data[0], sprite_width, sprite_height,
			sprite_transparency,	// trans
			x, y,
			prio,	// prio
			
			x_ofs,y_ofs, true
		);
		
		// Wait for input
		char c = getchar();
		
		// Clear background around sprite
		draw_buffer(bank_vis, x,x+sprite_width*2, y,y+sprite_height, x_ofs,y_ofs, true);
		
		switch(c) {
		case 'a':
		case 'A':
		case 0x1b:	// LEFT
			x -= spd;
			break;
		
		case 'd':
		case 'D':
		case 0x1a:	// RIGHT
			x += spd;
			break;
		
		case 'w':
		case 'W':
		case 24:	// MAME up
			y -= spd;	// UP
			break;
		
		case 's':
		case 'S':
		case 25:	// MAME down
			y += spd;	// DOWN
			break;
		
		
		case 'q':
		case 'Q':
		case 47:	// Mame minus
		//case ...:	// Mame num minus
			prio -= 1;	// BACK
			break;
		case 'e':
		case 'E':
		case 41:	// MAME plus
		case 43:	// MAME num plus
			prio += 1;	// FRONT
			break;
		
		
		case 'i':
		case 'I':
			y_src = 0;
			render = true;
			break;
		
		case 'k':
		case 'K':
			y_src = (AGI_FRAME_HEIGHT - LCD_HEIGHT);
			render = true;
			break;
		
		case 'j':
		case 'J':
			x_ofs = 0;
			redraw = true;
			break;
		
		case 'l':
		case 'L':
			x_ofs = (AGI_FRAME_WIDTH - LCD_WIDTH/2);
			redraw = true;
			break;
		
		case 'p':
		case 'P':
			// Show priority
			draw_buffer(bank_pri, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
			break;
		
		case 'r':
		case 'R':
			// Redraw
			redraw = true;
			break;
		
		#ifdef VAGI_MOUSE
		case 1:	// Mouse button!
			x = mouse_x;
			y = mouse_y;
			break;
		#endif
		
		default:
			printf("key="); printf_d(c); printf("?\n");
		}
		
		/*
		for(byte j = 0; j < 2; j++) {
			// go through the thresholds (z-depth)
			for(byte i = 0; i < 15; i++) {
				//printf("VIS");
				//draw_buffer(bank_vis, 0,0, true);
				//draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
				
				//printf("PRIO");
				//draw_buffer(bank_pri, x_ofs,y_ofs, true);
				//draw_buffer(bank_pri, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
				
				// Draw threshold
				byte thresh = 15 - i;
				lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1;
				printf("x="); printf_d(x_ofs);	// Status
				printf(", y="); printf_d(y_src);	// Status
				printf(", thresh="); printf_d(thresh);	// Status
				//lcd_clear();
				//draw_buffer_combined(bank_vis, bank_pri, thresh, x_ofs,y_ofs, true);
				//draw_buffer_combined(bank_vis, bank_pri, thresh, area_x1, area_x2, area_y1, area_y2, x_ofs,y_ofs, true);
				draw_buffer_combined(bank_vis, bank_pri, thresh, 0, LCD_WIDTH, 0, LCD_HEIGHT, x_ofs,y_ofs, true);
			}
			
			// Next time: Scroll to the other side (left / right)
			if (x_ofs == 0) x_ofs = (AGI_FRAME_WIDTH - (LCD_WIDTH/2));
			else x_ofs = 0;
		}
		// Next time: Crop to the other slice of the frame (upper / lower)
		if (y_src == 0) y_src = (AGI_FRAME_HEIGHT - LCD_HEIGHT);
		else y_src = 0;
		*/
		
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
	//test_draw_combined();
	//test_draw_agi_scroll();
	test_draw_agi_combined();
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
