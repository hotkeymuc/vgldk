#ifndef __AGI_CMD_TEST_C__
#define __AGI_CMD_TEST_C__

/*
 *  AGI Commands
* Based on GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 * For details: https://github.com/Davidebyzero/GBAGI.git
*/


#include "agi_commands.h"
//#include "gbagi.h"
//#include "logic.h"
//#include "commands.h"
//#include "input.h"
//#include "views.h"
//#include "parse.h"
//#include "invobj.h"
//#include "system.h"

// NULL evaluation
void cEvalZ()
{
	IF_RESULT = FALSE;
}

//if (equaln(vA,B)) { .....
//if (vA == B) { .....
//	Returns true if vA is equal to B.
void cEqualn()
{
	IF_RESULT = ( vars[ code[0] ] == code[1] );
	code += 2;
}

//if (equalv(vA,vB)) { .....
//if (vA == vB) { .....
//	Returns true if vA is equal to vB.
void cEqualv()
{
	IF_RESULT = ( vars[ code[0] ] == vars[ code[1] ] );
	code += 2;
}

//if (lessn(vA,B)) { .....
//if (vA < B) { .....
//	Returns true if vA is less than B.
void cLessn()
{
	IF_RESULT = ( vars[ code[0] ] < code[1] );
	code += 2;
}

//if (lessn(vA,vB)) { .....
//if (vA < vB) { .....
//	Returns true if vA is less than vB.
void cLessv()
{
	IF_RESULT = ( vars[ code[0] ] < vars[ code[1] ] );
	code += 2;
}

//if (greatern(vA,B)) { .....
//if (vA > B) {
//	Returns true if vA is greater than B.
void cGreatern()
{
	IF_RESULT = ( vars[ code[0] ] > code[1] );
	code += 2;
}

//if (greaterv(vA,vB)) { .....
//if (vA > vB) { .....
//	Returns true if vA is greater than vB.
void cGreaterv()
{
	IF_RESULT = ( vars[ code[0] ] > vars[ code[1] ] );
	code += 2;
}

//if (isset(fA)) { .....
//if (fA) { .....
//	Returns true if flag fA is set.
void cIsset()
{
	IF_RESULT = TestFlag( code[0] );
	code++;
}

//if (isset(vA)) { .....
//	Returns true if the flag determined by the value of vA is set.
void cIssetv()
{
	IF_RESULT = TestFlag( vars[ code[0] ] );
	code++;
}

//if (has(iA)) { .....
//	Returns true if the inventory item iA is in the player’s inventory (i.e. the
//	item’s room number is 255).
void cHas()
{
	IF_RESULT = (invObjRooms[*code++]==0xFF);
}

//if (obj.in.room(iA,vB)) { .....
//	Returns true if the room number if inventory item iA is equal to vB.
void cObjInRoom()
{
	IF_RESULT = (invObjRooms[code[0]]==vars[code[1]]);
	code += 2;
}

//if (posn(oA,X1,Y1,X2,Y2)) { .....
//	Returns true if the co-ordinates of the bottom-left pixel of object oA are
//	within the region (X1,Y1,X2,Y2).
void cPosn()
{
	VOBJ *v = &ViewObjs[code[0]];
	IF_RESULT = TEST_Y(v->x, v->x, v->y);
	code += 5;
}

//if (controller(cA)) { .....
//	Returns true if the menu item or key assigned to controller cA has been
//	selected or pressed during the current cycle.
void cController()
{
	IF_RESULT = controllers[code[0]];
	code++;
}

//if (have.key()) { .....
//	Returns true if the user has pressed a key.
void cHaveKey()
{
	int k;

	if(!(k=vars[vKEYPRESSED]))
		while((k = PollKey()) == -1);

	if(k) {
		vars[vKEYPRESSED] = k;
		IF_RESULT = TRUE;
	} else
		IF_RESULT = FALSE;
}

//if (said(“word1”,”word2”....”wordn”)) { .....
//	The said test command is different to all other commands in the language in
//	that it accepts a special type of parameter and can have any number of
//	parameters. It is used to test if the player has entered certain words.
//
//	First, the process of parsing player input needs to be explained. When the
//	player enters a command, the interpreter does the following things with it:
//	·	Removes certain punctuation characters
//	·	Assigns each word entered a number starting from 1. When doing this, it
//		tries to find the longest sequence of characters that match a word in
//		the WORDS.TOK file (so for example if the words “door”, “knob” and
//		“door knob” were in the WORDS.TOK file and the player entered “turn door
//		knob” then the words would be “turn” and “door knob” instead of “turn”,
//		“door” and “knob”). Words in group 0 (usually things like “a”, “my”,
//		“the”) are skipped (not assigned numbers).
//	·	If one or more words that are not in the WORDS.TOK file are found, it
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
//	Word group 1: “anyword” - if this word is given as a parameter, then any
//	word will do. So if you test for said(“eat”,”anyword”) then the result will
//	be true if the player enters “eat cake”, “eat chocolate”, “eat worm”, “eat
//	sword”, etc.
//
//	Word group 9999: “rol” (rest of line) - this means the rest of the line. If
//	you test for said(“kill”,”rol”) then the result will be true if the player
//	enters “kill lion”, “kill lion with sword”, etc.
void cSaid()
{
	int i,curWord,wordCnt=*code++,m=inpos;

	if(inpos&&(!TestFlag(fSAIDOK))&&TestFlag(fPLAYERCOMMAND)) {
		i=0;
		while(wordCnt) {
			curWord = bGetW(code);
			code+=2;
			wordCnt--;
			if(curWord==9999) {
				code += wordCnt<<1;
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
		code += wordCnt << 1;
		IF_RESULT = FALSE;
	}
}

//if (compare.strings(s1,s2)) { .....
//	Compares strings s1 and s2 and returns true if they are the same. The
//	comparison is not case-sensitive, and some characters such as space and
//	exclamation marks are ignored.
void cCompareStrings()
{
	IF_RESULT = (strcmpi(strings[code[0]],strings[code[1]])==0);
	code += 2;
}

//if (obj.in.box(oA,X1,Y1,X2,Y2)) { .....
//	Returns true if all of the bottom row of pixels of object oA are within the
//	region (X1,Y1,X2,Y2).
void cObjInBox()
{
	VOBJ *v = &ViewObjs[code[0]];
	IF_RESULT = TEST_Y(v->x,  v->x+(v->width>>1), v->y);
	code += 5;
}

//if (center.posn(oA,X1,Y1,X2,Y2)) { .....
//	Returns true if the co-ordinates of the bottom-middle pixel of object oA are
//	within the region (X1,Y1,X2,Y2).
void cCenterPosn()
{
	VOBJ *v = &ViewObjs[code[0]];
	IF_RESULT = TEST_Y(v->x+(v->width>>1),  v->x+(v->width>>1), v->y);
	code += 5;
}

//if (right.posn(oA,X1,Y1,X2,Y2)) { .....
//	Returns true if the co-ordinates of the bottom-right pixel of object oA are
//	within the region (X1,Y1,X2,Y2).
void cRightPosn()
{
	VOBJ *v = &ViewObjs[code[0]];
	IF_RESULT = TEST_Y(v->x+v->width-1,  v->x+v->width-1, v->y);
	code += 5;
}

#endif