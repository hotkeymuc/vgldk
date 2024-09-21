#ifndef __AGI_LOGIC_H__
#define __AGI_LOGIC_H__

/*
 *  AGI Logic
* Based on GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 * For details: https://github.com/Davidebyzero/GBAGI.git
*/

// Glue code
#define U8 byte
#define U16 word
#define S16 signed int
#define BOOL bool
#define TRUE true
#define FALSE false
#define agi_logic_get_byte() vagi_res_read(curLogic->res_h)
#define agi_logic_get_word() vagi_res_read_word(curLogic->res_h)
#define agi_logic_skip(n) vagi_res_skip(curLogic->res_hn)


// logic.h:
typedef struct {
	//U8 *code;
	//U8 *messages;
	U8 num;
	U8 msgTotal;
	
	// We stay inside a vagi_res_handle
	vagi_res_handle_t res_h;
	word ofs_code;
	word ofs_messages;
} LOGIC;

extern LOGIC *curLog,*log0;
extern BOOL IF_RESULT;
extern U16 logScan[256];

char *GetMessage(LOGIC *log, int num);
void InitLogicSystem(void);
//U8 *CallLogic(U8 num);
word CallLogic(U8 num);
//U8 *ExecuteLogic(LOGIC *log);
word ExecuteLogic(LOGIC *log);

U8 *NewRoom(U8 num);

//extern U8 *code;


#endif