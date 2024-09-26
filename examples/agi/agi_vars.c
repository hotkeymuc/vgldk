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


//#include "gbagi.h"
//#include "variables.h"
#include "agi_vars.h"

U8 vars[MAX_VARS];
U8 flags[MAX_FLAGS/8];
U8 toggleFlags[MAX_FLAGS];
U8 controllers[MAX_CONTROLLERS];
CTLMAP ctlMap[MAX_CONTROLLERS];
char strings[MAX_STRINGS][MAX_STRINGS_LEN+1];


U16 ABS(int x) {
	return (x<0)?-x:x;
}

void SetFlag(U8 num) {
	flags[num>>3] |= 0x80>>(num&7);
}

void ResetFlag(U8 num) {
	flags[num>>3] &= ~(0x80>>(num&7));
}

void ToggleFlag(U8 num) {
	flags[num>>3] ^= 0x80>>(num&7);
}

BOOL TestFlag(U8 num) {
	return (flags[num>>3] & (0x80>>(num&7)))?TRUE:FALSE;
}

void ClearVars() {
	memset(vars, 0, sizeof(vars));
}

void ClearFlags() {
	memset(flags, 0, sizeof(flags));
}

void ClearControllers() {
	memset(controllers, 0, sizeof(controllers));
}

void ClearControlKeys() {
	memset((byte *)&ctlMap[0], 0, sizeof(ctlMap));
}

