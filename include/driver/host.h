#ifndef __HOST_H
#define __HOST_H

/*
Abstract Host Communication
===========================

Allows sending "frames" (arbitrary binary packets) over selectable protocol and driver.
The host might be a PC connected via serial (SoftUART / SoftSerial) or a (modified) MAME emulator

2023-09-03 Bernhard "HotKey" Slawik
*/

// Hardware to use for communication (byte-level) - select ONE
//#define HOST_DRIVER_SOFTUART	// Re-direct to SoftUART (for real hardware)
//#define HOST_DRIVER_SOFTSERIAL	// Re-direct to SoftSerial (for real hardware)
//#define HOST_DRIVER_MAME	// Re-direct to MAME (for emulation)

// Protocol to use for communication (frame level) - select ONE
//#define HOST_PROTOCOL_BINARY	// Send using 8bit binary data (e.g. for MAME driver or reliable serial)
//#define HOST_PROTOCOL_BINARY_SAFE	// Send using binary, but with checksum and retransmission (e.g. SoftUART)
//#define HOST_PROTOCOL_HEX	// Send using hex text (if communication is not binary-proof)

#define HOST_MAX_DATA 144	//128	// To dismiss too large frames

#define HOST_CHECK_INIT 0x55
#define HOST_STATUS_ACK 0xAA
#define HOST_STATUS_NAK 0x99
#define HOST_STATUS_OK 0x00


// Device setup (byte level)

#ifdef HOST_DRIVER_SOFTUART
	// Use SoftUART functions
	#include <driver/softuart.h>
	#define host_send_byte	softuart_sendByte
	#define host_receive_byte	softuart_receiveByte	// non-blocking
#endif
#ifdef HOST_DRIVER_SOFTSERIAL
	// Use SoftUART functions
	#include <softserial.h>	// Comes bespoke for each architecture
	#define host_send_byte	serial_putchar
	#define host_receive_byte	serial_getchar_nonblocking	// non-blocking
	//#define host_receive_byte	serial_getchar	// blocking
#endif
#ifdef HOST_DRIVER_MAME
	// Use MAME functions
	#include <driver/mame.h>
	#define host_send_byte	mame_putchar
	#define host_receive_byte	mame_getchar	// mame_getchar is blocking
#endif

// Make sure one driver is selected
#ifndef host_send_byte
	#error One HOST_DRIVER must be set (SoftUART, SoftSerial, MAME, ...)!
#endif

// Error trace for protocols
//#define host_error(s)	printf(s)
#define host_error(s)	;	// Be quiet!


// Protocol (frame level)
#ifdef HOST_PROTOCOL_BINARY
	// Send using simple binary data
	
	void host_send_frame_binary(const byte *data, byte len) {
		byte i;
		
		// Send length
		host_send_byte(len);
		
		// Send data
		for (i = 0; i < len; i++) {
			host_send_byte(*data++);
		}
		
	}
	
	int host_receive_frame_binary(byte *data) {
		int l;
		byte i;
		//byte c;
		
		// Wait for non-zero byte, receive length
		//printf("RX=");
		do {
			l = host_receive_byte();
		} while (l <= 0);
		//printf_x2(l);
		//printf("..."); //getchar();
		
		// No buffer-overflow, please!
		if (l > HOST_MAX_DATA) {
			return -1;
		}
		
		// Receive data
		for (i = 0; i < l; i++) {
			
			//c = host_receive_byte();
			//printf_x2(c);
			//*data++ = c;
			
			*data++ = host_receive_byte();
		}
		
		return l;
	}
	#define host_send_frame host_send_frame_binary
	#define host_receive_frame host_receive_frame_binary
#endif


#ifdef HOST_PROTOCOL_BINARY_SAFE
	// Send using binary data and checksum+retransmission
	
	void host_send_frame_binary_safe(const byte *data, byte len) {
		byte i;
		byte b;
		//byte check;
		word check;
		const byte *p;
		int r;
		
		do {	// Retransmission
			
			//printf("re");
			check = HOST_CHECK_INIT;	// Initial checksum
			p = data;	// Re-wind data pointer
			
			// Send length
			host_send_byte(len);
			
			// Send data
			for (i = 0; i < len; i++) {
				b = *p++;
				host_send_byte(b);
				
				//check ^= b;	// XOR it
				check = (check << 1) ^ b;	// shl and XOR it
			}
			
			// Send Checksum
			//host_send_byte(check);	// 8 bit checksum
			host_send_byte(check >> 8);	// 16 bit checksum, MSB first
			host_send_byte(check & 0xff);
			
			// Wait for ACK/NACK
			r = -1;
			while (r <= 0) {
				r = host_receive_byte();
			}
			
		} while (r != HOST_STATUS_ACK);	// Repeat until "AA" is received
		
	}
	
	int host_receive_frame_binary_safe(byte *data) {
		int l;
		byte c;
		byte i;
		byte *p;
		//byte check;
		//byte check_received;	// or int to handle timeout
		word check;
		word check_received;	// or int to handle timeout
		
		do {	// Retransmission
			
			// Wait for non-zero byte, receive length
			do {
				l = host_receive_byte();
			} while (l <= 0);
			//printf_x2(l);
			
			// No buffer-overflow, please!
			if (l > HOST_MAX_DATA) {
				//host_error("Rmax!");
				
				// Flush?
				
				// Continue to "NAK"
				
			} else {
				// Receive data
				
				check = HOST_CHECK_INIT;	// Initial checksum
				p = data;
				for (i = 0; i < l; i++) {
					//printf_x2(i);
					c = host_receive_byte();
					*p++ = c;
					
					//check ^= c;	// XOR onto checksum
					check = (check << 1) ^ c;	// shl and XOR it
				}
				
				// Receive checksum
				//do {
				check_received = host_receive_byte();
				check_received = (check_received << 8) | host_receive_byte();	// 16 bit
				
				//} while (check_received < 0);
				
				// Check
				//if ((byte)check_received == check) {
				if (check_received == check) {
					// OK! Send ACK
					
					//printf("ACK");
					host_send_byte(HOST_STATUS_ACK);
					//printf("ed");
					
					// And return bytes received
					return l;
				}
				
				host_error("Rc!");
			}
			
			// Checksum mismatch! Send NAK (anything but 0xAA)
			//printf("NAK");
			host_send_byte(HOST_STATUS_NAK);	// Send NAK
			//printf("ed");
			
		} while (1);	//check_received != check);
		//
	}
	#define host_send_frame host_send_frame_binary_safe
	#define host_receive_frame host_receive_frame_binary_safe
#endif


#ifdef HOST_PROTOCOL_HEX
	
	#define HOST_MAX_LINE 255	// Maximum length of one incoming line
	#define HOST_RECEIVE_TIMEOUT 8192	// Counter to determine timeouts
	
	//#define host_debug(s)	printf(s)
	#define host_debug(s)	;
	
	// Helper for sending a whole "blob"
	void host_send_data(byte *data, word l) {
		for (word i = 0; i < l; i++) {
			host_send_byte(*data++);
		}
	}
	
	// Receive one text line
	byte *host_receive_line(byte *get_buf) {
		int c;
		byte *b;
		
		b = get_buf;
		while(1) {
			c = host_receive_byte();	// Non-blocking would be good
			if (c <= 0) continue;	// < 0 means "no data"
			
			// Check for end-of-line character(s)
			if ((c == 0x0a) || (c == 0x0d)) break;
			
			// Store in given buffer
			*b++ = c;
		}
		
		// Terminate string
		*b = 0;
		
		//return (word)b - (word)serial_get_buf;	// Return length
		return get_buf;	// Return buf (like stdlib gets())
	}
	
	
	// We need some HEX functions. Preferably from the already included "hex.h" (vgldk/includes/hex.h)
	//#include <hex.h>	// for hexDigit, parse_hexDigit, hextown, hextob
	#ifndef __HEX_H
		/*
		byte hexDigit(byte c) {
			if (c < 10)
				return ('0'+c);
			return 'A' + (c-10);
		}
		*/
		#define hexDigit bdos_hexDigit	// Since this is part of BDOS we can re-use its version
		
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
	
	int host_receive_byte_with_timeout() {
		// Get char, return -1 on timeout
		int c;
		word timeout;
		
		timeout = HOST_RECEIVE_TIMEOUT;
		
		c = -1;
		while (c <= 0) {
			c = host_receive_byte();	// non-blocking
			timeout --;
			if (timeout == 0) return -1;
		}
		return c;
	}
	
	int host_receive_hex_with_timeout() {
		// Receive two digits of hex, return -1 on timeout
		//byte r;
		int c;
		int c2;
		
		do {
			c = host_receive_byte_with_timeout();
			if (c < 0) return -1;
		} while (c == 'U');
		
		//c2 = host_receive_byte_with_timeout();
		c2 = host_receive_byte_with_timeout();
		
		if (c < 0) return -1;
		if (c2 < 0) return -1;
		//putchar(c2);
		//printf_x2(c); printf_x2(c2);	//@FIXME: Bit debugging
		
		return (parse_hexDigit(c) << 4) + parse_hexDigit(c2);
	}
	
	void host_send_frame_hex(const byte *data, byte ldata) {
		byte line[HOST_MAX_LINE];
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
			host_debug("TX");
			//__asm
			//	di
			//__endasm;
			
			// Send synchronization
			//serial_puts("UUUUUUUU");
			
			// Send line
			//serial_put(line, lline);
			host_send_data(line, lline);
			
			c = host_receive_hex_with_timeout();
			
			//__asm
			//	ei
			//__endasm;
			
			if (c < 0) {
				host_error("TT!");
				continue;
			}
			
			checkgiven = c;
			
			if (checkgiven == checkactual) {
				// OK!
				//printf("OK\n");
				break;
			} else {
				host_error("Tc!");
				//bdos_printf("C! g=");
				//bdos_printf_x2(checkgiven);
				//bdos_printf(",a=");
				//bdos_printf_x2(checkactual);
				//bdos_printf("!\n");
			}
		}
		host_debug("OK\n");
	}
	
	int host_receive_frame_hex(byte *data) {
		char line[HOST_MAX_LINE];
		byte l;
		char *pline;
		byte *pdata;
		byte linel;
		byte b;
		byte i;
		byte checkgiven;
		byte checkactual;
		//int r;
		
		host_debug("RX");
		while(1) {
			
			// Send synchronization pad / request to answer
			//serial_put("GGGG", 4);
			//serial_puts("GG\n");
			//printf(".");
			
			//@TODO: Use non-blocking get to detect timeouts?
			host_receive_line(&line[0]);
			linel = strlen(&line[0]);
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
				host_error("Rs!");
				//bdos_printf("S!");
				//bdos_printf_x2(linel);
				//bdos_printf("<4!\n");
				//continue;
				return -1;
			}
			
			// Get length
			l = hextob(pline); pline += 2;
			if (((2 + l*2 + 2) != linel) || (l > HOST_MAX_DATA)) {
				// Error in length! Given length > actual length
				host_error("Rl!");
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
				host_error("Rc!");
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
				host_send_data(&line[0], 6);
				
				return l;
			}
		}
		
	}
	
	#define host_send_frame host_send_frame_hex
	#define host_receive_frame host_receive_frame_hex
#endif

// Make sure one protocol is selected
#ifndef host_send_frame
	#error One HOST_PROTOCOL has to be selected (binary, binary_safe or hex)!
#endif

#endif // __HOST_H