#ifndef __KEYBOARD_H
#define __KEYBOARD_H


// Keyboard at 0x40w, 0x41r, 0x42r
// Keyboard matrix write
void port_out_0x40(byte a) __naked {(void)a;
__asm
	; Get parameter from stack into a
	ld hl,#0x0002
	add hl,sp
	ld a,(hl)
	
	; Put it to port
	out	(0x40), a
	ret
__endasm;
}
// Keyboard matrix read 1
byte port_in_0x41() __naked {
__asm
	in	a, (0x41)
	ld	l, a
	ret
__endasm;
}
// Keyboard matrix read 2
byte port_in_0x42() __naked {
__asm
	in	a, (0x42)
	ld	l, a
	ret
__endasm;
}





#define VGL_KEY_MOUSE_LMB 1
#define VGL_KEY_MOUSE_RMB 2

#define VGL_KEY_TOUCH_UP 24
#define VGL_KEY_TOUCH_DOWN 25
#define VGL_KEY_TOUCH_LEFT 27
#define VGL_KEY_TOUCH_RIGHT 26
#define VGL_KEY_TOUCH_LMB 1
#define VGL_KEY_TOUCH_RMB 2

#define VGL_KEY_UP 24
#define VGL_KEY_DOWN 25
#define VGL_KEY_LEFT 27
#define VGL_KEY_RIGHT 26

#define VGL_KEY_SPACE 0x20
#define VGL_KEY_BACKSPACE 0x08
#define VGL_KEY_ESCAPE 27
#define VGL_KEY_ENTER 13
#define VGL_KEY_INSERT 'I'

#define VGL_KEY_HELP '?'
#define VGL_KEY_ANSWER '!'
#define VGL_KEY_PLAYER 'P'
#define VGL_KEY_LEVEL 'L'
#define VGL_KEY_REPEAT 'R'

#define VGL_KEY_ACTIVITY_WORDGAMES 0x81
#define VGL_KEY_ACTIVITY_MATH 0x82
#define VGL_KEY_ACTIVITY_TRIVIA 0x83
#define VGL_KEY_ACTIVITY_LOGIC 0x84
#define VGL_KEY_ACTIVITY_BUSINESS 0x85
#define VGL_KEY_CARTRIDGE 254

#define VGL_KEY_OFF 'O'
#define VGL_KEY_SYMBOL '$'
#define VGL_KEY_ALT 'A'
#define VGL_KEY_CAPS 'C'
#define VGL_KEY_LEFT_SHIFT 'S'
#define VGL_KEY_RIGHT_SHIFT 'S'

const byte VGL_KEY_CODES[8*16] = {
	VGL_KEY_MOUSE_LMB,  '1', '9', 'e', '(', 'g', VGL_KEY_LEFT_SHIFT, ',',
	VGL_KEY_MOUSE_RMB,  '2', '0', 'r', '+', 'h', 'z', '.',
	VGL_KEY_TOUCH_UP ,  '3', '\'', 't', VGL_KEY_INSERT, 'j', 'x', '-',
	VGL_KEY_TOUCH_LMB,  '4', ')', 'y', VGL_KEY_CAPS, 'k', 'c', VGL_KEY_UP,
	VGL_KEY_TOUCH_RMB,  '5', VGL_KEY_BACKSPACE, 'u', 'a', 'l', 'v', VGL_KEY_RIGHT_SHIFT,
	VGL_KEY_TOUCH_DOWN, '6', VGL_KEY_ESCAPE, 'i', 's', '\\', 'b', VGL_KEY_HELP,
	               '?', '7', 'q', 'o', 'd', '/', 'n', VGL_KEY_SYMBOL, 
	VGL_KEY_OFF,        '8', 'w', 'p', 'f', VGL_KEY_ENTER, 'm', VGL_KEY_ANSWER,
	
	               VGL_KEY_SPACE, VGL_KEY_ACTIVITY_WORDGAMES, VGL_KEY_PLAYER, 0,0,0,0,0,
	VGL_KEY_ALT, VGL_KEY_ACTIVITY_MATH, VGL_KEY_LEVEL, 0,0,0,0,0,
	VGL_KEY_REPEAT, VGL_KEY_ACTIVITY_TRIVIA, VGL_KEY_CARTRIDGE, 0,0,0,0,0,
	VGL_KEY_LEFT, VGL_KEY_ACTIVITY_LOGIC, VGL_KEY_ACTIVITY_BUSINESS, 0,0,0,0,0,
	VGL_KEY_DOWN, 0,0,0,0,0,0,0,
	VGL_KEY_RIGHT, 0,0,0,0,0,0,0,
	VGL_KEY_TOUCH_LMB, 0,0,0,0,0,0,0,
	VGL_KEY_TOUCH_RMB, 0,0,0,0,0,0,0,
};


// Return "true" if a key is pressed
byte checkkey() {
	byte b;
	
	// Activate ALL matrix lines
	port_out_0x40(0x00);
	
	// Get both return values
	b = (port_in_0x41() & port_in_0x42());
	
	// Disable ALL matrix lines
	port_out_0x40(0xff);
	
	// check if ANY of the bits is LOW
	if (b != 0xff) return 1;
	
	return 0;	// No keys are pressed
}

char getchar_last = 0;
char getchar() {
	byte mx, my;
	byte b;
	byte scanCode;
	byte currentChar;
	//byte lastChar;
	
	scanCode = 0xff;
	
	while(1) {
		
		// Scan the keyboard matrix
		scanCode = 0xff;
		
		for(my = 0; my < 8; my++) {
			port_out_0x40(0xff - (1 << my));
			//port_out_0x40(my);
			
			b = port_in_0x41();
			for(mx = 0; mx < 8; mx++) {
				if (!(b & (1 << mx))) scanCode = my*8 + mx;
			}
			
			b = port_in_0x42();
			for(mx = 0; mx < 8; mx++) {
				if (!(b & (1 << mx))) scanCode = 0x40 + my*8 + mx;
			}
		}
		
		if (scanCode < 0xff) {
			currentChar = VGL_KEY_CODES[scanCode];
			
			// Key changed: break
			if (currentChar != getchar_last) break;
			
		} else {
			currentChar = 0xff;
		}
		
	}
	
	getchar_last = currentChar;	// Save for later
	
	return currentChar;
}



#endif	//__KEYBOARD_H