
#include "agi.h"

#include "agi_vars.h"	// To show status of variables

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

void dump_vars() {
	//for(int i = 0; i < MAX_VARS; i++) {
	printf("v:");
	for(int i = 0; i < 20; i++) {
		printf_x2(vars[i]);
	}
	//putchar("\n");
}

void WriteStatusLine() {
	if (!STATUS_VISIBLE) return;
	printf("STATS:");
	dump_vars();
}

bool MessageBox(char *t) {
	bool r;
	
	printf("MessageBox: [");
	printf(t);
	printf("]\n");
	
	//putchar('\n');
	
	r = true;
	if(TestFlag(fPRINTMODE)) {
		ResetFlag(fPRINTMODE);
	} else {
		
		if (vars[vPRINTDURATION]) {
			byte delay = vars[vPRINTDURATION];
			
			//@TODO: Delay (while checking for user reply)...
			r = (getchar() == 10);	//@FIXME: Must be non-blocking with TIMEOUT!
			
			vars[vPRINTDURATION] = 0;
			
		} else {
			r = (getchar() == 10);
		}
	}
	return r;
}
bool MessageBoxXY(char *t, byte x, byte y, byte w) {
	lcd_text_col = x;
	lcd_text_row = y;
	(void)w;
	return MessageBox(t);
}
/*
see agi.h:
	ERR_BAD_CMD,
	ERR_TEST_CMD,
	ERR_PIC_CODE,
	ERR_VOBJ_NUM,
	ERR_VOBJ_VIEW,
	ERR_VOBJ_LOOP,
	ERR_VOBJ_CEL,
	ERR_NO_VIEW,
	ERR_VOBJ_NO_CEL,
	ERR_PRINT,
	ERR_NOMENUSET,
	ERR_ADDMENUITEM,
	ERR_FINDMENUITEM,
*/
void ErrorMessage(int msg, int param) {
	printf("Error#");
	printf_d(msg); printf(": "); printf_d(param);
	getchar();
}
void ErrorMessage2(int msg, int param1, int param2) {
	printf("Error#");
	printf_d(msg); printf(": "); printf_d(param1); printf(", "); printf_d(param2);
	getchar();
}
void ErrorMessage3(int msg, int param1, int param2, int param3) {
	printf("Error#");
	printf_d(msg); printf(": "); printf_d(param1); printf(", "); printf_d(param2); printf(", "); printf_d(param3);
	getchar();
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


// from GBAGI:parse.c:
char *ParseInput(char *sStart) {
	//@TODO: Implement: Split input into words, look them up, store in wordStrings[] and input[]
	
	wordCount = 0;
	/*
	char *s=StripInput(sStart),*szWord;
	int l,group;
	while(*s) {
		l=0;
		if((szWord = FindWordN(s))==NULL) {
			vars[vUNKWORD] = ++wordCount;
			break;
		}
		
		group = bGetW(szWord-2)&0x1FFF;
		l = szWord[-3]-4;
		if(group!=9999) {
			wordStrings[wordCount] = s;
			input[wordCount++] = group;
		}
		if(!s[l]) break;
		s[l] = '\0';
		s+=l+1;
	}
	*/
	
	if (wordCount) SetFlag(fPLAYERCOMMAND);
	
	return wordStrings[0];
}