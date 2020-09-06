#ifndef __PARABUDDY_H
#define __PARABUDDY_H
/*

Parallel Buddy Test
===================

This is the VGL side of "VTechParallelBuddy" - an Arduino connected to the VGL parallel port

----> WORK IN PROGRESS!
Arduino sketch: /z/data/_code/_arduino/VTech/VTechParallelBuddy

TODO:
	Arduino should send start padding character

2019-07-16 Bernhard "HotKey" Slawik (sdcc/loader/app_parallelBuddy.c)
2020-08-02 Bernhard "HotKey" Slawik (vgldk.git)

*/


/*

TODO:
	* Use size_t
	* Clean up error codes (use negative for errors)
	* Send/receive arbitrary length data
	* Checksum!

*/

//#define PB_DEBUG_FRAMES	// Dump hex data to screen?
//#define PB_DEBUG_PROTOCOL_ERRORS	// Put information about protocol errors on screen
#define PB_RECEIVE_TIMEOUT 0x1000	// 0x0800 is good

//#define PB_USE_SOFTUART // Use SoftUART as physical layer (actual hardware)
//#define PB_USE_SOFTSERIAL // Use SoftSerial as physical layer (actual hardware)
//#define PB_USE_MAME	// Use MAME port traps (link to host) as a physical layer (running in emulation)
//#define PB_USE_ARDUINO  // Provide a custom physical layer (ParallelBuddy Arduino Sketch)

#define PB_MAX_FRAME_SIZE 64  // Length of a command frame
#define PB_MAX_FILENAME 32


#define PB_PREFIX 0xfe	// Must NOT collide with first byte! STX 0x02 or ENQ 0x05
//#define PB_TERMINATOR 0x03	// ETX 0x03 or EOT 0x04

#define PB_COMMAND_RETURN_OK 0x06	//ACK
#define PB_COMMAND_RETURN_BYTE 0x11
#define PB_COMMAND_RETURN_WORD 0x12
#define PB_COMMAND_RETURN_ASCIIZ 0x13
#define PB_COMMAND_RETURN_DATA 0x14
#define PB_COMMAND_RETURN_NACK 0x15	// NAK

// Functionality
#define PB_COMMAND_END_BOOTLOADER 0x1A

#define PB_COMMAND_PING 0xE0
//#define PB_COMMAND_PING_HOST 0xE1

//#define PB_COMMAND_SD_INIT 0x20
//#define PB_COMMAND_SD_EXISTS 0x21
//#define PB_COMMAND_SD_OPEN 0x22
//#define PB_COMMAND_SD_CLOSE 0x23
//#define PB_COMMAND_SD_REMOVE 0x4
//#define PB_COMMAND_SD_MKDIR 0x25
//#define PB_COMMAND_SD_RMDIR 0x26

#define PB_COMMAND_FILE_OPENDIR 0x30
#define PB_COMMAND_FILE_READDIR 0x31
#define PB_COMMAND_FILE_CLOSEDIR 0x32

#define PB_COMMAND_FILE_OPEN 0x40
#define PB_COMMAND_FILE_CLOSE 0x41
#define PB_COMMAND_FILE_EOF 0x42
#define PB_COMMAND_FILE_READ 0x43
#define PB_COMMAND_FILE_WRITE 0x44
//#define PB_COMMAND_FILE_SEEK 0x45
//#define PB_COMMAND_FILE_SIZE 0x46
#define PB_COMMAND_FILE_AVAILABLE 0x47


#define PB_ERROR_OK 0x00
#define PB_ERROR_UNKNOWN 0x01
#define PB_ERROR_LENGTH 0x02
#define PB_ERROR_TIMEOUT 0x03
#define PB_ERROR_CORRUPT 0x04

#define PB_FILE_READ 0
#define PB_FILE_WRITE 1

#define PB_NO_HANDLE 0xff
typedef byte pb_handle;


// Prepare "Physical Layer" (pb_sendRaw / pb_receiveRaw)

#ifndef PB_USE_SOFTUART
	#ifndef PB_USE_SOFTSERIAL
		#ifndef PB_USE_MAME
			#ifndef PB_USE_ARDUINO
				#error "parabuddy.h: No physical layer defined! Define either PB_USE_SOFTSERIAL, PB_USE_MAME or PB_USE_ARDUINO."
			#endif
		#endif
	#endif
#endif


//@TODO: Move this selection to softserial instead?
#ifdef PB_USE_MAME
	// Using MAME trapped port access instead of hardware
	#include "driver/mame.h"
	#define pb_getchar mame_getchar
	#define pb_putchar mame_putchar
	//#define pb_puts mame_put
	
	byte pb_receiveRaw(byte *f) {
		int c;
		byte l;
		word timeout = PB_RECEIVE_TIMEOUT;
		
		// Wait for first sync prefix byte
		do {
			c = pb_getchar();
			if (c < 0) {
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;  // Timeout while waiting for sync
				continue;
			}
			
		} while (c != PB_PREFIX);
		
		// c is now at the first encountered prefix byte
		
		// Skip over prefix padding
		do {
			c = pb_getchar();
			if (c < 0) {
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;  // Timeout while in prefix
				continue;
			}
		} while (c == PB_PREFIX);
		
		// c now contains the first character beyond PB_PREFIX
		// The first char is LENGTH
		l = c;
		
		// Store character
		#ifdef PB_DEBUG_FRAMES
		putchar('[');
		printf_x2(c);
		#endif
		*f++ = c;
		
		// Receive until termination
		while(l > 0) {
			c = pb_getchar();
			
			if (c < 0) {
				// No data? Update timeout
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;
				continue;
			}
			
			// Store character
			#ifdef PB_DEBUG_FRAMES
			printf_x2(c);
			#endif
			*f++ = c;
			
			l--;
		}
		#ifdef PB_DEBUG_FRAMES
		putchar(']');
		#endif
		
		// Finished!
		return PB_ERROR_OK;
	}
	void pb_sendRaw(byte *f, byte l) {
		//@FIXME: It suddenly just stopped working!
		//pb_put(f, l);
		
		//@FIXME: Do NOT send byte-by-byte. Too much overhead!
		while (l-- > 0) pb_putchar(*f++);
	}
#endif

#ifdef PB_USE_SOFTSERIAL
	// Use SoftSerial for communication
  #warning "SoftSerial is not binary proof and therefore not suited for parabuddy, yet. Use SoftUART instead!"
	#include <softserial.h>
	#define pb_getchar serial_getchar
	#define pb_putchar serial_putchar
	//#define pb_puts serial_put
	
	byte pb_receiveRaw(byte *f) {
		int c;
		byte l;
		word timeout = PB_RECEIVE_TIMEOUT;
		
		// Wait for first sync prefix byte
		do {
			c = pb_getchar();
			if (c < 0) {
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;  // Timeout while waiting for sync
				continue;
			}
			
		} while (c != PB_PREFIX);
		
		// c is now at the first encountered prefix byte
		
		// Skip over prefix padding
		do {
			c = pb_getchar();
			if (c < 0) {
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;  // Timeout while in prefix
				continue;
			}
		} while (c == PB_PREFIX);
		
		// c now contains the first character beyond PB_PREFIX
		// The first char is LENGTH
		l = c;
		
		// Store character
		#ifdef PB_DEBUG_FRAMES
		putchar('[');
		printf_x2(c);
		#endif
		*f++ = c;
		
		// Receive until termination
		while(l > 0) {
			c = pb_getchar();
			
			if (c < 0) {
				// No data? Update timeout
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;
				continue;
			}
			
			// Store character
			#ifdef PB_DEBUG_FRAMES
			printf_x2(c);
			#endif
			*f++ = c;
			
			l--;
		}
		#ifdef PB_DEBUG_FRAMES
		putchar(']');
		#endif
		
		// Finished!
		return PB_ERROR_OK;
	}
	void pb_sendRaw(byte *f, byte l) {
		
		//@FIXME: It suddenly just stopped working!
		//pb_put(f, l);
		
		//@FIXME: Do NOT send byte-by-byte. Too much overhead!
		while (l-- > 0) pb_putchar(*f++);
	}
#endif


#ifdef PB_USE_SOFTUART
	// Default: Use SoftSerial for communication
	#include "driver/softuart.h"
	#define pb_getchar softuart_receiveByte
	#define pb_putchar softuart_sendByte
	//#define pb_puts serial_put
	
	byte pb_receiveRaw(byte *f) {
		int c;
		byte l;
		word timeout = PB_RECEIVE_TIMEOUT;
		
		// Wait for first sync prefix byte
		do {
			c = pb_getchar();
			if (c < 0) {
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;  // Timeout while waiting for sync
				continue;
			}
			
		} while (c != PB_PREFIX);
		
		// c is now at the first encountered prefix byte
		
		// Skip over prefix padding
		do {
			c = pb_getchar();
			if (c < 0) {
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;  // Timeout while in prefix
				continue;
			}
		} while (c == PB_PREFIX);
		
		// c now contains the first character beyond PB_PREFIX
		// The first char is LENGTH
		l = c;
		
		// Store character
		#ifdef PB_DEBUG_FRAMES
		putchar('[');
		printf_x2(c);
		#endif
		*f++ = c;
		
		// Receive until termination
		while(l > 0) {
			c = pb_getchar();
			
			if (c < 0) {
				// No data? Update timeout
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;
				continue;
			}
			
			// Store character
			#ifdef PB_DEBUG_FRAMES
			printf_x2(c);
			#endif
			*f++ = c;
			
			l--;
		}
		#ifdef PB_DEBUG_FRAMES
		putchar(']');
		#endif
		
		// Finished!
		return PB_ERROR_OK;
	}
	void pb_sendRaw(byte *f, byte l) {
		
		//@FIXME: It suddenly just stopped working!
		//pb_put(f, l);
		
		//@FIXME: Do NOT send byte-by-byte. Too much overhead!
		while (l-- > 0) pb_putchar(*f++);
	}
#endif

#ifdef PB_USE_ARDUINO
	// Included in Arduino sketch: Use Hardware Serial (called "pb_serial")
	int pb_getchar() {
		//while (!pb_serial.available()) {}
		return pb_serial.read();
	}
	void pb_putchar(char c) {
		pb_serial.write(c);
	}
	
	byte pb_receiveRaw(byte *f) {
		int c;
		byte l;
		word timeout = PB_RECEIVE_TIMEOUT;
	
		#ifdef PB_DEBUG_FRAMES
			hostSerial.print("pre?");
		#endif
		// Wait for first sync prefix byte
		do {
			c = pb_getchar();
			if (c < 0) {
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;  // Timeout while waiting for sync
				continue;
			}
			
		} while (c != PB_PREFIX);
	
		#ifdef PB_DEBUG_FRAMES
			hostSerial.print("!");
		#endif
		// c is now at the first encountered prefix byte
		
		// Skip over prefix padding
		do {
			c = pb_getchar();
			if (c < 0) {
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;  // Timeout while in prefix
				continue;
			}
		} while (c == PB_PREFIX);
		
		// c now contains the first character beyond PB_PREFIX
		#ifdef PB_DEBUG_FRAMES
			hostSerial.print("D");
		#endif
		// The first char is LENGTH
		l = c;
		
		// Store character
		#ifdef PB_DEBUG_FRAMES
			hostSerial.print("[");
			hostSerial.print(c, HEX);
		#endif
		*f++ = c;
		
		// Receive until termination
		while(l > 0) {
			c = pb_getchar();
			
			if (c < 0) {
				// No data? Update timeout
				timeout--;
				if (timeout <= 0) return PB_ERROR_TIMEOUT;
				continue;
			}
			
			// Store character
			#ifdef PB_DEBUG_FRAMES
				hostSerial.print(c, HEX);
			#endif
			*f++ = c;
			
			l--;
		}
		#ifdef PB_DEBUG_FRAMES
			hostSerial.print("]");
		#endif
		
		// Finished!
		return PB_ERROR_OK;
	}
	
	void pb_sendRaw(byte *b, word l) {
	
		#ifdef PB_DEBUG_SENDING
			// Debug
			put_("#Sending l=");
			printf_d(l);
			put("");
		#endif
		
		// Send Pre-padding
		// Three sync bytes turn out to be quite reliable!
		//pb_putchar(PB_PREFIX);
		pb_serial.write(PB_PREFIX);
		pb_serial.write(PB_PREFIX);
		pb_serial.write(PB_PREFIX);
		//pb_serial.write(PB_PREFIX);
		
		// Actually send
		size_t lSent = pb_serial.write(b, l);
		
		//pb_serial.flush();  // Wait for data to be sent
	}
	
#endif


// Data Link Layer (independent of Physical Layer)
byte tmpFrame[PB_MAX_FRAME_SIZE];

void pb_sendFrame(byte cmd, byte *v, const byte dataLen) {
	byte *f;
	//byte l;
	
	f = &tmpFrame[0];
	*f++ = 1+dataLen;	// 1=cmd, x=dataLen
	*f++ = cmd;
	
	// Copy from buffer v to mem f
	memcpy(f, v, dataLen);
	
	/*
	l = dataLen;
	while (l > 0) {
		*f++ = *v++;
		l--;
	}
	// *f = PB_TERMINATOR;
	*/
	pb_sendRaw(&tmpFrame[0], 1+1+dataLen);	// 1=len, 1=cmd, x=dataLen
}
void pb_sendByte(byte cmd, byte v) {
	pb_sendFrame(cmd, &v, 1);
}
void pb_sendWord(byte cmd, word v) {
	pb_sendFrame(cmd, (byte *)&v, 2);
}
void pb_sendAsciiz(byte cmd, const char *v) {
	byte l;
	const char *c;
	
	// Get length of zero-terminated string
	c = v;
	l = 0;
	while(*c++ != 0) l++;
	
	// Send it
	pb_sendFrame(cmd, (byte *)v, l);
}
byte pb_receiveFrame(byte expectedCmd, byte *v, byte minDataLen, byte *dataLen) {
	byte l;
	byte c;
	byte *f;
	byte maxDataLen;
	
	if (pb_receiveRaw(&tmpFrame[0]) != PB_ERROR_OK) {
		return PB_ERROR_UNKNOWN;
	}
	
	f = &tmpFrame[0];
	
	l = *f++;
	
	if (l < 1) {
		#ifdef PB_DEBUG_PROTOCOL_ERRORS
		puts("L<1!");
		#endif
		return PB_ERROR_LENGTH;
	}
	
	// No dataLen given? Use minDataLen
	if (dataLen == NULL) {
		maxDataLen = minDataLen;
	} else {
		maxDataLen = *dataLen;	// Interpret given value of dataLen as maxDataLen...
	}
	
	// Set dataLen to the actual number of data bytes
	l --;
	
	if ((l < minDataLen) || (l > maxDataLen)) {
		#ifdef PB_DEBUG_PROTOCOL_ERRORS
		puts("L!");
		puts("="); printf_x2(l);
		puts(",min="); printf_x2(minDataLen);
		puts(",max="); printf_x2(maxDataLen);
		puts("!\n");
		getchar();
		//printf("L: %02X ! %02X\n", l, (1+1));
		//beep();
		#endif
		return PB_ERROR_LENGTH;
	}
	// Store actual received length back to *dataLen
	if (dataLen != NULL) *dataLen = l;
	
	// Check command byte
	c = *f++;
	if (c != expectedCmd) {
		#ifdef PB_DEBUG_PROTOCOL_ERRORS
		puts("C!");
		//printf("C: %02X != %02X\n", c, expectedCmd);
		//beep();
		#endif
		return PB_ERROR_CORRUPT;
	}
	
	// Stuff the rest into v
	/*
	while(l > 0) {
		*v++ = *f++;
		l--;
	}
	*/
	memcpy(v, f, l);
	
	return PB_ERROR_OK;
}

byte pb_receiveByte(byte *v) {
	return pb_receiveFrame(PB_COMMAND_RETURN_BYTE, v, 1, NULL);
}

byte pb_receiveWord(word *v) {
	return pb_receiveFrame(PB_COMMAND_RETURN_WORD, (byte *)v, 2, 0);
}

byte pb_receiveAsciiz(char* v) {
	byte r;
	byte l;
	
	l = PB_MAX_FILENAME;
	r = pb_receiveFrame(PB_COMMAND_RETURN_ASCIIZ, (byte *)v, 0, &l);
	*(v+l) = 0x00;	// Zero terminate
	
	return r;
}


// Functionality

word pingValue;
byte pb_ping() {
	word vPing;
	word vPong;
	
	//vPing = 0x1234;
	vPing = pingValue++;
	vPong = 0xffff;
	
	//printf("Ping %02X %02X...\n", (vPing >> 8), vPing & 0xff);
	//puts("Ping"); printf_x4(vPing); putchar('\n');
	
	//pb_sendComposed2(PB_COMMAND_PING, (vPing >> 8), vPing & 0x00ff);	// MSB
	//pb_sendComposed2(PB_COMMAND_PING, vPing & 0x00ff, (vPing >> 8));	// LSB
	pb_sendWord(PB_COMMAND_PING, vPing);
	
	//if (pb_receiveWord(&vPong) == 1) break;
	
	if (pb_receiveWord(&vPong) == 0) {
		//puts("failed.\n");
		//beep();
		return PB_ERROR_UNKNOWN;
	}

	//printf("Pong %04X\n", vPong);
	//printf("Pong %02X %02X\n", (vPong >> 8), vPong & 0xff);
	//puts("Pong"); printf_x4(vPong); putchar('\n');
	if (vPing != vPong) return PB_ERROR_CORRUPT;
	return PB_ERROR_OK;
}

void pb_endBootloader() {
	// Tell Arduino that we want to communicate with it (not the host PC)
	//printf("End bootloader...\n");
	pb_sendFrame(PB_COMMAND_END_BOOTLOADER, 0, 0);
}


pb_handle pb_file_opendir(const char *path) {
	pb_handle h;
	
	pb_sendAsciiz(PB_COMMAND_FILE_OPENDIR, (char *)path);
	
	// Receive handle
	/*
	// Stall until OK
	while (pb_receiveByte(&h) != PB_ERROR_OK) {
		// Delay / Timeout
	}
	return h;
	*/
	
	// Succeed or fail
	return (pb_receiveByte(&h) == PB_ERROR_OK) ? h : PB_NO_HANDLE;
}

void pb_file_closedir(pb_handle h) {
	pb_sendByte(PB_COMMAND_FILE_CLOSEDIR, h);
	// Has no return value
}

byte pb_file_readdir(pb_handle h, char *name) {
	
	pb_sendByte(PB_COMMAND_FILE_READDIR, h);
	
	//@TODO: Receive a whole dirent struct?
	//while(pb_receiveFrame(PB_COMMAND_RETURN_DATA, buf, 0, &l) == 1) {}
	
	// Receive name
	/*
	while (pb_receiveAsciiz(name) != PB_ERROR_OK) {
		// Wait
	}
	return PB_ERROR_OK;
	*/
	
	// Succeed or fail
	return pb_receiveAsciiz(name);
}

pb_handle pb_file_open(const char *filename, const char*mode) {
	pb_handle h;
	
	//@TODO: Mode!
	(void)mode;
	
	pb_sendAsciiz(PB_COMMAND_FILE_OPEN, (char *)filename);
	
	// Receive handle
	/*
	// Stall until OK
	while (pb_receiveByte(&h) != PB_ERROR_OK) {
		// Delay / Timeout
	}
	return h;
	*/
	// Succeed or fail
	return (pb_receiveByte(&h) == PB_ERROR_OK) ? h : PB_NO_HANDLE;
}

void pb_file_close(byte h) {
	pb_sendByte(PB_COMMAND_FILE_CLOSE, h);
	// Has no return value
}

byte pb_file_eof(pb_handle h) {
	byte r;
	
	pb_sendByte(PB_COMMAND_FILE_EOF, h);
	
	// Get return value
	/*
	// Stall until OK
	while (pb_receiveByte(&r) != PB_ERROR_OK) {
		// Delay / Timeout
	}
	return r;
	*/
	
	// Succeed or fail
	return (pb_receiveByte(&r) == PB_ERROR_OK) ? r : 1;
}



byte pb_file_read(pb_handle h, byte *buf, byte l) {
	struct {
		byte h ;
		byte l;
	} data;
	data.h = h;
	data.l = l;
	pb_sendFrame(PB_COMMAND_FILE_READ, (byte*)&data, sizeof(data));
	
	// Return data
	/*
	// Stall until OK
	while(pb_receiveFrame(PB_COMMAND_RETURN_DATA, buf, 0, &l) != PB_ERROR_OK) {
		// Delay / Timeout
	}
	return l;
	*/
	// Succeed or fail
	return (pb_receiveFrame(PB_COMMAND_RETURN_DATA, buf, 0, &l) == PB_ERROR_OK) ? l : 0;
}

byte pb_file_write(pb_handle h, byte *buf, byte l) {
	//@TODO: Implement
	(void)h;
	//pb_sendByte(PB_COMMAND_FILE_WRITE, h);
	
	//byte data[2];
	//data[0] = h;
	//data[1] = l;
	pb_sendFrame(PB_COMMAND_FILE_WRITE, buf, l);
	
	// Get bytes written
	
	/*
	// Stall until OK
	if (pb_receiveByte(&l) != PB_ERROR_OK)
		return 0;
	return l;
	*/
	// Succeed or fail
	return (pb_receiveByte(&l) == PB_ERROR_OK) ? l : 0;
}

/*
word pb_file_tell(byte handle) {
	PB_COMMAND_FILE_TELL
}
void pb_file_seek(byte handle, word o) {
	PB_COMMAND_FILE_SEEK
}

byte pb_file_bytesAvailable(pb_handle h) {
	byte l;
	//@TODO: Implement
	// fsize() - ftell()?	PB_COMMAND_FILE_SIZE?
	
	pb_sendByte(PB_COMMAND_FILE_AVAILABLE, h);
	
	return (pb_receiveByte(&l) == PB_ERROR_OK) ? l : 0;
}
*/
#endif 	//__PARABUDDY_H
