/*
	Trying to capture interrupts in IM0 mode.
	IM0 = The periphery puts an INSTRUCTION onto the bus.
	This instruction is usually "RSTx", which is a single-byte jump to 0x00 - 0x38 (in steps of 8)
	
	
	
	!!!!!!!!!!!!!!!!!!!!
	!! This is not possible in cart / app mode, since RST-instructions DO NOT pay respect to I register.
	!! So the interrupts always jump to the zero page at 0x0000 - 0x0038, which we do not control  :-(
	!!!!!!!!!!!!!!!!!!!!
	
	
	2022-02-15 Bernhard "HotKey" Slawik
*/


#include <vgldk.h>
#include <stdiomin.h>

//#define HEX_USE_DUMP
#include <hex.h>


// Interrupt service routines

//extern word int_count;
word dummy_int_count;
word int_count2;

void dummy_isp() __naked {
	__asm
	
	; Slow prolog
	push af
	push hl
	
	;; Quick prolog
	;ex af, af'	;'
	;exx
	
	
	; Increment int_count
	ld hl, (_dummy_int_count)
	inc hl
	ld (_dummy_int_count), hl
	
	
	; Slow epilog
	;ei
	pop hl
	pop af
	ei
	
	;; Quick epilog
	;exx
	;ex af, af'	;'
	;ei
	
	reti
	__endasm;
}


void int_isp2() __naked {
	__asm
	
	; Slow prolog
	push af
	push hl
	
	;; Quick prolog
	;ex af, af'	;'
	;exx
	
	
	; Increment int_count2
	ld hl, (_int_count2)
	inc hl
	ld (_int_count2), hl
	
	
	; Slow epilog
	;ei
	pop hl
	pop af
	ei
	
	;; Quick epilog
	;exx
	;ex af, af'	;'
	;ei
	
	reti
	__endasm;
}


#if VGLDK_SERIES == 0
// app
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
#else
// regular cart
void main() __naked {
#endif
	
	char c;
	word i;
	
	printf("Interrupt IM0 Test\n");
	
	printf("This is prett futile, since  RSTx instructions (as used by VTech) only support jumps to the zero page (0x0000-0x0040), which is ROM and not under our control.\n");
	//getchar();
	
	
	// Let's try IM0 with a table located at 0xd000
	#define INTTBL 0xd000
	
	// Prepare a jump table for IM0
	byte *itp = (byte *)INTTBL;
	
	// Clear / NOPs
	for(i = 0; i < 0x40; i += 8) {
		*itp++ = 0xc3;	// JP
		if (i == 0x30)	*(word *)itp = (word)&int_isp2;
		else			*(word *)itp = (word)&dummy_isp;
		itp += 7;
	}
	
	// Special interrupt handlers
	dummy_int_count = 0;
	int_count2 = 0;
	
	/*
	// 0xf7 = timer interrupt, ~128 Hz
	*(word *)(INTTBL + 0xf7) = (word)&timer_isp;
	timer_int_count = 0;
	
	// 0xe7 = speech interrupt, called for new data
	*(word *)(INTTBL + 0xe7) = (word)&speech_isp;
	speech_int_count = 0;
	*/
	
	#ifdef HEX_USE_DUMP
	dump(INTTBL, 0x40);
	getchar();
	#else
	printf("Press key to enable IM0");
	//getchar();
	#endif
	
	
	// Set up and enable interrupts
	
	
	__asm
		di
		nop
		nop
		im 0
		
		; !!!!!!!!!!!!!!!!
		ld a, #0xd0	; High byte of our interrupt table INTTBL. This has NO EFFECT on the RSTx instructions!
		ld i,a
		; !!!!!!!!!!!!!!!!
		
		ei
		nop
		nop
	__endasm;
	
	
	byte running = 1;
	while(running) {
		
		printf("count=");
		printf_x4(dummy_int_count);
		putchar(',');
		printf_x4(int_count2);
		printf("\n");
		
		putchar('?');
		c = getchar();
		
		switch(c) {
			case 13:
			case 10:
				break;
			
			case 'q':
			case 'Q':
				// Soft reset
				__asm
					;rst0
					call #0x0000
				__endasm;
				break;
			
			case 'x':
			case 'X':
				// Exit
				__asm
					;rst0
					;call #0x8002
					;return
				__endasm;
				running = 0;
				break;
			
			case 'h':
				// Help
				break;
			
			
			case 0x1b:	// LEFT
				break;
			case 0x1a:	// RIGHT
				break;
			
			default:
				printf_x2(c); putchar('?');
				break;
		}
	}
	
	//return 42;	// Apps can return stuff
}
