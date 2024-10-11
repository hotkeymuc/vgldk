#ifndef __AGI_LOGIC_H__
#define __AGI_LOGIC_H__

/*
 *  AGI Logic
* Based on GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 * For details: https://github.com/Davidebyzero/GBAGI.git
*/

#include "agi.h"



// logic.h:
typedef struct {
	//U8 *code;
	//U8 *messages;
	U8 num;
	U8 msgTotal;
	
	// We stay inside a vagi_res_handle
	vagi_res_handle_t res_h;	// Used for logic itself
	vagi_res_handle_t msg_res_h;	// Used for GetMessage (seeks around)
	word ofs_code;
	word ofs_messages;
} LOGIC;

extern LOGIC *curLog;
extern LOGIC *log0;
extern BOOL IF_RESULT;
extern U16 logScan[256];

extern BOOL new_room_called;	// VAGI
extern BOOL trace_ops;	// VAGI

// Helpers:
U8 code_get(void);
U8 code_peek(void);
U16 code_get_word();
void code_skip(U16 n);
void code_term(void);
//#define code_get() vagi_res_read(curLog->res_h)
//#define code_peek() vagi_res_peek(curLog->res_h)
//#define code_get_word() vagi_res_read_word(curLog->res_h)
//#define code_skip(n) vagi_res_skip(curLog->res_h, n)
//#define code_term() { vagi_res_close(curLog->res_h); vagi_res_close(curLog->msg_res_h); }


char *GetMessage(LOGIC *log, int num);
void InitLogicSystem(void);
//U8 *CallLogic(U8 num);
word CallLogic(U8 num);
//U8 *ExecuteLogic(LOGIC *log);
word ExecuteLogic(LOGIC *log);

U8 *NewRoom(U8 num);

//extern U8 *code;


#endif