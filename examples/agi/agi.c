
#include "agi.h"

VERTYPE AGIVER;

// from agimain.c:
BOOL PLAYER_CONTROL, TEXT_MODE, WINDOW_OPEN, REFRESH_SCREEN, MENU_SET, INPUT_ENABLED, QUIT_FLAG;
BOOL SOUND_ON, PIC_VISIBLE, PRI_VISIBLE, STATUS_VISIBLE, VOBJ_BLOCKING, WALK_HOLD;
U8 oldScore;
U8 horizon; 
U8 picNum;
U8 minRow,inputPos,statusRow;
U8 textColour,textAttr,textRow,textCol;
int minRowY,ticks;
RECT8 objBlock;
char cursorChar;
char szGameID[MAX_ID_LEN+1];
int pushedScriptCount, scriptCount;
//...


// invobj.c:
U8 invObjRooms[MAX_IOBJ];


// parse.c:
U16 input[MAX_INPUT],inpos;
int wordCount;
BOOL MORE_MODE;
char *wordStrings[MAX_INPUT];


// more:

bool MessageBox(char *t) {
	printf(t);
	return (getchar() == 10);
}
bool MessageBoxXY(char *t, byte x, byte y, byte w) {
	return MessageBox(t);
}
void ErrorMessage(int msg, int param) {
	printf_d(msg); printf(": "); printf_d(param);
}
void ErrorMessage2(int msg, int param1, int param2) {
	printf_d(msg); printf(": "); printf_d(param1); printf(", "); printf_d(param2);
}
void ErrorMessage3(int msg, int param1, int param2, int param3) {
	printf_d(msg); printf(": "); printf_d(param1); printf(", "); printf_d(param2); printf(", "); printf_d(param3);
}

#define STATE_BYTES 7
#define MULT 0x13B /* for STATE_BYTES==6 only */
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
