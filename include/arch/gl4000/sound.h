#ifndef __VGL_SOUND_H
#define __VGL_SOUND_H
/*
V-Tech Genius Leader Sound

2019-07-10 Bernhard "HotKey" Slawik
*/

void vgl_sound_off() {
__asm
	; Speaker off
	;ld	a, #0x00	; +20h
	
	in	a, (0x12)
	and	#0xf7
	
	out	(0x12), a
	ret
__endasm;
}

void vgl_sound(word frq, word len) {
	// Perform a beep (frq is actually a delay...)
	(void)frq;	// suppress warning "unreferenced function argument"
	(void)len;	// suppress warning "unreferenced function argument"
	
	//@TODO: Use BC and djnz for the loops!
	//@FIXME: We decrement and THEN test. So there are constraints on the vars... (lower byte must be > 0)
	frq |= 0x0001;
	//len |= 0x0101;
	len |= 0x0001;
	
	__asm
		di
		
		push	af
		push	hl
		push	de
		push	bc
		
		; Get frq param
		ld	hl, #2+0
		add	hl, sp
		ld	e, (hl)
		ld	hl, #2+1
		add	hl, sp
		ld	d, (hl)	; frq is now in DE
		;inc	de		; Inc by one
		ld	b, d	; Safe DE for later in BC
		ld	c, e
		
		;len to DE
		ld	hl, #4+0
		add	hl, sp
		ld	e, (hl)
		ld	hl, #4+1
		add	hl, sp
		ld	d, (hl)	; len is now in DE
		;inc	de		; Inc by one
		ld	h, b	; Copy old frq value back from BC to HL
		ld	l, c
		
		; Actual sound loop
		_sound_loop:
			; Speaker on
			ld	a, #0x08	; +20h
			out	(0x12), a
			call _sound_delay
			
			; Speaker off
			ld	a, #0x0	; +20h
			out	(0x12), a
			call _sound_delay
		
		
		;djnz _sound_loop
		
		;dec	e
		;jr	nz, _sound_loop
		;dec	d
		;jr	nz, _sound_loop
		
		dec	de
		ld	a, d
		or	e
		jr	nz, _sound_loop
		
		jr _sound_end
		
		_sound_delay:
			push    hl
			push    af
			_sound_delay_loop:
				dec	hl
				ld	a,h
				or	l
				jr	nz, _sound_delay_loop
			pop     af
			pop     hl
		ret
		
		
	_sound_end:
		pop	bc
		pop	de
		pop	hl
		pop	af
		ei
	__endasm;
	
}


void vgl_sound_note(word n, word len) {
	word frq;
	
	switch(n % 12) {
		case 0:	frq = 0x0900;	break;
		case 1:	frq = 0x087e;	break;
		case 2:	frq = 0x0804;	break;
		case 3:	frq = 0x0791;	break;
		case 4:	frq = 0x0724;	break;
		case 5:	frq = 0x06be;	break;
		case 6:	frq = 0x065d;	break;
		case 7:	frq = 0x0601;	break;
		case 8:	frq = 0x05ab;	break;
		case 9:	frq = 0x0559;	break;
		case 10:	frq = 0x050d;	break;
		case 11:	frq = 0x04c4;	break;
	}
	
	frq = frq >> (n/12);
	len = 150 * (len / frq);	// Length to wave length, correcting for rough milliseconds
	vgl_sound(frq, len);
}

void beep() {
	vgl_sound_note(12*4+0, 0x0111);
}
#endif // __VGL_SOUND_H