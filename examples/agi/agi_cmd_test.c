#ifndef __AGI_CMD_TEST_C__
#define __AGI_CMD_TEST_C__
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

//#include "gbagi.h"
//#include "logic.h"
//#include "commands.h"
//#include "input.h"
//#include "views.h"
//#include "parse.h"
//#include "invobj.h"
//#include "system.h"

#include "agi.h"
#include "agi_logic.h"
#include "agi_commands.h"
#include "agi_view.h"

#include <strcmpmin.h>


bool TEST_Y(U8 x1, U8 x2, U8 y) {
	//U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	U8 code_3 = code_get();
	U8 code_4 = code_get();
	return ((x1 >= code_1) && (y >= code_2) && (x2 <= code_3) && (y <= code_4));
}


// NULL evaluation
void cEvalZ() {
	IF_RESULT = FALSE;
}

//if (equaln(vA,B)) { .....
//if (vA == B) { .....
//	Returns true if vA is equal to B.
void cEqualn() {
	//IF_RESULT = ( vars[ code[0] ] == code[1] );
	//code += 2;
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	IF_RESULT = ( vars[ code_0 ] == code_1 );
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("v["); printf_d(code_0); printf("] ("); printf_d(vars[code_0]); printf(") = "); printf_d(code_1); printf("\n");
	#endif
}

//if (equalv(vA,vB)) { .....
//if (vA == vB) { .....
//	Returns true if vA is equal to vB.
void cEqualv() {
	//IF_RESULT = ( vars[ code[0] ] == vars[ code[1] ] );
	//code += 2;
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	IF_RESULT = ( vars[ code_0 ] == vars[ code_1 ] );
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("v["); printf_d(code_0); printf("] ("); printf_d(vars[code_0]); printf(") = v["); printf_d(code_1); printf("] ("); printf_d(vars[code_1]); printf(")\n");
	#endif
}

//if (lessn(vA,B)) { .....
//if (vA < B) { .....
//	Returns true if vA is less than B.
void cLessn() {
	//IF_RESULT = ( vars[ code[0] ] < code[1] );
	//code += 2;
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	IF_RESULT = ( vars[ code_0 ] < code_1 );
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("v["); printf_d(code_0); printf("] ("); printf_d(vars[code_0]); printf(") < "); printf_d(code_1); printf("\n");
	#endif
}

//if (lessn(vA,vB)) { .....
//if (vA < vB) { .....
//	Returns true if vA is less than vB.
void cLessv() {
	//IF_RESULT = ( vars[ code[0] ] < vars[ code[1] ] );
	//code += 2;
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	IF_RESULT = ( vars[ code_0 ] < vars[ code_1 ] );
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("v["); printf_d(code_0); printf("] ("); printf_d(vars[code_0]); printf(") < v["); printf_d(code_1); printf("] ("); printf_d(vars[code_1]); printf(")\n");
	#endif
}

//if (greatern(vA,B)) { .....
//if (vA > B) {
//	Returns true if vA is greater than B.
void cGreatern() {
	//IF_RESULT = ( vars[ code[0] ] > code[1] );
	//code += 2;
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	IF_RESULT = ( vars[ code_0 ] > code_1 );
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("v["); printf_d(code_0); printf("] ("); printf_d(vars[code_0]); printf(") > "); printf_d(code_1); printf("\n");
	#endif
}

//if (greaterv(vA,vB)) { .....
//if (vA > vB) { .....
//	Returns true if vA is greater than vB.
void cGreaterv() {
	//IF_RESULT = ( vars[ code[0] ] > vars[ code[1] ] );
	//code += 2;
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	IF_RESULT = ( vars[ code_0 ] > vars[ code_1 ] );
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("v["); printf_d(code_0); printf("] ("); printf_d(vars[code_0]); printf(") > v["); printf_d(code_1); printf("] ("); printf_d(vars[code_1]); printf(")\n");
	#endif
}

//if (isset(fA)) { .....
//if (fA) { .....
//	Returns true if flag fA is set.
void cIsset() {
	U8 code_0 = code_get();
	IF_RESULT = TestFlag( code_0 );
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("f["); printf_d(code_0); printf("] ("); printf_d(IF_RESULT); printf(")\n");	//if (IF_RESULT) printf("TRUE"); else printf("FALSE");
	#endif
}

//if (isset(vA)) { .....
//	Returns true if the flag determined by the value of vA is set.
void cIssetv() {
	U8 code_0 = code_get();
	IF_RESULT = TestFlag( vars[ code_0 ] );
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("f[v["); printf_d(code_0); printf("] ("); printf_d(vars[code_0]); printf("] ("); printf_d(TestFlag(vars[code_0])); printf(")\n");
	#endif
}

//if (has(iA)) { .....
//	Returns true if the inventory item iA is in the player�s inventory (i.e. the
//	item�s room number is 255).
void cHas() {
	//IF_RESULT = (invObjRooms[*code++]==0xFF);
	U8 code_0 = code_get();
	IF_RESULT = (invObjRooms[code_0]==0xFF);
}

//if (obj.in.room(iA,vB)) { .....
//	Returns true if the room number if inventory item iA is equal to vB.
void cObjInRoom() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	IF_RESULT = (invObjRooms[code_0]==vars[code_1]);
}

//if (posn(oA,X1,Y1,X2,Y2)) { .....
//	Returns true if the co-ordinates of the bottom-left pixel of object oA are
//	within the region (X1,Y1,X2,Y2).
void cPosn() {
	U8 code_0 = code_get();
	VOBJ *v = &ViewObjs[code_0];
	IF_RESULT = TEST_Y(v->x, v->x, v->y);
}

//if (controller(cA)) { .....
//	Returns true if the menu item or key assigned to controller cA has been
//	selected or pressed during the current cycle.
void cController() {
	U8 code_0 = code_get();
	IF_RESULT = controllers[code_0];
}

//if (have.key()) { .....
//	Returns true if the user has pressed a key.
void cHaveKey() {
	int k;
	
	// Get key (non-blocking).
	// k = getchar();	// Blocking!
	k = keyboard_inkey();
	
	if (k == KEY_CHARCODE_NONE) {
		IF_RESULT = FALSE;
	} else {
		vars[vKEYPRESSED] = k;
		IF_RESULT = TRUE;
	}
	
}

//if (said(�word1�,�word2�....�wordn�)) { .....
//	The said test command is different to all other commands in the language in
//	that it accepts a special type of parameter and can have any number of
//	parameters. It is used to test if the player has entered certain words.
//
//	First, the process of parsing player input needs to be explained. When the
//	player enters a command, the interpreter does the following things with it:
//	�	Removes certain punctuation characters
//	�	Assigns each word entered a number starting from 1. When doing this, it
//		tries to find the longest sequence of characters that match a word in
//		the WORDS.TOK file (so for example if the words �door�, �knob� and
//		�door knob� were in the WORDS.TOK file and the player entered �turn door
//		knob� then the words would be �turn� and �door knob� instead of �turn�,
//		�door� and �knob�). Words in group 0 (usually things like �a�, �my�,
//		�the�) are skipped (not assigned numbers).
//	�	If one or more words that are not in the WORDS.TOK file are found, it
//		will set v9 (unknown_word_no in the template game) to the first unknown
//		word.
//
//	Note: The above is based purely on observation. I am not sure if the
//	interpreter does exactly this, or in this order.
//
//	Once the player input has been received, flag 2 (input_recieved in the
//	template game) is set and flag 4 (input_parsed in the template game) is
//	reset.
//
//	When the said test command is used, it goes through each word given as a
//	parameter and compares it with the corresponding word entered by the player.
//	If they are the same (or are in the same word group), then it continues onto
//	the next word. The comparison is not case sensitive. If all the words are
//	the same, and there are no entered words left over, then the said command
//	returns true and sets flag 4.
//
//	There are a couple of special word groups:
//
//	Word group 1: �anyword� - if this word is given as a parameter, then any
//	word will do. So if you test for said(�eat�,�anyword�) then the result will
//	be true if the player enters �eat cake�, �eat chocolate�, �eat worm�, �eat
//	sword�, etc.
//
//	Word group 9999: �rol� (rest of line) - this means the rest of the line. If
//	you test for said(�kill�,�rol�) then the result will be true if the player
//	enters �kill lion�, �kill lion with sword�, etc.
void cSaid() {
	
	int i,curWord,wordCnt;
	int m = inpos;
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("cSaid()...");
	#endif
	
	wordCnt = code_get();
	
	if(inpos&&(!TestFlag(fSAIDOK))&&TestFlag(fPLAYERCOMMAND)) {
		i=0;
		while(wordCnt) {
			curWord = code_get_word();
			#ifdef AGI_LOGIC_DEBUG_IFS
				printf_d(curWord);printf("...");
			#endif
			wordCnt--;
			if(curWord == 9999) {	// Rest of line
				code_skip( wordCnt << 1);
				wordCnt=m=0;
				break;
			}
			if(!m) {
				m++;
				break;
			}
			if(curWord!=input[i]&&curWord!=1)
				break;
			i++;
			m--;
		}
	}
	if(!(wordCnt | m)) {
		SetFlag(fSAIDOK);
		IF_RESULT = TRUE;
	} else {
		code_skip( wordCnt << 1);	// Skip rest of parameters
		IF_RESULT = FALSE;
	}
}

//if (compare.strings(s1,s2)) { .....
//	Compares strings s1 and s2 and returns true if they are the same. The
//	comparison is not case-sensitive, and some characters such as space and
//	exclamation marks are ignored.
void cCompareStrings() {
	//IF_RESULT = (strcmpi(strings[code[0]],strings[code[1]])==0);
	//code += 2;
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	//IF_RESULT = (strcmpi(strings[code_0], strings[code_1])==0);
	IF_RESULT = (stricmp(strings[code_0], strings[code_1])==0);
}

//if (obj.in.box(oA,X1,Y1,X2,Y2)) { .....
//	Returns true if all of the bottom row of pixels of object oA are within the
//	region (X1,Y1,X2,Y2).
void cObjInBox() {
	U8 code_0 = code_get();
	VOBJ *v = &ViewObjs[code_0];
	IF_RESULT = TEST_Y(v->x,  v->x+(v->width>>1), v->y);
}

//if (center.posn(oA,X1,Y1,X2,Y2)) { .....
//	Returns true if the co-ordinates of the bottom-middle pixel of object oA are
//	within the region (X1,Y1,X2,Y2).
void cCenterPosn() {
	U8 code_0 = code_get();
	VOBJ *v = &ViewObjs[code_0];
	IF_RESULT = TEST_Y(v->x+(v->width>>1),  v->x+(v->width>>1), v->y);
}

//if (right.posn(oA,X1,Y1,X2,Y2)) { .....
//	Returns true if the co-ordinates of the bottom-right pixel of object oA are
//	within the region (X1,Y1,X2,Y2).
void cRightPosn() {
	U8 code_0 = code_get();
	VOBJ *v = &ViewObjs[code_0];
	IF_RESULT = TEST_Y(v->x+v->width-1,  v->x+v->width-1, v->y);
}

#endif