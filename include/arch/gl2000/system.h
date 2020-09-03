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

// Display
#define LCD_ROWS 2
#define LCD_COLS 20
#define LCD_PORT_CTRL 0x0a
#define LCD_PORT_DATA 0x0b
#include <driver/hd44780.h>


// Keyboard
//volatile __at (0xdce0) unsigned char KEY_STATUS;	// Controls reading from the keyboard on 2000 (put 0xc0 into it, wait for it to become 0xd0)
//volatile __at (0xdce4) unsigned char KEY_CURRENT;	// Holds the current key code on 2000
#include "keyboard.h"

// Sound
#include "sound.h"


// Publish function NAMES for STDIO
#define VGLDK_STDOUT_PUTCHAR lcd_putchar
#define VGLDK_STDIN_GETCHAR keyboard_getchar

//#define VGLDK_STDIN_GETS stdio_gets
//#define VGLDK_STDIN_INKEY keyboard_inkey

void vgldk_init() {
	lcd_init();
	vgl_sound_off();
	lcd_clear();
	
	//main();
	__asm
		jp _main;
	__endasm;
}

#endif