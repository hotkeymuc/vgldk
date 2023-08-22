#ifndef __MAME_H
#define __MAME_H
/*
MAME Trap Access
=============

Means of sending/receiving data from an emulated VGL to the MAME host computer.

By using a modified version of MAME we can communicate with the host.
This is especially useful when debugging serial communication.

This file creates "get" and "put" functions to send and receive bytes through an unused hardware port.
The MAME "pc2000" core intercepts this call and pipes it through to stdout where it can be piped to host applications (e.g. tools/host.py)

This requires small alterations to MAME's "src/mame/drivers/pc2000.cpp":
	* Make bank0, bank1, bank2 "read and write" in pc2000_mem():
		map(0x0000, 0x3fff).bankrw("bank0");	// Allow writing to internal ROM (for CP/M)
		map(0x4000, 0x7fff).bankrw("bank1");	// Allow writing to internal ROM (for CP/M)
		map(0x8000, 0xbfff).bankrw("bank2");    //0x8000 - 0xbfff tests a cartridge, header is 0x55 0xaa 0x59 0x45, if it succeeds a jump at 0x8004 occurs
	
	* Add port 0x13 in "pc2000_io()":
		map(0x13, 0x13).rw(FUNC(pc2000_state::debug_r), FUNC(pc2000_state::debug_w));
	
	* Add port read and write functions:
		// On some MAME versions:
		DECLARE_READ8_MEMBER( debug_r );
		DECLARE_WRITE8_MEMBER( debug_w );
		
		// alternative: uint8_t pc2000_state::debug_r() {
		READ8_MEMBER( pc2000_state::debug_r ) {
			int i;
			int rv;
			rv = scanf("%2X", &i);
			
			if (rv != 1) {
				printf("Warning: Stopped reading input due to bad value.\n");
			}
			
			return i;
		}
		
		// Alternative: void pc2000_state::debug_w(uint8_t data) {
		WRITE8_MEMBER( pc2000_state::debug_w ) {
			printf("%02X", data);
		}
		

2020-08-28 Bernhard "HotKey" Slawik
*/

#ifndef MAME_PORT
	#define MAME_PORT 0x13
#endif

__sfr __at MAME_PORT mame_port;

unsigned char mame_getchar() {
	return mame_port;
}

void mame_putchar(unsigned char c) {
	mame_port = c;
}

void mame_put(char *c, byte l) {
	byte i;
	char *pc;
	
	pc = c;
	for(i = 0; i < l; i++) {
		mame_putchar(*pc++);
	}
}
#endif //__MAME_H
