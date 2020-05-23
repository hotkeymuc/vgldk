#ifndef __VGL_SOUND_H
#define __VGL_SOUND_H
/*
V-Tech Genius Leader Sound

@FIXME: Not yet implemented!


Not quite sure about the hardware, yet.
It might be a TI TMS5220 LPC speech chip.
Alexandre Botzung suggests it might be a TI TSP50C0x.


In my traces I am seeing those port accesses:
	port 0x10	(4 and 0 are written there - clock?)
	port 0x60	(checked regularly)
	port 0x62	

? There is also an interrupt (RST7 i guess) which also accesses these ports
	-> using IRQ?

Interesting memory regions that seem to be connected to sound/music:
	0xF975, 0xF976, 0xF97B, 0xF97C, 0xF97D


2020-05-19 Bernhard "HotKey" Slawik
*/

void vgl_sound_off() {
__asm
	; Speaker off, as seen in GL6000SL ROM at 0x5F9B, just before HALT
	ld	a, #0xff
	out	(0x60), a
	ret
__endasm;
}

void vgl_sound(word frq, word len) {
	// Perform a beep (frq is actually a delay...)
	(void)frq;
	(void)len;
	
	//@TODO: Implement
	
}


void vgl_sound_note(word n, word len) {
	/*
	word frq;
	
	
	// My own, working great!
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
	*/
}

void beep() {
	//vgl_sound_note(12*4+0, 0x0111);
}
#endif // __VGL_SOUND_H