#ifndef __PORTS_H
#define __PORTS_H

byte port_in(byte p) __naked { (void)p;
	
	__asm
		push af
		push bc
		
		; Get parameter "p" from stack into C
		;ld hl,#0x0002
		ld hl,#0x0006
		add hl,sp
		ld c,(hl)
		
		in	a, (c)
		ld	l, a
		
		pop bc
		pop af
		ret
	__endasm;
	return 0;
}

void port_out(byte p, byte v) __naked {
	(void)p;	// suppress warning "unreferenced function argument"
	(void)v;	// suppress warning "unreferenced function argument"
	__asm
		push af
		push bc
		push hl
		
		; Get parameter "v" from stack into A
		ld hl,#0x0009
		add hl,sp
		ld a,(hl)
		
		; Get parameter "p" from stack into C
		ld hl,#0x0008
		add hl,sp
		ld c,(hl)
		
		out	(c), a
		
		pop hl
		pop bc
		pop af
		ret
	__endasm;
}


#endif	//__PORTS_H