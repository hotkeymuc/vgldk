#ifndef __SPEECH_H
#define __SPEECH_H
/*
	GL6000SL Speech
	
	Even though the GL6000SL does not "talk", it surely does not use a standard 1-bit buzzer.
	Just by the sound of it (it has no IC markings), I am pretty sure that the sound chip is some sort of TI TMS51x0/52x0 compatible speech chip.
	
	Data is pushed to port 0x11 through a NMI routine, which we cannot modify as cart/app, because it is spec'ed to be at 0x0066 (ROM).
	But we can at least feed the NMI routine some data it deems useful.
	
	----
	
	After "listening" through the whole GL6000SL ROM file using an emulated LPC-10 speech decoder (e.g. Arduino/Adafruit Talkie)
	I have actually found some LPC-10 encoded sound data at ROM address 0x6D000!
	This means, the sound chip does not use its own mask ROM, but rather gets fed data from the CPU.
	
	By analyzing the firmware in Ghidra, I found an NMI service routine (0x0066 -> 0x12d6) which feeds data to port 0x11 (speech data port).
	Since the NMI is hard-wired to this zero-page address, I cannot disable it (NON-maskable), so I first try to feed it my own data.
	
	!! The internal NMI (0x0066 --> 0x12d6) accepts "standard" LPC10 speech data, like the Speek&Spell.
	!! You just need to add two ZERO-bytes at the start!!
	!! Then set the state variables at 0xf981 (start address) and 0xf983 (end address) and start the chip (write 0x04/0x00 to port 0x10)
	
	
	Be careful: The Speech chip has an internal amplifier. When hitting the wrong resonance, you can almost short the power supply!
	Believe me! The LCD can flash weirdly, the speaker can make "Oooof!"-sounds and the USB power supply can and will brown out!


2022-02-19 Bernhard "HotKey" Slawik
*/

__sfr __at 0x10 speech_port_control;
__sfr __at 0x11 speech_port_data;


// "Magic" memory addresses used by the stock GL6000SL NMI
#define SPEECH_ADDR_STATE1 0xf97a
#define SPEECH_ADDR_STATE2 0xf97e
#define SPEECH_ADDR_POS 0xf981
#define SPEECH_ADDR_END 0xf983

#define SPEECH_ASSET_BEEP_ADDR 0x5131
#define SPEECH_ASSET_BEEP_LEN 11

//#define speech_printf printf
#define speech_printf(s) ;


void speech_delay(word n) {
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

void speech_start_hardware() {
	// Sets WRITE mode, starts "auto strobing" of 0x10
	speech_port_control = 0x04;
}
void speech_stop_hardware() {
	// clear WRITE, stop strobing
	speech_port_control = 0x00;
}

void speech_put(byte d) {
	// Put a byte (4 bit) onto the speech IC data pins
	// This can only be done if the internal NMI is not already pushing data
	
	byte v;
	word timeout;
	
	// Wait for the speech chip to signal a possible write
	timeout = 0x100;
	do {
		timeout--;
		if (timeout <= 0) {
			speech_printf("SPC:TO");
			break;
			//return v;
		}
		v = speech_port_control;
	//} while ((v & 0x07) == 5);	// "0x10 & 7 == 5" is used by the stock NMI in order to decide whether to put new data on the bus
	} while ((v & 0x02) == 2);	// ...but this also works when the chip is in an unknown state
	
	// Only the upper 4 bits are connected to the data pins of the IC
	speech_port_data = d << 4;
	
	//return 0x00;	// OK
}

void speech_reset() {
	// Issue the reset sequence
	
	speech_stop_hardware();
	
	speech_delay(10);
	
	// TMS: Command "Reset"
	//speech_set_data(0x0f);
	speech_port_data = 0xff;	// D0...D7: X111xxxx = "Reset" (e.g. 0x7 or 0xF)
	
	speech_delay(10);
	
	speech_start_hardware();
	
	// Flooding the FIFO buffer with 9 0xFF's should put the chip into a known state (says the TSP manual)
	for(byte i = 0; i < 9; i++) {
		speech_put(0xf);	// D0...D7: X111xxxx = "Reset" (e.g. 0x7 or 0xF)
		//delay(1);	// delay(1) leaves out about one STOBE
	}
	speech_stop_hardware();
}

void speech_speak_external() {
	// TMS: Command "Speak External" - further bytes are speech data until energy goes down
	speech_put(0xe);	// D0...D7: X110xxxx = "Speak External" (e.g. 0x6 or 0xE)
}

/*
#define SPEECH_DATA_BITS 4
byte speech_get_data() {
	// Get next bunch of bits (take care of 4, 7 or 8 bit transcoding)
	
	byte d;
	
	#if SPEECH_DATA_BITS == 4
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
	#elif SPEECH_DATA_BITS == 7
		// 7 bits
		word dw;
		
		dw = *(word*)(speech_ofs_byte);
		d = (dw >> speech_ofs_bit) & 0x7f;
		
		speech_ofs_bit += 7;
		if (speech_ofs_bit >= 8) {
			speech_ofs_bit -= 8;
			speech_ofs_byte++;
		}
	#else
		// 8 bits
		
		d = *(byte *)(speech_ofs_byte);
		speech_ofs_byte++;
	#endif
	
	
	return d;
}
*/


void speech_play(byte *addr_start, word l) {
	// Playback the given memory region of LPC10 encoded data using the stock NMI routine
	// The data should start with two zero bytes (0x00, 0x00)
	
	// These are "magic" RAM addresses used by the stock GL6000SL [German] firmware in its NMI service routine [0x0066 --> 0x12d6]
	*(word *)SPEECH_ADDR_POS = (word)addr_start;
	*(word *)SPEECH_ADDR_END = (word)addr_start + l;	//addr_end;
	*(byte *)SPEECH_ADDR_STATE1 = 1;
	*(word *)SPEECH_ADDR_STATE2 = 0xffff;	// ?
	
	//speech_stop_hardware();
	//delay(0x100);
	
	speech_reset();
	
	speech_delay(0x800);
	
	speech_start_hardware();
	
	
	// Speech should be playing right now
	// We could check the control port to see when it's done.
	
	
	/*
	// Delay until it should be finished
	//@TODO: Check speech_port_control to see when it has finished playing
	speech_delay(0x200);
	for(word i = 0; i < l; i++) {
		speech_delay(0x110);
	}
	
	// Stop
	*(byte *)SPEECH_ADDR_STATE1 = 0;	// Set to 0 to stop playback
	*(word *)SPEECH_ADDR_STATE2 = 0x0000;	// Set LO or HI to 0x00 to stop playback
	
	speech_delay(0x1000);
	speech_stop_hardware();
	*/
}

void speech_stop() {
	// Stop
	*(byte *)SPEECH_ADDR_STATE1 = 0;	// Set to 0 to stop playback
	*(word *)SPEECH_ADDR_STATE2 = 0x0000;	// Set LO or HI to 0x00 to stop playback
	
	speech_delay(0x1000);
	speech_stop_hardware();
}
#endif	//__SPEECH_H