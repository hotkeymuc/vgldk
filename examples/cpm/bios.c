#ifndef __BIOS_C
#define __BIOS_C

/*
	BIOS - Basic Input/Output System
	================================
	
	The BDOS uses the BIOS to allow "high-level" file and device access
	
*/

/*
The BIOS begins with the following jumps to service routines:
	JMP	BOOT	;-3: Cold start routine
	JMP	WBOOT	; 0: Warm boot - reload command processor
	JMP	CONST	; 3: Console status
	JMP	CONIN	; 6: Console input
	JMP	CONOUT	; 9: Console output
	JMP	LIST	;12: Printer output
	JMP	PUNCH	;15: Paper tape punch output
	JMP	READER	;18: Paper tape reader input
	JMP	HOME	;21: Move disc head to track 0
	JMP	SELDSK	;24: Select disc drive
	JMP	SETTRK	;27: Set track number
	JMP	SETSEC	;30: Set sector number
	JMP	SETDMA	;33: Set DMA address
	JMP	READ	;36: Read a sector
	JMP	WRITE	;39: Write a sector

In CP/M 2 and later, the following extra jumps appear:
	JMP	LISTST	;42: Status of list device
	JMP	SECTRAN	;45: Sector translation for skewing

In CP/M 3 and later, the rest also follows.
*/

#include "bios.h"

#include "cpm.h"	// For the banner

#include "bint.h"	// For resetting the timer on boot

#include <vgldk.h>	// For hardware specific stuff


// Use own stdiomin
//#include <stdiomin.h>	// for puts() putchar() gets() getchar()
//#include <hex.h>	// for printf_x2/x4

// Use SDCC's stdio
//#define VGLDK_STDOUT_PUTCHAR lcd_putchar
//#define VGLDK_STDIN_GETCHAR keyboard_getchar
//#define putchar VGLDK_STDOUT_PUTCHAR
//#define getchar VGLDK_STDIN_GETCHAR
int getchar(void) {
	return VGLDK_STDIN_GETCHAR();
}
int putchar(int c) {
	VGLDK_STDOUT_PUTCHAR(c);
	return 1;
}
#include <stdio.h>


#ifdef BIOS_PAPER_TAPE_TO_SOFTUART
	#include <driver/softuart.h>
#endif
#ifdef BIOS_PAPER_TAPE_TO_SOFTSERIAL
	#include <softserial.h>	// bespoke for each architecture
#endif
#ifdef BIOS_PAPER_TAPE_TO_MAME
	#include <driver/mame.h>
#endif

// Make sure one is selected
#ifndef BIOS_PAPER_TAPE_TO_DISPLAY
	#ifndef BIOS_PAPER_TAPE_TO_SOFTUART
		#ifndef BIOS_PAPER_TAPE_TO_SOFTSERIAL
			#ifndef BIOS_PAPER_TAPE_TO_MAME
				#error One BIOS_PAPER_TAPE destination has to be set (display, SoftUART, SoftSerial, MAME)!
			#endif
		#endif
	#endif
#endif


void bios();	// Forward declaration to function table (located at the end of this file)


// Stringmin
//@TODO: Use <stringmin.h>
void bios_memset(byte *addr, byte b, word count) {
	while(count > 0) {
		*addr++ = b;
		count--;
	}
}

#ifdef BIOS_SCROLL_WAIT
// Pause-after-one-page callback
// Can be installed after lcd_init() by setting "vgl_scroll_cb = &bios_scroll_cb;"
byte bios_scroll_counter;
void bios_scroll_cb() {
	// Scroll callback that pauses after each page of text
	
	while(lcd_y >= LCD_ROWS) {
		
		if (bios_scroll_counter >= LCD_ROWS) {
			// Sleep on scroll
			//sound_note(12*5, 100);
			beep();
			getchar();
			bios_scroll_counter = 0;
		}
		
		lcd_y--;
		lcd_scroll();	// Invoke native scroll function
		bios_scroll_counter++;
	}
}
#endif


// Initial function, invoked by CPM_CRT0.s on init
void bios_boot() __naked {
	// Cold boot - should never be invoked by a user
	
	__asm
		di
		ld sp, #BIOS_STACK	; GL4000: ld sp, #0xdff0 - leave just a little bit more
	__endasm;
	
	// BINT state
	bint_timer = 0;
	
	// Make memory cell 0x0000 be a "JP _BIOS WBOOT"
	//*(byte *)0x0000 = 0xc3;
	//*(word *)0x0001 = (word)&bios + 3;
	
	// Copy BIOS function jump table to 0x4A00
	//mem_copy((byte *)&bios, (byte *)0x4a00, 17*3);
	
	// 0x0003: iobyte
	bios_iobyte = 0;
	
	// 0x0004: curdsk
	bios_curdsk = 0;
	
	
	// Init VGL hardware
	lcd_init();
	
	keyboard_init();
	
	sound_off();
	
	
	#ifdef BIOS_SCROLL_WAIT
		// Add "wait" while scrolling callback
		bios_scroll_counter = LCD_ROWS;
		lcd_scroll_cb = &bios_scroll_cb;
	#endif
	
	#ifdef BIOS_SHOW_BANNER
		// Show banner
		puts(CPM_TITLE);
		puts(CPM_VERSION);
	#endif
	
	//sound_note(12*4, 250);
	
	// Show the tape config
	#ifdef BIOS_SHOW_PAPER_TAPE_MAPPING
		#ifdef BIOS_PAPER_TAPE_TO_DISPLAY
			puts("Tape = LCD");
		#endif
		#ifdef BIOS_PAPER_TAPE_TO_SOFTUART
			puts("Tape = SoftUART");
		#endif
		#ifdef BIOS_PAPER_TAPE_TO_SOFTSERIAL
			puts("Tape = SoftSerial");
		#endif
		#ifdef BIOS_PAPER_TAPE_TO_MAME
			puts("Tape = MAME");
		#endif
	#endif
	
	// continue with "warm boot"
	//bios_wboot();
	__asm
		jp _bios_wboot
	__endasm;
	
}


void bios_wboot() __naked {
	// Warm boot
	
	// * Close all handles
	
	// * Restore LowStorage
	// 0x0000: C3 biosLo biosHi
	
	// 0x0003: iobyte
	//bios_iobyte = 0;	// Set stdin/stdout
	
	// 0x0004: curdsk
	//bios_curdsk = 0;	// Only done in cold boot
	
	// 0x0005: JP BDOS+6: C3 (BDOS+6)lo (BDOS+6)hi
	
	// BIOS+3...: BIOS entry points: C3 (BIOS+x)lo (BIOS+x)hi
	
	// Disk Parameter Block(s)
	//...
	
	// Disk Parameter Headers
	//...
	
	
	// Clear stuff
	bios_curdsk = 0;
	bios_dma = (byte *)0x0080;
	
	bios_memset(bios_dma, 0x1a, 0x80);	// Fill DMA area with EOFs
	//memset((byte *)0x0100, 0x00, 0x1000);	// Clear transient area
	
	bios_trk = 0;
	bios_sec = 1;
	
	//bios_seldsk(bios_curdsk);
	
	
	// Load BDOS (and let it load and start CCP)
	
	//bdos_init();	// If bdos_init() is known at compile time
	__asm
		// jp _bdos_init	// If bdos_init() is known at compile time
		ld c, #0	// Call BDOS with function 0 = BDOS_FUNC_P_TERMCPM = bdos_init
		jp _bdos
	__endasm;
}


byte bios_const() {
	// Returns its status in A; 0 if no character is ready, 0FFh if one is.
	// console status, return 0ffh if character ready, 00h if not
	
	//@TODO: Handle bios_iobyte, bits 0,1: 00=TTY, 01=CRT, 10=BAT, 11=UC1
	if (keyboard_ispressed() > 0) return 0xff;	// Key pressed
	return 0x00;	// No key pressed
}

byte bios_conin() {
	// Wait until the keyboard is ready to provide a character, and return it in A.
	
	//@TODO: Handle bios_iobyte, bits 0,1: 00=TTY, 01=CRT, 10=BAT, 11=UC1
	return keyboard_getchar();
}

void bios_conout(byte c) {
	// Write the character in C to the screen.
	
	//@TODO: Handle bios_iobyte, bits 0,1: 00=TTY, 01=CRT, 10=BAT, 11=UC1
	putchar(c);
}

void bios_list(byte c) {
	// Write the character in C to the printer. If the printer isn't ready, wait until it is.
	
	//@TODO: Handle bios_iobyte, bits 6,7: 00=TTY, 01=CRT, 10=LPT, 11=UL1
	#ifdef BIOS_USE_PRINTER
		// Actually send to VTech printer periphery
		vgl_printer_putchar(c);
	#else
		putchar(c);
	#endif
}


void bios_punch(byte c) {
	// Write the character in C to the "paper tape punch" - or whatever the current auxiliary device is. If the device isn't ready, wait until it is.
	
	//@TODO: Handle bios_iobyte, bits 4,5: 00=TTY, 01=PTP, 10=UP1, 11=UP2
	
	//printf("punch"); printf_x2(c);
	
	#ifdef BIOS_PAPER_TAPE_TO_DISPLAY
		putchar(c);
	#endif
	#ifdef BIOS_PAPER_TAPE_TO_SOFTUART
		// Send to SoftUART
		softuart_sendByte(c);
	#endif
	#ifdef BIOS_PAPER_TAPE_TO_SOFTSERIAL
		// Send to SoftSerial
		serial_putchar(c);
	#endif
	#ifdef BIOS_PAPER_TAPE_TO_MAME
		mame_putchar(c);
	#endif
}

byte bios_reader() {
	// Read a character from the "paper tape reader" - or whatever the current auxiliary device is. If the device isn't ready, wait until it is. The character will be returned in A. If this device isn't implemented, return character 26 (^Z).
	
	//@TODO: Handle bios_iobyte, bits 2,3: 00=TTY, 01=PTR, 10=UR1, 11=UR2
	
	#ifdef BIOS_PAPER_TAPE_TO_DISPLAY
		return getchar();
	#endif
	#ifdef BIOS_PAPER_TAPE_TO_SOFTUART
		// Read from SoftUART (blocking)
		int r;
		do {
			r = softuart_receiveByte();
		} while(r < 0);
		return (byte)r;
	#endif
	#ifdef BIOS_PAPER_TAPE_TO_MAME
		return mame_getchar();
	#endif
	
	//byte c;
	// c = ...
	//bdos_printf_x2(c);
	//return c;
}

void bios_home() {
	// Move the current drive to track 0.
	bios_trk = 0;
	bios_sec = 1;
}

DPH *bios_seldsk(byte n) {
	// Select the disc drive in register C (0=A:, 1=B: ... 15=P:).
	// Called with E=0 or 0FFFFh.
	// SELDSK returns the address of a Disc Parameter Header in HL.
	// The exact format of a DPH varies between CP/M versions;
	// note that under CP/M 3, the DPH is in memory bank 0 and probably not visible to programs.
	// If the disc could not be selected it returns HL=0.
	bios_curdsk = n;
	return &bios_dummy_dph;
}

void bios_settrk(word t) {
	// Set the track in BC - 0 based.
	bios_trk = t;
}

void bios_setsec(word s) {
	// Set the sector in BC. Under CP/M 1 and 2 a sector is 128 bytes.
	bios_sec = s;
}

void bios_setdma(byte *a) {
	// The next disc operation will read its data from (or write its data to) the address given in BC.
	/*
	printf("bios_setdma=");
	printf_x4((word)a);
	*/
	bios_dma = a;
}

byte bios_read() {
	// Read the currently set track and sector at the current DMA address.
	// Returns A=0 for OK, 1 for unrecoverable error, 0FFh if media changed.
	
	//@FIXME: Implement!
	//@TODO: Don't forget to allow setting the activity led using led_on() and led_off() :-)
	return 0;
}

byte bios_write(byte c) {
	// Write the currently set track and sector. C contains a deblocking code:
	// C=0 - Write can be deferred / C=1 - Write must be immediate / C=2 - Write can be deferred, no pre-read is necessary.
	// Returns A=0 for OK, 1 for unrecoverable error, 2 if disc is readonly, 0FFh if media changed.
	(void) c;
	
	//@FIXME: Implement!
	//@TODO: Don't forget to allow setting the activity led using led_on() and led_off() :-)
	
	return 0;
}

byte bios_listst() {
	// Return status of current printer device: Returns A=0 (not ready) or A=0FFh (ready).
	return 0xff;
}

word bios_sectran(word n, byte *a) {
	// translate sector numbers to take account of skewing.
	// On entry, BC=logical sector number (zero based) and DE=address of translation table.
	// On exit, HL contains physical sector number. On a system with hardware skewing, this would normally ignore DE and return either BC or BC+1.
	(void)a;
	return n;
}


void bios() __naked {
	// BIOS jump vector table
	__asm
		jp _bios_boot
		jp _bios_wboot
		jp _bios_const
		jp _bios_conin
		jp _bios_conout
		jp _bios_list
		jp _bios_punch
		jp _bios_reader
		jp _bios_home
		jp _bios_seldsk
		jp _bios_settrk
		jp _bios_setsec
		jp _bios_setdma
		jp _bios_read
		jp _bios_write
		
	; CP/M 2:
		jp _bios_listst
		jp _bios_sectran
		
	;CP/M 3:
		
	__endasm;
}

#endif	// __BIOS_C