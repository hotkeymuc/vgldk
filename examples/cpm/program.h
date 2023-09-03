#ifndef __PROGRAM_H
#define __PROGRAM_H

/*

Include this to create a simple CP/M program.
It contains wrappers for BDOS function calls.

*/

//#define PROGRAM_GETS_LOCAL_ECHO	// Should gets() have local echo?
#define PROGRAM_GETS_MAX_SIZE 127

//#include <basictypes.h>
#ifndef __BASICTYPES_H__
typedef unsigned char byte;
typedef unsigned short word;
#endif

volatile byte ret_a;

// BDOS Essentials
void exit();


// BDOS essentials
void exit() __naked {
	__asm
		ld c, #0x00	; System reset
		ld d, #0x00	; DL=0: Do not keep resident
		call 5	; Call? I guess this is one-way...
	__endasm;
}


#endif	// __PROGRAM_H