/*
	A simple "Hello World" for the VGLDK
	
	2020-01-22 Bernhard "HotKey" Slawik
*/

#include <vgldk.h>
#include <stdio.h>


void main() __naked {
	char c;
	char s[64];
	
	//lcd_init();
	//vgl_sound_off();
	
	printf("Hello printf-World!\n");
	
	#ifdef VGLDK_SERIES
		#define xstr(s) str(s)
		#define str(s) #s
		printf("SERIES=" xstr(VGLDK_SERIES) );
	#endif
	
	printf("key:");
	c = getchar(); putchar(c);
	
	while(1) {
		
		
		printf(">");
		gets(s);
		printf(s);
	}
	
	
}
