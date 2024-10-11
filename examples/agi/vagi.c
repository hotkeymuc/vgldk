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

// Those are set by vagi_make.py to create separate code segments
//#define CODE_SEGMENT	// Indicate that the code is to be compiled in multiple segments
#ifndef CODE_SEGMENT
	// If nothing specified: Compile the whole code
	#define CODE_SEGMENT_0	// Include segment 0 code in compilation (main code)
	#define CODE_SEGMENT_1	// Include segment 1 code in compilation (extended code)
#endif

//#define VAGI_MOUSE	// Support mouse
//#define VAGI_MINIMAL	// Squeeze all as much space as possible. No stdio!

#define VGLDK_NO_SOUND	// We do not support it, so don't waste space

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


#ifdef VAGI_MOUSE
	#include <mouse.h>
#endif


// Platform specific helpers:
#include "vagi_bank.h"	// Bank switching and code segmentation. NOTE: This must stay at same address in all code segments!

#include "vagi_lcd.h"	// This abstracts access to the LCD screen
//#include "vagi_frame.h"	// This handles one big full-size AGI frame needed only once at render-time (only used while rendering PIC)
#include "vagi_buffer.h"	// This handles the (smaller) working buffer(s) needed at run-time (derived from frame)
#include "vagi_res.h"	// This allows reading from AGI resources as if they were files


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



// The AGI specific fun starts here:
//#define PACKED_DIRS 1	// Game contains "PACKED_DIRS" (see GBAGI/gbarom/makerom.c:"gi->version->flags&PACKED_DIRS" )
//#define PACKED_DIRS 0	// Game does not contain "PACKED_DIRS" (see GBAGI/gbarom/makerom.c:"gi->version->flags&PACKED_DIRS" )
#define VAGI_SHOW_EGO_INFO	// Show info about ego ViewObjs[0]
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
//#define AGI_LOGIC_DEBUG	// Debug control flow
#define AGI_LOGIC_DEBUG_OPS	// Verbose logic output (each OP, each CallLogic)
//#define AGI_COMMANDS_INCLUDE_NAMES	// Include command names for debugging, requires ~0x800+ bytes of space!

//#define AGI_LOGIC_DEBUG_IFS	// Even verboser logic debug: Show "IF v[x] > y" etc.
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


#define FRAMES_PER_SECOND 10	// Used for in-game time
byte timer_frame;


void vagi_init() {
	vagi_res_init();	// calls romfs_init()
	
	AGIVER.major = 2;
	AGIVER.minor = 0;
	
	szGameID[0] = '?';
	szGameID[1] = '\0';
	cursorChar = '>';
	
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
	
}

void vagi_handle_input() {
	// GBAGI: DoDelayNPoll() / PollInput()
	byte key;
	int i;
	
	//while((BOOL)(event = ReadEvent()) && (!TestFlag(fPLAYERCOMMAND))) {
	while (!TestFlag(fPLAYERCOMMAND)) {
		key = keyboard_inkey();
		
		if (key == KEY_CHARCODE_NONE) break;
		
		// Handle directions
		int d = -1;
		//@FIXME: Something got effed up with the keyboard scan codes!
		//if (key == 0xbb) d = dirLEFT;
		//if (key == 0xba) d = dirRIGHT;
		//if (key == 0xb8) d = dirUP;
		//if (key == 0xb9) d = dirDOWN;
		
		// It is supposed to be THIS:
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
			if (key == 'r') {
				// Re-set ego
				ViewObjs[0].x = 30;
				ViewObjs[0].y = 80;
				ViewObjs[0].priority = 14;
			} else
			if (key == 'p') {
				// Show priority
				draw_buffer(BUFFER_BANK_PRI, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0);	//, false);
			} else
			if (key == 'v') {
				// Show priority
				draw_buffer(BUFFER_BANK_VIS, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0);	//, false);
			} else
			if (
				//(INPUT_ENABLED) && (	// Input is usually only available if INPUT_ENABLED is set
				(key == ' ')
				//|| (key == 0xc0)	//@FIXME: Something got effed up with the keyboard scan codes!
				//)
			){
				//lcd_text_col = 0; lcd_text_row = inputPos; printf(">");
				DrawAGIString(">", 0, inputPos);
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
	vagi_handle_input();
	
	//
	if(QUIT_FLAG) {
		//printf("QUIT_FLAG");
		return false;	//break;
	}
	//SystemDoit();
	
	if (PLAYER_CONTROL) ViewObjs[0].direction = vars[vEGODIR];
	else vars[vEGODIR] = ViewObjs[0].direction;
	
	CalcVObjsDir();	// agi_view.c: This makes ViewObjs chose a new direction
	
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
	word timeout = 10;
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
	if (timeout <= 0) {
		printf("Script TO!"); getchar();
	}
	
	//} while (new_room_called);
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
	//byte i;
	
	//printf("VAGI\n");
	vagi_init();
	
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
	
	printf("END-OF-ROM");
	
	#if VGLDK_SERIES == 0
	return 42;	// Apps can return stuff
	#endif
}
#endif	// Code segmenting