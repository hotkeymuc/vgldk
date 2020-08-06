/*
	A simple "Hello World" for the VGLDK
	
	2020-01-22 Bernhard "HotKey" Slawik
*/

// Forward declarations
//int main(int argc, char *argv[]);	// as defined in monitor.c
//void main() __naked {	
//int main();
//void main();

/*
void app_start() __naked {
	__asm
		
		jp _main;
	__endasm;
}
*/

int app_start(int argc, char *argv[]) {	// as defined in monitor.c
	
	(void)argc;
	(void)argv;
	
	
	__asm
		call _vgldk_init
		//call _main;
	__endasm;
	
	
	return 0x42;
}



#include <vgldk.h>
#include <stdiomin.h>

//void main() __naked {
void main() {
	char c;
	
	printf("Hello World!\n");
	c = getchar();
	
	
	printf("Key to end");
	c = getchar();
	
	//while(1) { }
	//return;
	//return c;
}
