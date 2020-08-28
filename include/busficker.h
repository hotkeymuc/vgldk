#ifndef __BUSFICKER_H
#define __BUSFICKER_H
/*

Bus Ficker
==========

Functions for accessing the "Bus Ficker" cartridge I/O ports.

Since the hardware is still kinda flaky, it is recommended to employ some sort of error detection/checksums for now.
(Reason: The ~F0_RD and ~F0_WR on the "Bus Ficker" cartridge Rev.C are not clean and sometimes trigger twice/not at all)

See "busbuddy.h" for an application of this.

Structure:
	* bf_xxx functions do the hardware communication with the "Bus Ficker" cartridge (i.e. memory mapped I/O)
	* bb_xxx functions run on top of that, providing high-level access to the "Bus Buddy" Arduino sketch
	* V-Tech sends a "frame" to the memory mapped I/O, this data is parsed (checksum) and handled by the Arduino.
	* The Arduino answers to commands and re-transmits the last answer if so requested (checksum mismatch etc.)
	* If events should be fired, they need to be acively polled as there are no interrupts on the V-Tech side (yet?)

2019-07-27 Bernhard "HotKey" Slawik
*/


// Low-level stuff
#define BF_MMIO_ADDRESS (0x8000 + 0x2000)
//#define BB_DEBUG_TRAFFIC Show each payload character as it comes in frrom the bus

void bf_init() {
	// Switch some high address lines high to activate the BUSFICKER function decoder
	
	//@FIXME: Caution! "out (3),0x81" will crash the MAME emulation
	__asm
		push af
		
		ld a, #0x81
		out	(3), a
		
		pop af
	__endasm;
}

byte bf_getByte() {
	// Get the byte currently at BusFicker F0_IN
	return *((byte *)(BF_MMIO_ADDRESS));
}
void bf_putByte(byte v) {
	// Output a byte to BusFicker F0_OUT
	*(byte *)BF_MMIO_ADDRESS = v;
}
/*
void bf_put_forceful(byte v) {
	// Don't know the current MMIO address? No problem! :-D
	word aw;
	
	for(aw = 0x8000; aw < 0xc000; aw++) {	// HELLO!!! SOMEONE THERE?!?!? *KNOCK!*KNOCK!*KNOCK!*KNOCK!*
		*(byte *)(aw) = v;
	}
}
*/


#endif //__BUSFICKER_H