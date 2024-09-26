#ifndef __AGI_LOGIC_C__
#define __AGI_LOGIC_C__

/*
 *  AGI Logic
* Based on GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 * For details: https://github.com/Davidebyzero/GBAGI.git
*/

//#include "gbagi.h"
//#include "logic.h"
//#include "commands.h"
//#include "gamedata.h"
//#include "errmsg.h"
//#include "system.h"
//#include "views.h"
//#include "status.h"
//#include "text.h"
//#include "input.h"
//#include "screen.h"
//#include "picture.h"
#include "agi.h"
#include "agi_logic.h"
#include "agi_commands.h"

#include "agi_view.h"
#include "agi_pic.h"


LOGIC *curLog,*log0;
BOOL IF_RESULT;

#ifdef _WINDOWS
#define _PRINT_LOG
#endif

//U8 *code;	//@TODO: We need to re-direct this to agi_res_read()
word code_ofs;
U16 logScan[256];	// Stores offset inside given logic


// Helpers:
U8 code_get() {
	return vagi_res_read(curLog->res_h);
}
U16 code_get_word() {
	return vagi_res_read_word(curLog->res_h);
}
void code_skip(U8 n) {
	vagi_res_skip(curLog->res_h, n);
}
void code_term() {
	// We need to signal that this logic is done (original: "code == NULL")
	vagi_res_close(curLog->res_h);
}



char *GetMessage(LOGIC *log, int num) {                                             
	//return (char*)( log->messages + bGetW(log->messages+(num<<1)) );
	vagi_res_seek_to(log->res_h, log->ofs_messages + (num << 1));
	word o = vagi_res_read_word(log->res_h);
	vagi_res_seek_to(log->res_h, log->ofs_messages + o);
	o = vagi_res_read_word(log->res_h);
	return (char*)o;
}


void InitLogicSystem() {
	curLog 			= NULL;
	log0			= NULL;
	memset(logScan, 0, sizeof(logScan));
}


//U8 *CallLogic(U8 num) {
word CallLogic(U8 num) {
	LOGIC *prevLog,log;
	
	//U8 *c=code,*c2;
	word c = code_ofs;
	word c2;
	
	#ifdef _FULL_BLIT_RESFRESHES
		EraseBlitLists();
		DrawBlitLists();
	#endif
	
	// load the logic
	log.num			= num;
	//U8 *pLog = (U8*)logDir[num];
	//log.code		= pLog+7;
	//log.messages	= log.code+bGetW(pLog+5);	// Note: "pLog+5" = first byte in actual resource data (First 5 bytes: 0x12 0x34, vol, size_lo, size_hi)
	//log.msgTotal	= *log.messages++;
	log.res_h = vagi_res_open(AGI_RES_KIND_LOG, num);
	if (log.res_h < 0) {
		printf("LOG open ERR\n");
		return 0;
	}
	word code_size = vagi_res_read_word(log.res_h);
	log.ofs_code = vagi_res_tell(log.res_h);
	log.ofs_messages = log.ofs_code + code_size;
	
	// set the active pointer
	prevLog			= curLog;
	curLog			= &log;
	if(!num)
		log0 = curLog;
	
	c2 = ExecuteLogic(curLog);
	
	#ifdef _FULL_BLIT_RESFRESHES
		EraseBlitLists();
		DrawBlitLists();
	#endif
	
	curLog = prevLog;
	//code = c;
	code_ofs = c;
	
	// release the pointer
	return (c2);
}


#ifdef _PRINT_LOG
char zs[1000];
int cmdnum;
#endif
//U8 *ExecuteLogic(LOGIC *log) {
word ExecuteLogic(LOGIC *log) {
	register unsigned int op;
#ifdef _PRINT_LOG
	int i,l;
	cmdnum = 0;
#endif
	//code = log->code+logScan[log->num];
	code_ofs = log->ofs_code + logScan[log->num];
	vagi_res_seek_to(log->res_h, code_ofs);
	
	//while(code && (BOOL)(op = *code++)) {
	while((!vagi_res_eof(log->res_h)) && (BOOL)(op = vagi_res_read(log->res_h))) {
#ifdef _WINDOWS
	if(sndFlag!=-1) {
		SetFlag(sndFlag);
		sndFlag=-1;
	}                         
	if((vars[vSECONDS]%30)==0)
				SystemUpdate();
	if(++vars[vSECONDS]>=60) {
		vars[vSECONDS]=0;
		if(++vars[vMINUTES]>=60) {
			vars[vMINUTES]=0;
			if(++vars[vHOURS]>=60) {
				vars[vHOURS]=0;
				vars[vDAYS]++;
			}
		}
	}
	if(sndBuf)
		StopSound();
#endif
#ifdef _PRINT_LOG
		cmdnum++;
		sprintf(zs,"$%03d:%04X:\t%s ",log->num,(code-log->code)-1,agiCommands[op].name);
		for(i=0;i<agiCommands[op].nParams;i++) {
			l=strlen(zs);
			if(i){
				sprintf(zs+l,",");
				l++;}
			if((agiCommands[op].pMask<<i)&0x80) {
				sprintf(zs+l,"x%d(%d) ",code[i],vars[code[i]]);
			} else
				sprintf(zs+l,"%d",code[i]);
		}
		l=strlen(zs);
		sprintf(zs+l,"");
		fprintf(flog,"%s\n",zs);
#endif
		agiCommands[op].func();
	}
	
	//return code;
	return code_ofs;
}


void ExecuteGoto() {
	//code += (S16)(bGetW(code)+2);
	code_ofs = vagi_res_tell(curLog->res_h) + (S16)(vagi_res_read_word(curLog->res_h));
	vagi_res_seek_to(curLog->res_h, code_ofs);
}


void ExecuteIF() {
	BOOL IS_NOT = FALSE;
	register unsigned int orCnt=0;
	register unsigned int op;
#ifdef _PRINT_LOG
	int i,l;
#endif
	
	for(;;) {
		//if((op = *code++) >= 0xFC) {
		if((op = code_get()) >= 0xFC) {
			if(op == 0xFC) {
				if(orCnt) {
					orCnt = 0;
					SkipANDFalse();
					return;
				}
				orCnt++;
			} else if(op == 0xFF) {
				//code+=2;
				vagi_res_skip(curLog->res_h, 2);
				return;
			}
			if(op == 0xFD) {
				IS_NOT = !IS_NOT;
#ifdef _PRINT_LOG
		fprintf(flog,"\t\tNOT\n");
#endif
			}
		} else {
#ifdef _PRINT_LOG
		sprintf(zs,"$%03d:%04X:\t\t%s ",curLog->num,(code-curLog->code)-1,testCommands[op].name);
		for(i=0;i<testCommands[op].nParams;i++) {
			l=strlen(zs);
			if(i){
				sprintf(zs+l,",");
				l++;}
			if((testCommands[op].pMask<<i)&0x80) {
				sprintf(zs+l,"x%d(%d) ",code[i],vars[code[i]]);
			} else
				sprintf(zs+l,"%d ",code[i]);
		}
		l=strlen(zs);
#endif
			testCommands[op].func();
			if(IS_NOT)
				IF_RESULT	= !IF_RESULT;
			IS_NOT		= FALSE;
#ifdef _PRINT_LOG
		fprintf(flog,"%s\n\t\t\t=%s\n",zs,IF_RESULT?"TRUE":"FALSE");
#endif
			
			if(IF_RESULT) {
				if(orCnt) {
					orCnt = 0;
					SkipORTrue();
				}
			} else if(!orCnt) {
				SkipANDFalse();
				return;
			}
		}
	}
}


void SkipORTrue() {
	register unsigned int op;
	//while((op = *code++) != 0xFC)
	while((op = code_get()) != 0xFC) {
		if(op <= 0xFC) {
			//code += (op == 0xE)?(*code << 1) + 1:testCommands[op].nParams;
			if (op == 0xe)	vagi_res_skip(curLog->res_h, code_get() << 1);
			else			vagi_res_skip(curLog->res_h, testCommands[op].nParams);
		}
	}
}


void SkipANDFalse() {
	register unsigned int op;
	//while((op = *code++) != 0xFF)
	while((op = code_get()) != 0xFF) {
		if(op < 0xFC) {
			//code += (op == 0xE)?(*code << 1) + 1:testCommands[op].nParams;
			if (op == 0xe)	vagi_res_skip(curLog->res_h, code_get() << 1);
			else			vagi_res_skip(curLog->res_h, testCommands[op].nParams);
		}
	}
	//code += (S16)(bGetW(code)+2);	// address
	code_ofs = vagi_res_tell(curLog->res_h) + (S16)(vagi_res_read_word(curLog->res_h));
	vagi_res_seek_to(curLog->res_h, code_ofs);
}


U8 *NewRoom(U8 num) {
	VOBJ *vObj;
	int i;
	
	//for (vObj=ViewObjs; vObj<&ViewObjs[MAX_VOBJ]; vObj++) {
	for (i = 0; i < MAX_VOBJ; i++) {
		vObj = &ViewObjs[i];
		vObj->flags			&= ~(oANIMATE|oDRAWN);
		vObj->flags			|= oUPDATE;
		vObj->pCel			= NULL;
		vObj->pView			= NULL;
		vObj->blit			= NULL;
		vObj->stepTime		= 1;
		vObj->stepCount		= 1;
		vObj->cycleCount	= 1;
		vObj->cycleTime		= 1;
		vObj->stepSize		= 1;
	}
	
	//@TODO: Implement
	/*
	
	StopSound();
	ClearControllers();
	
	pPView		= pViews;
	pOverlay	= overlays;
	*/
	
	PLAYER_CONTROL	= TRUE;
	VOBJ_BLOCKING	= FALSE;
	horizon			= HORIZON_DEFAULT;
	
	vars[vROOMPREV]		= vars[vROOMNUM];
	vars[vROOMNUM]		= num;
	vars[vOBJECT]		=
	vars[vOBJBORDER]	= 0;
	vars[vMEMORY]		= 10;
	vars[vEGOVIEWNUM]	= ViewObjs[0].view;
	
	switch(vars[vEGOBORDER]) {
		case bdTOP: 	// coming from the top, go to the bottom
			ViewObjs[0].y = PIC_MAXY;
			break;
		case bdRIGHT:	// coming from the right, go to the left
			ViewObjs[0].x = 0;
			break;
		case bdBOTTOM:	// coming from the bottom, go up to the top
			ViewObjs[0].y = horizon;
			break;
		case bdLEFT:	// coming from the left, go to the right
			ViewObjs[0].x = PIC_WIDTH - ViewObjs[0].width;
	}
	vars[vEGOBORDER] = bdNONE;
	
	SetFlag(fNEWROOM);
	
	ClearControllers();
	//@TODO: Implement
	/*
	WriteStatusLine();
	*/
	
	return NULL;
}


#endif