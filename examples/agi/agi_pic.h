#ifndef __AGI_PIC_H__
#define __AGI_PIC_H__
/*
AGI PIC drawing
based on scummvm's agi implementation 2024-09-14
* ScummVM: https://github.com/scummvm/scummvm/tree/master/engines/agi

2024-09-16 Bernhard "HotKey" Slawik
*/

//#define AGI_PIC_FILL_CASES	// Optimize fill by pre-selecting a "draw_FillCheck_case" function to use throughout the fill

#include "agi.h"

// AGI picture version
enum AgiPictureVersion {
	AGIPIC_C64,
	AGIPIC_V1,
	AGIPIC_V15,
	AGIPIC_V2,
	AGIPIC_256
};

enum AgiPictureFlags {
	kPicFNone      = (1 << 0),
	kPicFCircle    = (1 << 1),
	kPicFStep      = (1 << 2),
	kPicFf3Stop    = (1 << 3),
	kPicFf3Cont    = (1 << 4),
	//kPicFTrollMode = (1 << 5)
};


// picture.h:
#define PRI_CONTROL		(0x01)	//(0x10)
#define PRI_SIGNAL		(0x02)	//(0x20)
#define PRI_WATER		(0x03)	//(0x30)
#define FLAG_CONTROL	(0x01)
#define FLAG_SIGNAL		(0x02)
#define FLAG_WATER		(0x04)


// flood fill
typedef struct {
	byte x;
	byte y;
} fill_stack_t;
//#define FILL_STACK_MAX 8
#define FILL_STACK_MAX 12
//#define FILL_STACK_MAX 16
//#define FILL_STACK_MAX 32
//#define FILL_STACK_MAX 128
//#define FILL_STACK_MAX 172
//#define FILL_STACK_MAX 250
//extern fill_stack_t stack[FILL_STACK_MAX+1];	// extern or on stack?

// Connections to the outer world
//bool agi_res_eof() { return (_dataOffset >= _dataSize); }
//bool agi_res_read() { return _data[_dataOffset++]; }
//bool agi_res_peek() { return _data[_dataOffset]; }

// Since we only have enough RAM to render EITHER the visual OR the priority frame, we need to keep track of the currently active frame type
#define VAGI_STEP_VIS 0
#define VAGI_STEP_PRI 1
extern byte vagi_drawing_step;	// = VAGI_STEP_VIS;	// Current rendering step (which kind of PIC data to process: 0=visual, 1=priority)
extern bool _dataOffsetNibble;	// = 0;
extern vagi_res_handle_t pic_res_h;	// Resource handle to read from (vagi_res.h)


// PictureMgr
extern uint8 _patCode;
extern uint8 _patNum;
extern uint8 _scrOn;	// = 0;
extern uint8 _priOn;	// = 0;
extern uint8 _scrColor;	// = 0;
extern uint8 _priColor;	// = 0;

extern uint8 _minCommand;	// = 0xf0;

//AgiPictureVersion _pictureVersion;
extern byte _pictureVersion;

extern int16 _width;	// = 160;
extern int16 _height;	// = 168;
//int16 _xOffset, _yOffset;

extern int _flags;
//extern int _currentStep;	// Only used in Mickey




void putVirtPixel(int x, int y);
byte getNextByte();
bool getNextParamByte(byte *b);
byte getNextNibble();

void plotPattern(int x, int y);
void plotBrush();
void draw_SetColor();
void draw_SetPriority();
void draw_SetNibbleColor();
void draw_SetNibblePriority();
void draw_Line(int16 x1, int16 y1, int16 x2, int16 y2);
void draw_LineShort();
void draw_LineAbsolute();


bool inline draw_FillCheck(int16 x, int16 y);

bool _draw_Fill(int16 x, int16 y);
void draw_Fill();
void draw_xCorner(bool skipOtherCoords);
void draw_yCorner(bool skipOtherCoords);

//void drawPictureC64();
void drawPictureV1();
void drawPictureV15();

//void drawPictureV2();
void drawPictureV2(word pic_num);

//void drawPictureAGI256();

#endif
