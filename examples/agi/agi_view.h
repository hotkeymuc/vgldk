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
#ifndef __AGI_VIEW_H__
#define __AGI_VIEW_H__

#include "agi.h"

#define HORIZON_DEFAULT	36

#define oDRAWN			0x0001
#define oIGNORECTL		0x0002
#define oFIXEDPRIORITY	0x0004
#define oINGOREHORIZON	0x0008
#define oUPDATE			0x0010
#define oCYCLE			0x0020
#define oANIMATE		0x0040
#define oBLOCK			0x0080
#define oWATER			0x0100
#define oIGNOREVOBJS	0x0200
#define oREPOSITIONING	0x0400
#define oONLAND			0x0800
#define oSKIPUPDATE		0x1000
#define oFIXEDLOOP		0x2000
#define oMOTIONLESS		0x4000
#define oCELMIRROR		0x8000

enum {
	mtNONE,
	mtWANDER,
	mtFOLLOW,
	mtMOVE
};

enum {
	cyNORMAL,
	cyENDOFLOOP,
	cyREVERSELOOP,
	cyREVERSECYCLE
};

enum {
	dirNONE = 0,
	dirUP,
	dirUPRIGHT,
	dirRIGHT,
	dirDOWNRIGHT,
	dirDOWN,
	dirDOWNLEFT,
	dirLEFT,
	dirUPLEFT
};

enum {
	lpRIGHT,
	lpLEFT,
	lpDOWN,
	lpUP,
	lpIGNORE
};

enum {
	bdNONE,
	bdTOP,
	bdRIGHT,
	bdBOTTOM,
	bdLEFT
};

typedef struct {
	U8	num;
	S16	x,y;
	S16 prevX,prevY;
	U8	width,height;
	U8 prevWidth,prevHeight;
	
	U8	view;
	U8	loop,totalLoops;
	U8	cel,totalCels;
	
	U8	direction;
	U8	motion;
	U8	priority;
	U16	flags;
	
	U8	stepTime,stepCount,stepSize;
	U8	cycle,cycleTime,cycleCount;
	
	U8	*pView,*pLoop,*pCel;
	
	//struct _BLIT *blit;
	
	union {
		struct {
			S16 x;
			S16 y;
			U8 stepSize;
			U8 flag;
		} move;
		
		struct {
			U8 stepSize;
			U8 flag;
			U8 count;
		} follow;
		
		U8 wanderCount;
		
		U8 loopFlag;
	};
} VOBJ;

/*
typedef struct _BLIT {
	struct _BLIT *prev, *next;
	VOBJ	*v;
	S16		x, y;
	S16		width, height;
	U8		*buffer;
} BLIT;

extern BLIT blUpdate, blStatic;


typedef struct {
	U8		view,loop,cel,x,y,pri;
} PVIEW;

#define MAX_PVIEWS 32
extern PVIEW pViews[MAX_PVIEWS], *pPView;
*/

extern U8 priTable[172];

//#define MAX_VOBJ			32 // erm..can be set to higher in the object file, but none use it more than 17 AFAIK (save me RAM!)
#define MAX_VOBJ			18	// GBAGI default: 32


//#define TEST_Y(x1,x2,y)	((x1 >= code[1]) && (y >= code[2]) && (x2 <= code[3]) && (y <= code[4]))


extern VOBJ picView,objView, ViewObjs[MAX_VOBJ];
//extern BLIT blUpdate, blStatic;
//extern BLIT blits[MAX_VOBJ];
extern BOOL PRI_FIXED;

void InitViewSystem(void);
void UpdateVObj(void);
void CalcVObjsDir(void);
BOOL CheckBlockPoint(U8 x, U8 y);
void UpdateObjsStep(void);
BOOL CheckObjCollision(VOBJ *v1);
BOOL CheckObjControls(VOBJ *v);
BOOL CheckObjInScreen(VOBJ *v);
void SolidifyObjPosition(VOBJ *v);
void UpdateObjMove(VOBJ *v);
void StopObjMoving(VOBJ *v);
int  FindDirection(int x, int y, int newX, int newY, int stepSize);
int  DistanceVSStep(int distance, int step);
void UpdateObjWander(VOBJ *v);
void UpdateObjFollow(VOBJ *v);
void SetObjView(VOBJ *v, int num);
void SetObjLoop(VOBJ *v, int loop);
void SetObjCel(VOBJ *v, int cel);
void DrawObj(int num);
void EraseObj(int num);
void UpdateObjCel(VOBJ *v);
void UpdateObjLoop(VOBJ *v);
/*
void SaveBlit(BLIT *b);
void RestoreBlit(BLIT *b);
*/
void UnBlitVObj(VOBJ *v);	// htk
void BlitVObj(VOBJ *v);
BOOL CheckUpdateVObj(VOBJ *v);
BOOL CheckStaticVObj(VOBJ *v);

void EraseBlitLists(void);
void DrawBlitLists(void);
/*
void UpdateBlitLists(void);
void EraseBlitList(BLIT *b);
BLIT *BuildBlitList( BOOL(*f)(VOBJ *) , BLIT *blParent);
void AddBlit(VOBJ *v, BLIT *blParent);
BLIT *NewBlit(VOBJ *v);
void DrawBlitList(BLIT *blParent);
void UpdateBlitList(BLIT *blParent);
*/
void AddToPic(U8 num, U8 loop, U8 cel, U8 x, U8 y, U8 pri);
void AddObjPicPri(VOBJ *v);
int  CalcPriY(int pri);
void ShowObj(int num);


#endif
