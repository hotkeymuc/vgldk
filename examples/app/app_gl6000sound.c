/*
	GL6000SL Speech experiments
	
	Even though the GL6000SL does not "talk", it surely does not use a standard 1-bit buzzer.
	Just by the sound of it (it has no IC markings), I am pretty sure that the sound chip is some sort of TI TMS51x0/52x0 compatible speech chip.
	
	After "listening" through the whole GL6000SL ROM file using an emulated LPC-10 speech decoder (e.g. Arduino/Adafruit Talkie)
	I have actually found some LPC-10 encoded sound data at ROM address 0x6D000!
	This means, the sound chip does not use its own mask ROM, but rather gets fed data from the CPU.
	
	Not knowing how the chip is connected to the bus, I'll just push data out of some ports I recorded in MAME and hope for the best...
	
	Be careful: The Speech chip has an internal amplifier. When hitting the wrong resonance, you can almost short the power supply!
	Believe me! The LCD can flash weirdly, the speaker can make "Oooof!"-sounds and the USB power supply can and will brown out!
	
	TODO:
		!!! TIMING ISSUES
			Though there is "some" sound - nothing is quite repeatable...
			This needs more "wait-for-ready"-checks that I don't know where to find (port/bit)
		
		* By hooking up an oscilloscope, it is clear that ports 0x10 and 0x11 can be used to push data to the IC
			! but I could not find ANY hint of the firmware ever accessing port 0x11!
			* So far it seems: Send 0x00 to port 0x10, send 0x04 to port 0x10, wait for status on port 0x10 (!=0xFD), then send DATA to port 0x11 - seems to work
		* The firmware fondles around with ports 0x10, 0x21, 0x22, 0x23, 0x29, 0x60, 0x61 and 0x62
		* There seems to be a very strict timing on when to talk to the chip and when not
		* Data sheet on sending a byte: Set ~WR low, wait for ~READY to go low, then send data and set ~WR high again
		* The firmware MAY be using the interrupt of the TMS chip in order to fill it, that's why MAME shows no port accesses to port 0x11?
		
	
	Port log:
		R 0x10 == 0xF9
		R 0x11 == 0xFF
		R 0x60 == 0x0B
		W 0x10 := 0x00
		
		R 0x10, R 0x11, R 0x60, ...
		
		W 0x10 := 0x04
		R 0x60 == 0x0F
		
		--------------
		R 0x10 = status?
			* idle?	0xF9	1111 1001
			* 0x01	0xF9	1111 1100
			* 0x02	0xF9	1111 1100
			* 0x04	0xFC	1111 1100
			* stuffed?	0xFD	1111 1101
			* ???	0xFF	1111 1101
	
	
	2022-01-19 Bernhard "HotKey" Slawik
*/


// When using VGLDK_VARIABLE_STDIO vgldk.h will define the entry point automatically and obtain the host p_putchar/p_getchar
#include <vgldk.h>
#include <stdiomin.h>
#include <ports.h>
#include <hex.h>


//#define CHECK_FRAMES 1024
#define CHECK_FRAMES 128
#define CHECK_MAX_SAMPLES 8
byte check_port(byte p) {
	byte samples[CHECK_MAX_SAMPLES];
	byte num_samples;
	word i;
	byte v;
	byte v_old;
	
	num_samples = 0;
	
	v = port_in(p);
	samples[num_samples++] = v;
	v_old = v;
	
	for(i = 0; i < CHECK_FRAMES; i++) {
		v = port_in(p);
		if (v != v_old) {
			// Value has changed
			samples[num_samples++] = v;
			v_old = v;
			if (num_samples >= CHECK_MAX_SAMPLES) {
				// Buffer is full!
				break;
			}
		}
	}
	
	// Output
	printf("0x"); printf_x2(p);
	//printf(" = ");
	putchar('=');
	for(i = 0; i < num_samples; i++) {
		if (i > 0) putchar(',');
		printf_x2(samples[i]);
		
	}
	if (num_samples >= CHECK_MAX_SAMPLES) {
		printf("...");
	}
	printf("\n");
	
	
	// Return last read value
	return v;
}

void delay(word n) {
	word i;
	for(i = 0; i < n; i++) {
		__asm
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
		__endasm;
	}
}

/*
byte check_port(byte p) {
	//char c;
	int i;
	byte v = 0x00;
	byte v_old = 0x00;
	
	for(i = 0; i < 1024; i++) {
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
*/


int tms_frame;

void tms_put(byte v) {
	byte c;
	
	//@TODO: The manual says (3.2 ~READY):
	// * ~WS and ~RS should be high when not in use
	//
	// * Set data to the bus (or shortly after next step)
	// * Set ~WS low (keep ~RS high - only one can be low at once!)
	// * Wait for READY to go low (it gets high 100ns after lowering either ~WS or ~RS)
	// * Set ~WS high
	// + remove data from bus
	
	// Bit-reverse? Datasheet says that D0 is the MSB!
	//v = (v & 0xF0) >> 4 | (v & 0x0F) << 4;	v = (v & 0xCC) >> 2 | (v & 0x33) << 2;	v = (v & 0xAA) >> 1 | (v & 0x55) << 1;
	
	//printf_x2(v); c = getchar();
	
	//if ((port_in(0x10) & 0x04) == 0x00) {
	
	// Set ~WR inactive (high)?
	port_out(0x10, 0x00);	// Working! Also for "anything other than 4", like 0x01, 0x02, 0x08
	
	// After OUT 0x10, 0x00:
	//	check_port(0x10);	// 0xF9
	
	// After OUT 0x10, 0x04
	//	check_port(0x10);	// 0xFF, 0xFD, 0xFF, 0xFD, ...
	
	
	// This check seems to prevent some brown-outs for me! Important?
	//while((port_in(0x10) & 0x07) == 0x05) { }
	
	// Set ~WR active (low)?
	port_out(0x10, 0x04);	// Works: Put 0x00, then 0x04, then enter data!
	
	// After OUT 0x10, 0x00:
	//	check_port(0x10);	// 0xF9
	// After OUT 0x10, 0x04:
	//	check_port(0x10);	// 0xFC (sound stopped?) -OR- 0xFD ... 0xFF ... 0xFD (sound running?)
	//	check_port(0x11);	// 0xFF
	//	check_port(0x21);	// 0xFF
	//	check_port(0x60);	// 0x0F
	//	check_port(0x62);	// 0xFF
	
	
	// TMS5220 Wait for ~READY to go low?
	//while(port_in(0x10) == 0xfd) { }	// Wait for wiggle (working!)
	while((port_in(0x10) & 0x07) == 0x05) { }	// OK
	
	// Latch data?
	port_out(0x11, v);	// Actually output data to the pins
	
	
	//check_port(0x10);	// 0xFC if sound stopped -OR- wiggling 0xFD ... 0xFF ... 0xFD
	//while(port_in(0x10) == 0xff) { }
	
	//c = port_in(0x10);
	
	// Set ~WR inactive (high) again?
	//port_out(0x10, 0x00);
	
}

void tms_flush() {
	// Manual: 100% guarantee for clean reset is to write nine bytes of "all ones" to the buffer, followed by a reset command
	
	// This will eventually read as "energy=0xF" and make the "speak external" stop
	for(int i = 0; i < 9; i++) {
		tms_put(0xff);
	}
}

void tms_reset() {
	
	printf("TMS:Reset...");
	//check_port(0x10);	// 0xFC = 0b11111100
	//check_port(0x60);	// 0x0F
	
	/*
	// Manual: 100% guarantee for clean reset is to write nine bytes of "all ones" to the buffer, followed by a reset command
	for(int i = 0; i < 9; i++) {
		tms_put(0xff);
	}
	*/
	
	// TMS: Command "Reset"
	tms_put(0xff);	// D0...D7: X111XXXX = "Reset" (e.g. 0xFF or 0x7E)
	//check_port(0x60);	// 0x0F
	//check_port(0x10);	// 0xFD = 0b11111101, 0xFF, = 0b11111111
}

void tms_speak_external() {
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
	byte d;
	byte v;
	char c;
	byte *o;
	
	printf("GL6000SL sound test\n");
	printf("\n");
	printf("!!! WARNING !!!\n");
	printf("THE SOUND CHIP CAN SINK A LOT OF CURRENT!\n");
	printf("THIS CAN DAMAGE THE DEVICE OR YOUR POWER SUPPLY!\n");
	printf("LIMIT YOUR SUPPLY CURRENT, DO NOT USE BATTERIES!\n");
	printf("CONTINUE AT YOUR OWN RISK!\n");
	c = getchar();
	printf("\n");
	
	
	// I have *NO* idea about this init sequence.
	// I just know: If I leave it in, there is more success.....
	
	// Also: To play a sound from firmware:
	// OUT 51 08; CALL 66F6
	
	// Boot:
	port_out(0x22, 0x00);
	port_out(0x21, 0xe0);
	
	//check_port(0x21);	// 0xff
	delay(0x1000);
	
	port_out(0x23, 0x60);
	port_out(0x23, 0x60);
	port_out(0x21, 0xe0);
	
	//check_port(0x21);	// 0xff
	delay(0x0800);
	
	port_out(0x61, 0xd0);
	port_out(0x10, 0x00);
	
	//check_port(0x21);	// 0xff
	port_out(0x21, 0xff);
	
	port_out(0x29, 0x01);
	port_out(0x43, 0x02);
	port_out(0x23, 0x20);
	
	delay(0x0800);
	//for (i=0; i < 3; i++) {
	//	check_port(0x21);	// 0xff
	//	check_port(0x60);	// 0x0b
	//}
	
	port_out(0x62, 0x00);
	port_out(0x10, 0x00);
	//check_port(0x10);	// 0xf9 = 0b11111001
	delay(0x1000);
	
	port_out(0x10, 0x04);
	
	//for (i=0; i < 3; i++) {
	//	check_port(0x21);	// 0xff
	//	check_port(0x60);	// 0x0F by now
	//}
	
	
	
	// End of init
	delay(0x2000);
	
	printf("End of init.\n");
	
	tms_reset();
	
	check_port(0x10);	// 0xf9 = 0b11111001
	delay(0x2000);
	
	printf("Ready.\n");
	//c = getchar();
	
	
	// Real frames?
	// spss011d.pdf, 6.1, page 180
	//	Parameter	Energy	Repeat	Pitch 	K1	K2	K3	K4	K5	K6	K7	K8	K9	K10	K11	K12
	//	# Bits		4		1		7		6	6	5	5	4	4	4	3	3	3	0	0
	// 56 bits = 7 bytes?
	
	// Speech data can be found in ROM:0x6D100+ (out 0x51,0x1b, mem[0x5100...])
	port_out(0x51, 0x1B);	// OUT 0x51, 0x1B	-> maps ROM:0x6C000 to CPU:0x4000
	//o = (byte *)0x5000;		// MEM:0x5000 now shows ROM:0x6D000 = sounds and stuff
	o = (byte *)0x513b;		// MEM:0x513B now shows ROM:0x6D13B = Jingle
	//o = (byte *)0x5141;		// MEM:0x5141 now shows ROM:0x6D141 = BOING-sound
	
	tms_speak_external();
	
	byte mon21 = 1;
	//check_port(0x10);
	
	while(true) {
		
		
		// Check TMS status
		//do {
			v = port_in(0x10);
			//printf_x2(v); c = getchar();
			
			if ((v & 0x03) == 2) {
				// hang
				printf("hang!"); c = getchar();
				
				tms_reset();
				check_port(0x10);
				continue;
			}
			
			if ((v & 0x07) == 4) {
			//if ((v & 0x03) == 0) {
				// TMS is idle!
				printf("Idle!");	//c = getchar();
				
				// Wait for key
				c = getchar();
				switch(c) {
					case 'r':
					case 'R':
						tms_reset();
						break;
					
					case 'd':
					case 'D':
						mon21 = 1-mon21;
						break;
					case '0':
						o = (byte *)0x5000;		// MEM:0x5000 now shows ROM:0x6D000 = sounds and stuff
						break;
					case '1':
						o = (byte *)0x513b;		// MEM:0x513B now shows ROM:0x6D13B = Jingle
						break;
					case '2':
						o = (byte *)0x5141;		// MEM:0x5141 now shows ROM:0x6D141 = BOING-sound
						break;
					}
				
				tms_speak_external();
				//check_port(0x10);
				continue;
			}
			/*
			if ((v & 0x03) == 1) {
				// Ready to receive data
				break;
			}
			*/
			//break;
		//} while(1);	// Wait until ready to receive data
		
		
		tms_frame++;
		d = *o;
		
		printf_x4((word)o); putchar(':');
		printf_x2(d);
		putchar(' ');
		while(port_in(0x10) != 0xfd) { }
		
		// Feed new data
		tms_put(d);
		
		
		//check_port(0x60);
		//check_port(0x10);
		
		/*
		if ((tms_frame % 0x02) == 0) {
			printf("Pause...");
			
			do {
				c = getchar();
				
				if ((c == 'r') || (c == 'R')) {
					// Output something
					tms_reset();
					tms_speak_external();
				} else {
					break;
				}
				
			} while(1);
		}
		*/
		
		if (mon21) {
			check_port(0x10);
			delay(0x100);
		} else {
			printf("\n");
			delay(0x2000);
		}
		
		
		o++;
	}
	
	
	printf("Key to end\n");
	c = getchar();
	
	//while(1) { }
	//return;
	//return c;
	return 0x42;
}
