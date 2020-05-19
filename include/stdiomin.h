#ifndef __STDIOMIN_H
#define __STDIOMIN_H

/*
Absolute bare minimum stdio functions
*/

/*
byte myscroll_counter;
void myscroll() {
	// Scroll callback that pauses every page
	
	while(lcd_y >= LCD_ROWS) {
		
		if (myscroll_counter >= LCD_ROWS) {
			// Sleep on scroll
			vgl_sound_note(12*5, 100);
			getchar();
			myscroll_counter = 0;
		}
		
		lcd_y--;
		vgl_lcd_scroll();
		myscroll_counter++;
	}
}
*/

//#include <stdio.h>	// for printf() putchar() gets() getchar()
void printf(char *pc) {
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
}


void gets(char *pc) {
	char *pcs;
	char c;
	pcs = pc;
	
	//myscroll_counter = LCD_ROWS - lcd_y - 1;	// Reset scroll counter, i.e. user has enough time to read current page
	while(1) {
		c = getchar();
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
		
		putchar(c);
		
		if ((c == '\n') || (c == '\r') || (c == 0)) {
			// End of string
			
			// Terminate string
			*pc = 0;
			return;
		}
		
		// Add char
		*pc++ = c;
	}
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