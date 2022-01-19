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

//void main() __naked {
//void main() {
int main() {
	char c;
	
	printf("Hello World!\n");
	c = getchar();
	
	
	printf("Key to end");
	c = getchar();
	
	//while(1) { }
	//return;
	//return c;
	return 0x43;
}
