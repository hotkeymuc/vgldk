#ifndef __VGL_SOUND_H
#define __VGL_SOUND_H
/*
V-Tech Genius Leader Sound

@FIXME: Not yet implemented!


Not quite sure about the hardware, yet.
It might be a TI TMS5220 LPC speech chip.
Alexandre Botzung suggests it might be a TI TSP50C0x.


In my traces I am seeing those port accesses:
	port 0x10	(4 and 0 are written there - clock?)
	port 0x60	(checked regularly)
	port 0x62	

? There is also an interrupt (RST7 i guess) which also accesses these ports
	-> using IRQ?

Interesting memory regions that seem to be connected to sound/music:
	0xF975, 0xF976, 0xF97B, 0xF97C, 0xF97D


- - - - - - - - - - - - - - - - - - - - 
From TI TSP50C0X manual 6-44:
"
Slave Mode
Setting bit 6 of the mode register high places the TSP50C0x/1x in the slave
mode. This specialized mode is intended for applications in which the
TSP50C0x/1x device needs to be controlled by a master microprocessor.
When in slave mode, the functionality of the following ports is modified:

PB0 becomes a chip enable strobe. It is normally held high. When it is taken
low, data is read from or written to the PA0- PA7 pins depending on the value
of PB1.

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
operations to the TSP50C0x/1x, all 8 pins of port A can be used to transfer
data.

During read operations from the slave TSP50C0x/1x, the master is
responsible for maintaining its outputs connected to the TSP50C0x/1x port A
in a high-impedance state. Otherwise, bus contention results.
The TSP50C0x/1x I/O ports must be configured in input mode for slave mode
to work properly. Pin PA7 may be put in output mode, if desired. It then
functions as a handshaking line rather than a polled handshake bit.
"
- - - - - - - - - - - - - - - - - - - - 

2020-05-19 Bernhard "HotKey" Slawik
*/

void vgl_sound_off() {
__asm
	; Speaker off, as seen in GL6000SL ROM at 0x5F9B, just before HALT
	ld	a, #0xff
	out	(0x60), a
	ret
__endasm;
}

void vgl_sound(word frq, word len) {
	// Perform a beep (frq is actually a delay...)
	(void)frq;
	(void)len;
	
	//@TODO: Implement
	
}


void vgl_sound_note(word n, word len) {
	/*
	word frq;
	
	
	// My own, working great!
	switch(n % 12) {
		case 0:	frq = 0x0900;	break;
		case 1:	frq = 0x087e;	break;
		case 2:	frq = 0x0804;	break;
		case 3:	frq = 0x0791;	break;
		case 4:	frq = 0x0724;	break;
		case 5:	frq = 0x06be;	break;
		case 6:	frq = 0x065d;	break;
		case 7:	frq = 0x0601;	break;
		case 8:	frq = 0x05ab;	break;
		case 9:	frq = 0x0559;	break;
		case 10:	frq = 0x050d;	break;
		case 11:	frq = 0x04c4;	break;
	}
	
	frq = frq >> (n/12);
	len = 150 * (len / frq);	// Length to wave length, correcting for rough milliseconds
	vgl_sound(frq, len);
	*/
}

void beep() {
	//vgl_sound_note(12*4+0, 0x0111);
}
#endif // __VGL_SOUND_H