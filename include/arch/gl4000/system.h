#ifndef __GL4000_H
#define __GL4000_H
/*
Header for
	VTech Genius LEADER 4000 Quadro
	VTech Genius LEADER 4004 Quadro L
	(CompuSavant?)




for use with SDCC compiler

2020-01-21 by Bernhard "HotKey" Slawik
*/

//#define ARCH GL4000

#define DISPLAY_COLS 20
#define DISPLAY_ROWS 4

volatile __at (0xdb00) unsigned char KEY_STATUS;	// Controls reading from the keyboard on 4000 (put 0xc0 into it, wait for it to become 0xd0)
volatile __at (0xdb01) unsigned char KEY_CURRENT;	// Holds the current key code on 4000

#include "lcd.h"
#include "keyboard.h"
#include "sound.h"

void vgldk_init {
	lcd_init();
	vgl_sound_off();
	lcd_clear();
}

#endif