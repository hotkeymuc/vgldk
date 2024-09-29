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


//#include <stdlib.h>
//#include "gbagi.h"
//#include "views.h"
//#include "picture.h"
//#include "screen.h"
//#include "errmsg.h"
//#include "gamedata.h"
//#include "system.h"
//#include "text.h"

#include "agi.h"
#include "agi_view.h"
#include "agi_pic.h"
#include "agi_vars.h"

VOBJ picView, ViewObjs[MAX_VOBJ], *viewPtrs[MAX_VOBJ];
PVIEW pViews[MAX_PVIEWS], *pPView;
BLIT blUpdate, blStatic;
//BLIT blits[MAX_VOBJ];
BOOL PRI_FIXED;         
int priYList[MAX_VOBJ];   
U8 priTable[172];

const int objDirTableX[] 	= { 0, 0, 1, 1, 1, 0,-1,-1,-1};
const int objDirTableY[] 	= { 0,-1,-1, 0, 1, 1, 1, 0,-1};
const int vObjDirs[]		= {dirUPLEFT, dirUP, dirUPRIGHT,  dirLEFT, dirNONE, dirRIGHT,  dirDOWNLEFT, dirDOWN, dirDOWNRIGHT};
const int loopDirsFull[]	= {lpIGNORE,lpIGNORE,lpRIGHT,lpRIGHT,lpRIGHT,lpIGNORE,lpLEFT,lpLEFT,lpLEFT,lpRIGHT};
const int loopDirsSingle[]	= {lpIGNORE,lpUP,lpRIGHT,lpRIGHT,lpRIGHT,lpDOWN,lpLEFT,lpLEFT,lpLEFT,lpRIGHT};
const U8 priTableStart[172] = {
	 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	10,10,10,10,10,10,10,10,10,10,10,10,
	11,11,11,11,11,11,11,11,11,11,11,11,
	12,12,12,12,12,12,12,12,12,12,12,12,
	13,13,13,13,13,13,13,13,13,13,13,13,
	14,14,14,14,14,14,14,14,14,14,14,14,
	0,0,0,0
};

void InitViewSystem() {
	int i = 0;
	VOBJ *v;
	
	pPView = pViews;
	
	PRI_FIXED = TRUE;
	memcpy(priTable, priTableStart, sizeof(priTable));
	
	memset((byte *)&ViewObjs[0], 0, sizeof(ViewObjs));
	//for(v=ViewObjs; v<&ViewObjs[MAX_VOBJ]; v++) {
	for(i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		v->num = i++;
	}
	/*
	memset(&blUpdate, 0,sizeof(BLIT));
	memset(&blStatic, 0,sizeof(BLIT)); 
	memset(blits,0,sizeof(blits));
	
	EraseBlitLists();
	*/
}

void UpdateVObj() {
	BOOL ANY_TO_DRAW=FALSE;
	int newLoop;
	VOBJ *v;
	int i;
	
	//for(v=ViewObjs; v<&ViewObjs[MAX_VOBJ]; v++) {
	for(i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		
		if((v->flags & (oDRAWN|oANIMATE|oUPDATE)) == (oDRAWN|oANIMATE|oUPDATE)) {
			ANY_TO_DRAW = TRUE;
			newLoop = lpIGNORE;
			
			if(!(v->flags & oFIXEDLOOP)) {
				if( (v->totalLoops==2)||(v->totalLoops==3) )
					newLoop =  loopDirsFull[v->direction];
				else if(AGIVER.major==3) {
					if(AGIVER.minor<=0x2086) {
						if(v->totalLoops>=4) // King's Quest 4
							newLoop = loopDirsSingle[v->direction];
					} else { // known in 0x2102, 0x2107, 0x2149  (kq4 demo, demo pack 4, mh1, mh2)
						if((v->totalLoops==4)||(TestFlag(fLOOPMODE)&&v->totalLoops>4))
							newLoop = loopDirsSingle[v->direction];
					}
				} else {
					if(v->totalLoops==4) // version 2
						newLoop = loopDirsSingle[v->direction];
				}
			}
			if((v->stepCount == 1) && (newLoop != lpIGNORE) && (v->loop != newLoop))
				SetObjLoop(v, newLoop);
			
			if( (v->flags & oCYCLE) && v->cycleCount ) {
				if(!--v->cycleCount) {
					UpdateObjLoop(v);
					v->cycleCount = v->cycleTime;
				}
			}
		}
	}
	
	if(ANY_TO_DRAW) {
		EraseBlitList(&blUpdate);
		UpdateObjsStep();
		DrawBlitList(BuildBlitList(CheckUpdateVObj, &blUpdate));
		UpdateBlitList(&blUpdate);
		ViewObjs[0].flags &= ~(oONLAND|oWATER);
	}
}     

void CalcVObjsDir() {
	VOBJ *v;
	int x,y;
	int i;
	
	//for(v=ViewObjs; v<&ViewObjs[MAX_VOBJ]; v++)
	for(i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		
		if((v->flags & (oDRAWN|oANIMATE|oUPDATE))==(oDRAWN|oANIMATE|oUPDATE)) {
			if(v->stepCount == 1) {
				switch(v->motion) {
					case mtWANDER:
						UpdateObjWander(v);
						break;
					case mtFOLLOW:
						UpdateObjFollow(v);
						break;
					case mtMOVE:
						UpdateObjMove(v);
						break;
				}
				
				if(VOBJ_BLOCKING) {
					if( (!(v->flags&oIGNORECTL)) && (v->direction) ) {
						x = v->x;
						y = v->y;
						
						if(CheckBlockPoint(x, y) == CheckBlockPoint(x + (v->stepSize * objDirTableX[v->direction]), y + (v->stepSize * objDirTableY[v->direction])))
							v->flags &= ~oBLOCK;
						else {
							v->flags |= oBLOCK;
							v->direction = dirNONE;
							if(v == &ViewObjs[0])
								vars[vEGODIR] = dirNONE;
						}
					}
				} else
					v->flags &= ~oBLOCK;
			}
		}
	}
}

BOOL CheckBlockPoint(U8 x, U8 y) {
	return (
		(objBlock.left < x) && (objBlock.right  > x) &&
		(objBlock.top  < y) && (objBlock.bottom > y)
	);
}

void UpdateObjsStep() {
	VOBJ *v;
	int border,oldX,oldY;
	int i;
	
	vars[vEGOBORDER] = vars[vOBJBORDER] = bdNONE;
	vars[vOBJECT] = 0;
	
	//for(v=ViewObjs; v<&ViewObjs[MAX_VOBJ]; v++)
	for(i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		
		if( (v->flags&(oDRAWN|oUPDATE|oANIMATE)) == (oDRAWN|oUPDATE|oANIMATE) ) {
			if(v->stepCount <= 1) {
				
				v->stepCount	= v->stepTime;
				border			= bdNONE;
				oldX			= v->x;
				oldY			= v->y;
				
				if(!(v->flags & oREPOSITIONING)) {
					v->x	+= v->stepSize * objDirTableX[v->direction];
					v->y	+= v->stepSize * objDirTableY[v->direction];
				}
				
				if(v->x < 0) {
					v->x 	= 0;
					border 	= bdLEFT;
				} else if((v->x+v->width) > PIC_WIDTH) {
					v->x 	= PIC_WIDTH - v->width;
					border 	= bdRIGHT;
				}
				
				if((v->y-v->height) < -1) {
					v->y	= v->height - 1;
					border	= bdTOP;
				} else if(v->y > PIC_MAXY) {
					v->y	= PIC_MAXY;
					border	= bdBOTTOM;
				} else if( (!(v->flags&oINGOREHORIZON)) && (horizon>=v->y) ) {
					v->y	= horizon + 1;
					border	= bdTOP;
				}
				
				if( (CheckObjCollision(v)) || (!CheckObjControls(v)) ) {
					v->x	= oldX;
					v->y	= oldY;
					border	= bdNONE;
					SolidifyObjPosition(v);
				}
				
				if(border) {
					if(!v->num)
						vars[vEGOBORDER]	= border;
					else {
						vars[vOBJECT]		= v->num;
						vars[vOBJBORDER]	= border;
					}
					
					if(v->motion == mtMOVE)
						StopObjMoving(v);
				}
				
				v->flags &= ~oREPOSITIONING;
			} else
				v->stepCount--;
		}
	}
}

BOOL CheckObjCollision(VOBJ *v1) {
	VOBJ *v2;
	int i2;
	
	if(!(v1->flags & oIGNOREVOBJS)) {
		//for(v2=ViewObjs; v2<ViewObjs+MAX_VOBJ; v2++) {
		for(i2 = 0; i2 < MAX_VOBJ; i2++) {
			v2 = &ViewObjs[i2];
			
			// messy but juicy!
			if(	( (v2->flags & (oDRAWN|oANIMATE)) == (oDRAWN|oANIMATE) ) &&
				(!(v2->flags & oIGNOREVOBJS)) &&
				(v1->num != v2->num) &&
				((v1->x  + v1->width)  >= v2->x) &&
				((v2->x + v2->width) >= v1->x ) ) {
					if( (v1->y == v2->y) || ( (v1->y > v2->y) && ( v1->prevY < v2->prevY) ) ||
						( (v1->y < v2->y) && (v1->prevY > v2->prevY) ) )
							return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CheckObjControls(VOBJ *v) {
	
	//@TODO: Implement
	/*
	U8 *p = MAKE_PICBUF_PTR(v->x,v->y);
	int flags = FLAG_CONTROL, x = v->pCel[0], pri;
	
	if(!(v->flags & oFIXEDPRIORITY))
		v->priority = priTable[v->y];
	
	if(v->priority != 15) {
		flags |= FLAG_WATER;
		do {
			if(!(BOOL)(pri = (*p++ & 0xF0))) {
				flags &= ~FLAG_CONTROL;
				break;
			}
			if(pri != PRI_WATER) {
				flags &= ~FLAG_WATER;
				if(pri == PRI_CONTROL) {
					if(!( v->flags&oIGNORECTL)) {
						flags &= ~FLAG_CONTROL;
						break;
					}
				} else if(pri == PRI_SIGNAL)
					flags |= FLAG_SIGNAL;
			}
		} while(--x);
		
		if(!x) {
			if(!(flags&FLAG_WATER)) {
				if(v->flags&oWATER)
					flags &= ~FLAG_CONTROL;
			} else if(v->flags&oONLAND)
				flags &= ~FLAG_CONTROL;
		}
	}
	
	if(!v->num) { // ego
		if(flags&FLAG_SIGNAL)
			SetFlag(fEGOONSIGNAL);
		else
			ResetFlag(fEGOONSIGNAL);
		if(flags&FLAG_WATER)
			SetFlag(fEGOONWATER);
		else
			ResetFlag(fEGOONWATER);
	}
	
	return (flags&FLAG_CONTROL);
	*/
}

BOOL CheckObjInScreen(VOBJ *v) {
	if( (v->x >= 0) && ((v->x + v->width) <= PIC_WIDTH ) &&
		((v->y - v->height) >= -1) && (v->y < PIC_HEIGHT) &&
		( (v->flags&oINGOREHORIZON) || (v->y>horizon) ) )
			return TRUE;
	return FALSE;
}

void SolidifyObjPosition(VOBJ *v) {
	int checkDir = dirLEFT, checkCnt = 1, checkLen = 1;
	
	if((v->y <= horizon) && (!(v->flags&oINGOREHORIZON)))
		v->y = horizon+1;
	
	if( CheckObjInScreen(v)	&& (!CheckObjCollision(v)) && CheckObjControls(v) )
		return;
	
	while( (!CheckObjInScreen(v)) || CheckObjCollision(v) || (!CheckObjControls(v)) )
		switch(checkDir) {
			case dirLEFT:
				v->x--;
				if(!--checkCnt) {
					checkDir	= dirDOWN;
					checkCnt	= checkLen;
				}
				break;
			case dirDOWN:
				v->y++;
				if(!--checkCnt) {
					checkDir	= dirRIGHT;
					checkLen++;
					checkCnt	= checkLen;
				}
				break;
			case dirRIGHT:
				v->x++;
				if(!--checkCnt) {
					checkDir	= dirUP;
					checkCnt	= checkLen;
				}
				break;
			default: // UP
				v->y--;
				if(!--checkCnt) {
					checkDir	= dirLEFT;
					checkLen++;
					checkCnt	= checkLen;
				}
				break;
		}
}

void UpdateObjMove(VOBJ *v) {
	v->direction = FindDirection(v->x, v->y, v->move.x, v->move.y, v->stepSize);
	if(v == &ViewObjs[0])
		vars[vEGODIR] = v->direction;
	if(!v->direction)
		StopObjMoving(v);
}

void StopObjMoving(VOBJ *v) {
	SetFlag(v->move.flag);
	v->motion	= mtNONE;
	v->stepSize	= v->move.stepSize;
	
	if(v == &ViewObjs[0]) {
		PLAYER_CONTROL	= TRUE;
		vars[vEGODIR]	= dirNONE;
	}
}

int FindDirection(int x, int y, int newX, int newY, int stepSize) {
	return vObjDirs[DistanceVSStep(newX-x, stepSize) + (DistanceVSStep(newY-y, stepSize) * 3)];
}

int DistanceVSStep(int distance, int step) {
	if(-step >= distance)
		return 0;
	if( step <= distance)
		return 2;
	return 1;
}

void UpdateObjWander(VOBJ *v) {
	int count = v->wanderCount--;
	if( (!count) || (v->flags & oMOTIONLESS) )	{
		v->direction = rand()%9;
		if(v == &ViewObjs[0])
			vars[vEGODIR]	= v->direction;
		while(v->wanderCount < 6)
			v->wanderCount	= rand() % 0x33;
	}
}

void UpdateObjFollow(VOBJ *v) {
	int viewX	= v->x + (v->width>>1),
		egoX	= ViewObjs[0].x + (ViewObjs[0].width>>1),
		newDir	= FindDirection(viewX, v->y, egoX, ViewObjs[0].y, v->follow.stepSize);
	
	if(!newDir) {
		v->direction = dirNONE;
		v->motion = mtNONE;
		SetFlag(v->follow.flag);
	} else {
		if( (v->follow.count<255) && (v->flags&oMOTIONLESS) ) {
			v->direction = (rand()&7)+1;
			if((((ABS(v->y-ViewObjs[0].y)+ABS(viewX-egoX))>>1)+1) <= v->stepSize)
				v->follow.count = v->stepSize;
			else
				do
					v->follow.count = rand()&7;
				while(v->follow.count < v->stepSize);
		} else {
			if(v->follow.count == 255)
				v->follow.count = 0;
			if(v->follow.count) {
				if(v->follow.count > v->stepSize)
					v->follow.count -= v->stepSize;
				else
					v->follow.count = 0;
			} else
				v->direction = newDir;
		}
	}
}

void SetObjView(VOBJ *v, int num) {
	//@TODO: Implement
	/*
	if(!viewDir[num])
		ErrorMessage(ERR_NO_VIEW,num);
	
	v->pView		= (U8*)viewDir[num]+5;
	v->view			= num;
	v->totalLoops	= v->pView[2];
	
	SetObjLoop(v, (v->loop >= v->totalLoops)?0:v->loop);
	*/
}

void SetObjLoop(VOBJ *v, int loop) {
	//@TODO: Implement
	/*
	if(v->pView == NULL)
		ErrorMessage(ERR_VOBJ_VIEW,v->num);
	if(loop > v->totalLoops)
		ErrorMessage(ERR_VOBJ_LOOP,v->num,loop);
	if(loop == v->totalLoops)
		loop = v->totalLoops - 1;
	
	v->loop			= loop;
	v->pLoop		= v->pView + bGetW(v->pView + 5 + (loop << 1));
	v->totalCels	= v->pLoop[0];
	
	if(v->cel >= v->totalCels)
		v->cel = 0;
	
	SetObjCel(v, v->cel);
	*/
}

void SetObjCel(VOBJ *v, int cel) {
	//@TODO: Implement
	/*
	if(v->pView == NULL)
		ErrorMessage(ERR_VOBJ_VIEW,v->num);
	if(cel > v->totalCels)
		ErrorMessage(ERR_VOBJ_CEL,v->num,cel);
	
	v->cel		= cel;
	v->pCel		= v->pLoop + bGetW(v->pLoop + (cel<<1) + 1);
	v->width	= v->pCel[0];
	v->height	= v->pCel[1];
	
	if((v->x + v->width) > PIC_WIDTH ) {
		v->flags |= oREPOSITIONING;
		v->x = PIC_WIDTH - v->width;
	}
	
	if((v->y - v->height) < -1) {
		v->flags |= oREPOSITIONING;
		v->y = v->height - 1;
		if((horizon > v->y) && (!(v->flags & oINGOREHORIZON)))
			v->y = horizon + 1;
	}
	*/
}

void DrawObj(int num) {
	//@TODO: Implement
	/*
	VOBJ *v = &ViewObjs[ num ];
	
	if(num >= MAX_VOBJ)
		ErrorMessage(ERR_VOBJ_NUM,num);
	if(v->pCel == NULL)
		ErrorMessage(ERR_VOBJ_NO_CEL,num);
	
	if(!(v->flags & oDRAWN)) {
		v->flags		|= oUPDATE;
		SolidifyObjPosition(v);
		v->prevWidth	= v->pCel[0];
		v->prevHeight	= v->pCel[1];
		v->prevX		= v->x;
		v->prevY		= v->y;
		
		EraseBlitList(&blUpdate);
		
		v->flags		|= oDRAWN;
		
		DrawBlitList(BuildBlitList(CheckUpdateVObj, &blUpdate));
		
		UpdateObjCel(v);
		v->flags		&= ~oSKIPUPDATE;
	}
	*/
}

void EraseObj(int num) {
	//@TODO: Implement
	/*
	BOOL NO_UPDATE;
	VOBJ *v = &ViewObjs[ num ];
	
	if(num >= MAX_VOBJ)
		ErrorMessage(ERR_VOBJ_NUM,num);
	
	if(v->flags & oDRAWN) {
		EraseBlitList(&blUpdate);
		
		if((NO_UPDATE = !(v->flags & oUPDATE))!=FALSE)
			EraseBlitList(&blStatic);
		
		v->flags &= ~oDRAWN;
		
		if(NO_UPDATE)
			DrawBlitList(BuildBlitList(CheckStaticVObj, &blStatic));
		
		DrawBlitList(BuildBlitList(CheckUpdateVObj, &blUpdate));
		UpdateObjCel(v);
	}
	*/
}

void UpdateObjCel(VOBJ *v) {
	//@TODO: Implement
	/*
	U8 *celData;
	int prevHeight, prevWidth;
	int x1, y1, x2, y2, width1, height1, width2, height2, width, height;
	
	if(!PIC_VISIBLE)
		return;
	
	celData = v->pCel;
	prevHeight = v->prevHeight;
	prevWidth = v->prevWidth;
	v->prevHeight	= celData[1];
	v->prevWidth	= celData[0];
	
	if(v->y < v->prevY) {
		y1		= v->prevY;
		y2		= v->y;
		height1	= prevHeight;
		height2	= celData[1];
	} else {
		y1		= v->y;
		y2		= v->prevY;
		height1	= celData[1];
		height2	= prevHeight;
	}
	
	height = ((y2-height2) > (y1-height1))? height1 : y1 - y2 + height2;
	
	if(v->x > v->prevX) {
		x1		= v->prevX;
		x2		= v->x;
		width1	= prevWidth;
		width2	= celData[0];
	} else {
		x1		= v->x;
		x2		= v->prevX;
		width1	= celData[0];
		width2	= prevWidth;
	}
	
	width = ((x2+width2) < (x1+width1))? width1 : width2 + x2-x1;
	
	
	if(x1+width > PIC_WIDTH+1)
		width = PIC_WIDTH-x1+1;
	if(height-y1 > 0)
		height = y1+1;
	
	RenderUpdate(x1, y1-height+1, x1+width+1, y1+1);
	*/
}

void UpdateObjLoop(VOBJ *v) {
	//@TODO: Implement
	/*
	int cel,max;
	
	if(v->flags & oSKIPUPDATE)
		v->flags &= ~oSKIPUPDATE;
	else {
		cel = v->cel;
		max = v->totalCels - 1;
		switch (v->cycle) {
			case cyNORMAL:
				cel++;
				if(cel > max)
					cel = 0;
				break;
			case cyENDOFLOOP:
				if((cel < max) && (++cel != max))
						break;
				SetFlag(v->loopFlag);
				v->flags		&= ~oCYCLE;
				v->direction	= dirNONE;
				v->cycle		= cyNORMAL;
				break;
			case cyREVERSELOOP:
				if( cel && (--cel) ) break;
				SetFlag(v->loopFlag);
				v->flags		&= ~oCYCLE;
				v->direction	= dirNONE;
				v->cycle		= cyNORMAL;
				break;
			case cyREVERSECYCLE:
				if(cel)
					cel--;
				else
					cel = max;
				break;
		}
		SetObjCel(v, cel);
	}
	*/
}

void SaveBlit(BLIT *b) {
	//@TODO: Implement
	/*
	U8 *pBuf = MAKE_PICBUF_PTR(b->x,b->y), *bBuf = b->buffer;
	int y = b->height;
	
	do {
		memcpy(bBuf, pBuf, b->width);
		pBuf += PIC_WIDTH;
		bBuf += b->width;
	} while(--y);
	*/
}

void RestoreBlit(BLIT *b) {
	//@TODO: Implement
	/*
	U8 *pBuf = MAKE_PICBUF_PTR(b->x,b->y), *bBuf = b->buffer;
	int y = b->height;
	
	do {
		memcpy(pBuf, bBuf, b->width);
		pBuf += PIC_WIDTH;
		bBuf += b->width;
	} while(--y);
	*/
}

#define CHECK_PIC_PRI()\
	if((pri = *pBuf & 0xF0) < 0x30) {\
		pPtr = pBuf;\
		lastPri = 0;\
		while((pPtr < pBufEnd) && (lastPri < 0x30) ) {\
			pPtr += PIC_WIDTH;\
			lastPri = *pPtr & 0xF0;\
		}\
		if(lastPri > celPri)\
			pri = -1;\
	} else\
		pri = (pri>celPri)?-1:celPri /*
int CheckPicPri(U8 *pBuf, U8 *pBufEnd, int celPri) {
	int lastPri=0, pri;
	if((pri = *pBuf & 0xF0) < 0x30) {
		while((pBuf < pBufEnd) && (lastPri < 0x30) ) {
			pBuf += PIC_WIDTH;
			lastPri = *pBuf & 0xF0;
		}
		if(lastPri > celPri)
			pri = -1;
	} else
		pri = (pri>celPri)?-1:celPri;
	return pri;
}     */

void BlitVObj(VOBJ *v) {
	//@TODO: Implement
	/*
	U8 *pBuf, *pBufStart, *pBufEnd, *celData = v->pCel, *pPtr;
	int
		run = 0, pri, col, lastPri,
		celPri = (v->priority << 4)&0xF0, transCol = celData[2]&0xF,
		celWidth = celData[0], celHeight = celData[1];
	BOOL
		VIEW_INVISIBLE	= TRUE,
		MIRRORED = (celData[2]&0x80) && (((celData[2]>>4)&7) != v->loop);
	
	pBuf	= MAKE_PICBUF_PTR(v->x,v->y-(celHeight-1));
	pBufEnd = pictureBuf+(PIC_WIDTH*PIC_MAXY);
	
	celData			+= 3;
	
		if(MIRRORED)
			pBuf+=celWidth-1;
		pBufStart = pBuf;
		// draw the rows of pixels decoding the RLE
		while(celHeight) {
			if(*celData) {
				// read the data. the byte's lo nybble is the run, the hi is the colour
				run = *celData & 0x0F;
				if((col = (*celData++ >> 4)) == transCol)
					if(MIRRORED)
						pBuf -= run;
					else
						pBuf += run;
				else {
					do {
						CHECK_PIC_PRI();
						if(pri != -1) {
							*pBuf	 		= pri|col;
							VIEW_INVISIBLE 	= FALSE;
						}
						if(MIRRORED)
							pBuf--;
						else
							pBuf++;
					} while(--run);
				}
			} else {
				// if the data is '\0', the row is done, go to the next one
				celData++;
				celHeight--;
				pBufStart += PIC_WIDTH;
				pBuf = pBufStart;
			}
		}
	
	if(!v->num) {  // 'tis the ego
		if(VIEW_INVISIBLE)
			SetFlag(fEGOHIDDEN);
		else
			ResetFlag(fEGOHIDDEN);
	}
	*/
}

BOOL CheckUpdateVObj(VOBJ *v) {
	return ((v->flags & (oDRAWN|oUPDATE|oANIMATE)) == (oDRAWN|oUPDATE|oANIMATE));
}


BOOL CheckStaticVObj(VOBJ *v) {
	return ((v->flags & (oDRAWN|oUPDATE|oANIMATE)) == (oDRAWN|oANIMATE));
}


void EraseBlitLists() {
	EraseBlitList(&blUpdate);
	EraseBlitList(&blStatic);
}


void DrawBlitLists() {
	DrawBlitList(BuildBlitList(CheckStaticVObj, &blStatic));
	DrawBlitList(BuildBlitList(CheckUpdateVObj, &blUpdate));
}


void UpdateBlitLists() {
	UpdateBlitList(&blStatic);
	UpdateBlitList(&blUpdate);
}

void EraseBlitList(BLIT *b) {
	//@TODO: Implement
	/*
	BLIT *blit, *next;
	
	blit=b->prev;
	while(blit) {
		RestoreBlit(blit);
		blit=blit->prev;
	}
	blit=b->prev;
	while(blit) {
		if(blit->buffer)
			free(blit->buffer); 
		next = blit->prev;
		memset(blit,0,sizeof(BLIT));
		blit = next;
	}
	
	b->next = NULL;
	b->prev = NULL;
	*/
}

BLIT *BuildBlitList( BOOL(*f)(VOBJ *) , BLIT *blParent) {
	//@TODO: Implement
	/*
	int blitPri, i, j, num, plast;
	VOBJ *s;
	int si;
	
	num = 0;
	plast = 256;
	
	//for(s=ViewObjs; s<&ViewObjs[MAX_VOBJ]; s++) {
	for(si = 0; si < MAX_VOBJ; si++) {
		s = &ViewObjs[si];
		
		if(f(s)) {
			viewPtrs[num] = s;
			priYList[num] = (s->flags&oFIXEDPRIORITY)?
				CalcPriY(s->priority):s->y;
			num++;
		}
	}
	
	for(i=0; i<num; i++) {
		blitPri = 255;
		for(j=0;j<num;j++)
			if(priYList[j] < blitPri) {
				blitPri = priYList[j];
				plast = j;
			}
		priYList[plast] = 255;
		AddBlit( viewPtrs[plast], blParent );
	}
	
	return blParent;
	*/
}

void AddBlit(VOBJ *v, BLIT *blParent) {
	//@TODO: Implement
	/*
	BLIT *blNew, *blPrev;
	
	if((BOOL) ((blNew = NewBlit(v))->prev = blParent->prev)) {
		blPrev = blNew->prev;
		blPrev->next = blNew;
	}
	blParent->prev = blNew;
	if(!blParent->next)
		blParent->next = blNew;
	*/
}

BLIT *NewBlit(VOBJ *v) {
	//@TODO: Implement
	/*
	BLIT *b;
	
	for(b=blits;b->v&&b<blits+MAX_VOBJ;b++);
	
	if(b==blits+MAX_VOBJ)
		return NULL;
	
	b->prev 	= b->next = NULL;
	b->buffer	= (U8*)malloc(v->width * v->height);
	b->v 		= v;
	b->x 		= v->x;
	b->y 		= v->y-(v->height-1);
	b->width 	= v->width;
	b->height	= v->height;
	
	return (BLIT *)(v->blit = b);
	*/
}

void DrawBlitList(BLIT *blParent) {
	//@TODO: Implement
	/*
	BLIT *b=blParent->next;
	while(b) {
		SaveBlit(b);
		BlitVObj(b->v);
		b=b->next;
	}
	*/
}

void UpdateBlitList(BLIT *blParent) {
	//@TODO: Implement
	/*
	VOBJ *v;
	BLIT *b=blParent->prev;
	while(b) {
		UpdateObjCel(v = b->v);
		if(v->stepCount == v->stepTime) {
			if( (v->x == v->prevX) && (v->y == v->prevY) )
				v->flags	|= oMOTIONLESS;
			else {
				v->prevX	= v->x;
				v->prevY	= v->y;
				v->flags	&= ~oMOTIONLESS;
			}
		}
		b = b->prev;
	}
	*/
}


void AddToPic(U8 num, U8 loop, U8 cel, U8 x, U8 y, U8 pri) {
	//@TODO: Implement
	/*
	SetObjView(&picView, num);
	SetObjLoop(&picView, loop);
	SetObjCel(&picView, cel);
	
	picView.prevHeight				= picView.pCel[1];
	picView.prevWidth				= picView.pCel[0];
	picView.x = picView.prevX		= x;
	picView.y = picView.prevY		= y;
	picView.flags					= oIGNOREVOBJS|oINGOREHORIZON|oFIXEDPRIORITY;
	picView.priority				= 15;
	
	SolidifyObjPosition(&picView);
	
	if(!((picView.priority = pri) & 0xF))
		picView.flags	= 0;
	
	EraseBlitLists();
	AddObjPicPri(&picView);
	DrawBlitLists();
	UpdateObjCel(&picView);
	
	if(pPView < pViews+MAX_PVIEWS) {
		pPView->view	= num;
		pPView->loop	= loop;
		pPView->cel		= cel;
		pPView->x		= x;
		pPView->y		= y;
		pPView->pri		= pri;
		pPView++;
	}
	*/
}

void AddObjPicPri(VOBJ *v) {
	
	//@TODO: Implement!
	/*
	U8 *pBuf;
	int x,y,height,celMaxX,pHigh=0;
	
	if(!(v->priority & 0xF))
		v->priority |= priTable[v->y];
	
	BlitVObj(v);
	
	if(v->priority&0xC0)
		return;
	
	// get the height
	y = v->y;
	do {
		pHigh++;
		if(y <= 0)
			break;
		y--;
	} while(priTable[y] == priTable[v->y]);
	
	height = (v->pCel[1] > pHigh)? pHigh:v->pCel[1];
	
	// draw  a control box for the view object
	pBuf = MAKE_PICBUF_PTR(v->x,v->y);
	
	// draw a control line on the bottom
	x = v->pCel[0];
	do
		*(pBuf++) = (v->priority&0xF0) | (*pBuf & 0x0F);
	while(--x);
	
	// now if the view is larger than 1px high, draw the rest of the box
	if(height > 1) {
		pBuf = MAKE_PICBUF_PTR(v->x,v->y);
		
		// draw the left and right sides of the box
		celMaxX = v->pCel[0] - 1;
		y = height-1;
		do {
			pBuf 			-= PIC_WIDTH;
			pBuf[0]			= (v->priority&0xF0) | (pBuf[0]&0xF);
			pBuf[celMaxX]	= (v->priority&0xF0) | (pBuf[celMaxX]&0xF);
		} while(--y);
		
		// draw the top line
		x = v->pCel[0] - 2;
		do
			*++pBuf = (v->priority&0xF0) | (*pBuf & 0x0F);
		while(--x);
	}
	*/
}


// calculates the priority for view objects
int CalcPriY(int pri) {
	int priY = PIC_MAXY;
	
	if(PRI_FIXED)
		return((pri-5)*12 + 48);
	
	while(priTable[priY] >= pri)
		if(--priY < 0)
			return -1;
	
	return priY;
}

// displays the view and it's description
void ShowObj(int num) {
	
	//@TODO: Implement!
	/*
	char *s;
	BLIT *b;
	static VOBJ objView; // static because of GCC bug
	
	// set up a temporary view object
	
	// set up the loop/cels
	objView.loop		= 0;
	objView.cel			= 0;
	SetObjView(&objView,num);
	
	// size/position/attributes
	objView.prevWidth	= objView.pCel[0];
	objView.prevHeight	= objView.pCel[1];
	objView.x 			=
	objView.prevX 		= (PIC_MAXX-objView.width)>>1;
	objView.y 			=
	objView.prevY 		= (PIC_MAXY-10);
	objView.priority	= 15;
	objView.flags		|= oFIXEDPRIORITY;
	objView.num			= 255;
	
	// draw it
	SaveBlit(b = NewBlit(&objView));
	BlitVObj(&objView);
	UpdateObjCel(&objView);
	
	// display the description
	s = (char*)viewDir[num]+5;
	MessageBox(s + bGetW(s+3));
	
	// clean up
	RestoreBlit(b);
	UpdateObjCel(&objView);
	*/
}

