#ifndef __HEX_H
#define __HEX_H
/*
Some simple hex utils
*/

byte hexDigit(byte c) {
	if (c < 10)
		return ('0'+c);
	return 'A' + (c-10);
}


//#include "hex.h"
void printf_x2(byte b) {
	//putchar(hexdigit[b >> 4]); putchar(hexdigit[b & 0x0f]);
	putchar(hexDigit(b >> 4)); putchar(hexDigit(b & 0x0f));
}

void printf_x4(word w) {
	printf_x2(w >> 8);
	printf_x2(w & 0x00ff);
}


byte parse_hexDigit(byte c) {
	if (c > 'f') return 0;
	
	if (c < '0') return 0;
	if (c <= '9') return (c - '0');
	
	if (c < 'A') return 0;
	if (c <= 'F') return (10 + c - 'A');
	
	if (c < 'a') return 0;
	return (10 + c - 'a');
}

word hextown(const char *s, byte n) {
	byte i;
	word r;
	char c;
	
	r = 0;
	for(i = 0; i < n; i++) {
		c = *s++;
		if (c < '0') break;	// Break at zero char and any other non-ascii
		r = (r << 4) + (word)parse_hexDigit(c);
	}
	return r;
}

byte hextob(const char *s) {
	return (byte)hextown(s, 2);
}

word hextow(const char *s) {
	return hextown(s, 4);
}

/*
byte parse_hex8(byte *text) {
	return parse_hexDigit(*text++) * 0x10 + parse_hexDigit(*text);
}

word parse_hex16(byte *text) {
	return
		(word)(parse_hexDigit(*text++)) * 0x1000 +
		(word)(parse_hexDigit(*text++)) * 0x0100 +
		(word)(parse_hexDigit(*text++)) * 0x0010 +
		(word)(parse_hexDigit(*text))   * 0x0001
	;
}

void put_hex8(byte c) {
	putchar(hexDigit(c >> 4));
	putchar(hexDigit(c & 0x0f));
}

void put_hex16(word d) {
	putchar(d >> 8);
	putchar(d & 0xff);
}



byte gethexchar() {
	char c = getchar();
	if ((c >= '0') && (c <= '9')) return c - '0';
	else if ((c >= 'A') && (c <= 'F')) return c - 'A' + 0x0a;
	else if ((c >= 'a') && (c <= 'f')) return c - 'a' + 0x0a;
	return 0;
}

word gethex(byte digits) {
	word v;
	byte b;
	byte i;
	
	v = 0;
	for(i = 0; i < digits; i++) {
		printf("_\b");
		b = gethexchar();
		printf("%01X", b);
		v = v * 0x10 + b;
	}
	return v;
}
*/

#ifdef HEX_USE_DUMP
// Some memory dump helpers
#define DUMP_WIDTH 4
void dump(word a, byte len) {
	byte i;
	byte b;
	byte *o;
	byte l;
	byte lLine;
	
	//clear();
	//12345678901234567890
	//AAAAhh hh hh hh ....
	//AAAA hh hh hh hh....
	//AAAA hhhhhhhh ....
	
	l = 0;
	while (l < len) {
		//printf("%04X|", a);
		printf_x4(a); putchar('|');
		
		lLine = l;
		o = (byte *)a;
		for (i = 0; i < DUMP_WIDTH; i++) {
			if (l < len) {
				b = *o;
				//printf("%02X", b);
				printf_x2(b);
			} else {
				printf("  ");
			}
			l++;
			o++;
		}
		putchar('|');
		l = lLine;
		o = (byte *)a;
		for (i = 0; i < DUMP_WIDTH; i++) {
			if (l < len) {
				b = *o;
				if (b < 0x20)	putchar('.');
				else			putchar(b);
			} else putchar(' ');
			l++;
			o++;
		}
		a += DUMP_WIDTH;
		printf("\n");
	}
	
	/*
	// AAAAbbbbbbbbbbbbbbbb
	// hh hh hh hh hh hh hh
	//  hh hh hh hh hh hh h
	// h hh hh
	printf("%04X", a);
	o = (byte *)a;
	for(i = 0; i < len; i++) {
		b = *o;
		//printf("%02X", b);
		if (b < 0x20)
			putchar('.');
		else
			putchar(b);
		o++;
	}
	
	
	if (len != 16) printf("\n");
	
	o = (byte *)a;
	for(i = 0; i < len; i++) {
		b = *o;
		//printf("%02X ", b);
		printf("%02X", b);
		o++;
	}
	printf("\n");
	*/
}
#endif // USE_HEX_DUMP
#endif //__HEX_H