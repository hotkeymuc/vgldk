/*
	A simple "Hello World" app for the VGLDK
	
	This demonstrates a minimal binary that can be uploaded to a host
	running monitor.c using a serial connection.
	
	To build:
		make app
	To upload:
		make upload
		(or manually invoke tools/send2monitor.py)
	
	To keep apps small, they can re-use the host's I/O:
	When using VGLDK_VARIABLE_STDIO (e.g. VGLDK_SERIES = 0) then vgldk.h will
	define a monitor.c compatible app entry point automatically and obtain the
	host's p_putchar/p_getchar on startup before jumping to main().
	
	So: Make sure vgldk.h can define the very first bytes of your app binary!
	(This also means: use an empty crt0 if possible)
	
	2020-01-22 Bernhard "HotKey" Slawik
*/

#include <vgldk.h>
#include <stdiomin.h>

//void main() __naked {
//void main() {
//int main() {
int main(int argc, char *argv[]) {
	char c;
	
	printf("Hello World!\n");
	c = getchar();
	
	for (c = 0; c < argc; c++) {
		printf_d(c);
		putchar(':');
		printf(argv[c]);
		putchar('\n');
	}
	
	printf("Key to end");
	c = getchar();
	
	//while(1) { }
	//return;
	//return c;
	return 0x43;
}
