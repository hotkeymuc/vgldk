/*

This is a test program for CP/M

*/

#include "program.h"
#include "program_stdio.h"

//void main() __naked {
void main() {
	
	puts("Hello world");
	
	//getchar();
	
	/*
	__asm
		ld sp, #0x4000
		
		ld c, #0x02	; Con output
		ld e, #0x40	; E=Character to output
		call 5
		
		ld c, #0x00	; System reset
		ld d, #0x00	; DL=0: Do not keep resident
		call 5
	__endasm;
	*/
	
	exit();
}
