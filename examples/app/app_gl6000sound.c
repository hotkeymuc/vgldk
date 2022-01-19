/*
	A simple "Hello World" for the VGLDK
	
	2020-01-22 Bernhard "HotKey" Slawik
*/


/*
// Forward declarations
//int main(int argc, char *argv[]);	// as defined in monitor.c
//void main() __naked {	
//int main();
//void main();

// Entry point for app should be the first bytes in memory
int app_start(int argc, char *argv[]) {	// as defined in monitor.c
int app_start(void *v_stdout_putchar, void *v_stdin_getchar) {
	
	(void)argc;
	(void)argv;
	
	
	__asm
		call _vgldk_init
		//call _main;
	__endasm;
	
	
	return 0x42;
}
*/


// When using VGLDK_VARIABLE_STDIO vgldk.h will define the entry point automatically and obtain the host p_putchar/p_getchar
#include <vgldk.h>
#include <stdiomin.h>
#include <ports.h>
#include <hex.h>

byte check_port(byte p) {
	//char c;
	byte v = port_in(p);
	
	printf("0x"); printf_x2(p); printf(" = 0x"); printf_x2(v); printf("\n");
	//c = getchar();
	return v;
}

//void main() __naked {
//void main() {
//int main() {
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	byte i;
	char c;
	byte *o;
	
	printf("Hello!\n");
	//c = getchar();
	
	o = (byte *)0x4000;
	
	port_out(0x22, 0x00);
	port_out(0x21, 0xe0);
	port_out(0x23, 0x60);
	//LCD
	port_out(0x23, 0x60);
	port_out(0x21, 0xe0);
	port_out(0x61, 0xd0);
	port_out(0x10, 0x00);
	
	//check_port(0x21);
	
	port_out(0x21, 0xff);
	port_out(0x29, 0x01);
	port_out(0x43, 0x02);
	port_out(0x23, 0x20);
	
	//for(i = 0; i < 4; i++) { check_port(0x21); }
	
	
	while(true) {
		
		
		//check_port(0x60);
		//do {
			c = port_in(0x60);	// 0x0B / 0x0F
			//if (c != 0x0b) printf_x2(c);
		//} while (c & 0x04);	// == 0x00);
		
		// Loop start
		port_out(0x62, 0x00);
		//port_out(0x10, 0x00);
		
		//for(i = 0; i < 4; i++) { check_port(0x21); }
		
		// INT
		//check_port(0x10);
		//c = port_in(0x10);	// 0x0B / 0xF9 / 0xFD
		
		if ((word)o % 0x20 == 0) printf_x4((word)o);
		c = *o;
		
		//port_out(0x22, 0x00);
		//port_out(0x21, 0xe0);
		//port_out(0x23, 0x60);
		
		
		port_out(0x10, c);
		port_out(0x11, c);
		
		//port_out(0x61, 0xd0);
		
		//port_out(0x62, 0xff);
		//port_out(0x10, 0x00);
		o++;
		
		/*
		putchar(' ');
		
		c = port_in(0x10);
		printf_x2(c);
		
		printf("\n");
		//putchar(' ');
		
		//c = keyboard_inkey();
		*/
	}
	
	
	printf("Key to end\n");
	c = getchar();
	
	//while(1) { }
	//return;
	//return c;
	return 0x42;
}
