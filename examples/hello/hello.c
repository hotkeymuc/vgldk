/*
	A simple "Hello World" for the VGLDK
	
	2020-01-22 Bernhard "HotKey" Slawik
*/

#include <vgldk.h>
#include <stdio.h>

void main() __naked {
	
	lcd_init();
	vgl_sound_off();
	
	/*
	putchar('H');
	putchar('e');
	putchar('l');
	putchar('l');
	putchar('o');
	putchar(' ');
	putchar('W');
	putchar('o');
	putchar('r');
	putchar('l');
	putchar('d');
	*/
	
	printf("Hello World!\n");
	
	while(1) {
	}
	
}
