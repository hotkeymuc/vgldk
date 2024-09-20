#ifndef __AGI_LOGIC_H__
#define __AGI_LOGIC_H__

/*
 *  AGI Logic
* Based on GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 * For details: https://github.com/Davidebyzero/GBAGI.git
*/

// logic.h:
typedef struct {
	U8 *code;
	U8 *messages;
	U8 num;
	U8 msgTotal;
} LOGIC;

extern LOGIC *curLog,*log0;
extern BOOL IF_RESULT;
extern U16 logScan[256];

char *GetMessage(LOGIC *log, int num);
void InitLogicSystem(void);
U8 *CallLogic(U8 num);
U8 *ExecuteLogic(LOGIC *log);

U8 *NewRoom(U8 num);

extern U8 *code;


#endif