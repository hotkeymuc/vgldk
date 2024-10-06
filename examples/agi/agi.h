#ifndef __AGI_H__
#define __AGI_H__

// Glue code
//#define U8 byte
//#define U16 word
//#define S16 signed int
//#define BOOL bool



// types.h:
typedef unsigned char	U8;
typedef signed char		S8;
typedef unsigned short	U16;
typedef signed short	S16;
typedef unsigned long	U32;
typedef signed long		S32;
typedef int				BOOL;

typedef struct {
	U8 left,top,right,bottom;
} RECT8;

typedef struct {
	U16 left,top,right,bottom;
} RECT16;

typedef struct {
	S16 left,top,right,bottom;
} _RECT;

#define TRUE			1
#define FALSE			0

#define ISNUM(x) ((x>='0')&&(x<='9'))

#define BUILD_DATE "2024/09/13"
#define BUILD_VERSION	"2.11"


#define CLIP(v,vmin,vmax) ((v >= vmax) ? vmax : ( (v <= vmin) ? vmin : v  ) )
#define SWAP(a,b) {tmp=b;b=a;a=tmp;}
/*
typedef byte bool;
typedef byte uint8;
typedef int int16;
typedef word uint16;
*/

// from screen.h:
#define PIC_WIDTH			160
#define PIC_HEIGHT			168
#define PIC_MAXX			159
#define PIC_MAXY			167
#define PIC_SIZE			26880


// from errmsg.h:
enum {
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
	
	TOTAL_ERRORS
};


// from gamedata.h:
typedef struct {
	U8 major;
    U16 minor;
} VERTYPE;
extern VERTYPE AGIVER;


// from agimain.h:
extern BOOL PLAYER_CONTROL, TEXT_MODE, WINDOW_OPEN, REFRESH_SCREEN, MENU_SET, INPUT_ENABLED;
extern BOOL SOUND_ON,PIC_VISIBLE,PRI_VISIBLE,STATUS_VISIBLE, VOBJ_BLOCKING,WALK_HOLD,QUIT_FLAG;
extern U8 oldScore;
extern U8 horizon;
extern U8 picNum;
extern U8 minRow,inputPos,statusRow;
extern U8 textColour,textAttr,textRow,textCol;
extern int minRowY,ticks;
extern RECT8 objBlock;
extern char cursorChar;

#define MAX_ID_LEN 7
//...


// invobj.h:
#define MAX_IOBJ			256
extern U8 invObjRooms[MAX_IOBJ];

void InitObjSystem(void);
void ExecuteInvDialog(void);
U8 FindObj(char *name);


// parse.h:
#define MAX_INPUT	10	// Number of dictionary indices
extern U16 input[MAX_INPUT],inpos;
extern char *wordStrings[MAX_INPUT];
extern int wordCount;
extern char szInput[64];



// more:
void dump_vars();
void WriteStatusLine();
bool MessageBox(char *t);
bool MessageBoxXY(char *t, byte x, byte y, byte w);
void ErrorMessage(int msg, int param);
void ErrorMessage2(int msg, int param1, int param2);
void ErrorMessage3(int msg, int param1, int param2, int param3);

U8 rand();

// from GBAGI:parse.h:
void InitParseSystem();
char *ParseInput(char *sStart);

#endif