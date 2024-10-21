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

/*
void dump_vars() {
	//for(int i = 0; i < MAX_VARS; i++) {
	printf("v:");
	for(int i = 0; i < 20; i++) {
		printf_x2(vars[i]);
	}
	putchar('\n');
}
*/

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
