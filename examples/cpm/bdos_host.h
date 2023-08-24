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

#define BDOS_HOST_MAX_DATA 128
#define BDOS_HOST_DMA_MAX_DATA 128

#include "bdos.h"	// Need some numbers and functions

#include "fcb.h"

#include <stringmin.h>	// memcpy, memset, strlen

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
		// Wait for non-zero
		do {
			l = bios_reader();
		} while (l == 0);
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
	
	void mame_send(const byte *data, byte len) {
		byte i;
		mame_putchar(len);
		for (i = 0; i < len; i++) {
			mame_putchar(*data++);
		}
	}
	
	int mame_receive(byte *data) {
		byte l;
		byte i;
		//byte c;
		
		// Wait for non-zero
		//bdos_printf("RX=");
		do {
			l = mame_getchar();
		} while (l == 0);
		//bdos_printf_x2(l);
		//bdos_printf("..."); //bdos_getchar();
		
		for (i = 0; i < l; i++) {
			//bdos_printf_x2(i);
			//bdos_putchar('.');
			
			*data++ = mame_getchar();
			
			//c = mame_getchar();
			//bdos_printf_x2(c);
			//*data++ = c;
		}
		return l;
	}
	
	#define host_send mame_send
	#define host_receive mame_receive
#endif

#ifdef BDOS_HOST_TO_SOFTUART
	#include <driver/softuart.h>
	
	#define HOST_SERIAL_MAX_LINE 255	// Maximum length of one incoming line
	#define HOST_SERIAL_TIMEOUT 8192
	
	// Compatibility for old "softserial"
	#define serial_getchar softuart_receiveByte
	#define serial_getchar_nonblocking softuart_receiveByte
	#define serial_putchar softuart_sendByte
	
	void serial_put(byte *data, word l) {
		for (word i = 0; i < l; i++) {
			serial_putchar(*data++);
		}
	}
	byte *serial_gets(byte *serial_get_buf) {
		int c;
		byte *b;
		
		b = serial_get_buf;
		while(1) {
			//c = serial_getchar();
			c = serial_getchar_nonblocking();
			if (c <= 0) continue;	// < 0 means "no data"
			
			// Check for end-of-line character(s)
			if ((c == 0x0a) || (c == 0x0d)) break;
			
			// Store in given buffer
			*b++ = c;
		}
		
		// Terminate string
		*b = 0;
		
		// Return length
		//return (word)b - (word)serial_get_buf;
		
		// Return buf (like stdlib gets())
		return serial_get_buf;
	}
	
	//#define serial_debug(s)	printf(s)
	#define serial_debug(s)	;
	
	#ifndef __HEX_H
		//#include <hex.h>	// for hexDigit
		/*
		byte hexDigit(byte c) {
			if (c < 10)
				return ('0'+c);
			return 'A' + (c-10);
		}
		*/
		#define hexDigit bdos_hexDigit
		
		
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
	#endif
	
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
	/*
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
	*/
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
			//__asm
			//	di
			//__endasm;
			
			// Send synchronization
			//serial_puts("UUUUUUUU");
			serial_put(line, lline);
			
			c = serial_gethex2();
			
			//__asm
			//	ei
			//__endasm;
			
			if (c < 0) {
				bdos_printf("T!\n");
				continue;
			}
			
			checkgiven = c;
			
			if (checkgiven == checkactual) {
				// OK!
				//printf("OK\n");
				break;
			} else {
				bdos_printf("C!\n");
				//bdos_printf("C! g=");
				//bdos_printf_x2(checkgiven);
				//bdos_printf(",a=");
				//bdos_printf_x2(checkactual);
				//bdos_printf("!\n");
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
				bdos_printf("S!\n");
				//bdos_printf("S!");
				//bdos_printf_x2(linel);
				//bdos_printf("<4!\n");
				//continue;
				return -1;
			}
			
			// Get length
			l = hextob(pline); pline += 2;
			if (((2 + l*2 + 2) != linel) || (l > BDOS_HOST_MAX_DATA)) {
				// Error in length! Given length > actual length
				bdos_printf("L!\n");
				//bdos_printf("L! g=");
				//bdos_printf_x2(l);
				//bdos_printf(",a=");
				//bdos_printf_x2((linel-4) >> 1);
				//bdos_printf("!\n");
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
				bdos_printf("C!\n");
				//bdos_printf("C! g=");
				//bdos_printf_x2(checkgiven);
				//bdos_printf(",a=");
				//bdos_printf_x2(checkactual);
				//bdos_printf("!\n");
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
	//bdos_printf("F"); bdos_printf_x2(num);
	
	// Dump to serial
	pdata = &data[0];
	
	*pdata++ = 'F';
	*pdata++ = num;
	
	*pdata++ = (word)fcb >> 8;
	*pdata++ = (word)fcb & 0xff;
	
	pfcb = (char *)fcb;
	for (i = 0; i < 36; i++) {
		*pdata++ = *pfcb++;
	}
	
	host_send(&data[0], 1+1+2+36);
	
	// Use host_receivefcb() to receive result and altered FCB
}


byte host_receivefcb(struct FCB *fcb) {
	byte data[1 + 36 + 16];	// Leave some extra to be sure
	byte l;
	byte r;
	
	// Receive result and FCB as one data frame
	// do {
		l = host_receive(data);
	// } while(l == 0);
	
	//@TODO: L must be 36 for a proper FCB (or 32 for dir listing). Return error if not.
	if (l > (1+36)) {
		bdos_puts("FCB>36!");
		return 0xff;
	}
	
	// Get result
	r = data[0];	// Result value
	
	if (r == 0xff) {
		//@TODO: Error codes!?
		//printf("FCB FUN ERR!\n");
	} else {
		// Copy the received FCB over
		memcpy((byte *)fcb, (byte *)&data[1], l);	// data[0] = result value, data[1...] = actual FCB data
	}
	
	return r;
}

word host_receivedma() {
	byte data[BDOS_HOST_DMA_MAX_DATA + 1];	// 128
	int l;
	byte *p;
	word ltotal;
	
	p = bios_dma;
	ltotal = 0;
	l = 16;
	// Using 1 dummy byte so we never send empty frames
	while((l > 1) && (ltotal < BDOS_HOST_DMA_MAX_DATA)) {
		//bdos_printf("rx=");
		l = host_receive(&data[0]);
		//bdos_printf_x2(l); bdos_printf(".");
		
		if ((l == 1) && (data[0] == 0xAA)) {
			//bdos_printf("EOF");
			break;
		}
		
		if (l > 1) {
			l -= 1;	// Skip dummy byte
			memcpy(p, &data[1], l);
			p += l;
			ltotal += l;
		}
	}
	//ltotal = (word)p - (word)bios_dma;
	//bdos_printf("DMA L="); bdos_printf_x2(ltotal);
	
	return ltotal;
}

byte host_receivebyte() {
	byte data[BDOS_HOST_MAX_DATA];	// 1 is enough, but....
	int l;
	
	l = 0;
	while(l == 0) {
		l = host_receive(data);
	}
	
	return data[0];
}

#endif