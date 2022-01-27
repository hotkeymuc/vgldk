/*
	GL6000SL Speech test
	
	Not knowing how the interface works, I'll just push data out of some ports and hope for the best...
	
	Be careful: The Speech chip has an internal amplifier. When hitting the wrong resonance, you can almost short the power supply!
	Believe me! The LCD flashed weirdly, the speaker made an "Oooof!"-sound and the USB power supply browned out!
	
	R 0x10 == 0xF9
	R 0x11 == 0xFF
	R 0x60 == 0x0B
	W 0x10 := 0x00
	
	R 0x10, R 0x11, R 0x60, ...
	
	W 0x10 := 0x04
	R 0x60 == 0x0F
	
	
	
	2022-01-19 Bernhard "HotKey" Slawik
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
	int i;
	byte v = 0x00;
	byte v_old = 0x00;
	
	for(i = 0; i < 1000; i++) {
		v = port_in(p);
		if ((i == 0) || (v != v_old)) {
			if (i == 0) {
				printf("0x"); printf_x2(p); printf(" = ");
			} else {
				putchar(',');
			}
			printf_x2(v);	//printf("\n");
			//c = getchar();
			v_old = v;
		}
	}
	printf("\n");
	return v;
}

/*

void monitor_ports() {
	byte v_10;
	byte v_60;
	int v_10_old;
	int v_60_old;
	int i;
	
	v_10_old = -1;	//port_in(0x10);	// 0x0B / 0xF9 / 0xFD
	v_60_old = -1;	//port_in(0x60);	// 0x0B / 0x0F
	
	for(i = 0; i < 400; i++) {
		v_10 = port_in(0x10);	// 0x0B / 0xF9 / 0xFD
		v_60 = port_in(0x60);	// 0x0B / 0x0F
		
		if (v_10 != v_10_old) {
			printf("10="); printf_x2(v_10); putchar(' ');
			v_10_old = v_10;
		}
		if (v_60 != v_60_old) {
			printf("60="); printf_x2(v_60); putchar(' ');
			v_60_old = v_60;
		}
	}
	
}
*/

int tsp_frame;

void tsp_put(byte v) {
	byte c;
	
	//printf_x2(v);
	//c = getchar();
	
	port_out(0x10, 0x00);
	
	// After OUT 0x10, 0x00:
	//check_port(0x60);	// 0x0F
	//check_port(0x10);	// 0xF9
	
	//port_out(0x10, 0x04);
	//check_port(0x10);	// After OUT 0x10, 0x04: 0xFF, 0xFD, 0xFF, 0xFD, ...
	
	//port_out(0x11, v);	// Actually output data to the pins
	
	//check_port(0x60);	// 0x0F
	//check_port(0x10);	// 0xF9
	
	
	//port_out(0x11, (v * 0x55) % 0x100);	// Testing
	
	port_out(0x10, 0x04);	// Works: Put 0x00, then 0x04, then enter data!
	
	//port_out(0x60, 0 + (tsp_frame++) % 6);	// Testing
	
	
	// After OUT 0x10, 0x00:
	//check_port(0x10);	// 0xF9
	
	// After OUT 0x10, 0x04:
	//check_port(0x10);	// 0xFC (sound stopped?) -OR- 0xFD ... 0xFF ... 0xFD (sound running?)
	//check_port(0x11);	// 0xFF
	//check_port(0x21);	// 0xFF
	//check_port(0x60);	// 0x0F
	//check_port(0x62);	// 0xFF
	
	while(port_in(0x10) == 0xfd) { }	// Wait for wiggle-state :)
	
	port_out(0x11, v);	// Actually output data to the pins
	
	//check_port(0x11);	// 0xFC if sound stopped -OR- wiggling 0xFD ... 0xFF ... 0xFD
	//check_port(0x10);	// 0xFC if sound stopped -OR- wiggling 0xFD ... 0xFF ... 0xFD
	
	//port_out(0x10, 0x04);
	
}

//void main() __naked {
//void main() {
//int main() {
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	byte i;
	byte v;
	char c;
	byte *o;
	
	printf("Hello!\n");
	//c = getchar();
	
	// Boot:
	port_out(0x22, 0x00);
	port_out(0x21, 0xe0);
	
	port_out(0x23, 0x60);
	port_out(0x23, 0x60);
	port_out(0x21, 0xe0);
	//LCD
	port_out(0x61, 0xd0);
	port_out(0x10, 0x00);
	
	check_port(0x21);	// 0xff
	port_out(0x21, 0xff);
	
	port_out(0x29, 0x01);
	port_out(0x43, 0x02);
	port_out(0x23, 0x20);
	
	//for (i=0; i < 3; i++) {
		check_port(0x21);	// 0xff
		check_port(0x60);	// 0x0b
	//}
	
	port_out(0x62, 0x00);
	port_out(0x10, 0x00);
	check_port(0x10);	// 0xf9
	port_out(0x10, 0x04);
	
	//for (i=0; i < 3; i++) {
		check_port(0x21);	// 0xff
		check_port(0x60);	// 0x0F by now
	//}
	
	
	// End of init
	
	
	/*
	// This actually produces glitchy sounds (and eventually browns out LOUDLY)
	port_out(0x51, 8);
	o = (byte *)0x4000;	//4320 - 12*4;	// Around 0x4320 it gets crazy!
	
	while(true) {
		port_out(0x62, 0x00);
		
		if ((word)o % 7 == 0) {
			printf_x4((word)o);
			c = getchar();
			printf("\n");
		}
		
		c = *o; o++;
		port_out(0x10, c);
		
		c = *o; o++;
		port_out(0x11, c);
	}
	*/
	
	
	/*
	for(i = 0; i < 10; i++) {
		printf("\nLoop: ");
		tsp_put(0x0e);
		tsp_put(0x02);
		tsp_put(0x21);
		tsp_put(0x67);
		tsp_put(0x4B);
		
		tsp_put(0x11);
		tsp_put(0x1B);
		tsp_put(0x00);
		tsp_put(0xfd);
		tsp_put(0x21);
		tsp_put(0x7d);	// Burp
		tsp_put(0x21);
	}
	*/
	
	/*
	// Just speculating...
	port_out(0x51, 0x08);
	o = (byte *)0x4000 + 0x00c0;	// + 0x0040;	//4320 - 12*4;	// Around 0x4320 it gets crazy!
	*/
	
	//@TODO: Real frames?
	// spss011d.pdf, 6.1, page 180
	//	Parameter	Energy	Repeat	Pitch 	K1	K2	K3	K4	K5	K6	K7	K8	K9	K10	K11	K12
	//	# Bits		4		1		7		6	6	5	5	4	4	4	3	3	3	0	0
	// 56 bits = 7 bytes?
	
	// Speech data can be found in ROM:0x6D141+ (out 0x51,0x1b, mem[0x5141...])
	port_out(0x51, 0x1B);	// OUT 0x51, 0x1B	-> maps ROM:0x6C000 to CPU:0x4000
	o = (byte *)0x5141;		// MEM:0x5141 now shows ROM:0x6D141 = BOING-sound
	//o = (byte *)0x5000;		// MEM:0x5000 now shows ROM:0x6D000 = sounds and stuff
	
	while(true) {
		v = *o;
		
		tsp_put(v);
		
		//if ((word)o % 2 == 0) {
			printf_x4((word)o); putchar(' '); printf_x2(v);
			c = getchar();
			printf("\n");
		
			//check_port(0x10);
		
		//}
		//monitor_ports();
		
		o++;
	}
	
	
	printf("Key to end\n");
	c = getchar();
	
	//while(1) { }
	//return;
	//return c;
	return 0x42;
}
