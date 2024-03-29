/*
	GL6000SL Speech experiments
	
	Even though the GL6000SL does not use speech samples, it surely does not employ a standard 1-bit buzzer.
	Just by the sound of it (it has no IC markings), I am pretty sure that the sound chip is some sort of TI TSP/TMS51x0/52x0 compatible speech chip.
	
	After "listening" through the whole GL6000SL ROM file using an emulated LPC-10 speech decoder (e.g. Arduino/Adafruit Talkie)
	I have actually found some LPC-10 encoded sound data at ROM address 0x6D000!
	This means, the sound chip does not use its own mask ROM, but rather gets fed data from the CPU.
	
	Not knowing how the chip is connected to the bus, I'll just push data out of some ports I recorded in MAME and hope for the best...
	
	Be careful: The Speech chip has an internal amplifier. When hitting the wrong resonance, you can almost short the power supply!
	Believe me! The LCD can flash weirdly, the speaker can make "Oooof!"-sounds and the USB power supply can and will brown out!
	
	! GL6000SL calls interrupt (IM2 offset + 0xe7) after a sound frame has played
	
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
		

	* Hardware Pins:
		TSP50C0X/1X Manual:
		|	Color 	   Function	   Pin | Pin 	Function 	Color	|
		|	----- 	   --------	   ---   --- 	-------- 	-----	|
		|	      	           	  ____   ____  	         	      	|
		|	yellow	        PA3	-| 1* \_/ 16 |-	PA4      	green 	|
		|	orange	        PA2	-| 2      15 |-	PA5      	blue  	|
		|	red   	        PA1	-| 3      14 |-	PA6      	purple	|
		|	brown 	        PA0	-| 4      13 |-	PA7      	gray  	|
		|	BLACK 	    Vss/GND	-| 5      12 |-	Vdd/5V   	RED   	|
		|	BLUE  	      ~INIT	-| 6      11 |-	DA1      	white2	|
		|	---   	       OSC1	-| 7      10 |-	PB1/DA2  	black2	|
		|	---   	       OSC2	-| 8       9 |-	PB0?     	BROWN 	|
		|	      	           	  -----------  	         	      	|

	* Tested:
		Pin	color 	function
		---	----- 	--------
		1	yellow	DATA7 / OUT 11 80
		2	orange	DATA6 / OUT 11 40
		3	red   	DATA5 / OUT 11 20
		4	brown 	DATA4 / OUT 11 10
		(5	BLACK 	(GND)
		(6	BLUE  	(~RESET: HIGH, LOW on RESET with slow rise)
		(7	---   	?OSC?)
		(8	---   	?OSC2?)
		(9	BROWN 	(HIGH always)
		(10	black2	(HIGH always)
	!	11	white2	!!! HIGH in my tests	==> When music: there is heavy activity! (HIGH with bundles of bursts of LOWs)
						=> Buffer state?
		(12	red   	(+5V)
	!	13	gray  	!!! LOW; Goes HIGH when starting to speak with "OUT 11 D6; OUT 10 04"; Goes LOW when resetting speak, with HIGH bursts starting between two short HIGH bursts of #16 (both go LOW at the same time in the end) (IN 10 = 0xFD 11111101)
						=> BUSY/"PA7", because for a write-transactions, it should go HIGH between STROBES (see manual), which this does!
	!	14	purple	!!! HIGH in my tests	===> When music: There are some LOW signals (rather long and slow).
						=> Interrupt? READY? Queue full?
	!	15	blue  	!!! After OUT 10 00: LOW  (IN 10 = 0xF9 11111001); After OUT 10 04: Goes HIGH FIRST (IN 10 = 0xFC 11111100)
						=> WRITE/read
	!	16	green 	!!! After OUT 10 00: HIGH (IN 10 = 0xF9 11111001); After OUT 10 04: Goes LOW 43us AFTER #15 GOT HIGH (IN 10 = 0xFD 11111101), after 115us it strobes HIGH for 4us, sometimes two bursts (and #13 goes HIGH between them!)
						=> STROBE?
		
		My Version:
			|    My Color     Function       Pin | Pin     Function     My Color |
			|    --------     --------       ---   ---     --------     -------- |
			|                               ____   ____                          |
			|    yellow    P11#7   PA3    -| 1* \_/ 16 |-  STROBE?      green    |
			|    orange    P11#6   PA2    -| 2      15 |-  RW?          blue     |
			|    red       P11#5   PA1    -| 3      14 |-  READY?INT    purple   |
			|    brown     P11#4   PA0    -| 4      13 |-  BUSY/PA7?    gray     |
			|    BLACK         Vss/GND    -| 5      12 |-  Vdd/5V       RED      |
			|    BLUE            ~INIT    -| 6      11 |-  QUEUE?       white2   |
			|    ---              OSC1    -| 7      10 |-  ?HIGH?       black2   |
			|    ---              OSC2    -| 8       9 |-  ?HIGH?       BROWN    |
			|                               -----------                          |
		
		==> Data is 4bit only, packed into the MSBs (PA4-PA7) of port 0x11 (that's why the command bits are all happening in the upper nibble)
		
		==> When doing "OUT 10 00; OUT 10 04" there is "communication" going on on pins #13 and #15 for several 100ms!
			* Initial bursts, then some "busy waiting" state?
			* Does the VTech have some sort of "auto strobe" feature?
		
		* Call intro:
			OUT 51 09; CALL 66F6
		
		
	
	* Port log:
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
//#include <ports.h>
#include <hex.h>

//#define TMS_SHOW_DISCLAIMER	// show WARNING

__sfr __at 0x10 speech_port_control;
__sfr __at 0x11 speech_port_data;

__sfr __at 0x51 bank_port_51;



//const byte spAFFIRMATIVE[180] = {0xA5,0x4F,0x7A,0xD3,0x3C,0x5A,0x8F,0xAE,0xC8,0xA9,0x70,0xED,0xBD,0xBA,0x2A,0x3B,0xC3,0xD9,0x8F,0x00,0x6C,0x4A,0x21,0x40,0xD3,0xCA,0x08,0x18,0xC2,0x04,0x01,0xC3,0x86,0x11,0x60,0xDA,0x4C,0x05,0x54,0x53,0xDA,0x9C,0x58,0x16,0xED,0xC8,0xEB,0x88,0xE2,0x4C,0xEC,0xC1,0x36,0x23,0xC8,0x65,0xD1,0x17,0xBA,0xB4,0x20,0xE5,0xE4,0x6A,0x8A,0x53,0xA2,0xAC,0x0B,0x73,0x38,0xC9,0xC8,0xB2,0x68,0xCE,0x92,0x24,0x33,0x5B,0x45,0xB1,0xA9,0x11,0xB6,0x6A,0x75,0x4D,0x96,0x98,0xC7,0xAA,0xD6,0x37,0x91,0xEC,0x12,0xAF,0xC8,0xD1,0xB1,0x88,0x97,0x25,0x76,0xC0,0x96,0x22,0x01,0xF8,0x2E,0x2C,0x01,0x53,0x99,0xAD,0xA1,0x7A,0x13,0xF5,0x7A,0xBD,0xE6,0xAE,0x43,0xD4,0x7D,0xCF,0xBA,0xBA,0x0E,0x51,0xF7,0xDD,0xED,0x6A,0xB6,0x94,0xDC,0xF7,0xB4,0xB7,0x5A,0x57,0x09,0xDF,0x9D,0xBE,0x62,0xDC,0xD4,0x75,0xB7,0xFB,0xAA,0x55,0x33,0xE7,0x3E,0xE2,0x2B,0xDC,0x5D,0x35,0xFC,0x98,0xAF,0x79,0x0F,0x0F,0x56,0x6D,0xBE,0xE1,0xA6,0xAA,0x42,0xCE,0xFF,0x03};

// Captured with Arduino VTechGL6000SLSoundBusInspector.ino
//const byte recMEEP[109] = {	0x60, 0xD9, 0x2B, 0x7C, 0xD5, 0xF9, 0xD6, 0x2B, 0x85, 0xA5, 0xF1, 0xB6, 0x4D, 0xA8, 0xDA, 0xF3, 0x70, 0x98, 0xD8, 0xBC, 0xE3, 0x4C, 0x90, 0xB8, 0x18, 0x14, 0xE2, 0xBC, 0x61, 0x46, 0xF6, 0x5D, 0x6E, 0x5A, 0xEB, 0x26, 0x3A, 0xFB, 0x9B, 0xAD, 0x05, 0x13, 0xD1, 0xC3, 0x5E, 0x61, 0xE1, 0xDA, 0x79, 0xD5, 0xAC, 0x06, 0x53, 0x13, 0x5C, 0xAB, 0x5C, 0xE3, 0xBF, 0x12, 0xB6, 0xB2, 0x73, 0xDA, 0xB9, 0xDF, 0x2C, 0x2A, 0xC4, 0xE2, 0xA4, 0x0A, 0xF6, 0xA3, 0xA7, 0x45, 0x93, 0xC8, 0xE4, 0x5E, 0x49, 0xFC, 0x67, 0x65, 0xE2, 0x32, 0x67, 0x81, 0xD8, 0xB4, 0x93, 0xF9, 0xDE, 0xD3, 0xDE, 0x2B, 0x9B, 0xE3, 0xD2, 0x65, 0x91, 0x8B, 0xC8, 0x6D, 0x4E, 0x03, 0x8A, 0x1A, 0x1A };
//const byte recMEEP[102] = {	0xE7, 0x0A, 0xB1, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE1, 0x6A, 0xBC, 0x3F, 0x62, 0xAB, 0xAB, 0xF1, 0xCE, 0xD8, 0xAB, 0x18, 0xAE, 0xC6, 0x3B, 0x63, 0xAF, 0xA2, 0xB1, 0xAE, 0xF8, 0xCB, 0xD8, 0xAE, 0x6A, 0xBC, 0x3F, 0x62, 0xAB, 0xAB, 0xF1, 0xCE, 0xD8, 0xAB, 0xE8, 0xAE, 0xC6, 0x3B, 0x63, 0xAF, 0xA2, 0xB1, 0xAE, 0xF8, 0xCB, 0xD8, 0xAE, 0x16, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x80, 0xF0, 0xC7, 0x59, 0x32, 0x46, 0x36, 0x5F	};
//const byte recBOING[572] = {	0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE1, 0x6A, 0xBC, 0x3F, 0x62, 0xAB, 0xAB, 0xF1, 0xCE, 0xD8, 0xAB, 0x18, 0xAE, 0xC6, 0x3B, 0x63, 0xAF, 0xA2, 0xB1, 0xAE, 0xF8, 0xCB, 0xD8, 0xAE, 0x6A, 0xBC, 0x3F, 0x62, 0xAB, 0xAB, 0xF1, 0xCE, 0xD8, 0xAB, 0xE8, 0xAE, 0xC6, 0x3B, 0x63, 0xAF, 0xA2, 0xB1, 0xAE, 0xF8, 0xCB, 0xD8, 0xAE, 0x16, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x80, 0xF0, 0xC7, 0x59, 0x32, 0x46, 0x36, 0x5F, 0xE7, 0x0B, 0x08, 0xAE, 0x16, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x63, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xEF, 0x8C, 0xBD, 0x8A, 0xE6, 0xAB, 0xC3, 0xF6, 0x2A, 0xBA, 0xBF, 0x1C, 0xED, 0x8A, 0xBE, 0x8A, 0xEC, 0x62, 0xB6, 0x3A, 0xFA, 0x2B, 0x1A, 0xF8, 0xC5, 0xD8, 0xAE, 0x6A, 0xBC, 0x32, 0xB6, 0x29, 0xBA, 0xCB, 0x17, 0x4B, 0xF0, 0xAB, 0x18, 0xAE, 0xC6, 0x3B, 0x63, 0xAF, 0xA2, 0x06, 0xDE, 0x92, 0x5B, 0xC7, 0xDC, 0xD2, 0x58, 0xFD, 0x9C, 0xD5, 0x67, 0x29, 0xBA, 0x8B, 0x58, 0xA7, 0xAD, 0x5A, 0xF4, 0x18, 0xBA, 0x6F, 0x4C, 0xD8, 0xA8, 0x2D, 0xA5, 0xA2, 0xFC, 0x3E, 0x39, 0x76, 0x0E, 0x91, 0x8C, 0xDA, 0x81, 0xBF, 0xC2, 0xEB, 0x3F, 0x43, 0xC6, 0x97, 0x0A, 0xBE, 0x8B, 0x1F, 0x1B, 0x8D, 0x1F, 0x48, 0xEF, 0x28, 0xB9, 0xC1, 0x6D, 0x12, 0x4D, 0x60, 0xF7, 0x6E, 0x53, 0xD3, 0x6E, 0x25, 0xAC, 0xEA, 0xB6, 0x28, 0x6D, 0x3A, 0x31, 0xA3, 0xFA, 0xB6, 0x92, 0x98, 0xB6, 0xAB, 0xD6, 0x01, 0x53, 0x13, 0x6D, 0x51, 0x9C, 0x93, 0xD5, 0x8E, 0x96, 0x46, 0x71, 0x7E, 0x13, 0xD1, 0xA1, 0xAD, 0x7F, 0x98, 0xDE, 0x57, 0xA4, 0xA9, 0xCE, 0x06, 0xB5, 0xA3, 0x61, 0x71, 0xA3, 0xE5, 0xDC, 0xA7, 0xBD, 0x5B, 0xC8, 0xEF, 0x32, 0xBE, 0xFB, 0x15, 0x2A, 0xBC, 0x6C, 0xB7, 0x25, 0x75, 0x3E, 0xD3, 0xA2, 0xBC, 0xB1, 0x92, 0xDA, 0xFE, 0x2E, 0xC2, 0x7A, 0xC3, 0x48, 0xEF, 0x26, 0x2D, 0xAE, 0x4F, 0x03, 0xAC, 0xA4, 0xF6, 0x9A, 0xC3, 0xE3, 0x0A, 0x37, 0xD4, 0x75, 0x29, 0x89, 0xA3, 0xAC, 0x18, 0xAE, 0xA4, 0xD5, 0x8E, 0x74, 0x9F, 0xDC, 0x26, 0x97, 0xC6, 0x5E, 0x52, 0xD3, 0x12, 0x64, 0x7D, 0x8D, 0x1D, 0x98, 0x4B, 0x54, 0x69, 0x32, 0xFE, 0x96, 0xD4, 0xE8, 0xDC, 0x37, 0xDE, 0x32, 0x3B, 0xD9, 0x6B, 0x2E, 0x8E, 0x43, 0x5D, 0xE2, 0x63, 0x5A, 0x5A, 0x97, 0x1A, 0x8B, 0xDC, 0xE8, 0xB6, 0x2D, 0x40, 0xEC, 0x03, 0x18, 0x0A, 0x21, 0x0A, 0x21, 0x0A, 0x21	};


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




// Speech stuff
word speech_ofs_byte;
byte speech_ofs_bit;

byte speech_playing;
byte speech_data_next;

// VTech custom TSPish interface
void speech_start() {
	// Sets WRITE mode, starts "auto strobing"
	//port_out(0x10, 0x04);
	speech_port_control = 0x04;
}
void speech_stop() {
	// clear WRITE, stop strobing
	//port_out(0x10, 0x00);
	speech_port_control = 0x00;
}
/*
void speech_set_data(byte d) {
	// Data can only be fed during signal
	
	// Data is contained in the highest 4 bits of port 0x11
	port_out(0x11, d << 4);
	//port_out(0x11, (d << 4) | (0x0f));	// Don't know what happens to the lower bits...
}
*/
//#define speech_set_data(d) port_out(0x11, d << 4)
//#define speech_set_data(dd) speech_port_data = (dd << 4)

/*
void speech_wait_busy() {
	// Wait for just the right moment...
	byte v;
	word timeout;
	
	timeout = 0x800;
	do {
		
		timeout--;
		if (timeout <= 0) {
			printf("TimeOut");
			return;	// false
		}
		
		v = port_in(0x10);
		
	} while ((v & 0x02) == 0);	// 0x02 == BUSY flag?
	return;	// true
}
*/

byte speech_put(byte d) {
	//speech_wait_busy();
	byte v;
	word timeout;
	
	speech_port_data = d << 4;
	
	timeout = 0x100;
	// Wait for queue to be empty?
	do {
		timeout--;
		if (timeout <= 0) {
			printf("TimeOut0");
			break;
			//return v;
		}
		v = speech_port_control;
	} while ((v & 0x02) == 2);
	
	
	
	//speech_port_control = 0x0f;
	/*
	
	// Wait for latch?
	timeout = 0x2000;
	do {
		timeout--;
		if (timeout <= 0) {
			printf("TimeOut1");
			return v;
		}
		v = speech_port_control;
	} while ((v & 0x02) > 0);
	*/
	
	
	/*
	// Wait for line to be clear
	do {
		timeout--;
		if (timeout <= 0) {
			printf("TimeOut2");
			return v;
		}
		v = speech_port_control;
	} while ((v & 0x02) == 2);
	*/
	
	// Latch data for next strobe
	// Data pins are in high nibble of the data port 0x11
	//speech_set_data(d);
	//speech_port_data = d << 4;
	//speech_port_data = (d << 4) | 0x0f;
	//speech_start();
	

	
	//speech_stop();
	
	
	/*
	do {
		timeout--;
		if (timeout <= 0) {
			printf("TimeOut3");
			return v;
		}
		v = speech_port_control;
	} while ((v & 0x02) == 0);	
	do {
		timeout--;
		if (timeout <= 0) {
			printf("TimeOut4");
			return v;
		}
		v = speech_port_control;
	} while ((v & 0x02) > 0);
	*/
	
	/*
	// Wait for right moment
	do {
		
		timeout--;
		if (timeout <= 0) {
			printf("TimeOut2");
			return v;	// false
		}
		//v = port_in(0x10);
		v = speech_port_control;
	} while ((v & 0x02) == 0);	// 0x02 == BUSY flag: while ((v & 0x02) == 0)
	//} while ((v & 0x06) == 4);	// 0x02 == BUSY flag?
	//} while (((v & 0x02) == 0) || ((v & 0x04) == 0));	// 0x02 == BUSY flag? 0x04 == QUEUE FULL?
	//} while ((v & 0x07) == 5);	// 0x02 == BUSY flag? 0x04 == QUEUE FULL?
	*/
	
	//speech_set_data(d);
	//v = speech_port_data;
	
	/*
	do {
		
		timeout--;
		if (timeout <= 0) {
			printf("TimeOut");
			return;	// false
		}
		
		v = speech_port_control;
		//v = port_in(0x10);
		
	} while ((v & 0x02) == 0);	// 0x02 == BUSY flag?
	*/
	
	return 0x00;	// OK
}

void speech_reset() {
	// TMS: Command "Reset"
	speech_playing = 0;
	
	speech_stop();
	
	delay(10);
	
	//speech_set_data(0x0f);
	speech_port_data = 0xff;	// D0...D7: X111xxxx = "Reset" (e.g. 0x7 or 0xF)
	
	delay(10);
	
	speech_start();
	
	for(byte i = 0; i < 9; i++) {
		speech_put(0xf);	// D0...D7: X111xxxx = "Reset" (e.g. 0x7 or 0xF)
		//delay(1);	// delay(1) leaves out about one STOBE
	}
	speech_stop();
}

void speech_speak_external() {
	// TMS: Command "Speak External"
	//speech_set_data(0xe);	// D0...D7: X110xxxx = "Speak External" (e.g. 0x6 or 0xE)
	speech_put(0xe);	// D0...D7: X110xxxx = "Speak External" (e.g. 0x6 or 0xE)
}

byte speech_get_data() {
	// Get next bunch of bits (take care of 4, 7 or 8 bit)
	byte d;
	
	// 8 bits
	/*
	d = *(byte *)(speech_ofs_byte);
	speech_ofs_byte++;
	*/
	
	/*
	// 7 bits
	word dw;
	
	dw = *(word*)(speech_ofs_byte);
	d = (dw >> speech_ofs_bit) & 0x7f;
	
	speech_ofs_bit += 7;
	if (speech_ofs_bit >= 8) {
		speech_ofs_bit -= 8;
		speech_ofs_byte++;
	}
	*/
	
	
	// 4 bits
	d = *(byte*)(speech_ofs_byte);
	
	// Reverse bits in byte
	//d = (d & 0xF0) >> 4 | (d & 0x0F) << 4;
	//d = (d & 0xCC) >> 2 | (d & 0x33) << 2;
	//d = (d & 0xAA) >> 1 | (d & 0x55) << 1;
	
	// Invert
	//d = d ^ 0xff;
	
	// Get 4 bits
	//d = (d >> speech_ofs_bit) & 0x0f;	// Low nibble first
	d = (d >> (4-speech_ofs_bit)) & 0x0f;	// High nibble first
	
	// Reverse 4 bits (inside nibble)
	//d = ((d & 0x01) << 3) | ((d & 0x02) << 1) | ((d & 0x04) >> 1) | ((d & 0x08) >> 3);
	
	// Shift bits: 3 0 1 2
	//d = ((d & 0x08) >> 3) | ((d & 0x01) << 1) | ((d & 0x02) << 1) | ((d & 0x04) << 1) ;
	// Shift bits: 2 1 0 3
	//d = ((d & 0x04) >> 2) | (d & 0x02) | ((d & 0x01) << 2) | (d & 0x08);
	
	
	speech_ofs_bit += 4;
	if (speech_ofs_bit >= 8) {
		speech_ofs_bit -= 8;
		speech_ofs_byte++;
	}
	
	return d;
}


void speech_play(word o, word l) {
	byte r;
	
	printf("Playing "); printf_x4(o); printf("...");
	
	//speech_stop();
	//delay(0x100);
	
	speech_reset();
	
	//delay(0x800);
	
	speech_ofs_byte = o;
	speech_ofs_bit = 0;
	speech_data_next = speech_get_data();
	speech_playing = l*2;	// 4 bit! We need to play tiwce as long to use up all bytes
	
	//delay(0x800);
	
	speech_start();
	
	// My recorded data already starts with "E", so this is only needed if I play my own data
//	speech_speak_external();
	
	/*
	//delay(0x100);
	
	l *= 2;	// 4 bit! We need to play tiwce as long to use up all bytes
	
	while(l > 0) {
		
		r = speech_put(speech_data_next);
		
		//putchar('.');
		//printf_x2(speech_data_next);
		//putchar(hexDigit(speech_data_next & 0x0f));
		
		if (r != 0x00) {
			// something went wrong!
			printf("E="); printf_x2(r); printf("!\n");
			break;
		}
		
		speech_data_next = speech_get_data();
		
		l--;
	}
	
	for(byte i = 0; i < 9; i++) {
		speech_put(0xf);	// D0...D7: X111xxxx = "Reset" (e.g. 0x7 or 0xF)
	}
	*/
	
	delay(0x4000);
	speech_playing = 1;
	delay(0x1000);
	
	speech_stop();
	
	
}



word dummy_int_count;
void dummy_isp() __naked {
	__asm
	
	; Slow prolog
	push af
	push hl
	
	;; Quick prolog
	;ex af, af'	;'
	;exx
	
	
	; Increment int_count
	ld hl, (_dummy_int_count)
	inc hl
	ld (_dummy_int_count), hl
	
	
	; Slow epilog
	;ei
	pop hl
	pop af
	ei
	
	;; Quick epilog
	;exx
	;ex af, af'	;'
	;ei
	
	ei
	reti
	__endasm;
}


word timer_count;
void timer_isp() __naked {
	__asm
	
	; Slow prolog
	;push af
	;push hl
	
	;; Quick prolog
	ex af, af'	;'
	exx
	
	
	; Increment timer_count
	ld hl, (_timer_count)
	inc hl
	ld (_timer_count), hl
	
	
	; Slow epilog
	;;ei
	;pop hl
	;pop af
	;ei
	
	;; Quick epilog
	exx
	ex af, af'	;'
	ei
	
	reti
	__endasm;
}

word speech_int_count;
void speech_isp() __naked {
	__asm
	
	; Slow prolog
	push af
	push hl
	
	;; Quick prolog
	;ex af, af'	;'
	;exx
	
	; Increment int_count2
	ld hl, (_speech_int_count)
	inc hl
	ld (_speech_int_count), hl
	
	__endasm;
	
	
	speech_port_data = speech_data_next << 4;
	
	speech_playing--;
	if (speech_playing > 0) {
		//speech_put(speech_data_next);
		speech_data_next = speech_get_data();
	} else {
		speech_stop();
		printf("STOP");
	}
	
	__asm
	
	
	; Slow epilog
	;ei
	pop hl
	pop af
	ei
	
	;; Quick epilog
	;exx
	;ex af, af'	;'
	;ei
	
	reti
	__endasm;
}

//void main() __naked {
//void main() {
//int main() {
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	word i;
	//word dw;
	//byte d;
	//byte v;
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
	
	/*
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
	
	//port_out(0x10, 0x04);	// Start speech?
	
	//for (i=0; i < 3; i++) {
	//	check_port(0x21);	// 0xff
	//	check_port(0x60);	// 0x0F by now
	//}
	
	
	
	// End of init
	delay(0x2000);
	*/
	printf("Ready.\n");
	
	// The speech chip may/seems to take advantage of INTs...
	
	// Let's try IM2
	#define INTTBL 0xd000
	
	// Prepare interrupt table for IM2
	word *itp = (word *)INTTBL;
	
	// For odd jump numbers this has to be shifted by one byte, since addresses are 16bit wide:
	itp = (word *)((word)itp + 1);
	// Fill up with dummy int handler
	for(i = 0; i < 0x100; i++) {
		*itp = (word)&dummy_isp;
		itp++;
	}
	dummy_int_count = 0;
	
	// Special interrupt handlers
	
	// 0xf7 = timer interrupt, ~128 Hz
	*(word *)(INTTBL + 0xf7) = (word)&timer_isp;
	timer_count = 0;
	
	// 0xe7 = speech interrupt, called for new data
	*(word *)(INTTBL + 0xe7) = (word)&speech_isp;
	speech_int_count = 0;
	
	
	__asm
		di
		im 2
		ld a, #0xd0
		ld i,a
		ei
	__endasm;
	
	
	// Real frames?
	// spss011d.pdf, 6.1, page 180
	//	Parameter	Energy	Repeat	Pitch 	K1	K2	K3	K4	K5	K6	K7	K8	K9	K10	K11	K12
	//	# Bits		4		1		7		6	6	5	5	4	4	4	3	3	3	0	0
	// 56 bits = 7 bytes?
	
	// Speech data can be found in ROM:0x6D100+ (out 0x51,0x1b, mem[0x5100...])
	//port_out(0x51, 0x1B);	// OUT 0x51, 0x1B	-> maps ROM:0x6C000 to CPU:0x4000
	bank_port_51 = 0x1b;
	
	//speech_ofs_byte = 0x5000;		// MEM:0x5000 now shows ROM:0x6D000 = sounds and stuff
	//speech_ofs_byte = 0x513b;		// MEM:0x513B now shows ROM:0x6D13B = Jingle
	//speech_ofs_byte = 0x5141;		// MEM:0x5141 now shows ROM:0x6D141 = BOING-sound
	//speech_ofs_byte = 0x5274;	//, l=0x20 + 20	# 6D274 = Hello (???)
	//speech_ofs_byte = 0x52F4;	//, l=0x40 + 20	# 6D2F4+- = Reverb / delete-sound
	//speech_ofs_byte = 0x55E6;	//, l=0x30 + 40	# 6D5E6 = mewewew
	speech_ofs_byte = 0x55b2;	//0x6d5b2 = Meep!
	
	// Found on Bus and ROM: 0x6D637 - 6D73E: Beauauauauauau
	speech_ofs_byte = 0x5637;	//, 264);	//, 6d637 = Beauauauau
	
	//mon21 = 0;	//1;	// Monitor port after writing data?
	//tms_frame = 0;
	speech_ofs_bit = 0;
	speech_data_next = speech_get_data();
	speech_playing = 0;
	
	int o = 0;	// play offset
	
	//delay(0x1000);
	// Start playing right away
	//speech_play(speech_ofs_byte, 0x30);
	
	byte running = 1;
	while(running) {
		
		printf("ints=");
		printf_x4(dummy_int_count);
		printf(",");
		//printf_x4(timer_int_count);
		printf_x4(speech_int_count);
		printf("\n");
		
		//printf_x4(speech_ofs_byte);
		putchar('?');
		c = getchar();
		
		switch(c) {
			case 13:
			case 10:
				speech_playing = 0x09;
				break;
			
			case 'P':
			case 'p':
				speech_start();
				speech_speak_external();
				speech_playing = 0x08;
				break;
			
			case 'T':
			case 't':
				speech_put(0xff);
				break;
			case 'Z':
			case 'z':
				speech_put(0x00);
				break;
			case 'U':
			case 'u':
				speech_put(0xff);
				speech_put(0xff);
				//delay(0x200);
				speech_put(0xff);
				speech_put(0xff);
				break;
			case 'I':
			case 'i':
				speech_put(0xff);
				speech_put(0x00);
				speech_put(0xff);
				speech_put(0x00);
				break;
			
			case 'S':
			case 's':
				speech_start();
				break;
			case 'D':
			case 'd':
				speech_stop();
				break;
			
			case 'R':
			case 'r':
				//speech_start();
				//speech_stop();
				speech_reset();
				//tsp_reset();
				//tsp_check_pa();
				break;
			
			case 'q':
			case 'Q':
				// Soft reset
				__asm
					;rst0
					call #0x0000
				__endasm;
				break;
			case 'x':
			case 'X':
				// Exit
				__asm
					;rst0
					;call #0x8002
					;return
				__endasm;
				running = 0;
				break;
			
			case 'f':
			case 'F':
				// Call intro
				//port_out(0x51, 0x09);
				bank_port_51 = 0x09;
				__asm
					;rst0
					call #0x66f6
				__endasm;
				break;
			
			case 'h':
				// Help
				break;
			
			
			case 0x1b:	// LEFT
				o --;
				break;
			case 0x1a:	// RIGHT
				o ++;
				break;
			case '0':
				//tsp_speak_external();
				//speech_ofs_byte = 0x5000;	
				//speech_ofs_bit = 0;
				//speech_data_next = speech_get_data();
				//speech_play(0x5000, 0x80);	// MEM:0x5000 now shows ROM:0x6D000 = sounds and stuff
				//speech_play((word)(&spAFFIRMATIVE), sizeof(spAFFIRMATIVE));
				//speech_play((word)(&recMEEP), sizeof(recMEEP));
				break;
			
			case '1':
				//speech_play(0x513a+o, 0x10);	// MEM:0x513B now shows ROM:0x6D13B = Jingle
				//speech_play(0x5153+o, 0x10);	// 5153 = key stroke sound or just random?
				//speech_play(0x5121+o, 0x10);	// 5121 = key stroke sound?
				//speech_play((word)(&recBOING), sizeof(recBOING));
				break;
			
			case '2':
				speech_play(0x5141+o, 0x60);	// MEM:0x5141 now shows ROM:0x6D141 = BOING-sound
				break;
			case '3':
				speech_play(0x5274+o, 0x60);	//, l=0x20 + 20	# 6D274 = Hello (???)
				break;
			case '4':
				speech_play(0x52F4+o, 48);	//, l=0x40 + 20	# 6D2F4+- = Reverb / delete-sound
				break;
			case '5':
				speech_play(0x55E6+o, 0x70);	//, l=0x30 + 40	# 6D5E6 = mewewew
				break;
			
			case '6':
				// Actual 1:1 raw BUS data found!
				speech_play(0x55b2, (0x5637-0x55b2));	// 0x6d5b2 = Meep!
				break;
			case '7':
				// Actual 1:1 raw BUS data found!
				speech_play(0x5637, 264);	//, 6d637 = Beauauauau
				break;
			
			
			case ' ':	// Cont
			case '.':	// Cont
				speech_playing = 0x01;
				break;
				
			default:
				// ?
				printf_x2(c); putchar('?');
				//speech_data_next = speech_get_data();
				//speech_put(speech_data_next);
				break;
		}
	}
	
	return 42;
}
