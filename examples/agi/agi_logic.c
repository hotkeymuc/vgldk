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


LOGIC *curLog;
LOGIC *log0;	// Only used for FormatString "%g..." to print message from logic0

BOOL IF_RESULT;
BOOL new_room_called;	// vagi
BOOL trace_ops;	// vagi

const char AGI_MESSAGES_KEY[] = "Avis Durgan";

#define AGI_MAX_MESSAGE_LENGTH 255	//MAX_STRINGS_LEN	//40	agi_vars.h:MAX_STRINGS_LEN
char agi_message_buf[AGI_MAX_MESSAGE_LENGTH];	// Buffer for GetMessage()



#ifdef _WINDOWS
#define _PRINT_LOG
#endif

//U8 *code;	//@TODO: We need to re-direct this to agi_res_read()
//word code_ofs;
U16 logScan[256];	// Stores offset inside given logic


// Helpers:

U8 inline code_get() {
	return vagi_res_read(curLog->res_h);
}
U8 inline code_peek() {
	return vagi_res_peek(curLog->res_h);
}
U16 inline code_get_word() {
	return vagi_res_read_word(curLog->res_h);
}
void inline code_skip(U16 n) {
	vagi_res_skip(curLog->res_h, n);
}
void inline code_seek_to(U16 o) {
	vagi_res_seek_to(curLog->res_h, o);
}
void inline code_seek_relative(S16 s) {
	word code_ofs = vagi_res_tell(curLog->res_h) + s;
	vagi_res_seek_to(curLog->res_h, code_ofs);
}

void code_term() {
	// We need to signal that this logic is done (original: "code == NULL")
	vagi_res_close(curLog->res_h);
	vagi_res_close(curLog->msg_res_h);
}



char *GetMessage(LOGIC *log, int num) {
	//return (char*)( log->messages + bGetW(log->messages+(num<<1)) );
	//printf("GetMessage("); printf_d(num); printf("/"); printf_d(log->msgTotal); printf(")...");
	
	// Seek inside string offset table
	
	// ofs_messages points to the message section:
	// u8       message count
	// u16      messages size (2 + offsets + strings)
	// u16[]    string offsets (relative to message section + 1)
	// string[] strings (null terminated, possibly encrypted)
	
	//vagi_res_seek_to(log->msg_res_h, log->ofs_messages + 1 + 2 + (num << 1));
	vagi_res_seek_to(log->msg_res_h, log->ofs_messages + 1 + 0 + (num << 1));
	word o = vagi_res_read_word(log->msg_res_h);	// Read offset of desired message
	//printf("o="); printf_x2(o >> 8); printf_x2(o & 0xff); getchar();
	
	// Decrypt the message block: XOR with "Avis Durgan"
	
	// Seek to (encrypted) message
	vagi_res_seek_to(log->msg_res_h, log->ofs_messages + 1 + o);
	//printf("Encrypted:\n"); dump((word)(log->ofs_messages + 1 + o), 16);
	
	byte k = (o - (2 + (log->msgTotal << 1))) % 11;	// Key offset (starts at first message data).
	char *p = &agi_message_buf[0];	// Destination pointer
	byte l = 0;	// Length counter
	
	while(l < AGI_MAX_MESSAGE_LENGTH) {
		byte b = vagi_res_read(log->msg_res_h);	// Get raw byte
		b ^= AGI_MESSAGES_KEY[k];	// XOR with key
		if (b == 0) break;	// Terminate on NULL
		k = ((k+1) % 11);	// Cycle key
		*p++ = (char)b;	// Store output
		l ++;	// Increase length
	}
	*p = 0; // Make sure it is terminated
	
	//printf("Result:\n"); dump((word)&agi_message_buf[0], l); getchar();
	//printf("GetMessage("); printf_d(num); printf("/"); printf_d(log->msgTotal); printf(") = \""); printf(agi_message_buf); printf("\"\n");
	
	//@TODO: Copy somewhere safe and return?
	//o = vagi_res_read_word(log->res_h);
	//return (char*)o;
	//return (char *)(vagi_res_point(log->msg_res_h));
	return &agi_message_buf[0];
}
/*
char *GetMessage(LOGIC *log, int num) {
	//DEBUG: Decode all messages:
	for(int i = 0; i < log->msgTotal; i++) {
		printf("msg["); printf_d(i); printf("/"); printf_d(log->msgTotal); printf("]: \""); printf(_GetMessage(log, i)); printf("\"\n");
	}
	return _GetMessage(log, num);
}
*/


void InitLogicSystem() {
	curLog 			= NULL;
	log0			= NULL;
	
	trace_ops = false;
	memset(logScan, 0, sizeof(logScan));
}


//U8 *CallLogic(U8 num) {
word CallLogic(U8 num) {
	LOGIC *prevLog;
	LOGIC log;
	
	//U8 *c=code,*c2;
	//word c = code_ofs;
	//word c = vagi_res_tell(curLog->res_h);
	word c2;
	
	#ifdef AGI_LOGIC_DEBUG
		//printf("Logic "); printf_d(num); printf("...\n");
		printf("CallLogic("); printf_d(num); printf(") {\n");
		//getchar();
	#endif
	
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
	
	// Open handle for reading code
	log.res_h = vagi_res_open(AGI_RES_KIND_LOG, num);
	if (log.res_h < 0) {
		//printf("LOG open ERR\n");
		printf("LOG err=-"); printf_d(-log.res_h); putchar('\n');
		return 0;
	}
	word code_size = vagi_res_read_word(log.res_h);
	log.ofs_code = 2;	//vagi_res_tell(log.res_h);
	
	// Open another handle for reading messages
	log.msg_res_h = vagi_res_open(AGI_RES_KIND_LOG, num);	// Exclusively for reading messages (skip around)
	log.ofs_messages = log.ofs_code + code_size;	// Put ofs_messages right at the message section header
	vagi_res_seek_to(log.msg_res_h, log.ofs_messages);
	log.msgTotal = vagi_res_read(log.msg_res_h);
	//log.ofs_messages++;
	
	// set the active pointer
	prevLog			= curLog;
	curLog			= &log;
	if (num == 0) log0 = curLog;
	
	c2 = ExecuteLogic(curLog);
	
	#ifdef _FULL_BLIT_RESFRESHES
		EraseBlitLists();
		DrawBlitLists();
	#endif
	
	// Free the logic resources
	//code_term();
	vagi_res_close(log.res_h);
	vagi_res_close(log.msg_res_h);
	
	curLog = prevLog;
	//code = c;
	//code_ofs = c;
	
	#ifdef AGI_LOGIC_DEBUG
		//printf("end-of-logic("); printf_d(num); printf(")\n");
		//getchar();
		printf("} EOC\n");
	#endif
	
	// release the pointer
	return (c2);
}


#ifdef _PRINT_LOG
char zs[1000];
int cmdnum;
#endif
//U8 *ExecuteLogic(LOGIC *log) {
word ExecuteLogic(LOGIC *log) {
	//register unsigned int op;
	//unsigned int op;
	U8 op;
#ifdef _PRINT_LOG
	int i,l;
	cmdnum = 0;
#endif
	#ifdef AGI_LOGIC_DEBUG
		//printf("ExecLogic("); printf_d(log->num); printf(") <\n");
	#endif
	//code = log->code+logScan[log->num];
	//code_ofs = log->ofs_code + logScan[log->num];
	vagi_res_seek_to(log->res_h, log->ofs_code + logScan[log->num]);
	
	//while(code && (BOOL)(op = *code++)) {
	//while((!vagi_res_eof(log->res_h)) && (BOOL)(op = vagi_res_read(log->res_h))) {
	while(!vagi_res_eof(log->res_h)) {
		op = vagi_res_read(log->res_h);
		#ifdef AGI_LOGIC_DEBUG_OPS
		if (trace_ops) {
			// Show logic num
			printf_x2(log->num); putchar(':');
			
			// Show Instruction pointer
			word cp = vagi_res_tell(log->res_h);
			printf_x2(cp >> 8); printf_x2(cp & 0x0f);
			putchar(':');
			
			// Show OP
			printf("op=");
			#ifdef AGI_COMMANDS_INCLUDE_NAMES
				printf(agiCommands[op].name);
			#else
				printf_d(op);
			#endif
			//for(i=0;i<agiCommands[op].nParams;i++) { code_peek()....
			//printf("...");
			//getchar();
		}
		#endif
		
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
		
		#ifdef AGI_LOGIC_DEBUG_IFS
			//if (IF_RESULT) printf("TRUE\n"); else printf("FALSE\n");
		#endif
		#ifdef AGI_LOGIC_DEBUG_OPS
		if (trace_ops) {
			printf(".\n");
		}
		#endif
		if (op == 0) return 1;	// op #0 = "return"
		if (new_room_called) return 0;
	}
	
	#ifdef AGI_LOGIC_DEBUG
		//printf(">EOX\n");
		if (log->num == 2) getchar();	// Debug SQ2 "broom"
	#endif
	
	//return code;
	//return code_ofs;
	return 0;
	//return vagi_res_eof(log->res_h);	// Main function loops until we return TRUE
}


void ExecuteGoto() {
	//code += (S16)(bGetW(code)+2);
	//code_ofs = vagi_res_tell(curLog->res_h) + (S16)(vagi_res_read_word(curLog->res_h));
	//vagi_res_seek_to(curLog->res_h, code_ofs);
	
	//word o = vagi_res_tell(curLog->res_h) + (S16)(vagi_res_read_word(curLog->res_h));
	//code_seek_to(o);
	code_seek_relative((S16)(code_get_word()));
}


void ExecuteIF() {
	BOOL IS_NOT = FALSE;
	//register unsigned int orCnt=0;
	//register unsigned int op;
	unsigned int orCnt=0;
	//unsigned int op;
	U8 op;
#ifdef _PRINT_LOG
	int i,l;
#endif
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("IF...");
	#endif
	for(;;) {
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
				code_skip(2);
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
			
			// Actually execute operation
			testCommands[op].func();
			
			#ifdef AGI_LOGIC_DEBUG_IFS
				printf("(res="); printf_d(IF_RESULT); printf(")");
			#endif
			
			if (IS_NOT) {
				#ifdef AGI_LOGIC_DEBUG_IFS
					printf("NOT");
				#endif
				IF_RESULT	= !IF_RESULT;
				IS_NOT		= FALSE;
			}
#ifdef _PRINT_LOG
		fprintf(flog,"%s\n\t\t\t=%s\n",zs,IF_RESULT?"TRUE":"FALSE");
#endif
			
			if (IF_RESULT) {
				#ifdef AGI_LOGIC_DEBUG_IFS
					printf("(HIT)");
				#endif
				if (orCnt) {
					#ifdef AGI_LOGIC_DEBUG_IFS
						printf("(skip or)");
					#endif
					orCnt = 0;
					SkipORTrue();
				}
			} else {
				#ifdef AGI_LOGIC_DEBUG_IFS
					printf("(not hit)");
				#endif
				if(!orCnt) {
					SkipANDFalse();
					return;
				}
			}
		}
	}
}


void SkipORTrue() {
	//register unsigned int op;
	//unsigned int op;
	U8 op;
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("(skipORtrue)");
	#endif
	
	while((op = code_get()) != 0xFC) {
		if(op <= 0xFC) {
			//code += (op == 0xE)?(*code << 1) + 1:testCommands[op].nParams;
			if (op == 0xe)	code_skip((code_peek() << 1) + 1);
			else			code_skip(testCommands[op].nParams);
		}
	}
}


void SkipANDFalse() {
	//register unsigned int op;
	//unsigned int op;
	U8 op;
	#ifdef AGI_LOGIC_DEBUG_IFS
		printf("(skipANDfalse)");
	#endif
	while((op = code_get()) != 0xFF) {
		if(op < 0xFC) {
			//code += (op == 0xE)?(*code << 1) + 1:testCommands[op].nParams;
			if (op == 0xe)	code_skip((code_peek() << 1) + 1);
			else			code_skip(testCommands[op].nParams);
		}
	}
	//code += (S16)(bGetW(code)+2);	// Note: bGetW() does not increment code, so "+2" skips ofer the word just read.
	//code_skip(code_get_word());
	
	//word o = vagi_res_tell(curLog->res_h) + (S16)(vagi_res_read_word(curLog->res_h));
	//code_seek_to(o);
	code_seek_relative(code_get_word());
}


U8 *NewRoom(U8 num) {
	VOBJ *vObj;
	int i;
	
	#ifdef AGI_LOGIC_DEBUG
		printf("NewRoom(");
		printf_d(num);
		printf(")...");
		getchar();
	#endif
	
	//for (vObj=ViewObjs; vObj<&ViewObjs[MAX_VOBJ]; vObj++) {
	for (i = 0; i < MAX_VOBJ; i++) {
		vObj = &ViewObjs[i];
		vObj->flags			&= ~(oANIMATE|oDRAWN);
		vObj->flags			|= oUPDATE;
		//vObj->pCel			= NULL;
		vObj->oCel			= 0;
		//vObj->pView			= NULL;
		vObj->viewLoaded	= false;
		//vObj->blit			= NULL;
		vObj->stepTime		= 1;
		vObj->stepCount		= 1;
		vObj->cycleCount	= 1;
		vObj->cycleTime		= 1;
		vObj->stepSize		= 1;
	}
	
	//@TODO: Implement
	//StopSound();
	ClearControllers();
	/*
	pPView		= pViews;
	pOverlay	= overlays;
	*/
	
	PLAYER_CONTROL	= TRUE;
	VOBJ_BLOCKING	= FALSE;
	horizon			= HORIZON_DEFAULT;
	
	vars[vROOMPREV]		= vars[vROOMNUM];
	vars[vROOMNUM]		= num;
	vars[vOBJECT]		= 0;	//?
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
			break;
	}
	vars[vEGOBORDER] = bdNONE;
	
	SetFlag(fNEWROOM);
	
	ClearControllers();
	
	//htk:
	new_room_called = true;
	//vagi_draw_pic(num);	//@FIXME: This should be triggered through LOGIC code!!!
	//WriteStatusLine();
	
	return NULL;
}


#endif