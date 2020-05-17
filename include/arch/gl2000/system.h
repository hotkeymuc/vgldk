#ifndef __GL2000_H
#define __GL2000_H
/*
Header for
	VTech PreComputer 2000
	YENO Mister X2
	VTech Genius LEADER 2000
	VTech Genius LEADER 2000 Compact
	VTech Genius LEADER 2000 PLUS
	(CompuSavant?)

for use with SDCC compiler

2020-01-21 by Bernhard "HotKey" Slawik
*/

//#define ARCH GL2000

#define DISPLAY_COLS 20
#define DISPLAY_ROWS 2

volatile __at (0xdce0) unsigned char KEY_STATUS;	// Controls reading from the keyboard on 2000 (put 0xc0 into it, wait for it to become 0xd0)
volatile __at (0xdce4) unsigned char KEY_CURRENT;	// Holds the current key code on 2000

#include "lcd.h"
#include "keyboard.h"
#include "sound.h"

void vgldk_init() {
	lcd_init();
	vgl_sound_off();
	lcd_clear();
	main();
}

#endif