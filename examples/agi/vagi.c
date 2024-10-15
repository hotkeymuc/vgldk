/*

VAGI - VTech AGI
===========

The VAGI code is heavily based on:
	* ScummVM's AGI: https://github.com/scummvm/scummvm/tree/master/engines/agi
	* Brian Provinciano's GBAGI: http://www.bripro.com
	* davidebyzero's fork of GBAGI: https://github.com/Davidebyzero/GBAGI.git

2024-09-14 Bernhard "HotKey" Slawik

*/
/***************************************************************************
 *  GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

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
		* or scale down, but keep track of fine-detailed control pixels in priority frame
	
	
	
	@TODO:
		* Sort views by priority before "Blitting" (see agi_view.c:UpdateVObj() ff.)
		* Inventory objects
		* bug: Not triggering signals (KQ1 when entering the castle in PIC 2; SQ2 when exiting the pod at the ship bay in PIC4?)
		* Handling/mapping of function keys (e.g. F10 / F6)
		* Show game title and score
		* Nicer MessageBox and bigger font
		* Re-locate VIEW code to other code segment as well (together with PIC code), to make room in main segment.
		+ Sound (1-channel via TI TTS chip, anaylze VTech GL6000SL intro)
	
	2024-09-11 Bernhard "HotKey" Slawik
*/


//#define VAGI_MOUSE	// Support mouse
//#define VAGI_MINIMAL	// Squeeze all as much space as possible. No stdio!
//#define VAGI_SHOW_EGO_INFO	// Show info about ego ViewObjs[0]

//#define AGI_LOGIC_DEBUG	// Debug control flow
//#define AGI_LOGIC_DEBUG_OPS	// Verbose logic output (each OP, each CallLogic)
//#define AGI_COMMANDS_INCLUDE_NAMES	// Include command names for debugging, requires ~0x800+ bytes of space!

//#define AGI_LOGIC_DEBUG_IFS	// Even verboser logic debug: Show "IF v[x] > y" etc.

//#define PACKED_DIRS 1	// Game contains "PACKED_DIRS" (see GBAGI/gbarom/makerom.c:"gi->version->flags&PACKED_DIRS" )
//#define PACKED_DIRS 0	// Game does not contain "PACKED_DIRS" (see GBAGI/gbarom/makerom.c:"gi->version->flags&PACKED_DIRS" )
#define VAGI_RES_IGNORE_COMPRESSED	// Do not show "unsupported" messages on compressed resources
//#define VAGI_PIC_IGNORE_FILL_STACK	// Do not show "FILL_STACK_MAX" if flood fill goes awry (what it NEVER should, but does...)
#define ROMFS_IGNORE_NOT_FOUND	// Do not show "not found" messages (e.g. while scanning for volumes on startup)

//#define VAGI_TRACE_STAGES	// Allow on-screen tracing of engine stages
//#define VAGI_SHOW_INTRO	// Show intro splash screen with credits

// VGLDK settings:
#define VGLDK_NO_SOUND	// We do not support it, so don't waste space
//#define LCD_FONT_4x6	// Default: Allows 60x16 text mode
//#define LCD_FONT_6x8	// Allows 40x12 text mode
#define LCD_FONT_6x5	// Allows 40x20 text mode

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
	//#define FONT_FULL_ASCII	// also include ASCII 128...255
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


// from agi.c:
#define STATE_BYTES 7
#define MULT 0x13B // for STATE_BYTES==6 only
#define MULT_LO (MULT & 255)
#define MULT_HI (MULT & 256)

/*
// Note: static values are buggy in VGLDK (i.e.: Not get initialized, yet...)
byte rand() {
	static byte state[STATE_BYTES] = { 0x87, 0xdd, 0xdc, 0x10, 0x35, 0xbc, 0x5c };
	static word c = 0x42;
	static int i = 0;
	word t;
	byte x;
	
	x = state[i];
	t = (word)x * MULT_LO + c;
	c = t >> 8;
#if MULT_HI
	c += x;
#endif
	x = t & 255;
	state[i] = x;
	if (++i >= sizeof(state))
		i = 0;
	return x;
}
*/

extern byte rand_state[STATE_BYTES];	// = { 0x87, 0xdd, 0xdc, 0x10, 0x35, 0xbc, 0x5c };
extern word rand_c;	// = 0x42;
extern int rand_i;	// = 0;
byte rand_state[STATE_BYTES];	// = { 0x87, 0xdd, 0xdc, 0x10, 0x35, 0xbc, 0x5c };
word rand_c;	// = 0x42;
int rand_i;	// = 0;
void rand_init() {
	rand_c = 0;
	rand_state[rand_c++] = 0x87;
	rand_state[rand_c++] = 0xdd;
	rand_state[rand_c++] = 0xdc;
	rand_state[rand_c++] = 0x10;
	rand_state[rand_c++] = 0x35;
	rand_state[rand_c++] = 0xbc;
	rand_state[rand_c++] = 0x5c;
	rand_c = 0x42;
	rand_i = 0;
}
byte rand() {
	word t;
	byte x;
	
	x = rand_state[rand_i];
	t = (word)x * MULT_LO + rand_c;
	rand_c = t >> 8;
#if MULT_HI
	rand_c += x;
#endif
	x = t & 255;
	rand_state[rand_i] = x;
	if (++rand_i >= sizeof(rand_state)) rand_i = 0;
	return x;
}

// End of essential glue

#ifdef VAGI_MOUSE
	#include <mouse.h>
#endif


// Platform specific helpers:
// Those are set by vagi_make.py to create separate code segments
//#define CODE_SEGMENT	// Indicate that the code is to be compiled in multiple segments
#ifndef CODE_SEGMENT
	// If nothing specified: Compile the whole code
	#define CODE_SEGMENT_0	// Include segment 0 code in compilation (main code)
	#define CODE_SEGMENT_1	// Include segment 1 code in compilation (extended code)
#endif

#include "vagi_bank.h"	// Bank switching and code segmentation. NOTE: This must stay at same address in all code segments!

#include "vagi_lcd.h"	// This abstracts access to the LCD screen
//#include "vagi_frame.h"	// This handles one big full-size AGI frame needed only once at render-time (only used while rendering PIC)
#include "vagi_buffer.h"	// This handles the (smaller) working buffer(s) needed at run-time (derived from frame)
#include "vagi_res.h"	// This allows reading from AGI resources as if they were files

/*
// Test sprite
#define sprite_width 8
#define sprite_height 14
#define sprite_transparency 3
static const byte sprite_data[sprite_width*sprite_height] = {
	  3,   3,   3, 0xf, 0xf,   3,   3,   3, 
	  3,   3, 0xf, 0x0, 0x0, 0xf,   3,   3, 
	  3, 0xf, 0x0, 0x0, 0x0, 0x0, 0xf,   3, 
	0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 
	0xf, 0x0, 0xf, 0x0, 0x0, 0xf, 0x0, 0xf, 
	0xf, 0x0, 0xf, 0x0, 0x0, 0xf, 0x0, 0xf, 
	0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 
	0xf, 0x0, 0x0, 0x5, 0x5, 0x0, 0x0, 0xf, 
	0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 
	0xf, 0x0, 0xf, 0x0, 0x0, 0xf, 0x0, 0xf, 
	0xf, 0x0, 0x0, 0xf, 0xf, 0x0, 0x0, 0xf, 
	  3, 0xf, 0x0, 0x0, 0x0, 0x0, 0xf,   3, 
	  3,   3, 0xf, 0x0, 0x0, 0xf,   3,   3, 
	  3,   3,   3, 0xf, 0xf,   3,   3,   3, 
};
*/


// The AGI specific fun starts here:
#define FRAMES_PER_SECOND 8	// Used for in-game time
extern byte timer_frame;
byte timer_frame;

#ifdef VAGI_TRACE_STAGES
	extern bool trace_stages;
	bool trace_stages;
	
	#define vagi_trace_stage(t) if (trace_stages) { lcd_text_row = 0; lcd_text_col = 0; printf(t); }
#else
	#define vagi_trace_stage(t)	;
#endif

#include "agi.h"


// PIC code (on separate code segment!)

#define AGI_PIC_SHOW_PROGRESS	// Show progress bar while rendering PICs, requires ~400 (0x195) bytes
#include "agi_pic.h"


#ifndef CODE_SEGMENT_1
	// Include known entry points in segment 1
	#include "code_segment_1.h"
	
	// Provide stubs ("trampolines") to functions in the other bank
	
	// Reserve RAM for other segment
	byte vagi_drawing_step;	// = VAGI_STEP_VIS;	// Current rendering step (which kind of PIC data to process: 0=visual, 1=priority)
	bool _dataOffsetNibble;	// = 0;
	vagi_res_handle_t pic_res_h;	// Resource handle to read from (vagi_res.h)
	
	
	// PictureMgr
	uint8 _patCode;
	uint8 _patNum;
	uint8 _scrOn;	// = 0;
	uint8 _priOn;	// = 0;
	uint8 _scrColor;	// = 0;
	uint8 _priColor;	// = 0;
	
	uint8 _minCommand;	// = 0xf0;
	
	//AgiPictureVersion _pictureVersion;
	byte _pictureVersion;
	
	int16 _width;	// = 160;
	int16 _height;	// = 168;
	//int16 _xOffset, _yOffset;
	
	int _flags;
	//int _currentStep;
	
	
	void vagi_pic_draw(byte pic_num) {
		// Call entry point of "vagi_pic_draw()" in other segment
		
		buffer_switch(BUFFER_BANK_PRI);
		buffer_clear(0x4);
		buffer_switch(BUFFER_BANK_VIS);
		buffer_clear(0xf);
		
		//printf("call...");getchar();
		code_segment_call_b(2, code_segment_1__vagi_pic_draw_addr, pic_num);
		//printf("back.");getchar();
	}
	
	void vagi_pic_show() {
		//lcd_clear();
		draw_buffer(BUFFER_BANK_VIS, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0);	//, false);
		
		//code_segment_call(2, code_segment_1__vagi_pic_show_addr);
	}
	
#endif
#ifdef CODE_SEGMENT_1
	
	// Include the PIC code
	#include "vagi_pic.h"
	
#endif	// Code segmenting



#ifndef CODE_SEGMENT_0
	// Provide a stub main() if the file is compiled without the main code segment
	void main() __naked {	// Regular ROM programs do not have any command line arguments
	}
#endif
#ifdef CODE_SEGMENT_0
// Exclusively in code segment 0

// Text/Parser
#include "agi_text.h"

// VIEW code
#include "agi_view.h"

// LOG code
#include "agi_vars.h"
#include "agi_commands.h"
#include "agi_logic.h"

// Include Implementations
//@FIXME: Only this main C file gets passed to the VGLDK compiler, so we need to include all C files manually here...
#include "agi.c"
#include "agi_text.c"
#include "agi_view.c"
#include "agi_vars.c"
#include "agi_cmd_test.c"
#include "agi_cmd_agi.c"
#include "agi_commands.c"
#include "agi_logic.c"





void vagi_init() {
	vagi_res_init();	// calls romfs_init()
	rand_init();	// Init data
	vagi_lcd_init();	// Reset dither error
	
	AGIVER.major = 2;
	AGIVER.minor = 0;
	
	szGameID[0] = '?';
	szGameID[1] = '\0';
	cursorChar = '>';
	
	#ifdef VAGI_TRACE_STAGES
	trace_stages = false;
	#endif
	timer_frame = 0;
	
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
	//minRowY			= 0;	//(minRow*(SCREEN_WIDTH*CHAR_HEIGHT));
	inputPos		= 12;	// 22
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
	
	//SetFlag(64);	// SpaceQuest2: Debug mode
	
	SetFlag(fNEWROOM);
	
	//InitSound();
	InitLogicSystem();
	InitViewSystem();
	//InitPicSystem(TRUE);
	//InitObjSystem();
	InitParseSystem();
	//InitSaveRestore();
	//if(!RESTART) InitMenuSystem();
	
	SOUND_ON	= TRUE;
	//SetFlag(fSOUND);
	ResetFlag(fSOUND);
	
	// Test!
	
	
}

void vagi_handle_input() {
	byte key;
	int i;
	
	// GBAGI: DoDelayNPoll() / PollInput()
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
			
			// Handle Input/GUI
			if (key == 't') {
				// Enable trace_ops (requires AGI_LOGIC_DEBUG_OPS)
				//trace_ops = 1 - trace_ops;
				//printf("trace_ops:"); if (trace_ops) printf("ON!"); else printf("off");
				#ifdef VAGI_TRACE_STAGES
					trace_stages = 1 - trace_stages;
					printf("trace_stages:"); if (trace_stages) printf("ON!"); else printf("off");
				#endif
			} else
			if (key == 'a') {
				ViewObjs[0].x --;
			} else
			if (key == 'd') {
				ViewObjs[0].x ++;
			} else
			if (key == 'w') {
				ViewObjs[0].y --;
			} else
			if (key == 's') {
				ViewObjs[0].y ++;
			} else
			if (key == 'r') {
				// Re-set ego state
				ViewObjs[0].priority = 14;
				ViewObjs[0].flags |= (oDRAWN|oUPDATE);
				ViewObjs[0].motion = 0;
			} else
			if (key == 'p') {
				// Show priority
				draw_buffer(BUFFER_BANK_PRI, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0);	//, false);
			} else
			if (key == 'v') {
				// Show vis
				draw_buffer(BUFFER_BANK_VIS, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0);	//, false);
			} else
			if (
				//(INPUT_ENABLED) && (	// Input is usually only available if INPUT_ENABLED is set
				(key == ' ')
				//)
			){
				//lcd_text_col = 0; lcd_text_row = inputPos; printf(">");
				//DrawAGIString(">", 0, inputPos);
				DrawAGIString(">", 0, inputPos - 4);
				gets(&szInput[0]);
				
				// fPLAYERCOMMAND and fSAIDOK are Reset in game loop before calling vagi_handle_input
				ParseInput(&szInput[0]);	// agi.c:ParseInput
				// This will automatically set the flag SetFlag(fPLAYERCOMMAND);
				lcd_text_col = 0; lcd_text_row = 0;
			}
			
			// Set controllers (if they have a key associated)
			//CTLMAP *c;
			for(i=0; i<MAX_CONTROLLERS-1; i++) {
				if(ctlMap[i].key == key) {
					lcd_text_col = 0;
					lcd_text_row = 0;
					printf("CTL"); printf_d(ctlMap[i].num); printf("!\n");
					controllers[ctlMap[i].num] = 1;
					//break;
				}
			}
			
			vars[vKEYPRESSED] = (U8)key;
		}
	}
}



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
	vagi_trace_stage("vagi_handle_input");
	vagi_handle_input();
	
	//
	if(QUIT_FLAG) {
		//printf("QUIT_FLAG");
		return false;	//break;
	}
	//SystemDoit();
	
	if (PLAYER_CONTROL) ViewObjs[0].direction = vars[vEGODIR];
	else vars[vEGODIR] = ViewObjs[0].direction;
	
	vagi_trace_stage("CalcVObjsDir");
	CalcVObjsDir();	// agi_view.c: This makes ViewObjs chose a new direction
	vagi_trace_stage("CalcVObjsDir: done.");
	
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
	#ifdef AGI_LOGIC_DEBUG
	//dump_vars();
	//printf("logic0...");
	lcd_clear();
	lcd_text_col = 0;
	lcd_text_row = 0;
	#endif
	new_room_called = false;
	word timeout = 10;	// Catch rogue logic
	
	vagi_trace_stage("LOG0...");
	
	//while (CallLogic(0) == 0) {
	while ((CallLogic(0) == 0) && (--timeout > 0)) {
		new_room_called = false;
		
		if(QUIT_FLAG) break;
		
		vars[vUNKWORD]   = 0;
		vars[vOBJBORDER] = 0;
		vars[vOBJECT]    = 0;
		ResetFlag(fPLAYERCOMMAND);
		oldScore = vars[vSCORE];
	}
	//} while (new_room_called);
	if (timeout <= 0) {
		printf("TO!"); getchar();
	}
	
	vagi_trace_stage("POST");
	
	ViewObjs[0].direction = vars[vEGODIR];
	
	//if( (oldScore!=vars[vSCORE]) || (TestFlag(fSOUND)!=SOUND_ON) ) WriteStatusLine();
	
	vars[vOBJBORDER] = 0;
	vars[vOBJECT]    = 0;
	
	ResetFlag(fNEWROOM);
	ResetFlag(fRESTART);
	ResetFlag(fRESTORE);
	
	
	if(!TEXT_MODE) {
		//UpdateGfx();	// screenc: Calls UpdateVObj()
		UpdateVObj();
	}
	
	vagi_trace_stage("UpdateVObj: done");
	
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
	//byte i;
	
	printf("VAGI - VGLDK Adventure Game Interpreter\n");
	#ifdef VAGI_SHOW_INTRO
		printf("by Bernhard \"HotKey\" Slawik\n\n");
		printf("Heavily based on:\n");
		printf(" * ScummVM's AGI\n");
		printf("   https://github.com/scummvm\n");
		printf(" * Brian Provinciano's GBAGI\n");
		printf("   http://www.bripro.com\n");
		printf(" * davidebyzero's fork of GBAGI\n");
		printf("   https://github.com/Davidebyzero/GBAGI\n");
	#endif
	vagi_init();
	
	// Run tests
	//test_draw_combined();	// Test drawing combined vis & prio
	//test_draw_agi_scroll();	// Test partial redraw
	//test_draw_agi_combined(VAGI_START_PIC_NUM, true);	// Test rendering actual AGI PIC data
	//test_draw_agi_combined(VAGI_START_PIC_NUM, false);	// Test rendering actual AGI PIC data
	//printf("end of render."); getchar();
	
	while (running) {
		
		// Clear top bar
		//memset((byte *)LCD_ADDR, 0x55, (LCD_WIDTH/8) * 7);
		
		//for(i = 0; i < spinner; i++) putchar(' '); putchar('.'); for(i = spinner; i < 4; i++) putchar(' ');
		
		/*
		// Dump vars/flags as pixels
		word a = LCD_ADDR;
		memcpy((byte *)a, &vars[0], MAX_VARS);
		a += MAX_VARS;
		memcpy((byte *)a, &flags[0], MAX_FLAGS/8);
		a += MAX_FLAGS/8;
		memcpy((byte *)a, (byte *)&ViewObjs[0], MAX_VOBJ*sizeof(VOBJ));
		*/
		
		#ifdef VAGI_SHOW_EGO_INFO
		lcd_text_col = 0;
		//lcd_text_row = 0;
		lcd_text_row = LCD_TEXT_ROWS-1; (LCD_HEIGHT/font_char_height) - 1;
		printf("F="); printf_x2(ViewObjs[0].flags);
		printf(" X="); printf_d(ViewObjs[0].x);
		printf(" Y="); printf_d(ViewObjs[0].y);
		printf(" PRI="); printf_x2(ViewObjs[0].priority);
		printf(" D="); printf_x2(ViewObjs[0].direction);
		printf(" M="); printf_x2(ViewObjs[0].motion);
		
		//printf(" V="); printf_d(vars[30]);	// v30 = SQ2, pic2: which wall we are on
		lcd_draw_glypth_at(game_to_screen_x(ViewObjs[0].x), game_to_screen_y(ViewObjs[0].y), ('0' + ViewObjs[0].num));
		#endif
		
		/*
		// Dump VOBJ state
		VOBJ *v;
		//byte cols = 3+1 + 2+2+2 + 1 + 3+1+3;
		byte cols = 2+1 + 2+ 1 + 2+1+2;
		byte x = (LCD_WIDTH/font_char_width) - cols - 1;
		byte y = 0;
		
		lcd_text_col = x; lcd_text_row = y++;
		printf("Room "); printf_d(vars[vROOMNUM]);
		//lcd_text_col = x; lcd_text_row = y++;
		//printf("Prev "); printf_d(vars[vROOMPREV]);
		
		for(i = 0; i < MAX_VOBJ; i++) {
			v = &ViewObjs[i];
			//if ((v->x == 0) && (v->y == 0)) continue;
			if (!(v->flags & oDRAWN)) continue;
			
			lcd_text_col = x; lcd_text_row = y++;
			if (y > 15) break;
			
			printf_x2(i); putchar(':');
			//printf_x2(v->flags);
			printf_x2(ViewObjs[i].flags);
			//printf_x2(v->motion);
			//printf_x2(v->stepCount);
			putchar(':');
			//printf_x2(v->x); putchar(','); printf_x2(v->y);
			printf_x2(ViewObjs[i].x); putchar(','); printf_x2(ViewObjs[i].y);
			
		}
		lcd_text_col = 0;
		lcd_text_row = 0;
		*/
		
		running = vagi_loop();
	}
	
	//printf("END-OF-ROM");
	
	#if VGLDK_SERIES == 0
	return 42;	// Apps can return stuff
	#endif
}
#endif	// Code segmenting