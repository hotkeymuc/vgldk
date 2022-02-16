
#include <vgldk.h>

byte hexDigit(byte c) {
	if (c < 10)
		return ('0'+c);
	return 'A' + (c-10);
}
void drawHex(byte x, byte y, byte c) {
	drawGlyph(x, y, hexDigit(c >> 4));
	drawGlyph(x + font_w, y, hexDigit(c & 0x0f));
}


void port_toggle(byte p, byte n) {
	byte data;
	data = port_in(p);
	data ^= n;
	port_out(p, data);
}


void main() __naked {
	//int i;
	byte x, y;
	byte x0;
	byte mx, my;
	byte b;
	//byte c;
	//byte *p;
	byte p;
	byte scanCode;
	byte currentChar;
	byte lastChar;
	
	
	
	//lcd_clear();
	//vgldk_init();
	
	//drawString(0, 0, "Hello!");
	
	lastChar = 0xff;
	currentChar = 0xff;
	p = 0x21;
	
	while(1) {
		
		x = 0;
		y = 0;
		
		b = port_in(0x04); // Mouse X
		drawString(x, y, "Mouse X="); x += 8 * font_w;
		drawHex(x, y, b); x += 2 * font_w;
		
		x += font_w;
		//y += font_h;
		
		b = port_in(0x05); // Mouse Y
		drawString(x, y, "Y="); x += 2 * font_w;
		drawHex(x, y, b); x += 2 * font_w;
		
		//x += font_w;
		//y += font_h;
		x0 = 192;
		
		x = x0; y += font_h;
		
		b = port_in(0x21);
		drawString(x, y, "21="); x += 3 * font_w;
		drawHex(x, y, b); x += 2 * font_w;
		
		//x += font_w;
		x = x0; y += font_h;
		
		b = port_in(p);
		drawHex(x, y, p); x += 2 * font_w;
		drawString(x, y, "="); x += 1 * font_w;
		drawHex(x, y, b); x += 2 * font_w;
		
		//x += font_w;
		x = x0; y += font_h;
		
		//
		
		
		x = 0;
		//y += font_h;
		y = 1 * font_h;
		
		
		// Keyboard Matrix
		
		// Check for ANY key
		port_out_0x40(0x00);	// Activate ALL
		b = port_in_0x41() & port_in_0x42();
		drawString(x, y, "Matrix="); x += 7 * font_w;
		drawHex(x, y, b);
		y += font_h;
		
		scanCode = 0xff;
		//if (b < 0xff) {
			// Some key is pressed
			lastChar = currentChar;
			
			// scan it
			scanCode = 0xff;
			for(my = 0; my < 8; my++) {
				port_out_0x40(0xff - (1 << my));
				//port_out_0x40(my);
				
				x = 0;
				
				b = port_in_0x41();
				for(mx = 0; mx < 8; mx++) {
					drawGlyph(x, y, (b & (1 << mx)) ? '.' : 'X');
					x += font_w;
					
					if (!(b & (1 << mx))) scanCode = my*8 + mx;
				}
				
				x += font_w;
				
				b = port_in_0x42();
				for(mx = 0; mx < 8; mx++) {
					drawGlyph(x, y, (b & (1 << mx)) ? '.' : 'X');
					x += font_w;
					
					if (!(b & (1 << mx))) scanCode = 0x40 + my*8 + mx;
				}
				
				y += font_h;
			}
			
			if (scanCode < 0xff) {
				currentChar = VGL_KEY_CODES[scanCode];
				
				// Show scan code
				x = 0;
				drawHex(x, y, scanCode); x += 2*font_w;
				
				x += font_w;
				
				// Show ASCII code
				drawGlyph(x, y, '"'); x += font_w;
				drawGlyph(x, y, currentChar); x += font_w;
				drawGlyph(x, y, '"'); x += font_w;
				
			} else {
				currentChar = 0xff;
			}
			
		//}
		
		
		// Handle key strokes
		if ((currentChar != 0xff) && (currentChar != lastChar)) {
			// New key press
			
			switch(currentChar) {
				case '1': port_toggle(p, 0x01); break;
				case '2': port_toggle(p, 0x02); break;
				case '3': port_toggle(p, 0x04); break;
				case '4': port_toggle(p, 0x08); break;
				case '5': port_toggle(p, 0x10); break;
				case '6': port_toggle(p, 0x20); break;
				case '7': port_toggle(p, 0x40); break;
				case '8': port_toggle(p, 0x80); break;
				
				case VGL_KEY_RIGHT: p++; break;
				case VGL_KEY_LEFT: p--; break;
			}
			
		}
		
	}
	
	
	
	// Loop forever
	//while(1) {}
	
}
