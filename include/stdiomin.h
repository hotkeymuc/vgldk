#ifndef __STDIOMIN_H
#define __STDIOMIN_H

/*
Some bare minimum stdio functions
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
	void putchar(char c) {
		(*p_stdout_putchar)(c);
	}
	char getchar() {
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


//#include <stdio.h>	// for printf() putchar() gets() getchar()
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
#define puts(s) printf(s)

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


byte strlen(const char *c) {
	byte l;
	l = 0;
	while (*c++ != 0)  {
		l++;
	}
	return l;
}


void memcpy(byte *src_addr, byte *dst_addr, word count) {
	word i;
	byte *ps;
	byte *pd;
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

#endif	// __STDIOMIN_H