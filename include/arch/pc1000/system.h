#ifndef __PC1000_H
#define __PC1000_H
/*
Header for
	VTech PreComputer 1000
	YENO Mister X
	(CompuSavant?)




for use with SDCC compiler

2020-01-21 by Bernhard "HotKey" Slawik
*/

//#define ARCH PC1000

#define DISPLAY_COLS 20
#define DISPLAY_ROWS 1

volatile __at (0xdce0) unsigned char KEY_STATUS;	// Controls reading from the keyboard on 2000 (put 0xc0 into it, wait for it to become 0xd0)
volatile __at (0xdce4) unsigned char KEY_CURRENT;	// Holds the current key code on 2000

// There is stuff going on with port 0xfe on start-up!

#endif