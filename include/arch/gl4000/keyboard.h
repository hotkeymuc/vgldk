#ifndef __KEYBOARD_H
#define __KEYBOARD_H
/*
VTech Genius Leader Keyboard

//@TODO: Handling of keyboard should happen inside an interrupt handler!
//@TODO: Unify into one single "matrix keyboard" driver

2019-11-03 - 2023-08-31 Bernhard "HotKey" Slawik
*/

// BIOS4000 01a6: Send 0xff to port 0x11 (although in original firmware, keyboard matrix works without it)
// This also makes the parallel port wiggle, so it interferes with serial/parallel communication!
#define KEYBOARD_LATCH

// Ports
#define KEYBOARD_PORT_ROW_OUT 0x10
#define KEYBOARD_PORT_COL_IN 0x10
#define KEYBOARD_PORT_COL_IN2 0x11
__sfr __at KEYBOARD_PORT_ROW_OUT keyboard_port_matrix_col_out;
__sfr __at KEYBOARD_PORT_COL_IN keyboard_port_matrix1_row_in;
__sfr __at KEYBOARD_PORT_COL_IN2 keyboard_port_matrix2_row_in;

#ifdef KEYBOARD_LATCH
	#define KEYBOARD_PORT_LATCH 0x11
	__sfr __at KEYBOARD_PORT_LATCH keyboard_port_matrix_latch;
#endif

/*
// If "__sfr" is not available: Use assembly
// Keyboard matrix write
void keyboard_matrix_out(byte a) __naked {(void)a;
__asm
	; Get parameter from stack into a
	ld hl,#0x0002
	add hl,sp
	ld a,(hl)
	
	; Put it to port
	out	(KEYBOARD_PORT_ROW_OUT), a	; 0x40
	ret
__endasm;
}
// Keyboard matrix read 1
byte keyboard_matrix_in1() __naked {
__asm
	in	a, (KEYBOARD_PORT_COL_IN1)	; 0x41
	ld	l, a
	ret
__endasm;
}
// Keyboard matrix read 2
byte keyboard_matrix_in2() __naked {
__asm
	in	a, (KEYBOARD_PORT_COL_IN2)	; 0x42
	ld	l, a
	ret
__endasm;
}
*/


// Key codes
#define KEY_NONE	0	// key code for "not a valid key"
#define KEY_CURSOR_LEFT	'h'
#define KEY_CURSOR_RIGHT	'l'

#define KEY_CAPS	'C'
#define KEY_SHIFT	'S'
#define KEY_ALT	'A'

#define KEY_TAB	'\t'	// Help / TAB
#define KEY_BREAK	(char)27	// Asterisk / Break
#define KEY_REPEAT	(char)8	// Repeat/Delete (it is at the position of Backspace, above ENTER)
#define KEY_PLAYER_LEFT	'\r'	// Insert / LeftPlayer
#define KEY_PLAYER_RIGHT (char)127	// Delete / RightPlayer
#define KEY_SPACE ' '	// And/or "+" / "Antwort"?
#define KEY_ENTER '\r'	// '\n'	// Zork expects 0x0d as "ENTER"
#define KEY_ANSWER '+'

#define KEY_ACTIVITY_BASE 0x80	// Start of activity keys
#define KEY_ACTIVITY(s) (keycode_t)(KEY_ACTIVITY_BASE + s)	// Macro for the numerous activity-buttons
//#define KEY_CARTRIDGE 254
#define KEY_OFF 'O'
#define KEY_ON KEY_NONE	// Not pollable

#define KEY_CHARCODE_NONE 0
#define KEY_CHARCODE_ENTER '\r'

typedef byte keycode_t;
typedef byte scancode_t;

#define KEY_MATRIX1_ROWS 8
#define KEY_MATRIX1_COLS 8

#define KEY_MATRIX2_COLS 8
#define KEY_MATRIX2_ROWS 5	// Matrix2 has only 40 activities (8 * 5)

// Map SCANCODE to KEYCODE (which can be the final char)
const keycode_t KEY_CODES[(KEY_MATRIX1_ROWS*KEY_MATRIX1_COLS) + (KEY_MATRIX2_ROWS*KEY_MATRIX2_COLS)] = {
	
	// German layout
	KEY_TAB,  	KEY_CAPS,       	'2',	'4',	'6',	'8',	'0',      	'X',                   
	KEY_BREAK,	'1',            	'3',	'5',	'7',	'9',	KEY_REPEAT,	'Y',                   
	'E',      	'q',            	'e',	't',	'u',	'o',	'ü',      	(char)KEY_CURSOR_LEFT, 
	'F',      	'a',            	's',	'f',	'h',	'k',	'ö',      	(char)KEY_CURSOR_RIGHT,
	'G',      	'y',            	'd',	'g',	'j',	'l',	'ä',      	KEY_ENTER,             
	'I',      	KEY_ALT,        	'x',	'v',	'n',	',',	'-',      	KEY_PLAYER_RIGHT,      
	'J',      	KEY_SPACE,      	'c',	'b',	'm',	'.',	KEY_SHIFT,	KEY_ANSWER,            
	'K',      	KEY_PLAYER_LEFT,	'w',	'r',	'z',	'i',	'P',      	'Z',                   
	
	// Activities...
	KEY_ACTIVITY(0x00), KEY_ACTIVITY(0x01), KEY_ACTIVITY(0x02), KEY_ACTIVITY(0x03), KEY_ACTIVITY(0x04), KEY_ACTIVITY(0x05), KEY_ACTIVITY(0x06), KEY_ON,
	KEY_ACTIVITY(0x07), KEY_ACTIVITY(0x08), KEY_ACTIVITY(0x09), KEY_ACTIVITY(0x0a), KEY_ACTIVITY(0x0b), KEY_ACTIVITY(0x0c), KEY_ACTIVITY(0x0d), KEY_OFF,
	KEY_ACTIVITY(0x0e), KEY_ACTIVITY(0x0f), KEY_ACTIVITY(0x10), KEY_ACTIVITY(0x11), KEY_ACTIVITY(0x12), KEY_ACTIVITY(0x13), KEY_ACTIVITY(0x14), KEY_ACTIVITY(0x15),
	KEY_ACTIVITY(0x16), KEY_ACTIVITY(0x17), KEY_ACTIVITY(0x18), KEY_ACTIVITY(0x19), KEY_ACTIVITY(0x1a), KEY_ACTIVITY(0x1b), KEY_ACTIVITY(0x1c), KEY_ACTIVITY(0x1d),
	KEY_ACTIVITY(0x1e), KEY_ACTIVITY(0x1f), KEY_ACTIVITY(0x20), KEY_ACTIVITY(0x21), KEY_ACTIVITY(0x22), KEY_ACTIVITY(0x23), KEY_ACTIVITY(0x24), KEY_ACTIVITY(0x25),
};

#define KEYBOARD_PRESSED_MAX 6	// 4	// How many scancodes can be pressed at once (roll-over)
#define KEYBOARD_BUFFER_MAX 8
#define KEYBOARD_SCANCODE_INVALID 0xff

#define KEYBOARD_MODIFIER_SHIFT 1
#define KEYBOARD_MODIFIER_ALT 2
//#define KEYBOARD_MODIFIER_SYMBOL 4	// not available on GL4000

byte keyboard_modifiers;

byte keyboard_num_pressed;
scancode_t keyboard_pressed[KEYBOARD_PRESSED_MAX];

byte keyboard_buffer_in;
byte keyboard_buffer_out;
char keyboard_buffer[KEYBOARD_BUFFER_MAX];


void keyboard_init() {
	byte i;
	
	// Clear pressed keys
	keyboard_buffer_in = 0;
	keyboard_buffer_out = 0;
	
	keyboard_num_pressed = 0;
	for (i = 0; i < KEYBOARD_PRESSED_MAX; i++) keyboard_pressed[i] = KEY_NONE;
	
	keyboard_modifiers = 0x00;
}

// Return "true" if a key is pressed
// formerly "keyboard_checkkey"
byte keyboard_ispressed() {
	
	byte b1;
	byte b2;
	
	#ifdef KEYBOARD_LATCH
		// Set all mux lines HIGH (although in original firmware, keyboard matrix works without it)
		keyboard_port_matrix_latch = 0xff;
	#endif
	
	// Set all outputs at the same time
	keyboard_port_matrix_col_out = 0xff;
	
	// Read back if anything is pressed at all
	b1 = keyboard_port_matrix1_row_in;
	b2 = keyboard_port_matrix2_row_in;
	
	// Set MUX back to idle
	keyboard_port_matrix_col_out = 0x00;
	
	if (b1 < 0xff) return 1;	//b1;	// Some key is pressed on matrix 1
	
	// Matrix 2:
	// IDLE	0x5F
	// ROW1	0x5E
	// ROW2	0x5D
	if ((b2 & 0x1f) < 0x1f) return 1;	// Some key is pressed on matrix 2
	
	return 0;	// No keys are pressed
}


void keyboard_update() {
	
	byte ix, iy;
	byte m, m2;
	byte b1;
	byte b2;
	int i, j;
	scancode_t scancode;
	keycode_t keycode;
	char charcode;
	
	byte keyboard_num_pressed_new;
	scancode_t keyboard_pressed_new[KEYBOARD_PRESSED_MAX];
	
	// Reset buffer
	keyboard_num_pressed_new = 0;
	
	// If nothing is pressed at all - skip scanning the matrix
	if (keyboard_ispressed() != 0) {
		// Scan the keyboard matrix
		
		#ifdef KEYBOARD_LATCH
			// BIOS4000 01a6: Send 0xff to port 0x11 (although in original firmware, keyboard matrix works without it)
			keyboard_port_matrix_latch = 0xff;
		#endif
		
		m = 0x01;	// Row bit mask
		for (ix = 0; ix < KEY_MATRIX1_COLS; ix++) {
			// Send bit mask to MUXer
			
			// BIOS4000 01bc: Send bit mask to 0x10
			keyboard_port_matrix_col_out = m;
			
			// Get matrix 1 state
			b1 = keyboard_port_matrix1_row_in;
			
			// Get matrix 2 state
			b2 = keyboard_port_matrix2_row_in;
			
			// BIOS4000 01c7: Reset it back to 0x00
			keyboard_port_matrix_col_out = 0x00;
			
			// Check matrix input 1
			if (b1 != 0xff) {
				m2 = 0x01;	// Bit mask
				for (iy = 0; iy < KEY_MATRIX1_ROWS; iy++) {
					if ((b1 & m2) == 0) {
						// Return first bit found. We could handle simulataneous presses!
						//return vgl_key_map[8 * row + col];
						
						// Just return the scan code
						//return 0x00 + (8 * row + col);
						
						// Store scan code
						if (keyboard_num_pressed_new < KEYBOARD_PRESSED_MAX)
							keyboard_pressed_new[keyboard_num_pressed_new++] = iy*KEY_MATRIX1_ROWS + ix;
					}
					m2 = m2 << 1;
				}
			}
			
			// Check matrix input 2
			(void)b2;
			(void)m2;
			
			if (b2 != 0xff) {
				m2 = 0x01;	// Bit mask
				
				// Caution: Matrix 2 has only bits 0..4 (for a total of 40 activity buttons, including "ON" and "OFF")
				for (iy = 0; iy < KEY_MATRIX2_ROWS; iy++) {
					if ((b2 & m2) == 0) {
						// Return first bit found. We could handle simulataneous presses!
						//return vgl_key_map2[8 * row + col];
						
						// Just return the scan code
						//return 0x80 + (8 * row + col);
						
						// Store scan code
						if (keyboard_num_pressed_new < KEYBOARD_PRESSED_MAX)
							keyboard_pressed_new[keyboard_num_pressed_new++] = (KEY_MATRIX1_ROWS*KEY_MATRIX1_COLS) + iy*KEY_MATRIX2_COLS + ix;
					}
					m2 = m2 << 1;
				}
			}
			
			
			// Next row
			m = m << 1;
		}
		
	}
	
	// Done scanning the matrix
	
	// Check for key releases (i.e. scancodes in keyboard_pressed[] that are not in keyboard_pressed_new[] any more)
	for (i = 0; i < keyboard_num_pressed; i++) {
		scancode = keyboard_pressed[i];
		
		// See if it is still pressed...
		for (j = 0; j < keyboard_num_pressed_new; j++) {
			if (keyboard_pressed_new[j] == scancode) {
				// Key is still pressed
				scancode = KEYBOARD_SCANCODE_INVALID;
				break;
			}
		}
		if (scancode == KEYBOARD_SCANCODE_INVALID) continue;
		// Key was released
		
		keycode = KEY_CODES[scancode];
		if (keycode == KEY_NONE) continue;
		
		//printf("KeyUp%02X", scancode);
		//putchar('U'); printf_x2(scancode);
		
		// Clear modifier status
		/*
		switch (keycode) {
			//case KEY_LEFT_SHIFT:	keyboard_modifiers &= (0xff - KEYBOARD_MODIFIER_SHIFT); break;
			//case KEY_RIGHT_SHIFT:	keyboard_modifiers &= (0xff - KEYBOARD_MODIFIER_SHIFT); break;
			case KEY_SHIFT:			keyboard_modifiers &= (0xff - KEYBOARD_MODIFIER_SHIFT); break;
			case KEY_ALT:			keyboard_modifiers &= (0xff - KEYBOARD_MODIFIER_ALT); break;
			//case KEY_SYMBOL: 		keyboard_modifiers &= (0xff - KEYBOARD_MODIFIER_SYMBOL); break;
		}
		*/
		if (keycode == KEY_SHIFT)	keyboard_modifiers &= ~KEYBOARD_MODIFIER_SHIFT;
		if (keycode == KEY_ALT)		keyboard_modifiers &= ~KEYBOARD_MODIFIER_ALT;
		
		// Remove (copy last element there)
		keyboard_pressed[i] = keyboard_pressed[--keyboard_num_pressed];
		i--;
	}
	
	// Check for key presses (i.e. scancodes in keyboard_pressed_new[] that are not in keyboard_pressed[])
	for (i = 0; i < keyboard_num_pressed_new; i++) {
		scancode = keyboard_pressed_new[i];
		
		// See if it was pressed before...
		for (j = 0; j < keyboard_num_pressed; j++) {
			if (keyboard_pressed[j] == scancode) {
				// Key was already pressed
				scancode = KEYBOARD_SCANCODE_INVALID;
				break;
			}
		}
		
		if (scancode == KEYBOARD_SCANCODE_INVALID) continue;
		
		// Key was previously unknown
		
		// Handle key press
		keycode = KEY_CODES[scancode];
		if (keycode == KEY_NONE) continue;
		
		//printf("KeyDown%02X", scancode);
		//putchar('D'); printf_x2(scancode);
		
		// Set modifier status
		/*
		switch (keycode) {
			//case KEY_LEFT_SHIFT:	keyboard_modifiers |= KEYBOARD_MODIFIER_SHIFT; break;
			//case KEY_RIGHT_SHIFT:	keyboard_modifiers |= KEYBOARD_MODIFIER_SHIFT; break;
			case KEY_SHIFT:			keyboard_modifiers |= KEYBOARD_MODIFIER_SHIFT; break;
			case KEY_ALT:			keyboard_modifiers |= KEYBOARD_MODIFIER_ALT; break;
			//case KEY_SYMBOL:		keyboard_modifiers |= KEYBOARD_MODIFIER_SYMBOL; break;
			
			default:
		*/
		if (keycode == KEY_SHIFT)	keyboard_modifiers |= KEYBOARD_MODIFIER_SHIFT;
		else
		if (keycode == KEY_ALT)		keyboard_modifiers |= KEYBOARD_MODIFIER_ALT;
		else {
			// Normal key (not a modifier)
			
			// Map keycode to charcode
			charcode = keycode;
			
			if (keycode == KEY_ENTER) {
				// Allow all kinds of ENTER mappings
				if (keyboard_modifiers & KEYBOARD_MODIFIER_SHIFT) charcode = '\n';	// Force NL
				else if (keyboard_modifiers & KEYBOARD_MODIFIER_ALT) charcode = '\r';	// Force CR
				else charcode = KEY_CHARCODE_ENTER;	// Default
			}
			else
			if ((keyboard_modifiers & KEYBOARD_MODIFIER_ALT) > 0) {
				// Like Ctrl: A=chr(1), B=chr(2), C=chr(3), ...
				charcode = 1 + (keycode - 'a');
			}
			else
			if ((keyboard_modifiers & KEYBOARD_MODIFIER_SHIFT) > 0) {
				// Shift
				
				switch(keycode) {
					// German layout
					case '1': charcode = '!'; break;
					case '2': charcode = '"'; break;
					case '3': charcode = 'ß'; break;	// § becomes "ß" on german layout
					case '4': charcode = '$'; break;
					case '5': charcode = '%'; break;
					case '6': charcode = '&'; break;
					case '7': charcode = '/'; break;
					case '8': charcode = '('; break;
					case '9': charcode = ')'; break;
					case '0': charcode = '='; break;
					
					case ',': charcode = ';'; break;
					case '.': charcode = ':'; break;
					case '-': charcode = '_'; break;
					
					case 'ä': charcode = 'Ä'; break;
					case 'ü': charcode = 'Ü'; break;
					case 'ö': charcode = 'Ö'; break;
					
					case KEY_CURSOR_LEFT: charcode = '<'; break;
					case KEY_CURSOR_RIGHT: charcode = '>'; break;
					
					// Add more shift-mappings here!
					default:
						if ((keycode >= 'a') && (keycode <= 'z')) {
							// Upper case
							charcode = 'A' + (keycode - 'a');
						}
				}
			}
			
			// Store CHARCODE to buffer
			keyboard_buffer[keyboard_buffer_in] = charcode;
			keyboard_buffer_in = (keyboard_buffer_in + 1) % KEYBOARD_BUFFER_MAX;
			// if (keyboard_buffer_in == keyboard_buffer_out) { FULL! }
		}
		
		// Store SCANCODE as "pressed"
		if (keyboard_num_pressed < KEYBOARD_PRESSED_MAX)
			keyboard_pressed[keyboard_num_pressed++] = scancode;
		
	}
	
}


#define KEY_CHARCODE_NONE 0
byte keyboard_inkey() {
	byte charcode;
	
	keyboard_update();
	
	// Check if a new key has been put to the buffer
	if (keyboard_buffer_in != keyboard_buffer_out) {
		
		// Get from buffer
		charcode = keyboard_buffer[keyboard_buffer_out];
		keyboard_buffer_out = (keyboard_buffer_out + 1) % KEYBOARD_BUFFER_MAX;
		
		// Return it
		return charcode;
	} else {
		// No key
		return KEY_CHARCODE_NONE;
	}
}


byte keyboard_getchar() {
	byte charcode;
	
	while((charcode = keyboard_inkey()) == KEY_CHARCODE_NONE) {
		// Block
	}
	
	return charcode;
	
}


#endif // __KEYBOARD_H