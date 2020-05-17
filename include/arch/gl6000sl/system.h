#ifndef __GL6000SL
#define __GL6000SL

/*

System header for the VTech Genius LEADER 6000SL / PreComputer Prestige


What's known so far (mostly from MAME's "prestige" driver):
	* Screen is memory-mapped to e000-ebb8, 1 bbp
	* Mouse is port-mapped to X=0x04, Y=0x05 - that's it!
	* Keyboard matrix is at ports 0x40 (write), 0x41, 0x42 (read)
	* Bank-switching is at 0x50...0x56 (and it's quite elaborate)
		map(0x04, 0x05).rw(FUNC(prestige_state::mouse_r), FUNC(prestige_state::mouse_w));
		
			0x10 = ??? is being read all the time! (see ROM 0x50D1)
					0x00 is being written there all the time!
		
		0x20, 0x21, 0x22, 0x23 -> Printer/PC-Link port
			IN 0x21
				bit 3 (0x08) = Left Mouse button
				bit 4 (0x10) = Right Mouse button
				
				bit 6 = busy?	(see ROM 0x7E2C)
				bit 7 (0x80) = cable connected? (0x00=yes, 0x80=no) 	(see ROM 0x7DAC)
			
			OUT 0x21, bit 6, bit 7 or 0xE0 (default)
				bit 6 (0x20) = CAPS LOCK light
			
			OUT 0x20, DATA	-> Prepare bits for output	(see ROM 0x7DCB)
			OUT 0x22, 0xFF / 0x00 (default)	-> Latch/strobe bits onto port	(see ROM 0x7DCD)
				! this is a holding register
			
			OUT 0x23, 0x20 / 0x60 (default) / 0xA0	-> ??? before reading 0x21 (see ROM 0x7E96, 0x7E7B, 0x7E8D)
					OUT 0x23, 0x20 to detect printer
			0x25 ?
		
		Those cases are listed at 0x7E6E:
			IN A, 0x21
			set 6, A
			OUT 0x21, A
			
			IN A, 0x21
			reset 6, A
			OUT 0x21, A
			LD A, 0x60
			OUT 0x23, A
			
			IN A, 0x21
			set 7, A
			OUT 0x21, A
			
			IN A, 0x21
			reset 7, A
			OUT 0x21, A
			LD A, 0xA0
			OUT 0x23, A
			
			OUT 0x22, A
			LD A, 0x20
			OUT 0x23, A
			
		0x29 = ?
		
		map(0x30, 0x3f).w(FUNC(prestige_state::lcdc_w));
		map(0x40, 0x40).w(FUNC(prestige_state::kb_w));
		map(0x41, 0x42).r(FUNC(prestige_state::kb_r));
		0x43
		0x47	= ??? holds value?
		0x48	= ??? holds value?
		map(0x50, 0x56).rw(FUNC(prestige_state::bankswitch_r), FUNC(prestige_state::bankswitch_w));
		
			0x60 = ??? is being read all the time!	(see 0x1388 on shutdown)
			0x61 = sound? (see ROM 0x4d42 boot port setup sequence)
			0x62 = ??? 0x00 is written there all the time - watch dog?
			
		0x76
		0xFE
		
		map(0x0000, 0x3fff).bankr("bank1");
		map(0x4000, 0x7fff).bankr("bank2");
		map(0x8000, 0xbfff).bankr("bank3");
		map(0xc000, 0xdfff).bankrw("bank4");
		map(0xe000, 0xffff).bankrw("bank5");	// This seems to be video memory
		
		m_bank1->configure_entries(0, 64, rom,  0x4000);	// i.e. 0x0000-0x3fff can display any 16KB of the 64 x 16KB = 1MB ROM
		m_bank1->configure_entries(64,32, cart, 0x4000);	// ...or 32 x 16 KB = 512KB cartridge address space
		m_bank2->configure_entries(0, 64, rom,  0x4000);
		m_bank2->configure_entries(64,32, cart, 0x4000);
		m_bank3->configure_entries(0, 64, rom,  0x4000);
		m_bank3->configure_entries(64,32, cart, 0x4000);
		m_bank4->configure_entries(0, 4,  ram,  0x2000);
		m_bank5->configure_entries(0, 4,  ram,  0x2000);	// This seems to be video memory

Init sequence (traced using custom MAME prestige.cpp):


Intro sound port accesses at 0x2x?:
			31 W: 03	// LCD refresh off?
		22 W: 00
		21 W: E0
		23 W: 60
			32 W: 1D	// LCD w
			33 W: FF	// ?
			34 W: 64	// LCD H
			35 W: 00	// ?
			3D W: 00	// LCD FB width
			36 W: 00	// LCD addr1 H
			37 W: 00	// LCD addr1 L
			3A W: 64	// LCD split pos
			3B W: 00	// ?
			38 W: B8	// LCD addr2 H
			39 W: 0B	// LCD addr2 L
			3D W: 00	// LCD FB width
			3E W: 00	// ?
			3F W: 00	// ?
		
		23 W: 60
		21 W: E0
		61 W: D0	// ????
		21 W: FF
		
		
		29 W: 01
		43 W: 02	// ???? (0x40, 0x41, 0x42 = keyboard matrix)
		23 W: 20
		
		
			31 W: 83	// LCD refresh on?
			31 W: 83	// LCD refresh on?
		...
		23 W: 20
		22 W: 00
		20 W: 00
		22 W: 00
		23 W: 20
		20 W: 1D
		22 W: FF
		21 W: BF
		23 W: 60
		...
		21 W: FF
		22 W: 00
		23 W: 20
		22 W: 00
		...
		22 W: 00
		23 W: 20
		20 W: 1D
		22 W: FF
		21 W: BF
		23 W: 60
		...
		22 W: 00
		23 W: 20
	
	then (looped):
		21 R	// Check mouse buttons
		21 R	// Check mouse buttons
		21 R	// Check mouse buttons
		60 R
		62 W: 00
		10 W: 00
		21 R	// Check mouse buttons
		21 R	// Check mouse buttons
		21 R	// Check mouse buttons
	



2020-05-14 Bernhard "HotKey" Slawik
*/

//#define VGL_GETCHAR
//#define VGL_PUTCHAR
//#define VGL_USE_SOUND
//#include <vgl.h>
//#include <vgl_lcd.h>
//
//#include <stdio.h>

//typedef unsigned char byte;
//typedef unsigned short word;

#include "lcd.h"

#include "ports.h"

#include "keyboard.h"

void vgldk_init() {
	__asm
	di
	
	/*
	// Continue initialization (see GL6000SL ROM at 0x0b61)
	// It is needed for initializing the LCD controller
	ld hl, #0x00
	ld de, #0x05
	call 0x2900	// ?
	
	// Call function 0x4108 (in ROM segment 0x0A = 0x28000-0x2BFFF mapped at BANK2 at 0x4000-0x7FFF)
	ld hl, #0x37f1	// 0x37f1: Call to L=08 H=41 OUT51=0A -> 0x4108 = physical ROM address 0x28108
	call 0x26eb
	
	// This clears the LCD
	// Call function at 0x4092 (in ROM segment 0x0A = 0x28000-0x2BFFF mapped at BANK2 at 0x4000-0x7FFF)
	ld hl, #0x37dd	// 0x37dd: Call to L=92 H=40 OUT51=0A -> 0x4092 = physical ROM address 0x2800A
	call 0x26eb
	
	//ld hl, #0x37dd
	*/
	__endasm;
	
	lcd_init();
	
	lcd_clear();
	
	keyboard_init();
	
	//main();
	__asm
		jp _main
	__endasm;
}

#endif //__GL6000SL