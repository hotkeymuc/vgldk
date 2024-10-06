#ifndef __VAGI_PIC_H__
#define __VAGI_PIC_H__
/*

VAGI PIC drawing (high-level)

*/

#include "vagi_frame.h"	// This handles one big full-size AGI frame needed only once at render-time

// Include the AGI PIC drawing code
#include "agi_pic.c"	//@FIXME: Only the main C file gets passed to the VGLDK compiler pass...

bool vagi_pic_render_frame(word pic_num, byte drawing_step) {
	// Render one full-size AGI PIC (either its visual or priority data)
	
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
	if (drawing_step == VAGI_STEP_VIS) frame_clear(0xf);	// VIS: bg=_scrColor=0xf
	if (drawing_step == VAGI_STEP_PRI) frame_clear(0x4);	// PRI: bg=_priColor=0x4
	
	//drawPictureV1(pic_num);
	//drawPictureV15(pic_num);
	drawPictureV2(pic_num);
	
	return true;
}


void vagi_pic_draw(byte pic_num) {
	// Render both frames and create working buffers
	//lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1;
	lcd_text_col = 0; lcd_text_row = 0;
	printf("Loading PIC "); printf_d(pic_num);
	
	// Render and process the VIS frame.
	bool ok = vagi_pic_render_frame(pic_num, VAGI_STEP_VIS);	// Render the full-size visual PIC frame (takes quite long...)
	if (ok) {
		// Crop/scale frame to visual working buffer
		process_frame_to_buffer(BUFFER_BANK_VIS, 0, 0);
		
		// Show visual buffer while priority is being rendered
		//lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1; printf("Drawing PIC "); printf_d(pic_num);
		//if (show_immediately) draw_buffer(BUFFER_BANK_VIS, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
		
		// Render and process the PRI frame. The buffer is co-located with the frame buffer, overwriting it in the process. Must be done last.
		//lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1; printf("Loading PRIO "); printf_d(pic_num);
		ok = vagi_pic_render_frame(pic_num, VAGI_STEP_PRI);	// Render the full-size priority PIC frame (takes quite long...)
		if (ok) {
			// Crop/scale frame to priority working buffer
			process_frame_to_buffer(BUFFER_BANK_PRI, 0, 0);
			//draw_buffer(BUFFER_BANK_PRI, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
		}
		//lcd_text_col = 0; lcd_text_row = (LCD_HEIGHT/font_char_height) - 1; printf("done.");
		
		// Partially redraw the lower part (where the progress bar was shown during rendering)
		//if (show_immediately) draw_buffer(BUFFER_BANK_VIS, 0,LCD_WIDTH, LCD_HEIGHT - (font_char_height*2),LCD_HEIGHT, 0,0, true);
	}
	
	// Reset cursor to start at the top
	lcd_text_col = 0; lcd_text_row = 0;
}

void vagi_pic_show() {
	draw_buffer(BUFFER_BANK_VIS, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);
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
			ok = vagi_pic_render_frame(pic_num, VAGI_STEP_VIS);	// Render the full-size visual PIC frame (takes quite long...)
			if (ok) {
				process_frame_to_buffer(bank_vis, x_src, y_src);	// Crop (upper or lower part) of frame to working buffer
				draw_buffer(bank_vis, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, false);	// Show visual buffer while priority is being rendered
				
				//lcd_text_col = 0; lcd_text_row = 0; printf("PRIO...");
				ok = vagi_pic_render_frame(pic_num, VAGI_STEP_PRI);	// Render the full-size priority PIC frame (takes quite long...)
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
	//		//buffer_clear(0x0);	// Caution! If re-using the same region for frame and buffer, this will clear the rendered frame!
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
#endif