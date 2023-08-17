#ifndef __KEYBOARD_H
#define __KEYBOARD_H

/*
Keyboard matrix for the VTech Genius LEADER 6000SL / PreComputer Prestige


TODO:
	
	* Turn on/off CAPS LOCK LED:
		OUT 0x21, bit 6, bit 7 or 0xE0 (default)
			bit 6 (0x20) = CAPS LOCK light

*/

// Keyboard matrix at 0x40w, 0x41r, 0x42r
#define KEYBOARD_PORT_ROW_OUT 0x40
#define KEYBOARD_PORT_COL_IN1 0x41
#define KEYBOARD_PORT_COL_IN2 0x42

/*
__sfr __at KEYBOARD_PORT_ROW_OUT keyboard_port_w;
__sfr __at KEYBOARD_PORT_COL_IN1 keyboard_port_r1;
__sfr __at KEYBOARD_PORT_COL_IN2 keyboard_port_r2;

#define keyboard_matrix_out(v) keyboard_port_w=v
#define keyboard_matrix_in1() keyboard_port_r1
#define keyboard_matrix_in2() keyboard_port_r2
*/
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



// Key codes
#define KEY_MOUSE_LMB 1
#define KEY_MOUSE_RMB 2

#define KEY_TOUCH_UP 24
#define KEY_TOUCH_DOWN 25
#define KEY_TOUCH_LEFT 27
#define KEY_TOUCH_RIGHT 26
#define KEY_TOUCH_LMB 1
#define KEY_TOUCH_RMB 2

#define KEY_UP 24
#define KEY_DOWN 25
#define KEY_LEFT 27
#define KEY_RIGHT 26

#define KEY_SPACE 0x20
#define KEY_BACKSPACE 0x08
#define KEY_ESCAPE 27
#define KEY_ENTER 13
#define KEY_INSERT 'I'

#define KEY_HELP '?'
#define KEY_ANSWER '!'
#define KEY_PLAYER 'P'
#define KEY_LEVEL 'L'
#define KEY_REPEAT 'R'

#define KEY_ACTIVITY_WORDGAMES 0x81
#define KEY_ACTIVITY_MATH 0x82
#define KEY_ACTIVITY_TRIVIA 0x83
#define KEY_ACTIVITY_LOGIC 0x84
#define KEY_ACTIVITY_BUSINESS 0x85
#define KEY_CARTRIDGE 254

#define KEY_OFF 'O'
#define KEY_SYMBOL '$'
#define KEY_ALT 'A'
#define KEY_CAPS 'C'
#define KEY_LEFT_SHIFT 'S'
#define KEY_RIGHT_SHIFT 'T'

typedef byte keycode_t;
typedef byte scancode_t;

// Map scancode to keycode (i.e. char)
const keycode_t KEY_CODES[8*8*2] = {
	KEY_MOUSE_LMB,  '1', '9', 'e', '(', 'g', KEY_LEFT_SHIFT, ',',
	KEY_MOUSE_RMB,  '2', '0', 'r', '+', 'h', 'z', '.',
	KEY_TOUCH_UP ,  '3', '\'', 't', KEY_INSERT, 'j', 'x', '-',
	KEY_TOUCH_LMB,  '4', ')', 'y', KEY_CAPS, 'k', 'c', KEY_UP,
	KEY_TOUCH_RMB,  '5', KEY_BACKSPACE, 'u', 'a', 'l', 'v', KEY_RIGHT_SHIFT,
	KEY_TOUCH_DOWN, '6', KEY_ESCAPE, 'i', 's', '\\', 'b', KEY_HELP,
	                '?', '7', 'q', 'o', 'd', '/', 'n', KEY_SYMBOL, 
	KEY_OFF,        '8', 'w', 'p', 'f', KEY_ENTER, 'm', KEY_ANSWER,
	
	KEY_SPACE, KEY_ACTIVITY_WORDGAMES, KEY_PLAYER, 0,0,0,0,0,
	KEY_ALT, KEY_ACTIVITY_MATH, KEY_LEVEL, 0,0,0,0,0,
	KEY_REPEAT, KEY_ACTIVITY_TRIVIA, KEY_CARTRIDGE, 0,0,0,0,0,
	KEY_LEFT, KEY_ACTIVITY_LOGIC, KEY_ACTIVITY_BUSINESS, 0,0,0,0,0,
	KEY_DOWN, 0,0,0,0,0,0,0,
	KEY_RIGHT, 0,0,0,0,0,0,0,
	KEY_TOUCH_LMB, 0,0,0,0,0,0,0,
	KEY_TOUCH_RMB, 0,0,0,0,0,0,0,
};


#define KEYBOARD_PRESSED_MAX 6
#define KEYBOARD_BUFFER_MAX 8
#define KEYBOARD_SCANCODE_INVALID 0xff

#define KEYBOARD_MODIFIER_SHIFT 1
#define KEYBOARD_MODIFIER_ALT 2
#define KEYBOARD_MODIFIER_SYMBOL 4

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
	for (i = 0; i < KEYBOARD_PRESSED_MAX; i++) keyboard_pressed[i] = 0xff;
	keyboard_modifiers = 0x00;
}


// Return "true" if a key is pressed
byte keyboard_ispressed() {
	byte b;
	
	// Activate ALL matrix lines
	keyboard_matrix_out(0x00);
	
	// Get both matrix return values (active LOW, so ANDing them will yield zeros if any of the lines is zero)
	//b = (keyboard_matrix_in1() & keyboard_matrix_in2());
	b = keyboard_matrix_in1();
	b &= keyboard_matrix_in2();
	
	// Disable ALL matrix lines
	keyboard_matrix_out(0xff);
	
	// Check if ANY of the bits are LOW
	// At least one key is pressed
	if (b != 0xff) return 1;
	
	// No keys are pressed
	return 0;
}

void keyboard_update() {
	byte mx, my;
	byte b;
	int i, j;
	scancode_t scancode;
	keycode_t keycode;
	char charcode;
	
	byte keyboard_num_pressed_new;
	scancode_t keyboard_pressed_new[KEYBOARD_PRESSED_MAX];
	
	keyboard_num_pressed_new = 0;
	
	if (keyboard_ispressed() > 0) {
		// Scan the keyboard matrix
		
		for(my = 0; my < 8; my++) {
			keyboard_matrix_out(0xff - (1 << my));
			
			// Check matrix input 1
			b = keyboard_matrix_in1();
			for(mx = 0; mx < 8; mx++) {
				if (!(b & (1 << mx))) {
					// Store scan code
					if (keyboard_num_pressed_new < KEYBOARD_PRESSED_MAX)
						keyboard_pressed_new[keyboard_num_pressed_new++] = my*8 + mx;
				}
			}
			
			// Check matrix input 2
			b = keyboard_matrix_in2();
			for(mx = 0; mx < 8; mx++) {
				if (!(b & (1 << mx))) {
					// Store scan code
					if (keyboard_num_pressed_new < KEYBOARD_PRESSED_MAX)
						keyboard_pressed_new[keyboard_num_pressed_new++] = (8*8) + my*8 + mx;
				}
			}
		}
	}
	
	// Check for key releases (i.e. scancodes in keyboard_pressed[] that are not in keyboard_pressed_new[] any more)
	for (i = 0; i < keyboard_num_pressed; i++) {
		scancode = keyboard_pressed[i];
		for (j = 0; j < keyboard_num_pressed_new; j++) {
			if (keyboard_pressed_new[j] == scancode) {
				// Key is still pressed
				scancode = KEYBOARD_SCANCODE_INVALID;
				break;
			}
		}
		if (scancode != KEYBOARD_SCANCODE_INVALID) {
			// Key was released
			keycode = KEY_CODES[scancode];
			//printf("KeyUp%02X", scancode);
			//putchar('U'); putchar(scancode);
			
			// Update modifier status
			switch (keycode) {
				case KEY_LEFT_SHIFT:	keyboard_modifiers &= (0xff - KEYBOARD_MODIFIER_SHIFT); break;
				case KEY_RIGHT_SHIFT:	keyboard_modifiers &= (0xff - KEYBOARD_MODIFIER_SHIFT); break;
				case KEY_ALT:			keyboard_modifiers &= (0xff - KEYBOARD_MODIFIER_ALT); break;
				case KEY_SYMBOL:		keyboard_modifiers &= (0xff - KEYBOARD_MODIFIER_SYMBOL); break;
			}
			
			// Remove (copy last element there)
			keyboard_pressed[i] = keyboard_pressed[--keyboard_num_pressed];
			i--;
		}
	}
	
	// Check for key presses (i.e. scancodes in keyboard_pressed_new[] that are not in keyboard_pressed[])
	for (i = 0; i < keyboard_num_pressed_new; i++) {
		scancode = keyboard_pressed_new[i];
		for (j = 0; j < keyboard_num_pressed; j++) {
			if (keyboard_pressed[j] == scancode) {
				// Key was already pressed
				scancode = KEYBOARD_SCANCODE_INVALID;
				break;
			}
		}
		if (scancode != KEYBOARD_SCANCODE_INVALID) {
			// Key was previously unknown
			
			// Handle key press
			keycode = KEY_CODES[scancode];
			//printf("KeyDown%02X", scancode);
			//putchar('D'); putchar(scancode);
			
			switch (keycode) {
				case KEY_LEFT_SHIFT:	keyboard_modifiers |= KEYBOARD_MODIFIER_SHIFT; break;
				case KEY_RIGHT_SHIFT:	keyboard_modifiers |= KEYBOARD_MODIFIER_SHIFT; break;
				case KEY_ALT:			keyboard_modifiers |= KEYBOARD_MODIFIER_ALT; break;
				case KEY_SYMBOL:		keyboard_modifiers |= KEYBOARD_MODIFIER_SYMBOL; break;
				
				default:
					// Normal key
					charcode = keycode;
				
					if (keyboard_modifiers & KEYBOARD_MODIFIER_SYMBOL > 0) {
						// Symbol
						charcode = keycode - 'a' + 0x01;
					} else
					if (keyboard_modifiers & KEYBOARD_MODIFIER_SHIFT > 0) {
						// Shift
						if ((keycode >= 'a') && (keycode <= 'z')) {
							charcode = 'A' + (keycode - 'a');
						}
						if ((keycode >= '1') && (keycode <= '9')) {
							charcode = '!' + (keycode - '1');
						}
					} else
					if (keycode == KEY_ENTER) {
						// ENTER to NewLine
						charcode = '\n';
					} else {
						// Char code equals key code
						charcode = keycode;
					}
					
					// Store to buffer
					keyboard_buffer[keyboard_buffer_in] = charcode;
					keyboard_buffer_in = (keyboard_buffer_in + 1) % KEYBOARD_BUFFER_MAX;
					// if (keyboard_buffer_in == keyboard_buffer_out) { FULL! }
			}
			
			
			// Store scancode
			if (keyboard_num_pressed < KEYBOARD_PRESSED_MAX)
				keyboard_pressed[keyboard_num_pressed++] = scancode;
			
		}
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

//#define getchar keyboard_getchar
//#define inkey keyboard_inkey

#endif	//__KEYBOARD_H