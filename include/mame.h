#ifndef __MAME_H
#define __MAME_H
/*
MAME Trap Access
=============

By using a modified version of MAME we can communicate with the host.
This is especially useful when debugging serial communication.

Use this patch for MAME (mame.git/src/mame/drivers/pc2000.cpp):
	DECLARE_READ8_MEMBER( debug_r );
	DECLARE_WRITE8_MEMBER( debug_w );
	
	READ8_MEMBER( pc2000_state::debug_r )
	{
		int i;
		int rv;
		rv = scanf("%2X", &i);
		
		if (rv != 1) {
			//printf("Warning: Stopped reading input due to bad value.\n");
		}
		
		return i;
	}
	
	WRITE8_MEMBER( pc2000_state::debug_w )
	{
		printf("%02X", data);
	}
	
	map(0x13, 0x13).rw(FUNC(pc2000_state::debug_r), FUNC(pc2000_state::debug_w));


2020-08-28 Bernhard "HotKey" Slawik
*/

#define MAME_PORT 0x13

__sfr __at MAME_PORT mame_port;

char mame_getchar() {
	return mame_port;
}

void mame_putchar(char c) {
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
