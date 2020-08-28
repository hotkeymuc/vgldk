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

//#define PB_DEBUG_PROTOCOL_ERRORS	// Put information about protocol errors on screen
#define PB_RECEIVE_TIMEOUT 0x1000	// 0x0800 is good
//#define PB_USE_MAME	// Instead of sending to softserial, bytes are sent to a port which MAME traps

#define PB_MAX_FRAME_SIZE 64  // Length of a command frame
#define PB_MAX_FILENAME 32
//#define PB_MAX_FILES 4  // Maximum number of simultaneous open files
typedef byte pb_handle;
#define PB_NO_HANDLE 0xff

//#define PB_TERMINATOR 0x0a
#define PB_PREFIX 0x20

#define PB_COMMAND_RETURN_OK 0x10
#define PB_COMMAND_RETURN_BYTE 0x11
#define PB_COMMAND_RETURN_WORD 0x12
#define PB_COMMAND_RETURN_ASCIIZ 0x13
#define PB_COMMAND_RETURN_DATA 0x14
#define PB_COMMAND_RETURN_NACK 0x1F

#define PB_ERROR_UNKNOWN 0x01
#define PB_ERROR_LENGTH  0x02

// Functionality

#define PB_COMMAND_END_BOOTLOADER 0x1A

#define PB_COMMAND_PING 0xE0
#define PB_COMMAND_PING_HOST 0xE1

#define PB_COMMAND_SD_INIT 0x20
#define PB_COMMAND_SD_EXISTS 0x21
#define PB_COMMAND_SD_OPEN 0x22
//#define PB_COMMAND_SD_CLOSE 0x23
//#define PB_COMMAND_SD_REMOVE 0x4
//#define PB_COMMAND_SD_MKDIR 0x25
//#define PB_COMMAND_SD_RMDIR 0x26

#define PB_COMMAND_FILE_OPEN 30
#define PB_COMMAND_FILE_CLOSE 0x31
#define PB_COMMAND_FILE_READ 0x32
#define PB_COMMAND_FILE_WRITE 0x33
//#define PB_COMMAND_FILE_SEEK 0x34
//#define PB_COMMAND_FILE_SIZE 0x35
#define PB_COMMAND_FILE_AVAILABLE 0x36


#define PB_FILE_READ 0
#define PB_FILE_WRITE 1

//@TODO: Move this selection to softserial instead
#ifdef PB_USE_MAME
	// Using MAME trapped port access instead of hardware
	#include "mame.h"
	#define pb_getchar mame_getchar
	#define pb_putchar mame_putchar
	#define pb_puts mame_put
#else
	// Default: Use SoftSerial for communication
	#include <softserial.h>
	#define pb_getchar serial_getchar
	#define pb_putchar serial_putchar
	#define pb_puts serial_put
#endif

byte tmpFrame[PB_MAX_FRAME_SIZE];

byte pb_receiveRaw(byte *f) {
	int c;
	byte l;
	word timeout = PB_RECEIVE_TIMEOUT;
	
	// Wait for first sync prefix byte
	do {
		c = pb_getchar();
		
		if (c < 0) {
			timeout--;
			if (timeout <= 0) return 0;	// Timeout while waiting for sync
			continue;
		}
		
	} while (c != PB_PREFIX);
	
	// c is now at the first encountered prefix byte
	
	// Skip over prefix padding
	do {
		c = pb_getchar();
		
		if (c < 0) {
			timeout--;
			if (timeout <= 0) return 0;	// Timeout while in prefix
			continue;
		}
		
		//if (c == PB_TERMINATOR) return 0;	// End inside prefix? Nah-ah.
	} while (c == PB_PREFIX);
	
	// c now contains the first character beyond PB_PREFIX
	// The first char is LENGTH
	l = c;
	
	// Store character
	*f++ = c;
	
	// Receive until termination
	while(l >= 2) {
		
		// Get next
		c = pb_getchar();
		
		if (c < 0) {
			// No data? Update timeout
			timeout--;
			if (timeout <= 0) return 0;
			continue;
		}
		
		// Check for termination character(s)
		//if (c == PB_TERMINATOR) return 1;	// Finished!
		
		// Store character
		*f++ = c;
		
		l--;
		//if (l <= 1) return 1;	// Finished!
	}
	
	// Finished!
	return 1;
}
void pb_sendRaw(byte *f, byte l) {
	
	//@FIXME: It suddenly just stopped working!
	//pb_put(f, l);
	
	//@FIXME: Do NOT send byte-by-byte. Too much overhead!
	while (l-- > 0) pb_putchar(*f++);
}

void pb_sendData(byte cmd, byte *v, const byte dataLen) {
	byte *f;
	byte l;
	
	f = &tmpFrame[0];
	*f++ = 1+dataLen;	// 1=cmd, x=dataLen
	*f++ = cmd;
	
	//@TODO: Use memcpy()
	l = dataLen;
	while (l > 0) {
		*f++ = *v++;
		l--;
	}
	// *f = PB_TERMINATOR;
	pb_sendRaw(&tmpFrame[0], 1+1+dataLen);	// 1=len, 1=cmd, x=dataLen
}
void pb_sendByte(byte cmd, byte v) {
	pb_sendData(cmd, &v, 1);
}
void pb_sendWord(byte cmd, word v) {
	pb_sendData(cmd, (byte *)&v, 2);
}
void pb_sendAsciiz(byte cmd, const char *v) {
	byte l;
	const char *c;
	
	// Get length of zero-terminated string
	c = v;
	l = 0;
	while(*c++ != 0) l++;
	
	// Send it
	pb_sendData(cmd, (byte *)v, l);
}
byte pb_receiveData(byte expectedCmd, byte *v, byte minDataLen, byte *dataLen) {
	byte l;
	byte c;
	byte *f;
	byte maxDataLen;
	
	if (pb_receiveRaw(&tmpFrame[0]) == 0) {
		#ifdef PB_DEBUG_PROTOCOL_ERRORS
		printf("TO");
		#endif
		return 0;
	}
	
	f = &tmpFrame[0];
	
	l = *f++;
	
	// No dataLen given? Use minDataLen
	if (dataLen == 0x0000) {
		maxDataLen = minDataLen;
	} else {
		maxDataLen = *dataLen;	// Interpret dataLen as maxDataLen...
		
		// ...and set dataLen to the actual number
		if (l <= 2)	*dataLen = 0;
		else		*dataLen = l-2;
	}
	
	if ((l < (1+1 + minDataLen)) || (l > 1+1 + maxDataLen)) {
		#ifdef PB_DEBUG_PROTOCOL_ERRORS
		printf("L: %02X ! %02X\n", l, (1+1));
		//beep();
		#endif
		return 0;
	}
	
	c = *f++;
	if (c != expectedCmd) {
		#ifdef PB_DEBUG_PROTOCOL_ERRORS
		printf("C: %02X != %02X\n", c, expectedCmd);
		//beep();
		#endif
		return 0;
	}
	
	// Stuff the rest into v
	l -= 2;
	while(l > 0) {
		*v++ = *f++;
		l--;
	}
	
	return 1;
}


byte pb_receiveByte(byte *v) {
	return pb_receiveData(PB_COMMAND_RETURN_BYTE, v, 1, NULL);
}

byte pb_receiveWord(word *v) {
	return pb_receiveData(PB_COMMAND_RETURN_WORD, (byte *)v, 2, 0);
}

byte pb_receiveAsciiz(char* v) {
	byte r;
	byte l;
	
	r = pb_receiveData(PB_COMMAND_RETURN_ASCIIZ, (byte *)v, 0, &l);
	*(v+l) = 0x00;	// Zero terminate
	
	return r;
}




word pingValue;
void pb_ping() {
	word vPing;
	word vPong;
	
	//vPing = 0x1234;
	vPing = pingValue++;
	vPong = 0xffff;
	
	//printf("Ping %02X %02X...\n", (vPing >> 8), vPing & 0xff);
	puts("Ping"); printf_x4(vPing); putchar('\n');
	
	//pb_sendComposed2(PB_COMMAND_PING, (vPing >> 8), vPing & 0x00ff);	// MSB
	//pb_sendComposed2(PB_COMMAND_PING, vPing & 0x00ff, (vPing >> 8));	// LSB
	pb_sendWord(PB_COMMAND_PING, vPing);
	
	//if (pb_receiveWord(&vPong) == 1) break;
	
	if (pb_receiveWord(&vPong) == 0) {
		puts("failed.\n");
		//beep();
		return;
	}

	//printf("Pong %04X\n", vPong);
	//printf("Pong %02X %02X\n", (vPong >> 8), vPong & 0xff);
	puts("Pong"); printf_x4(vPong); putchar('\n');
}

void pb_endBootloader() {
	// Tell Arduino that we want to communicate with it (not the host PC)
	printf("End bootloader...\n");
	//pb_sendComposed0(PB_COMMAND_END_BOOTLOADER);
	pb_sendData(PB_COMMAND_END_BOOTLOADER, 0, 0);
}

byte pb_sd_init() {
	byte ok;
	
	pb_sendData(PB_COMMAND_SD_INIT, 0, 0);
	
	while (pb_receiveByte(&ok) != 1) {
		// Delay / Timeout
	}
	
	return ok;
}

pb_handle pb_sd_open(const char *filename, byte mode) {
	pb_handle handle;
	
	//@TODO: Mount/Driver (SD, SOCK, HOST)!
	//@TODO: Mode!
	(void)mode;
	
	pb_sendAsciiz(PB_COMMAND_SD_OPEN, (char *)filename);
	
	while (pb_receiveByte(&handle) != 1) {
		// Delay / Timeout
	}
	return handle;
}

byte pb_sd_exists(const char *filename) {
	byte ex;
	
	pb_sendAsciiz(PB_COMMAND_SD_EXISTS, (byte *)filename);
	while (pb_receiveByte(&ex) != 1) {
		// Delay / Timeout
	}
	return ex;
}



pb_handle pb_file_open(const char *filename, byte mode) {
	pb_handle handle;
	
	//@TODO: Mount/Driver (SD, SOCK, HOST)!
	//@TODO: Mode!
	(void)mode;
	
	pb_sendAsciiz(PB_COMMAND_FILE_OPEN, (char *)filename);
	
	while (pb_receiveByte(&handle) != 1) {
		// Delay / Timeout
	}
	return handle;
}


void pb_file_close(byte h) {
	pb_sendByte(PB_COMMAND_FILE_CLOSE, h);
	// Has no return value
}

byte pb_file_read(pb_handle h, byte *buf, byte l) {
	byte data[2];
	data[0] = h;
	data[1] = l;
	pb_sendData(PB_COMMAND_FILE_READ, &data[0], 2);
	
	while(pb_receiveData(PB_COMMAND_RETURN_DATA, buf, 0, &l) == 1) {
		// Delay / Timeout
	}
	
	return l;
}

byte pb_file_write(pb_handle h, byte *buf, byte l) {
	//@TODO: Implement
	(void)h;
	//pb_sendByte(PB_COMMAND_FILE_WRITE, h);
	
	//byte data[2];
	//data[0] = h;
	//data[1] = l;
	pb_sendData(PB_COMMAND_FILE_WRITE, buf, l);
	
	if (pb_receiveByte(&l) != 1)
		return 0;
	
	return l;
}


//word pb_file_tell(byte handle) {
//PB_COMMAND_FILE_TELL
//}
//void pb_file_seek(byte handle, word o) {
//PB_COMMAND_FILE_SEEK
//}

byte pb_file_bytesAvailable(pb_handle h) {
	byte l;
	//@TODO: Implement
	// fsize() - ftell()?	PB_COMMAND_FILE_SIZE?
	
	pb_sendByte(PB_COMMAND_FILE_AVAILABLE, h);
	
	if (pb_receiveByte(&l) != 1)
		return 0;
	
	return l;
}

/*

int main(int argc, char *argv[]) {
	char c;
	
	byte handle;
	byte tmpData[32];
	byte l;
	byte r;
	
	(void)argc;	(void)argv;	// Mark args as "not used" to suppress warnings
	
	
	printf("Parallel Buddy\n");
	//beep(); printf("Key to start..."); getchar();
	
	pb_endBootloader();
	
	pingValue = 0x0123;
	
	do {
		printf("EPQS\n");
		
		c = getchar();
		
		switch(c) {
			case 'e':
				pb_endBootloader();
				break;
			
			case 'h':
				serial_puts("puts()!\n");
				break;
			
			case 'i':
				serial_put("put()!\n", 7);
				break;
			
			case 'j':
				pb_sendWord(PB_COMMAND_PING, 0x6667);
				for(r = 0; r < 10; r++) {
					c = serial_getchar();
					printf("%02X", c);
				}
				getchar();
				break;
			
			case 'p':
				pb_ping();
				break;
			
			case 'o':
				//pb_sendComposed2(PB_COMMAND_PING_HOST, 0x12, 0x34);
				pb_sendAsciiz(PB_COMMAND_PING_HOST, "Hello host! This is parallelBuddy!");
				break;
			
			case 's':
				printf("Init SD...");
				r = sd_init();
				if (r == 1)	printf("OK\n");
				else		printf("E=%d\n", r);
				
				getchar();
				if (r != 1) continue;
				
				
				printf("Open file...");
				handle = sd_open("test.txt", PB_FILE_READ);
				printf("OK\n");
				
				getchar();
			
				printf("Read...");
				l = file_read(handle, &tmpData[0], 32);
				printf("%d bytes\n", l);
				
				getchar();
				
				printf("\"%s\"\n", (char *)&tmpData[0]);
				getchar();
			
				printf("Close file...");
				file_close(handle);
				printf("OK\n");
				break;
			
			default:
				printf("?\n");
		}
		
	} while(c != 'q');
	
	
	return 0;
}

*/

/*
// Sanity check
int main(int argc, char *argv[]) {
	char c;
	
	(void)argc; (void)argv;
	
	serial_puts("Hello!");
	printf("Press keys to send to serial: ");
	while(1) {
		
		//printf("Hello LCD\n");
		//serial_puts("Hello SERIAL!\n");
		c = getchar();
		printf("%c", c);
		serial_putchar(c);
		
		if (c == 'h') {
			serial_puts("Hello!");
		}
		
	}
}
*/
#endif 	//__PARABUDDY_H