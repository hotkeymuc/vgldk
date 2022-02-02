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




// TSP50C0x trials
word tsp_ofs_byte;
byte tsp_ofs_bit;

// PB0 becomes a chip enable strobe. It is normally held high.
// When it is taken low, data is read from or written to the PA0- PA7 pins
// depending on the value of PB1.
void tsp_set_str_low() {
	port_out(0x10, 0x00);
	//port_out(0x10, 0x04);
}
void tsp_set_str_high() {
	port_out(0x10, 0x04);
	//port_out(0x10, 0x00);
}

#define tsp_set_rw_read tsp_set_rw_high
void tsp_set_rw_high() {
	//port_out(0x10, 0xfb);	// str HIGH and RW HIGH?
	//port_out(0x61, 0xdf);	// ???
	//port_out(0x62, 0xff);	// ???
}
#define tsp_set_rw_write tsp_set_rw_low
void tsp_set_rw_low() {
	//port_out(0x61, 0xd0);	// ???
	//port_out(0x62, 0x00);	// ???
}

byte tsp_get_pa() {
	byte v;
	v = port_in(0x10);
	//v = port_in(0x11);
	//v = port_in(0x60);
	return v;
}
void tsp_set_pa(byte v) {
	port_out(0x11, v);
}

void tsp_check_pa() {
	check_port(0x10);
	//check_port(0x11);
	//check_port(0x60);
	//check_port(0x61);
	//check_port(0x62);
}

void tsp_write(byte d) {
	byte v;
	byte v_old;
	word timeout;
	
	// Send a byte (slave-mode) according to protocol
	printf("{");
	
	// 1) The master microprocessor sets R/W high to indicate a read operation.
	tsp_set_rw_read();
	
	
	// 2) The master polls the output state of PA7 by pulsing STR (on PB0) low...
	// ...and reading the state of PA7 while STR is low.
	tsp_set_str_low();
	tsp_set_str_high();
	while((port_in(0x10) & 0x07) == 0x05) { }
	
	/*
	v_old = 0x55;	// something invalid, so first change gets put to screen
	timeout = 0x4000;
	do {
		// 2) The master polls the output state of PA7 by pulsing STR (on PB0) low...
		tsp_set_str_low();
		
		// ...and reading the state of PA7 while STR is low.
		v = tsp_get_pa();
		
		tsp_set_str_high();
		
		
		if (v != v_old) {
			printf_x2(v);
			v_old = v;
		}
		timeout--;
		
		// 3) Eventually, the TSP50C0x/1x completes processing any previous data or instructions from the master.
		// When it does, it writes a one to the PA7 output latch.
		
		// Wait until PA7=1	(0x10 0xFC --> 0xFD)
	//} while (timeout > 0);
	//} while ((v&0x01) == 0x00);
	} while (((v&0x07) == 0x05) && (timeout > 0));
	//} while (((v & 0x01) != 0x01) && (timeout > 0));
	*/
	
	// 4) When the master senses that PA7 has gone high, it sets the R/W signal low to indicate a write operation.
	tsp_set_rw_write();
	
	//5) The master presents valid data to port PA0- PA6.
	tsp_set_pa(d);
	//tsp_set_pa( (d & 0x7f) );	// Only 7 bit!
	//tsp_set_pa(((d & 0x7f) << 1) | 0x00);	// Only 7 bit!
	//tsp_set_pa( (d & 0x7f) | 0x80 );	// Only 7 bit!
	
	
	//6) The master pulses STR (on PB0) low, which causes the data on port PA0- PA6 pins to be latched to the port A input latch.
	// The TSP50C0x/1x hardware causes the PA7 output latch to be cleared to zero, indicating that the TSP50C0x/1x has accepted the data.
	
	//tsp_set_str_low();
	
	/*
	// @FIXME: Verify PA7=0
	printf("-");
	//tsp_check_pa();
	//v = tsp_get_pa();
	//printf_x2(v);
	v_old = 0x55;	// something invalid
	timeout = 0x1000;
	do {
		v = tsp_get_pa();
		if (v != v_old) {
			printf_x2(v);
			v_old = v;
		}
		timeout--;
		// Wait until PA7=0	// 0xFF to 0xFD? or 0xFC?
	//} while (((v & 0x02) == 0x02) && (timeout > 0));
	//} while (((v&0x07) != 0x07) && (timeout > 0));
	} while (((v&0x07) == 0x05) && (timeout > 0));
	*/
	
	tsp_check_pa();
	
	
	// 7) The TSP50C0x/1x polls the PA7 output latch. When it sees it go low, it knows that data is being written to the port A input latch.
	
	
	// 8) The TSP50C0x/1x polls the PB0 (STR) input line. When PB0 goes high, the write is complete, and the data in PA0 is valid.
	
	//tsp_set_str_high();
	
	
	// 9) When it is ready to accept another command, the TSP50C0x/1x writes a one to the PA7 output latch, thus starting another cycle.
	printf("}");
}


void tsp_flush() {
	// Manual: 100% guarantee for clean reset is to write nine bytes of "all ones" to the buffer, followed by a reset command
	
	// This will eventually read as "energy=0xF" and make the "speak external" stop
	for(int i = 0; i < 9; i++) {
		tsp_write(0xff);
	}
}

void tsp_reset() {
	printf("TSP:Reset...");
	
	/*
	// Manual: 100% guarantee for clean reset is to write nine bytes of "all ones" to the buffer, followed by a reset command
	for(int i = 0; i < 9; i++) {
		tsp_write(0xff);
	}
	*/
	
	// TMS: Command "Reset"
	tsp_write(0xff);	// D0...D7: X111XXXX = "Reset" (e.g. 0xFF or 0x7E)
}

void tsp_speak_external() {
	printf("TSP:Speak...");
	tsp_write(0xE7);	// D0...D7: X110XXXX = "Speak External" (e.g. 0xE7 or 0x66)
}




//void main() __naked {
//void main() {
//int main() {
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	word i;
	word dw;
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
	
	printf("Ready.\n");
	
	
	// Real frames?
	// spss011d.pdf, 6.1, page 180
	//	Parameter	Energy	Repeat	Pitch 	K1	K2	K3	K4	K5	K6	K7	K8	K9	K10	K11	K12
	//	# Bits		4		1		7		6	6	5	5	4	4	4	3	3	3	0	0
	// 56 bits = 7 bytes?
	
	// Speech data can be found in ROM:0x6D100+ (out 0x51,0x1b, mem[0x5100...])
	port_out(0x51, 0x1B);	// OUT 0x51, 0x1B	-> maps ROM:0x6C000 to CPU:0x4000
	//tsp_ofs_byte = 0x5000;		// MEM:0x5000 now shows ROM:0x6D000 = sounds and stuff
	tsp_ofs_byte = 0x513b;		// MEM:0x513B now shows ROM:0x6D13B = Jingle
	//tsp_ofs_byte = 0x5141;		// MEM:0x5141 now shows ROM:0x6D141 = BOING-sound
	
	//mon21 = 0;	//1;	// Monitor port after writing data?
	//tms_frame = 0;
	tsp_ofs_bit = 0;
	
	tsp_check_pa();
	
	tsp_speak_external();
	
	//tsp_check_pa();
	
	while(1) {
		printf("\n");
		printf_x4(tsp_ofs_byte);
		putchar('?');
		c = getchar();
		
		switch(c) {
			
			case 'S':
			case 's':
				tsp_speak_external();
				//tsp_check_pa();
				break;
			case 'R':
			case 'r':
				tsp_reset();
				tsp_check_pa();
				break;
			
			case 'q':
			case 'Q':
				// Soft reset
				__asm
					;rst0
					call #0x0000
				__endasm;
				break;
			
			case 'f':
			case 'F':
				// Call intro
				port_out(0x51, 0x09);
				__asm
					;rst0
					call #0x66f6
				__endasm;
				break;
			
			case 'h':
				// Help
				break;
			
			
			case '0':
				//tsp_speak_external();
				tsp_ofs_byte = 0x5000;		// MEM:0x5000 now shows ROM:0x6D000 = sounds and stuff
				tsp_ofs_bit = 0;
				break;
			
			case '1':
				//tsp_speak_external();
				tsp_ofs_byte = 0x513b;		// MEM:0x513B now shows ROM:0x6D13B = Jingle
				tsp_ofs_bit = 0;
				break;
			
			case '2':
				//tsp_speak_external();
				tsp_ofs_byte = 0x5141;		// MEM:0x5141 now shows ROM:0x6D141 = BOING-sound
				tsp_ofs_bit = 0;
				break;
			
			
			case ' ':	// Cont
			case '.':	// Cont
			default:
				// Play...
				
				// 8 bits
				/*
				d = *(byte *)(tsp_ofs_byte);
				tsp_ofs_byte++;
				*/
				
				// 7 bits
				dw = *(word*)(tsp_ofs_byte);
				
				d = (dw >> tsp_ofs_bit) & 0x7f;
				
				
				tsp_ofs_bit += 7;
				if (tsp_ofs_bit >= 8) {
					tsp_ofs_bit -= 8;
					tsp_ofs_byte++;
				}
				
				
				printf_x2(d);
				tsp_write(d);
				
				
				break;
		}
	}
	
}
