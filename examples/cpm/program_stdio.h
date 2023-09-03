#ifndef __PROGRAM_STDIO_H
#define __PROGRAM_STDIO_H

/*

Include this to create a simple CP/M program.
It contains wrappers for BDOS function calls.

*/

//#define PROGRAM_GETS_LOCAL_ECHO	// Should gets() have local echo?
#define PROGRAM_GETS_MAX_SIZE 127

#include "program.h"


// BDOS STDIO
void _putchar(char b) __naked {
	(void)b;	// Silence the compiler
	__asm
		
		push hl
		push de
		
		; Get parameter from stack into E
		; Take note of how many things were pushed before this line (number x 2) and the call address (2)
		ld hl,#0x0006
		add hl,sp
		ld e,(hl)
		
		push bc
		ld c, #0x02	; 2 = BDOS_FUNC_C_WRITE = Console output
		
		call 5
		pop bc
		
		pop de
		pop hl
		
		ret
	__endasm;
	//return;
}

char _getchar() __naked {
	__asm
		;push af
		;push hl
	
		push bc
		ld c, #0x01	; 1 = BDOS_FUNC_C_READ = Console input (with echo)
		
		call 5
		pop bc
		
		; Result is in L
		;ld a, l
		;ld (_getchar_tmp), a
		
		;pop hl
		;pop af
		ret	; Naked return
	__endasm;
	//return 0xff;	// Suppress compiler warning "must return a value" if not declared "__naked"
	//return getchar_tmp;
}

/*
volatile char *gets_tmp;
//char *gets(char *pc) {
//void gets(char *pc, byte max_size) {
void gets(char *pc) {
	const byte max_size = PROGRAM_GETS_MAX_SIZE;
	byte *ps;
	byte *pd;
	byte i;
	
	// Use BDOS function 10 = BDOS_FUNC_C_READSTR =Read console buffer
	// DE = pointer to buffer, first byte contains max length
	gets_tmp = pc;
	*gets_tmp = max_size;	// first byte: max buffer size
	*(byte *)((word)gets_tmp + 1) = 0;	// second byte: bytes in buffer
	
	__asm
		push bc
		push de
		push hl
		
		ld de, (_gets_tmp)
		ld c, #10	; 10 = BDOS_FUNC_C_READSTR = Read console buffer
		call 0x0005
		
		pop hl
		pop de
		pop bc
	__endasm;
	
	byte l = *(gets_tmp+1);	// Read back the input length (second byte)
	//printf("(length="); program_printf_x2(l); printf(")");
	
	// Shift over, so that first input byte is at the beginning of buffer
	//memcpy(pc, pc+2, l);
	ps = pc+2;
	pd = pc;
	for (i = 0; i < l; i++) {
		*pd++ = *ps++;
	}
	
	// Terminate string with zero
	*(pc+l) = 0;
	
	//return pc;	// Return original pointer
}
*/


// Make compatible to stdio
int getchar(void) {
	return _getchar();
}
int putchar(int c) {
	_putchar(c);
	return 1;
}
#include <stdio.h>


#endif	// __PROGRAM_STDIO_H