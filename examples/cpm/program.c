/*

CP/M program helpers (I/O etc.)

*/

#include "program.h"

#include <stringmin.h>	// For memcpy()

//#include <hex.h>
byte program_hexDigit(byte c) {
	if (c < 10) return ('0'+c);
	return 'A' + (c-10);
}
void program_printf_x2(byte b) {
	putchar(program_hexDigit(b >> 4)); putchar(program_hexDigit(b & 0x0f));
}
/*
//void program_printf_d(char *pc, byte d) {
void program_printf_d(byte d) {
	byte i;
	i = 100;	// Maximum decimal digit (1/10/100/1000/...)
	while(i > 0) {
		putchar('0' + ((d / i) % 10));
		i /= 10;
	}
}
*/

// BDOS STDIO

// Manual argument handling
void putchar(char b) __naked {
	(void)b;	// Silence the compiler
	__asm
		
		push hl
		push de
		
		; Get parameter from stack into E
		; Take note of how many things were pushed before this line (number x 2) and the call address (2)
		ld hl,#0x0006
		add hl,sp
		ld e,(hl)
		
		push bc
		ld c, #0x02	; 2 = BDOS_FUNC_C_WRITE = Console output
		
		call 5
		pop bc
		
		pop de
		pop hl
		
		ret
	__endasm;
	//return;
}

/*
// Using volatile temp variable
volatile char putchar_tmp;
void putchar(char b) {
	putchar_tmp = b;
	
	__asm
		push af
		push bc
		push de
		push hl
		
		ld a, (_putchar_tmp)
		ld e, a
		
		ld c, #2	; 2 = BDOS_FUNC_C_WRITE = Console output
		call 0x0005
		
		pop hl
		pop de
		pop bc
		pop af
	__endasm;
}
*/

//volatile char getchar_tmp;
char getchar() __naked {
//char getchar() {
	
	//@FIXME: something goes wrong here with the stack...
	
	__asm
		;push af
		;push hl
	
		push bc
		ld c, #0x01	; 1 = BDOS_FUNC_C_READ = Console input (with echo)
		
		call 5
		pop bc
		
		; Result is in L
		;ld a, l
		;ld (_getchar_tmp), a
		
		;pop hl
		;pop af
		ret	; Naked return
	__endasm;
	//return 0xff;	// Suppress compiler warning "must return a value" if not declared "__naked"
	//return getchar_tmp;
}



void puts(const char *s) {
	
	//@TODO: Use BDOS_FUNC_C_WRITESTR 9 = Output string (terminated by '$')
	
	while (*s != 0) {
		putchar(*s++);
	}
	putchar('\n');
}


volatile char *gets_tmp;
//char *gets(char *pc) {
//void gets(char *pc, byte max_size) {
void gets(char *pc) {
	
	byte max_size = PROGRAM_GETS_MAX_SIZE;
	
	// Use BDOS function 10 = BDOS_FUNC_C_READSTR =Read console buffer
	// DE = pointer to buffer, first byte contains max length
	gets_tmp = pc;
	*gets_tmp = max_size;	// first byte: max buffer size
	*(byte *)((word)gets_tmp + 1) = 0;	// second byte: bytes in buffer
	
	//putchar('{');
	
	__asm
		push bc
		push de
		push hl
		
		ld de, (_gets_tmp)
		ld c, #10	; 10 = BDOS_FUNC_C_READSTR = Read console buffer
		call 0x0005
		
		pop hl
		pop de
		pop bc
	__endasm;
	
	//putchar('}');
	
	byte l = *(gets_tmp+1);	// Read back the input length (second byte)
	//printf("(length="); program_printf_x2(l); printf(")");
	
	// Shift over, so that first input byte is at the beginning of buffer
	memcpy(pc, pc+2, l);
	
	// Terminate string with zero
	*(pc+l) = 0;
	
	/*
	// Manually implement gets using BDOS getchar()
	//@TODO: Handle cursor position/insert/overwrite/del/backspace
	
	char c;
	//myscroll_counter = LCD_ROWS - lcd_y - 1;	// Reset scroll counter, i.e. user has enough time to read current page
	while(1) {
		
		
		//@FIXME: something goes wrong here with the stack...
		//#error "continue bugfixing here! Something corrupts the stack after typing a key"
		putchar('[');	// for debugging!
		c = getchar();
		putchar(']');	// for debugging!
		
		// Force local echo? getchar() outputs to conout anyway!
		#ifdef PROGRAM_GETS_LOCAL_ECHO
		putchar(c);
		#endif
		if ((c < 0x20) || (c > 'z')) {
			program_printf_x2(c);
		}
		
		if ( (c == 8) || (c == 127) ) {
			// Backspace/DEL
			if (pc > pcs) {
				pc--;
				
				//	if (lcd_x > 0) {
				//		lcd_x--;
				//		vgl_lcd_set_cursor();
				//	}
				
			}
			continue;
		}
		
		if ((c == '\n') || (c == '\r') || (c == 0)) {
			// End of string
			
			//@FIXME: Add '\r' or '\n' at end of string in CP/M?
			
			// Terminate string
			*pc = 0;
			
			//return pcs;
			return;
		}
		
		// Add char
		*pc++ = c;
	}
	return pcs;
	*/
	//return pc;	// Return original pointer
}


// puts wihtout new line
void printf(const char *s) {
	
	//@TODO: Use BDOS_FUNC_C_WRITESTR 9 = Output string (terminated by '$')
	
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