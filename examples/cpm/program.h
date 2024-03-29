#ifndef __PROGRAM_H
#define __PROGRAM_H
/*

Include this to create a simple CP/M program.
It contains wrappers for BDOS function calls.

*/


//#include <basictypes.h>
#ifndef __BASICTYPES_H__
typedef unsigned char byte;
typedef unsigned short word;
#endif


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