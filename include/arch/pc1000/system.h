#ifndef __PC1000_H
#define __PC1000_H
/*
Header for
	VTech PreComputer 1000
	YENO Mister X
	(CompuSavant?)



@FIXME: This system architecture is not working correctly, yet!
	!!! LCD is broken


for use with SDCC compiler

2020-01-21 by Bernhard "HotKey" Slawik
*/

//#define ARCH PC1000

#warning "The system architecture PC1000 is not working correctly, yet! Screen will be blank!"

#define LCD_MINIMAL

// Display
#define LCD_ROWS 1
#define LCD_COLS 20
#define LCD_PORT_CTRL 0x20
#define LCD_PORT_DATA 0x21
//@TODO: Use "lcd_scroll_cb" to pause after each line
#include <driver/hd44780.h>

// Keyboard
volatile __at (0xdce0) unsigned char KEY_STATUS;	// Controls reading from the keyboard on 2000 (put 0xc0 into it, wait for it to become 0xd0)
volatile __at (0xdce4) unsigned char KEY_CURRENT;	// Holds the current key code on 2000
// There is stuff going on with port 0xfe on start-up!
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
		jp _main
	__endasm;
}

#endif