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
	
	AGI implementations are based on:
		* ScummVM: https://github.com/scummvm/scummvm/tree/master/engines/agi
		* GBAGI: https://github.com/Davidebyzero/GBAGI.git
	
	2024-09-11 Bernhard "HotKey" Slawik
*/


//#define VAGI_MOUSE	// Support mouse
#define VAGI_START_PIC_NUM 1	//5	// On which PIC to start

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
	
	#define HEX_USE_DUMP	// For dump(addr, count)
	#define HEX_DUMP_WIDTH 16	// Usually 4 for 20-character screens
	#include <hex.h>	// provides printf_x2
#endif

// Glue

#include <stringmin.h>	// for memcpy()
void strncpy(char *dst, char *src, byte maxl) {
	byte l = strlen(src);
	memcpy(dst, src, (l < maxl) ? l : maxl);
}

typedef byte bool;
typedef byte uint8;
typedef int int16;
typedef word uint16;


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
__sfr __at 0x55 bank_type_port;	// This controls whether a bank is mapped to internal ROM (0) or external cartridge ROM (1)


// Platform specific helpers:
#include "vagi_lcd.h"	// This abstracts access to the LCD screen
#include "vagi_frame.h"	// This handles one big full-size AGI frame needed only once at render-time
#include "vagi_buffer.h"	// This handles the (smaller) working buffer(s) needed at run-time (derived from frame)
#include "vagi_res.h"	// This allows reading from AGI resources as if they were files

// The AGI specific fun starts here:
#include "agi.h"
#include "agi.c"	//@FIXME: Only the main C file gets passed to the VGLDK compiler pass...

#include "agi_view.h"
#include "agi_view.c"	//@FIXME: Only the main C file gets passed to the VGLDK compiler pass...

// PIC code
//@TODO: The PIC code would lend itself well to be put onto a separate code segment for bank switching

// Since we only have enough RAM to render EITHER the visual OR the priority frame, we need to keep track of the currently active frame type
#define VAGI_STEP_VIS 0
#define VAGI_STEP_PRI 1
byte vagi_drawing_step = VAGI_STEP_VIS;	// Current rendering step (which kind of PIC data to process: 0=visual, 1=priority)

#include "agi_pic.h"
#include "agi_pic.c"	//@FIXME: Only the main C file gets passed to the VGLDK compiler pass...

bool render_frame_agi(word pic_num, byte drawing_step) {
	// Draw one AGI PIC (either its visual or priority data)
	
	// Mount our cartridge ROM to address 0x4000 (data must be inside the ROM binary at position 0x4000 * n)
	//bank_type_port = bank_type_port | 0x02;	// Switch address region 0x4000-0x7FFF to use cartridge ROM (instead of internal ROM)
	//bank_type_port = bank_type_port | 0x04;	// Switch address region 0x8000-0xBFFF to use cartridge ROM
	//bank_0x4000_port = 0x20 | 1;	// Mount ROM segment n=1 (offset 0x4000 * n) to address 0x4000
	//dump(0x4000, 16);
	
	//vagi_drawing_step = VAGI_STEP_VIS;	// Only perform drawing operations for visual (screen) frame
	//vagi_drawing_step = VAGI_STEP_PRI;	// Only perform drawing operations for priority frame
	vagi_drawing_step = drawing_step;	// Only perform drawing operations for either screen OR priority
	
	// Actually call AGI drawing routine...
	//frame_clear(0x00);
	//for(i = 0; i < 10; i++) { draw_Line(0,i*4, 159,167); }	// Test pattern
	if (drawing_step == VAGI_STEP_VIS) frame_clear(0xf * 0x11);	// VIS: bg=_scrColor=0xf ( * 0x11 = "on both nibbles")
	if (drawing_step == VAGI_STEP_PRI) frame_clear(0x4 * 0x11);	// PRI: bg=_priColor=0x4 ( * 0x11 = "on both nibbles")
	
	//drawPictureV1(pic_num);
	//drawPictureV15(pic_num);
	drawPictureV2(pic_num);
	
	return true;
}



void vagi_draw_pic(byte pic_num) {
	// vagi:
	const byte bank_vis = BUFFER_BANK_VIS;	// 3
	const byte bank_pri = BUFFER_BANK_PRI;	// 1 (caution! Shared with VAGI_FRAME_BANK_LO), must be done last, overwriting the frame in the process
	
	// Render both frames and create working buffers
	//lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1;
	lcd_text_col = 0; lcd_text_row = 0;
	printf("Loading PIC "); printf_d(pic_num);
	
	// Render and process the VIS frame.
	bool ok = render_frame_agi(pic_num, VAGI_STEP_VIS);	// Render the full-size visual PIC frame (takes quite long...)
	if (ok) {
		// Crop/scale frame to visual working buffer
		process_frame_to_buffer(bank_vis, 0, 0);
		
		// Show visual buffer while priority is being rendered
		//lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1; printf("Drawing PIC "); printf_d(pic_num);
		draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
		
		// Render and process the PRI frame. The buffer is co-located with the frame buffer, overwriting it in the process. Must be done last.
		//lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1; printf("Loading PRIO "); printf_d(pic_num);
		ok = render_frame_agi(pic_num, VAGI_STEP_PRI);	// Render the full-size priority PIC frame (takes quite long...)
		if (ok) {
			// Crop/scale frame to priority working buffer
			process_frame_to_buffer(bank_pri, 0, 0);
			//draw_buffer(bank_pri, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
		}
		//lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1; printf("done.");
		
		// Partially redraw the lower part (where the progress bar was shown during rendering)
		draw_buffer(bank_vis, 0,LCD_WIDTH, LCD_HEIGHT - (font_char_height*2),LCD_HEIGHT, 0,0, true);
	}
	
	// Reset cursor to start at the top
	lcd_text_col = 0; lcd_text_row = 0;
}


/*
void test_draw_agi_combined(byte pic_num, bool interactive) {
	// Test drawing a sprite while respecing the priority buffer (i.e. what Sierra called "3D")
	
	byte bank_vis = BUFFER_BANK_VIS;	//3;
	byte bank_pri = BUFFER_BANK_PRI;	//1;	Caution! collides with VAGI_FRAME_BANK_LO/HI
	byte x_src = 0;
	byte y_src = 0;
	byte x_ofs = 0;
	byte y_ofs = 0;
	
	y_src = (AGI_FRAME_HEIGHT - LCD_HEIGHT) / 2;	// Start in the middle
	byte x = AGI_FRAME_WIDTH / 2;
	byte y = AGI_FRAME_HEIGHT / 2;
	byte prio = 5;
	byte spd = 5;
	
	//word pic_num = 1; //5;	// Which PIC resource to display first
	
	bool render = true;	// Force re-rendering of the full-size frame
	bool redraw = true;	// Force re-drawing of the full visual buffer
	bool ok;
	
	for(;;) {
		if (render) {
			render = false;
			redraw = true;
			
			// Render both frames and create working buffers
			lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1; printf("Loading PIC "); printf_d(pic_num);
			
			//lcd_text_col = 0; lcd_text_row = 0; printf("VIS...");
			ok = render_frame_agi(pic_num, VAGI_STEP_VIS);	// Render the full-size visual PIC frame (takes quite long...)
			if (ok) {
				process_frame_to_buffer(bank_vis, x_src, y_src);	// Crop (upper or lower part) of frame to working buffer
				draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);	// Show visual buffer while priority is being rendered
				
				//lcd_text_col = 0; lcd_text_row = 0; printf("PRIO...");
				ok = render_frame_agi(pic_num, VAGI_STEP_PRI);	// Render the full-size priority PIC frame (takes quite long...)
				process_frame_to_buffer(bank_pri, x_src, y_src);	// Crop (upper or lower part) of frame to working buffer
				//draw_buffer(bank_pri, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
				
				//redraw = true;
				// Partially redraw the lower part (where the progress bar was shown during rendering)
				draw_buffer(bank_vis, 0,LCD_WIDTH, LCD_HEIGHT - (font_char_height*2),LCD_HEIGHT, x_ofs,y_ofs, true);
				redraw = false;	// The screen contents should be usable, so we do not need to re-draw it again
				
			} else {
				// Something went wrong while loading the resource
				redraw = false;
			}
			
			// Draw full background
			//draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
		}
		
		if (redraw) {
			// Redraw the full visual buffer
			//memset((byte *)LCD_ADDR, 0xff, LCD_HEIGHT * (LCD_WIDTH/8));	// Clear screen
			draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, x_ofs,y_ofs, true);
			redraw = false;	// Prevent re-drawing again next time
		}
		
		// Quit if not interactive
		if (!interactive) return;
		
		// Show some stats
		//lcd_text_col = 0; lcd_text_row = 0;
		lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1;
		printf("x="); printf_d(x);
		printf(", y="); printf_d(y);
		printf(", prio="); printf_d(prio);
		//putchar('\n');
		
		// Draw ego sprite
		draw_buffer_sprite_priority(
			//bank_vis,	// The drawing routine simply skips visual background pixels. Smart, eh?
			bank_pri,
			
			&sprite_data[0], sprite_width, sprite_height,
			sprite_transparency,	// trans
			x, y,
			prio,	// prio
			
			x_ofs,y_ofs, true
		);
		
		// Wait for user input
		char c = getchar();
		
		// Clear background around ego sprite
		draw_buffer(bank_vis, x,x+sprite_width*2, y,y+sprite_height, x_ofs,y_ofs, true);
		
		// Handle keyboard input
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
		
		case 48:	//  0
			break;
		case 49:	//  1
			if (pic_num > 1) {
				pic_num--;
				render = true;
			}
			break;
		case 50:	//  2
			pic_num++;
			render = true;
			break;
		
		default:
			printf("key="); printf_d(c); printf("?\n");
		}
		
		/ *
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
		* /
		
	}
}
*/


/*
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
	
	//	
	//	byte bank;	// Which bank to use for buffer
	//	bank = 3;	// Easy: Read from frame banks 1 and 2, write to 3rd bank
	//	//bank = 1;	// Advanced: Read from frame banks 1 and 2, write BACK to 1st bank (overwriting the frame!)
	//	
	//	// Render one full frame in full internal AGI resolution (160x168) at 4 bpp
	//	// One 4 bit frame (either visual or priority) at internal AGI resolution (160x168) is:
	//	// 160x168 x 4bit = 26880 / 2 = 13440 bytes = 8192 + 5248 bytes across 2 banks
	//	printf("Rendering...");
	//	//render_frame();
	//	//render_frame_spirals_large();
	//	render_frame_spirals_small();
	//	printf("OK\n");
	//	
	//	// Now crop and scroll that full frame
	//	//x = 0;
	//	for (y = 0; y < 68; y++) {
	//		
	//		// Clear destination buffer
	//		//buffer_switch(bank);	// Map destination working buffer to 0xc000
	//		//buffer_clear();	// Caution! If re-using the same region for frame and buffer, this will clear the rendered frame!
	//		
	//		// Crop and scale from frame to working buffer
	//		//printf("Processing...");
	//		process_frame_to_buffer(bank, 0, y);	// Cropy a slice, scrolling vertically
	//		//printf("OK\n");
	//		
	//		// Clear screen (which might be "dirty" because of contiguous buffer)
	//		//lcd_clear();
	//		
	//		// Draw from working buffer (4 bpp) to screen (1 bpp)
	//		// Draw the buffer 1:1
	//		//draw_buffer(bank, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
	//		
	//		// Draw the buffer scaled horizontally (like the original games)
	//		// Scroll the 40 pixels that are missing (120 pixels of frame are shown at 2x scale)
	//		for (x = 0; x < 40; x+=1) {
	//			draw_buffer(bank, 0,LCD_WIDTH, 0,LCD_HEIGHT, x,0, false);	// X-stretch and scroll horizontally
	//		}
	//	}
	//	
	
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

*/



// Logic!
//#define AGI_LOGIC_DEBUG	// Verbose logic output (each OP, each CallLogic)
//#define AGI_LOGIC_DEBUG_IFS	// Even verboser logic debug: Show "IF v[x] > y" etc.
#define AGI_COMMANDS_NO_NAMES	// Do not include command names, safes about 0x800+ bytes of space

#include "agi_vars.h"
#include "agi_vars.c"	//@FIXME: Makefile only compiles the main .C file

#include "agi_commands.h"
#include "agi_cmd_test.c"	//@FIXME: Makefile only compiles the main .C file
#include "agi_cmd_agi.c"	//@FIXME: Makefile only compiles the main .C file
#include "agi_commands.c"	//@FIXME: Makefile only compiles the main .C file

#include "agi_logic.h"
#include "agi_logic.c"	//@FIXME: Makefile only compiles the main .C file

void vagi_init() {
	vagi_res_init();	// calls romfs_init()
	
	QUIT_FLAG = FALSE;
	
	// agimain.c:AGIInit():
	PLAYER_CONTROL	= TRUE;
	INPUT_ENABLED	= FALSE;
	TEXT_MODE		= FALSE;
	STATUS_VISIBLE	= FALSE;
	VOBJ_BLOCKING	= FALSE;
	WINDOW_OPEN		= FALSE;
	REFRESH_SCREEN	= FALSE;
	PIC_VISIBLE		= FALSE;
	PRI_VISIBLE		= FALSE;
	WALK_HOLD		= FALSE;
	//MENU_SELECTABLE	= TRUE;
	
	scriptCount 	= 0;
	
	//ydiff 			= 0;
	minRow 			= 1;
	minRowY			= 0;	//(minRow*(SCREEN_WIDTH*CHAR_HEIGHT));
	inputPos		= 22;
	statusRow		= 0;
	
	textColour		= 0x0F;
	textAttr		= 0;
	
	//msgX			= -1;
	//msgY			= -1;
	//maxWidth		= -1;
	
	ClearVars();
	ClearFlags();
	ClearControllers();
	//if(!RESTART) ClearControlKeys();
	
	vars[vCOMPUTER]		= 0; // PC
	vars[vMONTIOR]	= 3; // EGA
	vars[vSOUNDTYPE]		= 1; // PC
	vars[vMAXINPUT]		= MAX_STRINGS_LEN;
	vars[vMEMORY]		= 10;
	
	SetFlag(fNEWROOM);
	
	//InitSound();
	InitLogicSystem();
	InitViewSystem();
	//InitPicSystem(TRUE);
	//InitObjSystem();
	//InitParseSystem();
	//InitSaveRestore();
	//if(!RESTART) InitMenuSystem();
	
	SOUND_ON	= TRUE;
	//SetFlag(fSOUND);
	ResetFlag(fSOUND);
	
}

void vagi_handle_input() {
	// GBAGI: DoDelayNPoll() / PollInput()
	byte key;
	
	//while((BOOL)(event = ReadEvent()) && (!TestFlag(fPLAYERCOMMAND))) {
	while (!TestFlag(fPLAYERCOMMAND)) {
		key = keyboard_inkey();
		
		if (key == KEY_CHARCODE_NONE) break;
		
		// Handle directions
		int d = -1;
		if (key == KEY_LEFT) d = dirLEFT;
		if (key == KEY_RIGHT) d = dirRIGHT;
		if (key == KEY_UP) d = dirUP;
		if (key == KEY_DOWN) d = dirDOWN;
		if (d >= 0) {
			if (d == ViewObjs[0].direction) {	// Press the button twice?
				vars[vEGODIR] = dirNONE;	// Stop
			} else {
				vars[vEGODIR] = d;	// New direction
			}
			if (PLAYER_CONTROL) {
				ViewObjs[0].motion = mtNONE;
			}
		} else {
			
			//@TODO: Handle GUI
			if (key == 'r') {
				// Re-set ego
				ViewObjs[0].x = 30;
				ViewObjs[0].y = 80;
				ViewObjs[0].priority = 14;
			}
			
			/*
			// Set controllers (if they have a key associated)
			CTLMAP *c;
			for(c = ctlMap; c < ctlMap+MAX_CONTROLLERS; c++) {
				if(key == c->key) {
					controllers[c->num] = 1;
					break;
				}
			}
			*/
			vars[vKEYPRESSED] = (U8)key;
		}
	}
}


#define FRAMES_PER_SECOND 10	// Used for in-game time
byte timer_frame = 0;


bool vagi_loop() {
	// aka. GBAGI: agimain.c:AGIMain() (without the outer loop)
	
#ifdef SKIPTOSCREEN
	int m=1;
#endif
	
	ClearControllers();
	
	ResetFlag(fPLAYERCOMMAND);
	ResetFlag(fSAIDOK);
	vars[vKEYPRESSED]	= 0;
	vars[vUNKWORD]		= 0;
	
	// Timer
	timer_frame++;
	if (timer_frame > FRAMES_PER_SECOND) {
		timer_frame = 0;
		if(++vars[vSECONDS]>=60) {
			vars[vSECONDS]=0;
			if(++vars[vMINUTES]>=60) {
				vars[vMINUTES]=0;
				if(++vars[vHOURS]>=60) {
					vars[vHOURS]=0;
					vars[vDAYS]++;
				}
			}
		}
	}
	
	// Handle user inputs NOW!
	vagi_handle_input();
	
	
	//
	if(QUIT_FLAG) {
		//printf("QUIT_FLAG");
		return false;	//break;
	}
	//SystemDoit();
	
	if (PLAYER_CONTROL) ViewObjs[0].direction = vars[vEGODIR];
	else vars[vEGODIR] = ViewObjs[0].direction;
	
	CalcVObjsDir();	// agi_view.c: This makes ViewObjs move/wander/follow
	
	oldScore = vars[vSCORE];
	SOUND_ON = TestFlag(fSOUND);
	
#ifdef SKIPTOSCREEN
	if(vars[0]==25) {
	if(m==1) {
		ViewObjs[0].x = 10;
		ViewObjs[0].y = 150;
		NewRoom(SKIPTOSCREEN);
	}
	if(m<2) m++;}
#endif
	
	// Invoke the game logic (i.e. call LOG0)
	//dump_vars();
	//printf("logic0...");
	while(!CallLogic(0)) {
		if(QUIT_FLAG) break;
		
		vars[vUNKWORD]   = 0;
		vars[vOBJBORDER] = 0;
		vars[vOBJECT]    = 0;
		ResetFlag(fPLAYERCOMMAND);
		oldScore = vars[vSCORE];
	}
	//printf("logic0 finished.\n");
	
	ViewObjs[0].direction = vars[vEGODIR];
	
	//if( (oldScore!=vars[vSCORE]) || (TestFlag(fSOUND)!=SOUND_ON) ) WriteStatusLine();
	
	vars[vOBJBORDER] = 0;
	vars[vOBJECT]    = 0;
	
	ResetFlag(fNEWROOM);
	ResetFlag(fRESTART);
	ResetFlag(fRESTORE);
	
	/*
	if(!TEXT_MODE) {
		UpdateGfx();	// screenc: Calls UpdateVObj
	}
	*/
	UpdateVObj();
	
	if (QUIT_FLAG) return false;
	return true;
}


// Main entry point (called by vgldk_init / crt)
#if VGLDK_SERIES == 0
// Platform independent "apps" (VGLDK_SERIES=0) are invoked with arguments
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
#else
// Regular ROM programs do not have any command line arguments
void main() __naked {
	
#endif
	
	byte running = 1;
	//char c;
	//word i;
	
	//printf("VAGI\n");
	AGIVER.major = 2;
	AGIVER.minor = 0;
	
	vagi_init();
	
	
	//test_draw_combined();	// Test drawing combined vis & prio
	//test_draw_agi_scroll();	// Test partial redraw
	//test_draw_agi_combined(VAGI_START_PIC_NUM, true);	// Test rendering actual AGI PIC data
	//test_draw_agi_combined(VAGI_START_PIC_NUM, false);	// Test rendering actual AGI PIC data
	//printf("end of render."); getchar();
	
	byte i;
	byte spinner = 0;
	const char spinner_char[4] = { '-', '/', '|', '\\'};
	
	while (running) {
		
		
		// Clear top bar
		//memset((byte *)LCD_ADDR, 0x55, (LCD_WIDTH/8) * 7);
		
		/*
		// Write activity to top line
		lcd_text_col = 0; lcd_text_row = 0;
		//dump_vars();
		for(int i = 0; i < 24; i++) {	// MAX_VARS
			printf_x2(vars[i]);
		}
		for(int i = 0; i < 24; i++) {	// MAX_FLAGS/8
			printf_x2(flags[i]);
		}
		*/
		//for(i = 0; i < spinner; i++) putchar(' ');
		//putchar('.');
		//for(i = spinner; i < 4; i++) putchar(' ');
		/*
		putchar(spinner_char[spinner % 4]);
		spinner = (spinner + 1) % 4;
		putchar('\n');
		*/
		
		// Dump vars/flags as pixels
		word a = LCD_ADDR;
		memcpy((byte *)a, &vars[0], MAX_VARS);
		a += MAX_VARS;
		memcpy((byte *)a, &flags[0], MAX_FLAGS/8);
		a += MAX_FLAGS/8;
		//memcpy((byte *)a, (byte *)&ViewObjs[0], MAX_VOBJ*sizeof(VOBJ));
		
		/*
		lcd_text_col = 0;
		//lcd_text_row = 0;
		lcd_text_row = (LCD_HEIGHT/font_char_height) - 1;
		printf("x="); printf_d(ViewObjs[0].x);
		printf(" y="); printf_d(ViewObjs[0].y);
		printf(" pri="); printf_d(ViewObjs[0].priority);
		//printf(" dir="); printf_d(ViewObjs[0].direction);
		*/
		// Dump VOBJ state
		VOBJ *v;
		byte n = 12;	//MAX_VOBJ;
		byte cols = 3+1 + 2+2+2 + 1 + 3+1+3;
		for(i = 0; i < n; i++) {
			v = &ViewObjs[i];
			lcd_text_col = (LCD_WIDTH/font_char_width) - cols;
			lcd_text_row = i;
			printf_d(i); putchar(':');
			printf_x2(v->flags);
			printf_x2(v->motion);
			printf_x2(v->stepCount);
			putchar('|');
			printf_d(v->x); putchar(','); printf_d(v->y);
		}
		lcd_text_col = 0;
		lcd_text_row = 0;
		
		//running = vagi_loop();
		vagi_loop();
		
		/*
		//putchar('?');
		printf('vagi>');
		c = getchar();
		
		switch(c) {
			
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
		*/
	}
	
	#if VGLDK_SERIES == 0
	return 42;	// Apps can return stuff
	#endif
}
