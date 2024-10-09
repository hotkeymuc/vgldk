#ifndef __STDIOMIN_H
#define __STDIOMIN_H

/*
Some bare minimum stdio functions
@TODO: Use optimized SDCC/Z88DK library functions!
*/

#ifdef VGLDK_VARIABLE_STDIO
	// Allow custom stdio at runtime
	
	// Types of IO callbacks
	typedef void (t_putchar)(char);
	typedef char (t_getchar)(void);
	//typedef p_char (t_gets)(char *);
	//typedef int (t_inkey)(void);
	
	// Callback variables
	t_putchar *p_stdout_putchar;
	t_getchar *p_stdin_getchar;
	//t_gets *p_stdin_gets;
	//t_inkey *p_stdin_inkey;
	byte stdio_echo;
	
	// Proxy functions
	void inline putchar(char c) {
		(*p_stdout_putchar)(c);
	}
	char inline getchar() {
		return (*p_stdin_getchar)();
	}
	/*
	char *gets(char *s) {
		return (*p_stdin_gets)(s);
	}
	int inkey() {
		return (*p_stdin_inkey)();
	}
	
	char *stdio_gets(char *pc);	// Forward
	*/
	void stdio_init() {
		// Set hardware stdio as the default
		#ifdef VGLDK_STDOUT_PUTCHAR
			p_stdout_putchar = (t_putchar *)&VGLDK_STDOUT_PUTCHAR;
		#endif
		#ifdef VGLDK_STDIN_GETCHAR
			p_stdin_getchar = (t_getchar *)&VGLDK_STDIN_GETCHAR;
		#endif
		//p_stdin_gets = (t_gets *)&VGLDK_STDIN_GETS;
		//p_stdin_inkey = (t_inkey *)&VGLDK_STDIN_INKEY;
		
		stdio_echo = 1;
	}
	
#else
	// Define hardware stdio as the one and only implementation
	#define putchar VGLDK_STDOUT_PUTCHAR
	#define getchar VGLDK_STDIN_GETCHAR
	
	//#define gets VGLDK_STDIN_GETS	//stdio_gets
	//#define inkey VGLDK_STDIN_INKEY
	
	#define stdio_init() ;	// Not needed
	#define stdio_echo 1
#endif




// The C library function int puts(const char *str) writes a string to stdout up to but not including the null character.
// A newline character is appended to the output.
int puts(const char *str) {
	while(*str) putchar(*str++);
	putchar('\n');
	return 1;
}
//#define puts(s) printf(s)

void printf(const char *pc) {
	/*
	char c;
	c = *pc;
	while(c != 0) {
		putchar(c);
		pc++;
		c = *pc;
	}
	*/
	while(*pc) putchar(*pc++);
}

//void printf_d(char *pc, byte d) {
void printf_d(byte d) {
	
	byte i;
	
	//printf(pc);
	i = 100;
	while(i > 0) {
		putchar('0' + ((d / i) % 10));
		i /= 10;
	}
	
	//printf("\n");
	
	/*
	putchar('0' + ( d / 100));
	putchar('0' + ((d % 100) / 10));
	putchar('0' + ( d % 10));
	*/
}

/*

char *gets(char *pc) {
	char *pcs;
	char c;
	pcs = pc;
	
	//@TODO: Handle cursor position/insert/overwrite/del/backspace
	byte add_char;
	
	//myscroll_counter = LCD_ROWS - lcd_y - 1;	// Reset scroll counter, i.e. user has enough time to read current page
	while(1) {
		c = getchar();
		
		if ( (c == 8) || (c == 127) ) {
			// Backspace/DEL
			if (pc <= pcs)  continue;	// Beep!
			pc--;
			add_char = 0;
		} else {
			add_char = 1;
		}
		
		// Local echo
		if (stdio_echo)
			putchar(c);
		
		if ((c == '\n') || (c == '\r') || (c == 0)) {	// EOF?
			// End of string
			
			// Terminate string
			*pc = 0;
			return pcs;
		}
		
		if (add_char) {
			// Add char
			*pc++ = c;
		}
	}
	//return pcs;
}

*/

char *gets(char *pc) {
	char *pcs;
	char c;
	pcs = pc;
	
	//@TODO: Handle cursor position/insert/overwrite/del/backspace
	
	//myscroll_counter = LCD_ROWS - lcd_y - 1;	// Reset scroll counter, i.e. user has enough time to read current page
	while(1) {
		c = getchar();
		
		
		// Local echo
		if (stdio_echo)
			putchar(c);
		
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

#endif	// __STDIOMIN_H