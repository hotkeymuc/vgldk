#ifndef __BDOS_HOST_H__
#define __BDOS_HOST_H__

/*

This file contains functions to allow sending FCBs top a host.
The host might be connected via serial (SoftUART) or be a MAME emulator running a virtual instance of CP/M.

2023-08-22 Bernhard "HotKey" Slawik
*/

//#define BDOS_HOST_TO_PAPER_TAPE	// Re-direct to BIOS paper tape routines (and let BIOS decide what to do)
//#define BDOS_HOST_TO_SOFTUART	// Re-direct to SoftUART (if included in BDOS source)
//#define BDOS_HOST_TO_MAME	// Re-direct to MAME (if included in BDOS source)

#ifdef BDOS_HOST_TO_PAPER_TAPE
	void papertape_send(const byte *data, byte len) {
		byte i;
		
		bios_punch(len);
		for (i = 0; i < len; i++) {
			bios_punch(*data++);
		}
	}
	int papertape_receive(byte *data) {
		byte l;
		byte i;
		l = bios_reader();
		for (i = 0; i < l; i++) {
			*data++ = bios_reader();
		}
		return l;
	}
	#define host_send papertape_send
	#define host_receive papertape_receive
#endif

#ifdef BDOS_HOST_TO_MAME
	#include <driver/mame.h>
	
	#define host_send mame_put
	#define host_receive mame_getchar
#endif

#ifdef BDOS_HOST_TO_SOFTUART
	#include <driver/softuart.h>
	
	#define HOST_SERIAL_MAX_LINE 255	// Maximum length of one incoming line
	#define HOST_SERIAL_TIMEOUT 8192
	
	//#define serial_debug(s)	printf(s)
	#define serial_debug(s)	;
	/*
	void dump(byte *pb) {
		byte i;
		byte j;
		//printf_x4((word)pb); putchar(' ');
		
		for(j = 0; j < 4; j++) {
			printf_x2((word)pb & 0xff);
			putchar(':');
			for(i = 0; i < 8; i++) {
				printf_x2(*pb++);
			}
			printf("\n");
		}
	}
	*/
	/*
	void serial_put_d(byte num)  {
		serial_putchar('0' + ((num / 100) % 10));
		serial_putchar('0' + ((num / 10) % 10));
		serial_putchar('0' + (num % 10));
	}

	void serial_put_hexdigit(byte d)  {
		if (d > 9) serial_putchar('A' - 9 + d);
		else serial_putchar('0' + d);
	}
	void serial_put_hex(byte num)  {
		serial_put_hexdigit(num >> 4);
		serial_put_hexdigit(num & 0x0f);
	}
	*/

	int serial_getchar2() {
		// Get char, return -1 on timeout
		int c;
		word timeout;
		
		timeout = HOST_SERIAL_TIMEOUT;
		
		c = -1;
		while (c <= 0) {
			c = serial_getchar();
			timeout --;
			if (timeout == 0) return -1;
		}
		return c;
	}

	int serial_gets2(byte *serial_get_buf) {
		// Receive with timeout. Returns -1 on timeout or length
		int c;
		byte *b;
		byte l;
		word timeout;
		
		timeout = HOST_SERIAL_TIMEOUT;
		
		l = 0;
		b = serial_get_buf;
		while(1) {
			c = serial_getchar();
			
			if (c < 0) {
				// < 0 means "no data"
				timeout --;
				if (timeout == 0) return -1;
				continue;
			}
			timeout = HOST_SERIAL_TIMEOUT;
			
			// Check for end-of-line character(s)
			if ((c == 0x0a) || (c == 0x0d)) break;
			
			// Store in given buffer
			*b++ = c;
			l++;
			
		}
		
		// Terminate string
		*b++ = 0;
		
		// Return length
		//return (word)b - (word)serial_get_buf;
		
		// Return buf (like stdlib gets())
		return l;
	}

	int serial_gethex2() {
		// Receive two digits of hex, return -1 on timeout
		//byte r;
		int c;
		int c2;
		
		do {
			c = serial_getchar2();
			if (c < 0) return -1;
		} while (c == 'U');
		
		//c2 = serial_getchar2();
		c2 = serial_getchar();
		
		if (c < 0) return -1;
		if (c2 < 0) return -1;
		//putchar(c2);
		//printf_x2(c); printf_x2(c2);	//@FIXME: Bit debugging
		
		return (parse_hexDigit(c) << 4) + parse_hexDigit(c2);
	}

	void serial_sendSafe(const byte *data, byte ldata) {
		byte line[HOST_SERIAL_MAX_LINE];
		const byte *pdata;
		byte *pline; 
		byte lline;
		byte b;
		byte i;
		byte checkactual;
		byte checkgiven;
		int c;
		
		// Build HEX packet
		pline = &line[0];
		lline = 0;
		
		// Pad
		//line[0] = (byte)'U'; line[1] = (byte)'U'; line[2] = (byte)'U'; line[3] = (byte)'U'; lline = 4; pline = &line[4];
		line[0] = (byte)'U'; lline = 1; pline = &line[lline];
		
		// Len
		*pline++ = hexDigit(ldata >> 4);
		*pline++ = hexDigit(ldata & 0x0f);
		lline += 2;
		
		
		// Data
		checkactual = ldata;
		pdata = &data[0];
		for (i = 0; i < ldata; i++) {
			b = *pdata++;
			*pline++ = hexDigit(b >> 4);
			*pline++ = hexDigit(b & 0x0f);
			lline += 2;
			checkactual ^= b;
		}
		
		// Check
		*pline++ = hexDigit(checkactual >> 4);
		*pline++ = hexDigit(checkactual & 0x0f);
		lline += 2;
		
		// EOL
		*pline++ = '\n';
		lline++;
		
		*pline++ = 0;	// Zero-terminate
		
		//printf(line);
		
		while(1) {
			serial_debug("TX");
			__asm
				di
			__endasm;
			
			// Send synchronization
			//serial_puts("UUUUUUUU");
			serial_put(line, lline);
			
			
			c = serial_gethex2();
			
			__asm
				ei
			__endasm;
			
			if (c < 0) {
				printf("T!\n");
				continue;
			}
			
			checkgiven = c;
			
			if (checkgiven == checkactual) {
				// OK!
				//printf("OK\n");
				break;
			} else {
				//printf("C!\n");
				printf("C! g=");
				printf_x2(checkgiven);
				printf(",a=");
				printf_x2(checkactual);
				printf("!\n");
			}
		}
		serial_debug("OK\n");
	}

	int serial_receiveSafe(byte *data) {
		char line[HOST_SERIAL_MAX_LINE];
		byte l;
		char *pline;
		byte *pdata;
		byte linel;
		byte b;
		byte i;
		byte checkgiven;
		byte checkactual;
		//int r;
		
		serial_debug("RX");
		while(1) {
			
			// Send synchronization pad / request to answer
			//serial_put("GGGG", 4);
			//serial_puts("GG\n");
			//printf(".");
			
			//@TODO: Use non-blocking get to detect timeouts?
			serial_gets(&line[0]);
			linel = strlen(&line[0]);
			/*
			r = serial_gets2(&line[0]);
			if (r < 0) {
				//printf(".");
				//continue;
				return -1;
			}
			
			linel = r;
			*/
			//printf(line);
			
			pline = &line[0];
			
			// Strip sync header
			while(*pline == 'U') {
				linel--;
				pline++;
				if (linel < 4) break;
			}
			if (linel < 4) {
				// Too small to have length and check
				//printf("S!\n");
				printf("S!");
				printf_x2(linel);
				printf("<4!\n");
				//continue;
				return -1;
			}
			
			// Get length
			l = hextob(pline); pline += 2;
			if (((2 + l*2 + 2) != linel) || (l > MAX_DATA)) {
				// Error in length! Given length > actual length
				//printf("L!\n");
				printf("L! g=");
				printf_x2(l);
				printf(",a=");
				printf_x2((linel-4) >> 1);
				printf("!\n");
				//continue;
				return -1;
			}
			
			// Get data
			checkactual = l;
			pdata = data;
			for(i = 0; i < l; i++) {
				b = hextob(pline); pline+=2;
				*pdata++ = b;
				checkactual ^= b;
			}
			
			// Get checksum
			checkgiven = hextob(pline);	pline+= 2;
			
			if (checkgiven != checkactual) {
				// Checksums mismatch
				printf("C! g=");
				printf_x2(checkgiven);
				printf(",a=");
				printf_x2(checkactual);
				printf("!\n");
				//continue;
				return -1;
				
			} else {
				// ACK
				line[0] = 'U';
				line[1] = 'U';
				line[2] = 'U';
				line[3] = hexDigit(checkactual >> 4);
				line[4] = hexDigit(checkactual & 0x0f);
				line[5] = '\n';
				serial_put(&line[0], 6);
				
				return l;
			}
		}
		
	}

	
	#define host_send serial_sendSafe
	#define host_receive serial_receiveSafe
#endif


#ifdef BDOS_TRACE_CALLS
	volatile byte bdos_s;
	volatile byte bdos_p;
	void host_sendstack() {
		byte data[MAX_DATA];
		byte *pdata;
		byte *pstack;
		byte stack_count;
		
		/*
		//word bdos_sp;
		
		// Get stack pointer
		__asm
			; Write SP to bdos_param_s
			push af
			push hl
			
			ld hl, #0x04	; Skip the items we just pushed ourselves
			add hl, sp
			
			; Write to a global
			ld a, h
			ld (_bdos_s), a
			ld a, l
			ld (_bdos_p), a
			
			pop hl
			pop af
		__endasm;
		*/
		
		// Dump to serial
		pdata = &data[0];
		
		*pdata++ = 'D';	// D for Debug
		
		// Add registers
		#ifdef BDOS_SAVE_ALL_REGISTERS
			*pdata++ = bdos_param_a;
			*pdata++ = bdos_param_b;
		#else
			*pdata++ = 0;
			*pdata++ = 0;
		#endif
		
		*pdata++ = bdos_param_c;
		*pdata++ = bdos_param_d;
		*pdata++ = bdos_param_e;
		
		#ifdef BDOS_SAVE_ALL_REGISTERS
			*pdata++ = bdos_param_h;
			*pdata++ = bdos_param_l;
		#else
			*pdata++ = 0;
			*pdata++ = 0;
		#endif
		
		// Add stack pointer
		//*pdata++ = (word)bdos_sp >> 8;
		//*pdata++ = (word)bdos_sp & 0xff;
		*pdata++ = bdos_s;
		*pdata++ = bdos_p;
		
		// Add stack dump
		stack_count = 16;
		*pdata++ = stack_count;
		pstack = (byte *)(((word)bdos_s * 256) + (word)bdos_p);
		memcpy(pdata, pstack, stack_count * 2);
		
		host_send(data, 1 + 7 + 2 + 1 + stack_count * 2);
	
	}
#endif



void host_sendfcb(byte num, struct FCB *fcb) {
	byte data[1+1+2+36];
	byte *pdata;
	byte *pfcb;
	byte i;
	
	// Show on screen
	//printf_d("F", num); printf(fcb2name(fcb));
	
	
	// Dump to serial
	pdata = data;
	
	*pdata++ = 'F';
	*pdata++ = num;
	
	*pdata++ = (word)fcb >> 8;
	*pdata++ = (word)fcb & 0xff;
	
	pfcb = (char *)fcb;
	for (i = 0; i < 36; i++) {
		*pdata++ = *pfcb++;
	}
	
	host_send(data, 1+1+2+36);
	
	//@TODO: Handle answer from companion
}


byte host_receivefcb(struct FCB *fcb) {
	byte data[1 + 36 + 16];
	byte l;
	byte r;
	
	// Receive result only (make FCB alterations manually)
	//r = host_receivebyte();
	
	// Receive result and FCB
	do {
		l = host_receive(data);
	} while(l == 0);
	
	// Get result
	r = data[0];
	
	if (r == 0xff) {
		//@TODO: Error codes!?
		//printf("FCB FUN ERR!\n");
	} else {
		// Copy the received FCB over
		memcpy((byte *)fcb, (byte *)&data[1], 36);
	}
	
	return r;
}

word host_receivedma() {
	byte data[255];
	int l;
	byte *p;
	word ltotal;
	
	p = bios_dma;
	ltotal = 0;
	l = 1;
	while(l > 0) {
		l = host_receive(data);
		if (l > 0) {
			memcpy(p, data, l);
			p += l;
		}
	}
	
	ltotal = p - bios_dma;
	return ltotal;
}

byte host_receivebyte() {
	byte data[MAX_DATA];
	int l;
	
	l = 0;
	while(l == 0) {
		l = host_receive(data);
	}
	
	return data[0];
}

#endif