#ifndef __KEYBOARD_H
#define __KEYBOARD_H
/*
VTech Genius Leader Keyboard

//@TODO: Handling of keyboard should happen inside an interrupt handler!
//@TODO: Handle roll-over like in GL6000SL's implementation

2019-11-03 Bernhard "HotKey" Slawik
*/

#include "ports.h"


// BIOS4000 01a6: Send 0xff to port 0x11 (although in original firmware, keyboard matrix works without it)
// This also makes the parallel port wiggle, so it might interfere with serial/parallel communication!
#define VGL4000_KEYBOARD_LATCH_0x11

#define VGL_KEYS_MASK_CAPS	0x20

#define KEY_CAPS	'c'
#define KEY_SHIFT	's'
#define KEY_ALT	'a'
#define KEY_TAB	'\t'	// Help / TAB
#define KEY_BREAK	(char)27	// Asterisk / Break
#define KEY_REPEAT	(char)8	// Repeat/Delete (Backspace)
#define KEY_INS	'\r'	// Insert / LeftPlayer
#define KEY_DEL (char)127	// Delete / RightPlayer
#define KEY_SPACE ' '	// And/or "+" / "Antwort"?
#define KEY_CURSOR_LEFT	'h'
#define KEY_CURSOR_RIGHT	'l'
#define KEY_ENTER '\r'	// '\n'	// Zork expects 0x0d as "ENTER"
#define KEY_ESC (char)27
const char vgl_key_map[8*8] = {
	KEY_TAB,	KEY_BREAK,	'e',	'f',	'g',	'i',	'j',	'k',
	KEY_CAPS, '1', 'Q', 'A', 'Z', KEY_ALT, KEY_SPACE, KEY_INS,
	'2', '3', 'E', 'S', 'D', 'X', 'C', 'W',
	'4', '5', 'T', 'F', 'G', 'V', 'B', 'R',
	'6', '7', 'U', 'H', 'J', 'N', 'M', 'Y',
	'8', '9', 'O', 'K', 'L', ',', '.', 'I',
	'0', KEY_REPEAT, '=', ':' /*';'*/, '\'', '/', KEY_SHIFT, 'P',
	'x', 'y', (char)KEY_CURSOR_LEFT, (char)KEY_CURSOR_RIGHT, KEY_ENTER, KEY_DEL, KEY_ESC, 'z',
};
/*
const char vgl_key_map2[8*8] = {
	(char)0x80, (char)0x81, (char)0x82, (char)0x83, (char)0x84, (char)0x85, (char)0x86, (char)0x87,
	(char)0x90, (char)0x91, (char)0x92, (char)0x93, (char)0x94, (char)0x95, (char)0x96, (char)0x97,
	(char)0xa0, (char)0xa1, (char)0xa2, (char)0xa3, (char)0xa4, (char)0xa5, (char)0xa6, (char)0xa7,
	(char)0xb0, (char)0xb1, (char)0xb2, (char)0xb3, (char)0xb4, (char)0xb5, (char)0xb6, (char)0xb7,
	(char)0xc0, (char)0xc1, (char)0xc2, (char)0xc3, (char)0xc4, (char)0xc5, (char)0xc6, (char)0xc7,
	(char)0xd0, (char)0xd1, (char)0xd2, (char)0xd3, (char)0xd4, (char)0x05, (char)0xd6, (char)0xd7,
	(char)0xe0, (char)0xe1, (char)0xe2, (char)0xe3, (char)0xe4, (char)0xe5, (char)0xe6, (char)0xe7,
	(char)0xf0, (char)0xf1, (char)0xf2, (char)0xf3, (char)0xf4, (char)0xf5, (char)0xf6, (char)0xf7,
};
*/

//byte vgl_keys_state[16];

byte keyboard_inkey() {
	
	//@TODO: Re-activate the second matrix (function keys)
	
	byte b1;
	//byte b2;
	byte m, m2;
	byte row, col;
	char r;
	
	#ifdef VGL4000_KEYBOARD_LATCH_0x11
	// BIOS4000 01a6: Send 0xff to port 0x11 (although in original firmware, keyboard matrix works without it)
	//port_0x11_out(0xff);
	__asm
		ld a, #0xff
		out (0x11), a
	__endasm;
	#endif
	
	r = 0;
	//m = 0x80;
	//row = 7;
	//while(1) {
	m = 0x01;
	for(row = 0; row < 8; row++) {
		// Send bit mask to MUXer
		
		// BIOS4000 01bc: Send bit mask to 0x10
		port_0x10_out(m);
		
		// Read back 0x10
		b1 = port_0x10_in();
		
		// Read back 0x11
		//b2 = port_0x11_in();
		
		
		// BIOS4000 01c7: Reset it back to 0x00
		//port_0x10_out(0x00);
		__asm
			ld a, #0x00
			out (0x10), a
		__endasm;
		
		
		if (b1 < 0xff) {
			m2 = 0x01;
			for(col = 0; col < 8; col++) { 
				if ((b1 & m2) == 0) {
					// Return first bit found. We could handle simulataneous presses!
					return vgl_key_map[8 * row + col];
					//return 0x00 + (8 * row + col);	// Just return the scan code
				}
				m2 = m2 << 1;
			}
		}
		
		/*
		if (b2 < 0xff) {	//@FIXME: Row 2 has only bits 0..4?
			m2 = 0x01;
			//@FIXME: Row 2 has only bits 0..4?
			for(col = 0; col < 8; col++) { 
				if ((b2 & m2) == 0) {
					// Return first bit found. We could handle simulataneous presses!
					//return vgl_key_map2[8 * row + col];
					return 0x80 + (8 * row + col);	// Just return the scan code
				}
				m2 = m2 << 1;
			}
		}
		*/
		
		m = m << 1;
		
		/*
		if (row == 0) break;
		m = m >> 1;
		row --;
		*/
	}
	//putchar('.');
	//port_0x12_out(port_0x12_in() | VGL_KEYS_MASK_CAPS);	// CAPS on?
	//port_0x12_out(port_0x12_in() & (0xff - VGL_KEYS_MASK_CAPS));	// CAPS off?
	
	// Non-blocking: Return "0" if nothing pressed
	return 0;
}


// Return "true" if a key is pressed
byte keyboard_checkkey() {
	
	byte b1;
	//byte b2;
	
	#ifdef VGL4000_KEYBOARD_LATCH_0x11
	// Set all mux lines HIGH (although in original firmware, keyboard matrix works without it)
	port_0x11_out(0xff);
	#endif
	
	port_0x10_out(0xff);
	b1 = port_0x10_in();
	//b2 = port_0x11_in();
	port_0x10_out(0x00);
	if (b1 < 0xff) return b1;	// Some key is pressed
	//if (b2 < 0xff) return b2;	// Some key is pressed
	
	return 0;	// No keys are pressed
}


char keyboard_getchar_last = 0;
char keyboard_getchar() {
	char c;
	
	//while (checkkey()) {}
	//c = getchar_blocking();
	
	while(1) {
		c = keyboard_inkey();
		if (c == 0) keyboard_getchar_last = 0;
		if (c != keyboard_getchar_last) break;
	}
	
	keyboard_getchar_last = c;
	return c;
}

#endif // __KEYBOARD_H