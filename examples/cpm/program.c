/*

CP/M program helpers

*/
#include "program.h"

// BDOS STDIO
void putchar(char b) __naked {
	(void)b;
	__asm
		push hl
		
		; Get parameter from stack into a
		ld hl,#0x0004
		add hl,sp
		ld e,(hl)
		
		ld c, #0x02	; 2 = BDOS_FUNC_C_WRITE = Console output
		
		call 5
		
		pop hl
		ret
	__endasm;
}

char getchar() __naked {
	__asm
		;push hl
		
		ld c, #0x01	; 1 = BDOS_FUNC_C_READ = Console input (with echo)
		
		call 5
		
		; Result is in L
		
		;pop hl
		ret
	__endasm;
}

char *gets(char *pc) {
	char *pcs;
	char c;
	pcs = pc;
	
	//@TODO: Handle cursor position/insert/overwrite/del/backspace
	
	//myscroll_counter = LCD_ROWS - lcd_y - 1;	// Reset scroll counter, i.e. user has enough time to read current page
	while(1) {
		c = getchar();
		
		// Local echo
		#ifdef PROGRAM_GETS_LOCAL_ECHO
		putchar(c);
		#endif
		
		if ( (c == 8) || (c == 127) ) {
			// Backspace/DEL
			if (pc > pcs) {
				pc--;
				/*
				if (lcd_x > 0) {
					lcd_x--;
					vgl_lcd_set_cursor();
				}
				*/
			}
			continue;
		}
		
		if ((c == '\n') || (c == '\r') || (c == 0)) {
			// End of string
			
			// Terminate string
			*pc = 0;
			return pcs;
		}
		
		// Add char
		*pc++ = c;
	}
	//return pcs;
}

void printf(const char *s) {
	while (*s != 0) {
		putchar(*s++);
	}
}

void exit() __naked {
	__asm
		ld c, #0x00	; System reset
		ld d, #0x00	; DL=0: Do not keep resident
		call 5	; Call? I guess this is one-way...
	__endasm;
}

/*
void main() __naked {
	
	printf("Hello world");
	
	__asm
		ld sp, #0x4000
		
		ld c, #0x02	; Con output
		ld e, #0x40	; E=Character to output
		call 5
		
		ld c, #0x00	; System reset
		ld d, #0x00	; DL=0: Do not keep resident
		call 5
	__endasm;
	
	exit();
}
*/