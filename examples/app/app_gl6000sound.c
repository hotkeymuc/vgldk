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
		* Try TSP50C0x Slave-Mode:
			""
6.7 Slave Mode
Setting bit 6 of the mode register high places the TSP50C0x/1x in the slave
mode. This specialized mode is intended for applications in which the
TSP50C0x/1x device needs to be controlled by a master microprocessor.

When in slave mode, the functionality of the following ports is modified:
PB0 becomes a chip enable strobe. It is normally held high. When it is taken
low, data is read from or written to the PA0- PA7 pins depending on the value of PB1.

PB1 becomes a read/write select input. If PB1 is low, data is written to the
TSP50C0x/1x when PB0 goes low. If PB1 is high, data may be read from the
TSP50C0x/1x when PB0 goes low.

Port A becomes a general bidirectional port controlled by PB0 and PB1. Pin
PA7 is used as a busy signal. If bit 7 in the output latch is set high by the
software, PA7 of the output latch is reset to a low state when PB0 goes low to
write data to the TSP50C0x/1x.

Because the PA7 output latch is used as a busy flag, leaving only PA0- PA6
for data, normally only seven bits of data may be exchanged between the
master and the slave in any one read operation from the TSP50C0x/1x. In write
operations to the TSP50C0x/1x, all 8 pins of port A can be used to transfer data.

During read operations from the slave TSP50C0x/1x, the master is
responsible for maintaining its outputs connected to the TSP50C0x/1x port A
in a high-impedance state. Otherwise, bus contention results.
			""
		
		
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
		Port 0x10 statuses:
			* signal	0xF8	1111 1000	Short signal after OUT 0x10, 0x04 it goes "0xF9 0xF8 0xF9"
			* ready	0xF9	1111 1001	State after OUT 0x10, 0x00
			* -----	0xFA	1111 1010	---
			* -----	0xFB	1111 1011	---
			* finish	0xFC	1111 1100	State after finished playback. Buffer empty?
			* play1	0xFD	1111 1101	State while playing, it wiggles "0xFD 0xFF 0xFF ..."
			* err?	0xFE	1111 1110	"hang" state?
			* play2	0xFF	1111 1111	State at cold-boot and while playing, it wiggles "0xFD 0xFF 0xFF ..."
	
	
	2022-01-19 Bernhard "HotKey" Slawik
*/


// When using VGLDK_VARIABLE_STDIO vgldk.h will define the entry point automatically and obtain the host p_putchar/p_getchar
#include <vgldk.h>
#include <stdiomin.h>
#include <ports.h>
#include <hex.h>

//#define TMS_SHOW_DISCLAIMER	// show WARNING

//#define CHECK_FRAMES 1024
#define CHECK_FRAMES 0x400
#define CHECK_MAX_SAMPLES 8
byte check_port(byte p) {
	byte samples[CHECK_MAX_SAMPLES];
	word deltas[CHECK_MAX_SAMPLES];
	byte num_samples;
	word i;
	word t;
	byte v;
	byte v_old;
	
	num_samples = 0;
	t = 0;
	
	// Initial measurement
	v = port_in(p);
	deltas[0] = t;
	samples[num_samples++] = v;
	v_old = v;
	
	for(i = 0; i < CHECK_FRAMES; i++) {
		v = port_in(p);
		t++;
		
		if (v != v_old) {
			// Value has changed
			deltas[num_samples] = t;
			samples[num_samples++] = v;
			t = 0;	// Reset "timer"
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
		if (i > 0) {
			//putchar(',');
			
			// Timer
			putchar('+');
			
			if (deltas[i] < 0x100) printf_x2(deltas[i]);
			else printf_x4(deltas[i]);
			
			putchar('=');
		}
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

int tms_frame;
byte *tms_play_offset;
byte mon21;	// Monitor while playing?
void user_input();	// Forward



//@FIXME: I don't know any of these masks. This ist purely experimental!
#define TMS_READY_TIMEOUT 0x1fff	// Timeout while waiting for "~READY", un-set for no timeout
#define TMS_NREADY_MASK 0x02	// Which (input) bit is connected to TMS "~READY"? 2?
#define TMS_NWRITE_MASK 0x04	// Which (output) bit is connected to TMS "~WR"?
byte tms_get_ready() {
	// Is ~READY LOW?
	//return (port_in(0x10) & TMS_NREADY_MASK) == 0;
	return (port_in(0x10) & 0x07) != 0x05;
	//return (port_in(0x10) & 0x01) == 0x01;
}

byte tms_wait_ready() {
	#ifdef TMS_READY_TIMEOUT
	word t;
	t = 0;
	#endif
	
	do {
		
		#ifdef TMS_READY_TIMEOUT
		t++;
		if (t >= TMS_READY_TIMEOUT) {
			printf("TIMEOUT! ");
			check_port(0x10);
			user_input();
			return 0;
		}
		#endif
	} while (tms_get_ready() == 0);
	return 1;
}

/*
byte tms_get_playing() {
	// 0xFC = stopped

}
*/

void tms_wait_tick() {
	// After sending a command, the lowest bit wiggles (0xF8/F9 and 0xFD/FF). Is it "clock pulses"?
	byte v;
	// Pre-wait: Wait for "tick"
	v = port_in(0x10);
	if (v == 0xfd) while(port_in(0x10) != 0xff) { }
	if (v == 0xf9) while(port_in(0x10) != 0xf8) { }
}

void tms_set_write_on() {
	// Start Write Request: Set ~WR LOW
	//port_out(0x61, 0xd0);
	//port_out(0x62, 0x00);
	//port_out(0x10, 0x04);
	port_out(0x10, 0x00);
	
	//port_out(0x10,  port_in(0x10) | TMS_NWRITE_MASK);
	//port_out(0x10,  port_in(0x10) & (0xff-TMS_NWRITE_MASK));
}
void tms_set_write_off() {
	// Stop Write Request: Set ~WR HIGH
	//port_out(0x61, 0xd0);
	//port_out(0x62, 0x00);
	port_out(0x10, 0x04);
	//port_out(0x10, 0x00);
	
	//port_out(0x10,  port_in(0x10) & (0xff-TMS_NWRITE_MASK));
	//port_out(0x10,  port_in(0x10) | TMS_NWRITE_MASK);
}
void tms_set_data(byte v) {
	port_out(0x11, v);	// Actually output data to the pins
}

void tms_put_internal(byte v) {
	
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
	
	
	
	tms_set_write_on();
	
	tms_set_write_off();
	
	// Make sure we don't interrupt an ongoing transaction
	tms_wait_ready();	// This seems important
	
	// Latch data onto bus
	tms_set_data(v);
	
	
	/*
	
	port_out(0x10, 0x00);	// Works: Put 0x00, then 0x04, then enter data!
	
	// This check seems to prevent some brown-outs for me! Important?
	//while((port_in(0x10) & 0x07) == 0x05) { }
	
	port_out(0x10, 0x04);	// Works: Put 0x00, then 0x04, then enter data!
	
	
	// After OUT 0x10, 0x00:
	//	check_port(0x10);	// 0xF9, 0xF8
	// After OUT 0x10, 0x04:
	//	check_port(0x10);	// 0xFC (sound stopped) -OR- 0xFD ... 0xFF ... 0xFD (sound running?)
	//	check_port(0x11);	// 0xFF
	//	check_port(0x21);	// 0xFF
	//	check_port(0x60);	// 0x0F
	//	check_port(0x62);	// 0xFF
	*/
}

void tms_put(byte v) {
	
	// Looking into MAME's tms5110.cpp implementation, the ctrl port only takes 4 bits!
	// So maybe we push both nibbles serially?
	
	
	tms_put_internal(v);
	
	//tms_wait_clock();
	
	
	// Maybe it is only 4 bits at a time?
	// Bit-reverse? Datasheet says that D0 is the MSB!
	//byte v_reverse = (v & 0xF0) >> 4 | (v & 0x0F) << 4;	v = (v & 0xCC) >> 2 | (v & 0x33) << 2;	v = (v & 0xAA) >> 1 | (v & 0x55) << 1;
	//printf_x2(v); c = getchar();
	
	/*
	// This produces sound all the time (but not the correct one)
	tms_put_internal(((v & 0xf) << 4));
	
	//tms_wait_clock();
	
	tms_put_internal(((v >> 4) << 4));
	
	//tms_wait_clock();
	
	*/
	
	//tms_put_internal(((v & 0xf) ));
	//tms_put_internal(((v >> 4) ));
	
	
	// TMS ignores lowest bit?
	//tms_put_internal(((v & 0xf) << 1) | 0x00);
	//tms_put_internal(((v >> 4) << 1) | 0x81);
	
	// 3,0,1,2
	//tms_put_internal( ((v&8)>>3) | ((v&1)<<1) | ((v&2)<<1) | ((v&3)<<1));
	//tms_put_internal( ((v&128)>>7) | ((v&16)>>3) | ((v&32)>>3) | ((v&64)>>3));
	
	//tms_put_internal( ((v&16)>>1) | ((v&32)>>5) | ((v&64)>>5) | ((v&128)>>5));
	//tms_put_internal( ((v&1 )<<3) | ((v&2 )>>1) | ((v&4 )>>1) | ((v&8  )>>1));
	
	/*
	tms_put_internal((v & 0x0f) | (v_reverse & 0xf0));
	tms_put_internal((v & 0xf0) | (v_reverse & 0x0f));
	*/
}

void tms_flush() {
	// Manual: 100% guarantee for clean reset is to write nine bytes of "all ones" to the buffer, followed by a reset command
	
	// This will eventually read as "energy=0xF" and make the "speak external" stop
	for(int i = 0; i < 9; i++) {
		tms_put_internal(0xff);
	}
}

void tms_reset() {
	
	printf("TMS:Reset...");
	
	/*
	// Manual: 100% guarantee for clean reset is to write nine bytes of "all ones" to the buffer, followed by a reset command
	for(int i = 0; i < 9; i++) {
		tms_put(0xff);
	}
	*/
	
	// TMS: Command "Reset"
	//tms_put(0xff);	// D0...D7: X111XXXX = "Reset" (e.g. 0xFF or 0x7E)
	tms_put_internal(0xff);	// D0...D7: X111XXXX = "Reset" (e.g. 0xFF or 0x7E)
}

void tms_speak_external() {
	// TMS: Command "Speak External"
	printf("TMS:Speak...");
	//tms_put(0xE7);	// D0...D7: X110XXXX = "Speak External" (e.g. 0xE7 or 0x66)
	tms_put_internal(0xE7);	// D0...D7: X110XXXX = "Speak External" (e.g. 0xE7 or 0x66)
	
	//check_port(0x10);
	//tms_wait_clock();
}




void user_input() {
	char c;
	
	tms_frame = 1;	// Reset counter
	
	while(1) {
		printf("\n");
		printf_x4((word)tms_play_offset);
		putchar('?');
		c = getchar();
		
		switch(c) {
			
			case 'h':
				// Help
				printf("RS12PDQF\n");
				break;
			
			case ' ':	// Cont
			case '.':	// Cont
				return;
			
			
			case 0x18:	// UP
				tms_play_offset-=0x10;
				break;
			case 0x19:	// DOWN
				tms_play_offset+=0x10;
				break;
			case 0x1a:	// RIGHT
				tms_play_offset++;
				break;
			case 0x1b:	// LEFT
				tms_play_offset--;
				break;
			
			case 10:
			case 13:
				// Play for a while
				//tms_frame = 16;
				tms_frame = 8;
				return;
				break;
			
			case 's':
			case 'S':
				tms_speak_external();
				//return;
				break;
			
			case 'p':
			case 'P':
				check_port(0x10);
				check_port(0x11);
				check_port(0x60);
				check_port(0x61);
				break;
			
			case 'f':
			case 'F':
				// F** things up: This will glitch, play back a sample and reset
				port_out(0x51, 0x08);
				__asm
					call #0x66f6
				__endasm;
				break;
			
			case 'q':
			case 'Q':
				// Soft reset
				__asm
					;rst0
					call #0x0000
				__endasm;
				break;
			
			case 'r':
			case 'R':
				tms_flush();
				delay(10);
				tms_reset();
				//return;
				break;
			
			case 'd':
			case 'D':
				mon21 = 1-mon21;
				if (mon21) printf("ON"); else printf("OFF");
				//return;
				break;
			
			case '0':
				//tms_speak_external();
				tms_play_offset = (byte *)0x5000;		// MEM:0x5000 now shows ROM:0x6D000 = sounds and stuff
				//return;
				break;
			
			case '1':
				//tms_speak_external();
				tms_play_offset = (byte *)0x513b;		// MEM:0x513B now shows ROM:0x6D13B = Jingle
				//return;
				break;
			
			case '2':
				//tms_speak_external();
				tms_play_offset = (byte *)0x5141;		// MEM:0x5141 now shows ROM:0x6D141 = BOING-sound
				//return;
				break;
			
			default:
				printf_x2((byte)c);
				return;
				//break;
		}
	}
}


//void main() __naked {
//void main() {
//int main() {
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	word i;
	byte d;
	byte v;
	char c;
	
	printf("GL6000SL sound test\n");
	#ifdef TMS_SHOW_DISCLAIMER
	printf("\n");
	printf("!!! WARNING !!!\n");
	printf("THE SOUND CHIP CAN SINK A LOT OF CURRENT!\n");
	printf("THIS CAN DAMAGE THE DEVICE OR YOUR POWER SUPPLY!\n");
	printf("LIMIT YOUR SUPPLY CURRENT, DO NOT USE BATTERIES!\n");
	printf("CONTINUE AT YOUR OWN RISK!\n");
	c = getchar();
	printf("\n");
	#endif
	
	
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
	delay(0x100);
	
	port_out(0x10, 0x04);
	
	//for (i=0; i < 3; i++) {
	//	check_port(0x21);	// 0xff
	//	check_port(0x60);	// 0x0F by now
	//}
	
	
	
	// End of init
	delay(0x2000);
	
	printf("End of init.\n");
	
	//tms_reset();
	//check_port(0x10);	// 0xf9 = 0b11111001
	//delay(0x2000);
	
	printf("Ready.\n");
	//c = getchar();
	
	
	// Real frames?
	// spss011d.pdf, 6.1, page 180
	//	Parameter	Energy	Repeat	Pitch 	K1	K2	K3	K4	K5	K6	K7	K8	K9	K10	K11	K12
	//	# Bits		4		1		7		6	6	5	5	4	4	4	3	3	3	0	0
	// 56 bits = 7 bytes?
	
	// Speech data can be found in ROM:0x6D100+ (out 0x51,0x1b, mem[0x5100...])
	port_out(0x51, 0x1B);	// OUT 0x51, 0x1B	-> maps ROM:0x6C000 to CPU:0x4000
	//tms_play_offset = (byte *)0x5000;		// MEM:0x5000 now shows ROM:0x6D000 = sounds and stuff
	tms_play_offset = (byte *)0x513b;		// MEM:0x513B now shows ROM:0x6D13B = Jingle
	//tms_play_offset = (byte *)0x5141;		// MEM:0x5141 now shows ROM:0x6D141 = BOING-sound
	
	tms_speak_external();
	//delay(0x200);
	
	mon21 = 0;	//1;	// Monitor port after writing data?
	tms_frame = 0;
	//check_port(0x10);
	
	while(true) {
		
		//if ((tms_frame % tms_break_frames) == 0) {
		if (tms_frame == 0) {
			tms_frame = 1;
			//printf("P");
			user_input();
		}
		
		
		//tms_frame++;
		tms_frame--;
		
		d = *tms_play_offset;
		
		if (mon21) {
			printf_x4((word)tms_play_offset); putchar(':');
			printf_x2(d);
			putchar(' ');
		}
		
		
		// Pre-check
		v = port_in(0x10);
		if (v == 0xf9) {
			// Idle state?
			tms_speak_external();
			v = port_in(0x10);
		}
		if (v == 0xfc) {
			// Sound ended?
			tms_speak_external();
			v = port_in(0x10);
		}
		
		/*
		// Pre-wait: Wait for "tick"
		v = port_in(0x10);
		if (v == 0xfd) while(port_in(0x10) != 0xff) { }
		if (v == 0xf9) while(port_in(0x10) != 0xf8) { }
		*/
		tms_wait_tick();
		
		// Feed new data
		tms_put(d);
		//check_port(0x60);
		
		if (mon21) {
			check_port(0x10);
			//delay(0x100);
			//delay(0x400);
		} else {
			putchar('.');
			//printf("\n");
			//delay(0x800);
			//delay(0x1800);
			//for(i = 0; i < 0x1000; i++) {
			//	delay(0x1);
			//	port_in(0x10);	// Poll?
			//}
		}
		
		
		tms_play_offset++;
	}
	
	/*
	printf("Key to end\n");
	c = getchar();
	
	//while(1) { }
	//return;
	//return c;
	return 0x42;
	*/
}
