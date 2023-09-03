#ifndef __CCP_H
#define __CCP_H

/*
	CCP - Console Command Processor
	===============================
	
	This is the "shell" of CP/M.
	It is loaded as a standard program into the transient area.
	
*/

//#include <basictypes.h>	// byte, word, true, false, NULL, ...
#include "fcb.h"

//#define CCP_SHOW_BANNER	// Show "CCP" on startup
//#define CCP_MAX_INPUT 128	//32
#define CCP_MAX_INPUT PROGRAM_GETS_MAX_SIZE

//#define CCP_CMD_PORT	// Port access

#ifdef BDOS_TRAP
	#define CCP_CMD_TRAP	// Enable trap functions, e.g. bdos.c:trap_set()
#endif

#define CCP_CMD_DUMP	// Memory dumping

//#define CCP_USE_HOST	// Include loading from host

// Include a test program
//#define CCP_INCLUDE_TEST_PROGRAM	// Merge some data into code space for testing?

//#define CCP_TEST_PROGRAM_FILENAME "data_sysgen.h"
//#define CCP_TEST_PROGRAM_DATANAME DATA_SYSGEN
//#define CCP_TEST_PROGRAM_SIZENAME DATA_SYSGEN_SIZE

//#define CCP_TEST_PROGRAM_FILENAME "data_zork1.h"
//#define CCP_TEST_PROGRAM_DATANAME DATA_ZORK1
//#define CCP_TEST_PROGRAM_SIZENAME DATA_ZORK1_SIZE

//#define CCP_TEST_PROGRAM_FILENAME "data_asm.h"
//#define CCP_TEST_PROGRAM_DATANAME DATA_ASM
//#define CCP_TEST_PROGRAM_DATANAME DATA_ASM_SIZE

#ifdef CCP_INCLUDE_TEST_PROGRAM
	//#include "data_asm.h"	// DATA_ASM[]
	//#include "data_sysgen.h"	// DATA_SYSGEN[]
	#include CCP_TEST_PROGRAM_FILENAME
#endif	// CCP_INCLUDE_TEST_PROGRAM


struct FCB __at(0x005c) def_fcb;	// Default FCB at 0x005c

char __at(0x0080) ccp_argl;	// Arg length at 0x0080 (including initial space)
char __at(0x0081) ccp_args[128];	// Args at 0x0081 (starting with initial space)

char __at(0x0080) ccp_dma[128];	// DMA at 0x0080

unsigned char __at(0x0003) bios_iobyte;
unsigned char __at(0x0004) bios_curdsk;

byte ccp_running;	// To be able to stop the input loop
char ccp_input[CCP_MAX_INPUT];

void main() __naked;
void handle(char *input);
void ccp();

#endif	// __CCP_H