
#include "agi.h"

#include "agi_vars.h"	// To show status of variables

VERTYPE AGIVER;

// from agimain.c:
BOOL PLAYER_CONTROL, TEXT_MODE, WINDOW_OPEN, REFRESH_SCREEN, MENU_SET, INPUT_ENABLED, QUIT_FLAG;
BOOL SOUND_ON, PIC_VISIBLE, PRI_VISIBLE, STATUS_VISIBLE, VOBJ_BLOCKING, WALK_HOLD;
U8 oldScore;
U8 horizon; 
U8 picNum;
//int minRowY;
int ticks;
RECT8 objBlock;
char szGameID[MAX_ID_LEN+1];
int pushedScriptCount, scriptCount;
//...


// invobj.c:
U8 invObjRooms[MAX_IOBJ];


void dump_vars() {
	//for(int i = 0; i < MAX_VARS; i++) {
	printf("v:");
	for(int i = 0; i < 20; i++) {
		printf_x2(vars[i]);
	}
	putchar('\n');
}

/*
#define STATE_BYTES 7
#define MULT 0x13B // for STATE_BYTES==6 only
#define MULT_LO (MULT & 255)
#define MULT_HI (MULT & 256)

U8 rand() {
	static U8 state[STATE_BYTES] = { 0x87, 0xdd, 0xdc, 0x10, 0x35, 0xbc, 0x5c };
	static U16 c = 0x42;
	static int i = 0;
	U16 t;
	U8 x;
	
	x = state[i];
	t = (U16)x * MULT_LO + c;
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
