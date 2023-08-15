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
	
	//@TODO: Use Opcode for faster copy!!!
	ps = src_addr;
	pd = dst_addr;
	for (i = 0; i < count; i++) {
		*pd++ = *ps++;
	}
}

void memset(byte *addr, byte b, word count) {
	while(count > 0) {
		*addr++ = b;
		count--;
	}
}


#endif	// __STRINGMIN_H