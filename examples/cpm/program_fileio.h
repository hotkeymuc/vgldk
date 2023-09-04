#ifndef __PROGRAM_FILEIO_H
#define __PROGRAM_FILEIO_H
/*

Include this to create a simple CP/M program.
It contains wrappers for BDOS function calls.

*/

#include "program.h"	// for byte/word

#include "fcb.h"	// for FCB struct

struct FCB __at(0x005c) def_fcb;	// Default FCB at 0x005c

byte fopen(char *filename);
byte fclose();
byte fread();
byte freadrand(byte r0, byte r1, byte r2);


volatile byte ret_a;

// BDOS File I/O
byte fopen(char *filename) {
	byte i;
	char c;
	char *pc;
	
	//@FIXME: Extract drive from filename!
	//def_fcb.dr = bios_curdsk;	// Default drive (setting it to bios_curdsk will throw "invalid drive" in yaze)
	def_fcb.dr = 0;	// this works in YAZE
	
	// Convert filename
	pc = filename;
	for(i = 0; i < 8; i++) {
		c = *pc;
		if (c == '.') break;
		if (c == 0) break;
		def_fcb.f[i] = c;
		pc++;
	}
	while (i < 8) def_fcb.f[i++] = 0x20;
	pc++;
	
	for(i = 0; i < 3; i++) {
		c = *pc;
		if (c == 0) break;
		def_fcb.t[i] = c;
		pc++;
	}
	while (i < 3) def_fcb.t[i++] = 0x20;
	
	// Prepare FCB
	def_fcb.ex = 0;
	def_fcb.s1 = 0;
	def_fcb.s2 = 0;
	
	def_fcb.cr = 0;
	def_fcb.r0 = 0;
	def_fcb.r1 = 0;
	def_fcb.r2 = 0;
	
	//printf("Opening...\r\n");
	//dump((word)(&def_fcb), 36);
	
	__asm
		;push af
		push bc
		push de
		;push hl
		
		ld c, #15	; 15 = BDOS_FUNC_F_OPEN
		ld d, #>_def_fcb	; Address of FCB (high)
		ld e, #<_def_fcb	; Address of FCB (low)
		call 5
		
		// Result FCB should now (also?) be at DMA + A*32
		
		ld (_ret_a), a
		
		;pop hl
		pop de
		pop bc
		;pop af
	__endasm;
	
	return ret_a;
}


byte fclose() {
	__asm
		;push af
		push bc
		push de
		;push hl
		ld c, #16	; BDOS_FUNC_F_CLOSE = 16
		ld d, #>_def_fcb	; Address of FCB (high)
		ld e, #<_def_fcb	; Address of FCB (low)
		call 5
		
		ld (_ret_a), a
		
		;pop hl
		pop de
		pop bc
		;pop af
	__endasm;
	
	return ret_a;
}


byte fread() {	// Read next record (from def_fcb to ccp_dma/bios_dma)
	__asm
		;push af
		push bc
		push de
		;push hl
		
		ld c, #20	; 20 = BDOS_FUNC_F_READ
		ld d, #>_def_fcb	; Address of FCB (high)
		ld e, #<_def_fcb	; Address of FCB (low)
		call 5
		
		// 0 = OK, 1 = EOF, 9 = invalid FCB, 10 = media changed/checksum error, 11 = unlocked/verification error, 0xff = hardware error
		ld (_ret_a), a
		
		;pop hl
		pop de
		pop bc
		;pop af
	__endasm;
	
	return ret_a;
}

byte freadrand(byte r0, byte r1, byte r2) {
	def_fcb.r0 = r0;
	def_fcb.r1 = r1;
	def_fcb.r2 = r2;
	
	__asm
		;push af
		push bc
		push de
		;push hl
		
		ld c, #33	; 33 = BDOS_FUNC_F_READRAND
		ld d, #>_def_fcb	; Address of FCB (high)
		ld e, #<_def_fcb	; Address of FCB (low)
		call 5
		
		// 0 = OK, 1 = EOF, 9 = invalid FCB, 10 = media changed/checksum error, 11 = unlocked/verification error, 0xff = hardware error
		ld (_ret_a), a
		
		;pop hl
		pop de
		pop bc
		;pop af
	__endasm;
	
	return ret_a;
}

#endif	// __PROGRAM_FILEIO_H