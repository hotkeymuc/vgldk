/*
	Test the keyboard driver
	
	The GL4000 keyboard driver is rudimentary (no shift, no activity-keys).
	I am now adapting the keyboard code for GL6000 for GL4000.
	This is the test for it.
	
	2022-01-29 Bernhard "HotKey" Slawik
*/


// When using VGLDK_VARIABLE_STDIO vgldk.h will define the entry point automatically and obtain the host p_putchar/p_getchar
#include <vgldk.h>
#include <stdiomin.h>

#include <hex.h>	// for printf_x2()

// Note: apps are compiled as "VGLDK_SERIES=0"
#include "../../include/arch/gl4000/keyboard.h"


//void main() __naked {
//void main() {
//int main() {
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	char c;
	printf("Keyboard test\n");
	keyboard_init();	// Explicitly init
	
	while(1) {
		c = keyboard_getchar();
		putchar(c);
		
		/*
		// Debug second matrix
		#ifdef KEYBOARD_LATCH
			// Set all mux lines HIGH (although in original firmware, keyboard matrix works without it)
			keyboard_port_matrixLatch = 0xff;
		#endif
		
		// Set all outputs at the same time
		keyboard_port_matrixRowOut = 0xff;
		
		// Read back if anything is pressed at all
		//b1 = keyboard_port_matrixColIn;
		c = keyboard_port_matrixColIn2;
		
		// Set MUX back to idle
		keyboard_port_matrixRowOut = 0x00;
		
		putchar('x');
		printf_x2(c); //b2);
		putchar(' ');
		*/
	}
	
	//while(1) { }
	//return;
	//return c;
	return 0x43;
}
