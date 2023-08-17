/*
	Test the platform independent "Soft UART" implementation(s)
	
	As of now, the GL4000 version works fine up to 19200,
	but the GL6000SL is still stuck at 9600 with a bespoke implementation.
	
	Let's fix softuart.h and make it multi-arch!
	
	2022-01-29 Bernhard "HotKey" Slawik
*/


// When using VGLDK_VARIABLE_STDIO vgldk.h will define the entry point automatically and obtain the host p_putchar/p_getchar
#include <vgldk.h>
#include <stdiomin.h>

// Since apps are compiled as "VGLDK_SERIES=0", but softuart needs to know the hardware, we specify it.

#define SOFTUART_SERIES 4000
//#define SOFTUART_SERIES 6000

//#define SOFTUART_BAUD 9600
#define SOFTUART_BAUD 19200
#include <driver/softuart.h>

#include <hex.h>	// for print_x2()

//void main() __naked {
//void main() {
//int main() {
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	int i;
	int j;
	char c;
	
	printf("SoftUART test\n");
	printf("Press key to send...");
	c = getchar();
	
	//softuart_sendByte(0x00);
	
	for(j = 0; j < 3; j++) {
		printf_d(j);
		
		/*
		// Binary sync pattern
		for(i = 0; i < 64; i++) {
			//softuart_sendByte(i);
			softuart_sendByte(0x00);
			softuart_sendByte(0x01);
			softuart_sendByte(0x00);
			softuart_sendByte(0x02);
			softuart_sendByte(0x00);
			softuart_sendByte(0x55);
			softuart_sendByte(0x00);
			softuart_sendByte(0xAA);
			softuart_sendByte(0x00);
			softuart_sendByte(0xFF);
		}
		softuart_sendByte('\n');
		*/
		
		// ABC...
		for(i = 0; i < 26; i++) {
			softuart_sendByte(0x40 + i);
		}
		softuart_sendByte('\n');
		
	}
	printf("done.\nNow receiving...\n");
	
	while(1) {
		i = softuart_receiveByte();
		if (i < 0) {
			// Timeout
			//putchar('.');
		} else {
			putchar(i);
			//printf_x2(i);
		}
	}
	
	
	//while(1) { }
	//return;
	//return c;
	return 0x43;
}
