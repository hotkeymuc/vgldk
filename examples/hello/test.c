/*
	A simple STDIO test for the VGLDK
	
	2020-01-22 Bernhard "HotKey" Slawik
*/

#include <vgldk.h>
#include <stdiomin.h>


void main() __naked {
	char c;
	char s[64];
	
	//lcd_init();
	//vgl_sound_off();
	
	printf("Testing printf!");
	//beep();
	
	#ifdef VGLDK_SERIES
		#define xstr(s) str(s)
		#define str(s) #s
		printf("SERIES=" xstr(VGLDK_SERIES) );
	#endif
	
	printf("key:");
	c = getchar(); putchar(c);
	beep();
	
	while(1) {
		
		
		printf(">");
		gets(s);
		printf(s);
	}
	
	
}
