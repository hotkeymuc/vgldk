/*
	GL6000SL Speech test
	
	I am pretty sure that the sound chip is some sort of TI TMS51x0/52x0 speech chip
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

int tms_frame;

void tms_put(byte v) {
	byte c;
	
	//printf_x2(v);
	//c = getchar();
	
	port_out(0x10, 0x00);	// Also 0x01
	
	// After OUT 0x10, 0x00:
	//check_port(0x60);	// 0x0F
	//check_port(0x10);	// 0xF9
	
	//port_out(0x10, 0x04);
	//check_port(0x10);	// After OUT 0x10, 0x04: 0xFF, 0xFD, 0xFF, 0xFD, ...
	
	//port_out(0x11, v);	// Actually output data to the pins
	
	//check_port(0x60);	// 0x0F
	//check_port(0x10);	// 0xF9
	
	
	port_out(0x10, 0x04);	// Works: Put 0x00, then 0x04, then enter data!
	
	//port_out(0x60, 0 + (tms_frame++) % 6);	// Testing
	
	// After OUT 0x10, 0x00:
	//check_port(0x10);	// 0xF9
	
	// After OUT 0x10, 0x04:
	//check_port(0x10);	// 0xFC (sound stopped?) -OR- 0xFD ... 0xFF ... 0xFD (sound running?)
	//check_port(0x11);	// 0xFF
	//check_port(0x21);	// 0xFF
	//check_port(0x60);	// 0x0F
	//check_port(0x62);	// 0xFF
	
	// TMS5220 ~RS and ~WS?
	while(port_in(0x10) == 0xfd) { }	// Wait for wiggle
	//while((port_in(0x10) & 0x01) == 0) { }	// Wait for ready
	
	port_out(0x11, v);	// Actually output data to the pins
	
	//check_port(0x10);	// 0xFC if sound stopped -OR- wiggling 0xFD ... 0xFF ... 0xFD
	
}

void tms_reset() {
	// TMS: Command "Reset"
	printf("TMS:Reset...");
	//check_port(0x10);	// 0xFC = 0b11111100
	//check_port(0x60);	// 0x0F
	tms_put(0xff);	// D0...D7: X111XXXX = "Reset" (e.g. 0xFF or 0x7E)
	//check_port(0x60);	// 0x0F
	//check_port(0x10);	// 0xFD = 0b11111101, 0xFF, = 0b11111111
}

void tms_speak() {
	// TMS: Command "Speak External"
	printf("TMS:Speak...");
	//check_port(0x10);	// 0xFD = 0b11111101, 0xFF, = 0b11111111
	tms_put(0xE7);	// D0...D7: X110XXXX = "Speak External" (e.g. 0xE7 or 0x66)
	//check_port(0x10);	// 0xFC = 0b11111100
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
	check_port(0x10);	// 0xf9 = 0b11111001
	port_out(0x10, 0x04);
	
	//for (i=0; i < 3; i++) {
		check_port(0x21);	// 0xff
		check_port(0x60);	// 0x0F by now
	//}
	
	
	// End of init
	
	//printf("End of init.\n");
	
	tms_reset();
	
	
	printf("Ready.\n");
	//c = getchar();
	
	tms_speak();
	
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
		
		printf_x4((word)o); putchar(':');
		
		
		// Check TMS status
		do {
			v = port_in(0x10);
			//printf_x2(v); c = getchar();
			
			if ((v & 0x03) == 2) {
				// hang
				printf("hang!"); c = getchar();
				
				tms_reset();
				continue;
			}
			
			if ((v & 0x03) == 0) {
				// TMS is idle!
				
				printf("Idle!");	//c = getchar();
				
				tms_speak();
				continue;
			}
			
			if ((v & 0x02) == 0) {
				// Ready to receive data
				break;
			}
		} while(1);	// Wait until ready to receive data
		
		
		// Feed new data
		tms_frame++;
		v = *o;
		
		tms_put(v);
		
		printf_x2(v);
		
		//check_port(0x60);
		//check_port(0x10);
		//c = getchar();
		
		if ((tms_frame % 0x04) == 0) {
			printf("Pause...");
			c = getchar();
			printf("\n");
		}
		
		
		/*
		do {
			c = getchar();
			
			if ((c == 'r') || (c == 'R')) {
				// Output something
				//port_out(0x10, (tms_frame++) % 16);
				//port_out(0x60, tms_frame++);
				//port_out(0x11, tms_frame++);
				tms_reset();
				tms_speak();
			} else
			break;
			
		} while(1);
		
		*/
		printf("\n");
		
		
		o++;
	}
	
	
	printf("Key to end\n");
	c = getchar();
	
	//while(1) { }
	//return;
	//return c;
	return 0x42;
}
