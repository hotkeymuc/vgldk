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

// Display
#define LCD_ROWS 4
#define LCD_COLS 20
#define LCD_PORT_CTRL 0x0a
#define LCD_PORT_DATA 0x0b
#include <driver/hd44780.h>

// Keyboard
#include "keyboard.h"

// Sound
#include "sound.h"


// Publish function NAMES for STDIO
#define VGLDK_STDOUT_PUTCHAR lcd_putchar
#define VGLDK_STDIN_GETCHAR keyboard_getchar

//#define VGLDK_STDIN_GETS stdio_gets
//#define VGLDK_STDIN_INKEY keyboard_inkey

void vgldk_init() __naked {
	__asm
		di
		
		;; Set stack pointer directly above top of memory.
		ld	sp, #0xdff0	; Load StackPointer to 0xdff0
	__endasm;
	
	lcd_init();
	vgl_sound_off();
	lcd_clear();
	
	//main();
	__asm
		jp _main
	__endasm;
}

#endif