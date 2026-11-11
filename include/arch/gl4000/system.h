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

// Power the machine off. Verified on a real Spanish VTech Genio 2000 (Genius LEADER 2000/4000 family hardware); not tested on other models.
// Bit 0 of port 0x12 requests power-off. This is the sequence the original firmware uses: write 1 to the port, then halt.
// Interrupts are disabled first. It does not return: if the power does not drop, the CPU just stays halted. (Bytes: F3 3E 01 D3 12 76 18 FD)
void poweroff() __naked {
	__asm
		di
		ld	a, #0x01
		out	(0x12), a
	00001$:
		halt
		jr	00001$
	__endasm;
}

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
		call _main
		;; main() returned: switch off instead of running into undefined code
		jp _poweroff
	__endasm;
}

#endif	// __GL4000_SYSTEM_H