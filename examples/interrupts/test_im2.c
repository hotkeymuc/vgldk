/*
	Even though we can not overwrite the internal interrupt table in ROM, we can make use of "IM 2"
	See: http://www.z80.info/1653.htm
	
	GL6000SL hardware interrupt(s) seems to continuously call offset +0x__F7 (0x7B*2+1) of the IM2 jump table
	(e.g. when register I=0xD0, the interrupt will call address stored at 0xD0F7)
	Frequency is about ~128 times per second
	
	2022-02-07 Bernhard "HotKey" Slawik
*/


// When using VGLDK_VARIABLE_STDIO vgldk.h will define the entry point automatically and obtain the host p_putchar/p_getchar
#include <vgldk.h>
#include <stdiomin.h>
#include <hex.h>


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
	
	printf("Interrupt IM2 Test\n");
	
	
	// Let's try IM2 with a table located at 0xd000
	#define INTTBL 0xd000
	
	// Prepare an interrupt table for IM2
	word *itp = (word *)INTTBL;
	
	// For odd interrupts this has to be shifted by one byte:
	itp = (word *)((word)itp + 1);
	for(i = 0; i < 0x100; i++) {
		//if ((i % 2) == 1)
		if (i == 0x7B) {
			printf("int2=0x"); printf_x4((word)itp); printf("\n");
			*itp = (word)&int_isp2;
		} else
			*itp = (word)&dummy_isp;
		itp++;
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
	printf("Press key to enable IM2");
	getchar();
	#endif
	
	// Set up and enable interrupts
	__asm
		di
		nop
		nop
		im 2
		ld a, #0xd0
		ld i,a
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
	
	//return 42;	// only for app mode
}
