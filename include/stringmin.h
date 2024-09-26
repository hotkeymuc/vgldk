#ifndef __STRINGMIN_H
#define __STRINGMIN_H

/*
Some bare minimum string functions
@TODO: Use optimized SDCC/Z88DK library functions!
*/

byte strlen(const char *c) {
	byte l;
	l = 0;
	while (*c++ != 0)  {
		l++;
	}
	return l;
}


void memcpy(byte *dst_addr, byte *src_addr, word count) {
	word i;
	byte *ps;
	byte *pd;
	
	//@FIXME: Handle forward overlaps: copy from back to front in those cases!
	//@TODO: Use Opcode for faster copy!!!
	ps = src_addr;
	pd = dst_addr;
	for (i = 0; i < count; i++) {
		*pd++ = *ps++;
	}
	/*
	__asm
	push bc
	push de
	push hl
	
	// Scroll one line
	ld bc, #((LCD_W * (LCD_H - LCD_SCROLL_AMOUNT)) / 8)	// Number of bytes to scroll (i.e. whole screen minus x lines)
	ld hl, #(LCD_ADDR + (LCD_W / 8) * LCD_SCROLL_AMOUNT)	//#0xE01E	// Offset of 2nd line, i.e. LCD_ADDR + bytes-per-line * x
	ld de, #LCD_ADDR	//#0xE000	// Offset of 1st line
	ldir	// Copy BC bytes from HL to DE
	
	pop hl
	pop de
	pop bc
	__endasm;
	*/
}

void memset(byte *addr, byte b, word count) {
	
	//@FIXME: Use an opcode like "LDIR"?
	while(count > 0) {
		*addr++ = b;
		count--;
	}
}


#endif	// __STRINGMIN_H