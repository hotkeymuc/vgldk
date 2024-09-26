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

// Connections to the outer world
//bool agi_res_eof() { return (_dataOffset >= _dataSize); }
//bool agi_res_read() { return _data[_dataOffset++]; }
//bool agi_res_peek() { return _data[_dataOffset]; }

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

// flood fill
typedef struct {
	byte x;
	byte y;
} fill_stack_t;
#define FILL_STACK_MAX 128


bool inline draw_FillCheck(int16 x, int16 y);

void _draw_Fill(int16 x, int16 y);
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
