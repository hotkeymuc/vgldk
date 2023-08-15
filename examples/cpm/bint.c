#ifndef __BINT_C
#define __BINT_C

/*
	BINT - Global Interrupt Handler
*/

#include "bint.h"

// __interrupt
void bint() __naked {
	
	__asm
		;.asciz '[BINT]'	; Marker to find segment in binary
		push	af
		push	bc
		push	de
		push	hl
		push	ix
		push	iy
	__endasm;
	
	
	//@TODO: Timer functions
	bint_timer ++;
	
	//@TODO: Keyboard with auto-repeat
	
	/*
	// Check keyboard
	if (want_key == 1) {
		current_key = inkey();
		
		/ *
		if (current_key != 0) {
			// Reset auto power off timer
			timer = 0;
		}
		* /
		
		want_key = 0;
	}
	*/
	
	/*
	// Shutdown timer
	// This interrupt is triggered ~50 times per second
	timer += 1;
	if (timer > 50) {
		//printf("T");
		vgl_lcd_writeData('T');
		timer = 0;
	}
	*/
	
	__asm
		pop	iy
		pop	ix
		pop	hl
		pop	de
		pop	bc
		pop	af
	__endasm;
	
	
	__asm
		ei
		reti
		
		;.asciz '[BINT end]'
	__endasm;
}


#endif	// __BINT_C