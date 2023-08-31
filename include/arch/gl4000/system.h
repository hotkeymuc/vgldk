#ifndef __GL4000_SYSTEM_H
#define __GL4000_SYSTEM_H
/*
Header file for
	VTech Genius LEADER 4000 Quadro (or 4004 Quadro L)

Other international names:
	- Genio 2000 (Spanish version of Genius Leader 4000 Quadro)
	- Genius 4000 (French version of Genius Leader 4000 Quadro)
	- PreComputer Power Pad (English version of Genius Leader 4000 Quadro)

(for use with SDCC compiler)

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
#ifdef KEYBOARD_MINIMAL
	// Use minimal keyboard
	#include "keyboardmin.h"
#else
	#include "keyboard.h"
#endif

// Non-essential (but fun) features:

// Sound
#include "sound.h"

// Printer
//#include "printer.h"

// LED
#include "led.h"


// Publish function NAMES for STDIO
#define VGLDK_STDOUT_PUTCHAR lcd_putchar
#define VGLDK_STDIN_GETCHAR keyboard_getchar

//#define VGLDK_STDIN_GETS stdio_gets
//#define VGLDK_STDIN_INKEY keyboard_inkey

// Entry point from crt0.s
void vgldk_init() __naked {
	__asm
		di
		
		;; Set stack pointer directly above top of memory.
		ld	sp, #0xdff0	; Load StackPointer to 0xdff0
	__endasm;
	
	lcd_init();
	//lcd_scroll_cb = ...
	keyboard_init();
	sound_off();
	
	lcd_clear();
	
	//main();
	__asm
		jp _main
	__endasm;
}

/*
void shutdown() {
	__asm
		; V-Tech power down:
		
		; LCD off?
		ld	a, 1
		out	(0ah), a
		
		; Power off
		ld	a, 1
		out	(12h), a
		halt
	__endasm;
}
*/
#endif	// __GL4000_SYSTEM_H