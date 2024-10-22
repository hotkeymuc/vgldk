/***************************************************************************

VAGI - VTech AGI
===========
2024-09-14 Bernhard "HotKey" Slawik

The VAGI code is heavily based on:
	* ScummVM's AGI: https://github.com/scummvm/scummvm/tree/master/engines/agi
	* Brian Provinciano's GBAGI
		Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
	* davidebyzero's fork of GBAGI
		https://github.com/Davidebyzero/GBAGI.git

 **************************************************************************
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
 ***************************************************************************
 */


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

//#define VIEW_SHOW_NUM	// Draw VObj numbers as labels

//U8 priTable[172];	// moved to main
//VOBJ picView, ViewObjs[MAX_VOBJ];	// moved to main

//VOBJ *viewPtrs[MAX_VOBJ];
//PVIEW pViews[MAX_PVIEWS], *pPView;
//BLIT blUpdate, blStatic;
//BLIT blits[MAX_VOBJ];
BOOL PRI_FIXED;
//int priYList[MAX_VOBJ];




const int objDirTableX[] 	= { 0, 0, 1, 1, 1, 0,-1,-1,-1};
const int objDirTableY[] 	= { 0,-1,-1, 0, 1, 1, 1, 0,-1};
const int vObjDirs[]		= {dirUPLEFT, dirUP, dirUPRIGHT,  dirLEFT, dirNONE, dirRIGHT,  dirDOWNLEFT, dirDOWN, dirDOWNRIGHT};	// for FindDirection()
const int loopDirsFull[]	= {lpIGNORE,lpIGNORE,lpRIGHT,lpRIGHT,lpRIGHT,lpIGNORE,lpLEFT,lpLEFT,lpLEFT,lpRIGHT};
const int loopDirsSingle[]	= {lpIGNORE,lpUP,lpRIGHT,lpRIGHT,lpRIGHT,lpDOWN,lpLEFT,lpLEFT,lpLEFT,lpRIGHT};
/*
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
*/

void InitViewSystem() {
	int i = 0;
	VOBJ *v;
	
	//pPView = pViews;
	
	PRI_FIXED = TRUE;
	
	// Populate priTable:
	//memcpy(priTable, priTableStart, sizeof(priTable));
	// 12* 4,4,4,4, 5-14
	memset(&priTable[0], 0, 172);	//sizeof(priTable));
	for(i = 0; i < 14; i++) {
		memset(&priTable[i*12], (i<3) ? 4 : i+1, 12);
	}
	memset(&priTable[168], 0, 4);
	
	
	
	memset((byte *)&ViewObjs[0], 0, sizeof(ViewObjs));
	//for(v=ViewObjs; v<&ViewObjs[MAX_VOBJ]; v++) {
	for(i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		v->num = i;
	}
	/*
	memset(&blUpdate, 0,sizeof(BLIT));
	memset(&blStatic, 0,sizeof(BLIT)); 
	memset(blits,0,sizeof(blits));
	
	EraseBlitLists();
	*/
}

void UpdateVObj() {
	// This is called by the main game loop after the logic has run
	
	BOOL ANY_TO_DRAW=FALSE;
	int newLoop;
	VOBJ *v;
	int i;
	
	vagi_trace_stage("UpdateVObj...");
	
	for(i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		/*
		//printf("UpdateVObj");printf_d(i);printf("...");
		//@FIXME: For now: Just always try drawing every ViewObj
		//if (v->flags & (oDRAWN) > 0) {
		if (v->flags & (oDRAWN|oUPDATE) == (oDRAWN|oUPDATE)) {
			if ((v->flags & (oSKIPUPDATE))) {
			} else {
				//UnBlitVObj(v);
				//BlitVObj(v);
			}
		}
		*/
		
		if((v->flags & (oDRAWN|oANIMATE|oUPDATE)) == (oDRAWN|oANIMATE|oUPDATE)) {
			ANY_TO_DRAW = TRUE;
			newLoop = lpIGNORE;
			
			//BlitVObj(v);
			
			if(!(v->flags & oFIXEDLOOP)) {
				if( (v->totalLoops==2)||(v->totalLoops==3) ) {
					newLoop =  loopDirsFull[v->direction];
				} else
				if(AGIVER.major==3) {
					if(AGIVER.minor<=0x2086) {
						if(v->totalLoops>=4) { // King's Quest 4
							newLoop = loopDirsSingle[v->direction];
						}
					} else { // known in 0x2102, 0x2107, 0x2149  (kq4 demo, demo pack 4, mh1, mh2)
						if((v->totalLoops==4)||(TestFlag(fLOOPMODE)&&v->totalLoops>4)) {
							newLoop = loopDirsSingle[v->direction];
						}
					}
				} else {
					if (v->totalLoops==4) {
						// version 2
						newLoop = loopDirsSingle[v->direction];
					}
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
		//EraseBlitList(&blUpdate);
		vagi_trace_stage("UpdateObjsStep");
		UpdateObjsStep();	// This moves all objects
		
		//vagi_trace_stage("DrawBlitLists");
		//DrawBlitList(BuildBlitList(CheckUpdateVObj, &blUpdate));
		DrawBlitLists();	//@FIXME: This draws all!
		//UpdateBlitList(&blUpdate);
		
		ViewObjs[0].flags &= ~(oONLAND|oWATER);
	}
	
}

void CalcVObjsDir() {
	// Called by the main game loop BEFORE the logic is run
	VOBJ *v;
	int x,y;
	int i;
	
	for (i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		
		if ((v->flags & (oDRAWN|oANIMATE|oUPDATE)) == (oDRAWN|oANIMATE|oUPDATE)) {
			if (v->stepCount == 1) {
				switch (v->motion) {
					case mtWANDER:
						vagi_trace_stage("UpdateObjWander");
						UpdateObjWander(v);
						break;
					case mtFOLLOW:
						vagi_trace_stage("UpdateObjFollow");
						UpdateObjFollow(v);
						break;
					case mtMOVE:
						vagi_trace_stage("UpdateObjMove");
						UpdateObjMove(v);
						break;
				}
				vagi_trace_stage("updated motion");
				
				if (VOBJ_BLOCKING) {
					if( (!(v->flags&oIGNORECTL)) && (v->direction) ) {
						x = v->x;
						y = v->y;
						
						if(CheckBlockPoint(x, y) == CheckBlockPoint(x + (v->stepSize * objDirTableX[v->direction]), y + (v->stepSize * objDirTableY[v->direction]))) {
							v->flags &= ~oBLOCK;
						} else {
							v->flags |= oBLOCK;
							v->direction = dirNONE;
							if(v == &ViewObjs[0])
								vars[vEGODIR] = dirNONE;
						}
					}
				} else {
					v->flags &= ~oBLOCK;
				}
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
	// Called by the main game loop after logic is run, through UpdateVObj() only if any of the objects need to move
	
	VOBJ *v;
	int border,oldX,oldY;
	int i;
	
	vars[vEGOBORDER] = vars[vOBJBORDER] = bdNONE;
	vars[vOBJECT] = 0;
	
	for(i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		
		// Copied here from UpdateBlitList:
		if(v->stepCount == v->stepTime) {
			if( (v->x == v->prevX) && (v->y == v->prevY) )
				v->flags	|= oMOTIONLESS;
			else {
				v->prevX	= v->x;
				v->prevY	= v->y;
				v->flags	&= ~oMOTIONLESS;
			}
		}
		
		
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
					if(!v->num) {
						vars[vEGOBORDER]	= border;
					} else {
						vars[vOBJECT]		= v->num;
						vars[vOBJBORDER]	= border;
					}
					
					if(v->motion == mtMOVE) {
						StopObjMoving(v);
					}
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
	
	int sx = game_to_screen_x(v->x);
	int sy = game_to_screen_y(v->y);
	int bx = screen_to_buffer_x(sx);
	int by = screen_to_buffer_y(sy);
	
	if (bx < 0) bx = 0;
	if (bx >= BUFFER_WIDTH) bx = BUFFER_WIDTH-1;
	if (by < 0) by = 0;
	if (by >= BUFFER_HEIGHT) by = BUFFER_HEIGHT-1;
	
	int flags = FLAG_CONTROL;
	int x = screen_to_buffer_x(game_to_screen_x(v->width));	//v->pCel[0];
	int pri;
	
	
	if (!(v->flags & oFIXEDPRIORITY))
		v->priority = priTable[v->y];
	
	if (v->priority != 15) {
		buffer_switch(BUFFER_BANK_PRI);	// Map destination working buffer to 0xc000
		
		flags |= FLAG_WATER;
		do {
			pri = buffer_get_pixel_4bit(bx++, by);
			if (bx >= BUFFER_WIDTH) {
				//@TODO: flags &= ~FLAG_CONTROL; ?
				break;
			}
			
			if (pri == 0) {
				flags &= ~FLAG_CONTROL;
				break;
			}
			if (pri != PRI_WATER) {
				flags &= ~FLAG_WATER;
				if (pri == PRI_CONTROL) {
					if(!( v->flags&oIGNORECTL)) {
						flags &= ~FLAG_CONTROL;
						break;
					}
				} else
				if (pri == PRI_SIGNAL) {
					flags |= FLAG_SIGNAL;
				}
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
	
	if (v->num == 0) { // ego
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
	
	while( (!CheckObjInScreen(v)) || CheckObjCollision(v) || (!CheckObjControls(v)) ) {
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
	
}

void UpdateObjMove(VOBJ *v) {
	// Rotate in the optimum direction to face v->move.x/y
	v->direction = FindDirection(v->x, v->y, v->move.x, v->move.y, v->stepSize);
	
	if(v == &ViewObjs[0])
		vars[vEGODIR] = v->direction;
	
	// If no new direction found (i.e. reached destination)
	if(!v->direction) {
		//printf("Object reached move dest:\n");
		//printf_d(v->x);putchar('=');printf_d(v->move.x);putchar('\n');
		//printf_d(v->y);putchar('=');printf_d(v->move.y);putchar('\n');
		//getchar();
		StopObjMoving(v);
	}
}

void StopObjMoving(VOBJ *v) {
	SetFlag(v->move.flag);	// Signal to the logic that the object has reached its destination
	
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
	if( (!count) || (v->flags & oMOTIONLESS) ) {
		v->direction = rand()%9;
		if(v == &ViewObjs[0])
			vars[vEGODIR]	= v->direction;
		while (v->wanderCount < 6)
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
	vagi_res_handle_t h;
	
	/*
	if(!viewDir[num]) ErrorMessage(ERR_NO_VIEW,num);
	*/
	//v->pView		= (U8*)viewDir[num]+5;
	h = vagi_res_open(AGI_RES_KIND_VIEW, num);
	if (h < 0) {
		//printf("VIEW err=-"); printf_d(-h); getchar();
		return;
	}
	//v->pView = (U8 *)num;	// Just so it isn't NULL
	v->viewLoaded = true;
	v->invisible = false;
	v->view			= num;
	
	vagi_res_skip(h, 2);	// unknown header
	
	v->totalLoops	= vagi_res_read(h);
	v->descPos	= vagi_res_read_word(h);	// If view == object
	
	vagi_res_close(h);
	
	SetObjLoop(v, (v->loop >= v->totalLoops)?0:v->loop);
}

void SetObjLoop(VOBJ *v, int loop) {
	vagi_res_handle_t h;
	
	//if(v->pView == NULL) ErrorMessage(ERR_VOBJ_VIEW, v->num);
	if(!v->viewLoaded) { ErrorMessage(ERR_VOBJ_VIEW, v->num); return; }
	
	//if(loop > v->totalLoops) ErrorMessage2(ERR_VOBJ_LOOP, v->num, loop);
	
	if(loop == v->totalLoops)
		loop = v->totalLoops - 1;
	
	v->loop			= loop;
	
	h = vagi_res_open(AGI_RES_KIND_VIEW, v->view);
	
	// Skip 2*U8 ?, U8 num_loops,  WORD desc_pos
	vagi_res_skip(h, 5 + (loop << 1));
	
	word o = vagi_res_read_word(h);
	//v->pLoop		= (U8 *)o;	//@TODO: ... = v->pView + bGetW(v->pView + 5 + (loop << 1));
	v->oLoop		= o;
	
	vagi_res_seek_to(h, o);
	v->totalCels	= vagi_res_read(h);	// = v->pLoop[0];
	
	vagi_res_close(h);
	
	if(v->cel >= v->totalCels)
		v->cel = 0;
	
	SetObjCel(v, v->cel);
	
}

void SetObjCel(VOBJ *v, int cel) {
	vagi_res_handle_t h;
	
	//if(v->pView == NULL) ErrorMessage(ERR_VOBJ_VIEW, v->num);
	if (!v->viewLoaded) { ErrorMessage(ERR_VOBJ_VIEW, v->num); return; }
	
	if (cel > v->totalCels) { ErrorMessage2(ERR_VOBJ_CEL, v->num, cel); return; }
	v->cel		= cel;
	
	
	//v->pLoop + bGetW(v->pLoop + (cel<<1) + 1);
	h = vagi_res_open(AGI_RES_KIND_VIEW, v->view);
	vagi_res_seek_to(h, v->oLoop);
	
	// 1 byte "no of cels", then offsets U16
	vagi_res_skip(h, 1 + (cel << 1));
	word o = vagi_res_read_word(h);	// Offset AFTER loop offset
	
	//v->pCel		= (U8 *)o;
	v->oCel		= v->oLoop + o;
	
	vagi_res_seek_to(h, v->oCel);
	v->width	= vagi_res_read(h);	//sprite_width		//... = v->pCel[0];
	v->height	= vagi_res_read(h);	//sprite_height;	//... = v->pCel[1];
	v->settings = vagi_res_read(h);	// HI nibble: mirroring; LO nibble: transparency index
	//	cell_mirroring = cell_settings >> 4
	//	cell_transparency = cell_settings & 0b00001111	// transparency palette index
	//	data...
	vagi_res_close(h);
	
	
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
	
}

void DrawObj(int num) {
	VOBJ *v;
	
	if(num >= MAX_VOBJ) {
		ErrorMessage(ERR_VOBJ_NUM,num);
		//return;
	}
	
	v = &ViewObjs[ num ];
	
	//if(v->pCel == NULL) ErrorMessage(ERR_VOBJ_NO_CEL,num);
	
	if(!(v->flags & oDRAWN)) {
		v->flags		|= oUPDATE;
		SolidifyObjPosition(v);
		//v->prevWidth	= v->pCel[0];
		//v->prevHeight	= v->pCel[1];
		v->prevWidth	= v->width;	//v->pCel[0];
		v->prevHeight	= v->height;	//v->pCel[1];
		v->prevX		= v->x;
		v->prevY		= v->y;
		
		/*
		EraseBlitList(&blUpdate);
		*/
		
		v->flags		|= oDRAWN;
		
		/*
		DrawBlitList(BuildBlitList(CheckUpdateVObj, &blUpdate));
		*/
		
		UpdateObjCel(v);
		
		v->flags		&= ~oSKIPUPDATE;
	}
	
	
}

void EraseObj(int num) {
	//BOOL NO_UPDATE;
	VOBJ *v;
	
	if(num >= MAX_VOBJ) {
		ErrorMessage(ERR_VOBJ_NUM,num);
	}
	
	v = &ViewObjs[ num ];
	//@TODO: Release vagi_res_handle_t
	
	if(v->flags & oDRAWN) {
		/*
		EraseBlitList(&blUpdate);
		if((NO_UPDATE = !(v->flags & oUPDATE))!=FALSE) EraseBlitList(&blStatic);
		*/
		
		//@TODO: Partial redraw: Re-draw that section from buffer, since we won't be having enough RAM for background blits I think...
		//draw_buffer(BUFFER_BANK_VIS, v->x,v->x+v->width, v->y, v->y+v->height, 0,0,true);
		UnBlitVObj(v);
		
		v->flags &= ~oDRAWN;
		/*
		if (NO_UPDATE) DrawBlitList(BuildBlitList(CheckStaticVObj, &blStatic));
		DrawBlitList(BuildBlitList(CheckUpdateVObj, &blUpdate));
		*/
		UpdateObjCel(v);
		
	}
	
}

void UpdateObjCel(VOBJ *v) {
	
	//U8 *celData;
	int prevHeight, prevWidth;
	int x1, y1, x2, y2, width1, height1, width2, height2, width, height;
	
	if(!PIC_VISIBLE)
		return;
	
	//celData = v->pCel;
	prevHeight = v->prevHeight;
	prevWidth = v->prevWidth;
	v->prevHeight	= v->height;	//celData[1];
	v->prevWidth	= v->width;	//celData[0];
	
	if(v->y < v->prevY) {
		y1		= v->prevY;
		y2		= v->y;
		height1	= prevHeight;
		height2	= v->height;	//celData[1];
	} else {
		y1		= v->y;
		y2		= v->prevY;
		height1	= v->height;	//celData[1];
		height2	= prevHeight;
	}
	
	height = ((y2-height2) > (y1-height1))? height1 : y1 - y2 + height2;
	
	if(v->x > v->prevX) {
		x1		= v->prevX;
		x2		= v->x;
		width1	= prevWidth;
		width2	= v->width;	//celData[0];
	} else {
		x1		= v->x;
		x2		= v->prevX;
		width1	= v->width;	//celData[0];
		width2	= prevWidth;
	}
	
	width = ((x2+width2) < (x1+width1))? width1 : width2 + x2-x1;
	
	
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	
	if(x1+width+1 >= PIC_WIDTH)
		width = PIC_WIDTH-1-x1-1;
	if(y1-height-1 <= 0)
		height = y1-1;
	
	//RenderUpdate(x1, y1-height+1, x1+width+1, y1+1);
	
	// Draw over its PREVIOUS position and size
	byte sx1 = game_to_screen_x(x1);
	byte sy1 = game_to_screen_y(y1);
	byte sw = game_to_screen_x(width);
	byte sh = game_to_screen_y(height);
	
	// Redraw object background
	draw_buffer(
		BUFFER_BANK_VIS,
		
		sx1-1,       sx1+sw+1,
		sy1-sh-1, sy1+1,
		
		0,0	//, true
	);
	
}

void UpdateObjLoop(VOBJ *v) {
	
	int cel,max;
	
	if(v->flags & oSKIPUPDATE)
		v->flags &= ~oSKIPUPDATE;
	else {
		cel = v->cel;
		max = v->totalCels - 1;
		switch (v->cycle) {
			case cyNORMAL:
				cel++;
				if(cel > max) cel = 0;
				break;
			case cyENDOFLOOP:
				if((cel < max) && (++cel != max)) break;
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
	
}

/*
void SaveBlit(BLIT *b) {
	//@TODO: Implement
	U8 *pBuf = MAKE_PICBUF_PTR(b->x,b->y), *bBuf = b->buffer;
	int y = b->height;
	
	do {
		memcpy(bBuf, pBuf, b->width);
		pBuf += PIC_WIDTH;
		bBuf += b->width;
	} while(--y);
	
}


void RestoreBlit(BLIT *b) {
	//@TODO: Partial redraw: Re-draw that section from buffer, since we won't be having enough RAM for background blits I think...
	//draw_buffer(BUFFER_BANK_VIS, b->x,b->x+b->width, b->y, b->y+b->height, 0,0,true);
	
	
	U8 *pBuf = MAKE_PICBUF_PTR(b->x,b->y), *bBuf = b->buffer;
	int y = b->height;
	
	do {
		memcpy(pBuf, bBuf, b->width);
		pBuf += PIC_WIDTH;
		bBuf += b->width;
	} while(--y);
	
}
*/

BOOL CheckUpdateVObj(VOBJ *v) {
	return ((v->flags & (oDRAWN|oUPDATE|oANIMATE)) == (oDRAWN|oUPDATE|oANIMATE));
}


BOOL CheckStaticVObj(VOBJ *v) {
	return ((v->flags & (oDRAWN|oUPDATE|oANIMATE)) == (oDRAWN|oANIMATE));
}


void EraseBlitLists() {
	//EraseBlitList(&blUpdate);
	//EraseBlitList(&blStatic);
	
	// Refresh all backgrounds?
}


void DrawBlitLists() {
	//DrawBlitList(BuildBlitList(CheckStaticVObj, &blStatic));
	//DrawBlitList(BuildBlitList(CheckUpdateVObj, &blUpdate));
	
	// Cheap! Draw all sprites
	// Partial redraw: Re-draw that section from buffer, since we won't be having enough RAM for background blits I think...
	//draw_buffer(BUFFER_BANK_VIS, v->x,v->x+v->width, v->y, v->y+v->height, 0,0,true);
	VOBJ *v;
	byte i;
	
	// Unblit all
	for(i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		if (v->flags & (oDRAWN|oUPDATE) == (oDRAWN|oUPDATE)) {
		//if (v->flags & (oDRAWN) == (oDRAWN)) {
			//if (!(v->flags & oSKIPUPDATE))
			
			
			
			// Trying to be smart:
			// Only redraw if something changed
			
			//@FIXME: Also if mirroring/direction changed!! See SQ2 "broom" scene while wandering
			//@FIXME: Also if cel changed? (Updates separately?)
			
			if ((v->flags & oMOTIONLESS) || ((v->x == v->prevX) && (v->y == v->prevY) && (v->width == v->prevWidth) && (v->height == v->prevHeight))) {
				// Not moved: Skip it
				
			} else {
				//UnBlitVObj(v);	// Draw over with background
				UpdateObjCel(v);	// Only redraw the pixels that were crossed during movement
			}
			
		//	BlitVObj(v);
		}
	}
	// Re-blit all
	for(i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		//if (v->flags & (oDRAWN|oUPDATE) == (oDRAWN|oUPDATE)) {
		if (v->flags & (oDRAWN) == (oDRAWN)) {
			//if (!(v->flags & oSKIPUPDATE))
			//UnBlitVObj(v);
			BlitVObj(v);
		}
	}
}

void DrawBlitList() {
	// ...
}


/*
void UpdateBlitLists() {
	UpdateBlitList(&blStatic);
	UpdateBlitList(&blUpdate);
}

void EraseBlitList(BLIT *b) {
	//@TODO: Implement
	
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
	
}

BLIT *BuildBlitList( BOOL(*f)(VOBJ *) , BLIT *blParent) {
	//@TODO: Implement
	
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
	
}

void AddBlit(VOBJ *v, BLIT *blParent) {
	//@TODO: Implement
	
	BLIT *blNew, *blPrev;
	
	if((BOOL) ((blNew = NewBlit(v))->prev = blParent->prev)) {
		blPrev = blNew->prev;
		blPrev->next = blNew;
	}
	blParent->prev = blNew;
	if(!blParent->next)
		blParent->next = blNew;
	
}

BLIT *NewBlit(VOBJ *v) {
	//@TODO: Implement
	
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
	
}

void DrawBlitList(BLIT *blParent) {
	//@TODO: Implement
	
	BLIT *b=blParent->next;
	while(b) {
		SaveBlit(b);
		BlitVObj(b->v);
		b=b->next;
	}
	
}

void UpdateBlitList(BLIT *blParent) {
	//@TODO: Implement
	
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
	
}
*/




void AddToPic(U8 num, U8 loop, U8 cel, U8 x, U8 y, U8 pri) {
	//@TODO: Implement
	
	SetObjView(&picView, num);
	SetObjLoop(&picView, loop);
	SetObjCel(&picView, cel);
	
	
	picView.prevHeight				= picView.width;	//.pCel[1];
	picView.prevWidth				= picView.height;	//pCel[0];
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
	/*
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
	vagi_res_handle_t h;
	
	char obj_message_buf[255];
	
	char c;
	char *p = &obj_message_buf[0];	// Destination pointer
	
	//BLIT *b;
	static VOBJ objView; // static because of GCC bug
	
	// set up a temporary view object
	
	// set up the loop/cels
	objView.loop		= 0;
	objView.cel			= 0;
	SetObjView(&objView, num);
	
	// size/position/attributes
	objView.prevWidth	= objView.width;
	objView.prevHeight	= objView.height;
	objView.x 			=
	objView.prevX 		= (PIC_MAXX-objView.width)>>1;
	objView.y 			=
	objView.prevY 		= (PIC_MAXY-10);
	objView.priority	= 15;
	objView.flags		|= oFIXEDPRIORITY;
	objView.num			= 255;
	
	// draw it
	//SaveBlit(b = NewBlit(&objView));
	BlitVObj(&objView);
	//UpdateObjCel(&objView);
	
	// display the description
	/*
	s = (char*)viewDir[num]+5;
	vagi_res_seek_to( objView.descPos )..
	MessageBox(s + bGetW(s+3));
	*/
	// Seek to view...
	h = vagi_res_open(AGI_RES_KIND_VIEW, num);
	if (h >= 0) {
		vagi_res_seek_to(h, objView.descPos);
		p = &obj_message_buf[0];	// Destination pointer
		while (c = vagi_res_read(h)) {
			*p++ = c;
		}
		*p = 0;
		vagi_res_close(h);
		
		// Display!
		MessageBox(&obj_message_buf[0]);
	}
	//if (TestFlag(fPRINTMODE))
	getchar();
	
	// Clean up
	//RestoreBlit(b);
	//UpdateObjCel(&objView);
	UnBlitVObj(&objView);
	
}

