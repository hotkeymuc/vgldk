#ifndef __BUSBUDDY_H
#define __BUSBUDDY_H
/*

Bus Buddy
=========

-> Highly experimental / Proof-of-concept stage!

Function for calling functions on an external microcontroller.
Using the "V-Tech Bus Ficker"-Cartridge and the "Bus Buddy" sketch running on an Arduino MEGA.

This header file provides functions to communicate with the BusBuddy, issue commands and get results.

Since my "Bus Ficker" cartridge hardware is kinda flaky, I need to employ checksums to correctly (re)transmit data between the V-Tech and Arduino.
(Reason: The ~F0_RD and ~F0_WR on the "Bus Ficker" cartridge Rev.C are not clean and sometimes trigger twice/not at all)

How this file is structured works:
	* bf_xxx functions do the hardware communication with the "Bus Ficker" cartridge (i.e. memory mapped I/O)
	* bb_xxx functions run on top of that, providing high-level access to the "Bus Buddy" Arduino sketch
	* V-Tech sends a "frame" to the memory mapped I/O, this data is parsed (checksum) and handled by the Arduino.
	* The Arduino answers to commands and re-transmits the last answer if so requested (checksum mismatch etc.)
	* If events should be fired, they need to be acively polled as there are no interrupts on the V-Tech side (yet?)

2019-07-27 Bernhard "HotKey" Slawik
*/

#include "busficker.h"

// BusBuddy defines
//#define BB_USE_FILES	// Include everything needed to handle files/SD cards
//#define BB_USE_HOST	// Include everything needed to handle communicate with host

//#define BB_NO_CHECKSUM  // Disable checksums! Use at own risk!
//#define BB_DEBUG_TRAFFIC	// Show each payload byte on screen
//#define BB_DEBUG_PROTOCOL_ERRORS_PRINTF	// Report protocol errors to screen
//#define BB_IGNORE_PROTOCOL_ERRORS	// Use at own risk! Will break cause-and-effect relation
//#define BB_PAUSE_ON_PROTOCOL_ERRORS	// Wait for key on error
//#define BB_HOST_GETS_NONBLOCKING	// host_gets() returns empty string instead of handling error/resending


#ifndef BB_BUFFER_SIZE
	#define BB_BUFFER_SIZE 64
#endif

#define BB_MAX_FILENAME 32

// Temporary buffer for a frame
byte bb_frame[BB_BUFFER_SIZE];

// Special protocol bytes
const byte BB_PAD = 0x00;
const byte BB_START = 0xff;	//'#';
//const byte BB_END = '\n';

// Commands
#define BB_CMD_PING 0x01
#define BB_CMD_RESEND 0x02
#define BB_CMD_DEBUG 0x03
//#define BB_CMD_BOOT 0x04

#ifdef BB_USE_HOST
	#define BB_CMD_HOST_BYTES_AVAILABLE 0x10
	#define BB_CMD_HOST_GETCHAR 0x11
	#define BB_CMD_HOST_PUTCHAR 0x12
	#define BB_CMD_HOST_GETS 0x13
	#define BB_CMD_HOST_PUTS 0x14
#endif

#ifdef BB_USE_FILES
	#define BB_CMD_SD_INIT 0x20
	#define BB_CMD_SD_EXISTS 0x21
	#define BB_CMD_SD_OPEN 0x22
	#define BB_CMD_SD_OPEN_NEXT 0x23
	#define BB_CMD_FILE_INFO 0x24
	
	#define BB_FILE_BYTES_AVAILABLE 0x30
	#define BB_CMD_FILE_CLOSE 0x31
	#define BB_CMD_FILE_READ 0x33
	#define BB_CMD_FILE_WRITE 0x34
	//#define BB_CMD_FILE_SEEK 0x35
	//#define BB_CMD_FILE_SIZE 0x36
	
	//#define BB_CMD_FILE_REMOVE 0x37
	//#define BB_CMD_FILE_MKDIR 0x38
	//#define BB_CMD_FILE_RMDIR 0x39
	
	
	
	#define BB_HANDLE byte
	#define BB_NO_HANDLE 0xff // Meaning "no file returned"
	
	#define BB_FILE_MODE_READ 0
	#define BB_FILE_MODE_WRITE 1
	#define BB_FILE_MODE_DIRECTORY 8
#endif	// BB_USE_FILES

// BB Protocol


#ifdef BB_DEBUG_PROTOCOL_ERRORS_PRINTF
	#define bb_debugError(s) printf(s)
	#define bb_debugError2(s,v1,v2) printf(s,v1,v2)
#else
	#define bb_debugError(s) ;
	#define bb_debugError2(s,v1,v2) ;
#endif

#ifdef BB_DEBUG_TRAFFIC
	//byte bb_debugTraffic = 0;
	//#define debugTraffic(f, v) {if (bb_debugTraffic) printf(f, v);}
	#define debugTraffic(f, v) printf(f, v)
#else
	#define debugTraffic(f, v) ;
#endif



void bb_init() {
	bf_init();
	
	//#ifdef BB_DEBUG_TRAFFIC
	//	bb_debugTraffic = 0;
	//#endif
}

#define bb_getByte bf_getByte
word bb_getWord() {
	return ((word)bb_getByte() << 8) | bb_getByte();
}

#define bb_putByte(b) bf_putByte(b)
void bb_putWord(word w) {
	bf_putByte(w >> 8);
	bf_putByte(w & 0xff);
}

void bb_flush() {
	// Remove all waiting bytes
	
	while(bb_getByte() != BB_PAD) {
	}
}




void bb_putFrame(byte *txData, word txSize) {
	word i;
	byte *d;
	byte b;
	byte check;
	
	// Send txData
	bb_putByte(BB_START);
	
	bb_putWord(txSize);
	
	check = 0x55;
	d = txData;
	for (i = 0; i < txSize; i++) {
		b = *d++;
		bb_putByte(b);
		check ^= b;
	}
	
	#ifdef BB_NO_CHECKSUM
		// Do not send checksum
	#else
		bb_putByte(check);
	#endif
	
	//bb_putByte(BB_END);
}

int bb_getFrame(byte *rxData, word rxMaxSize) {
	word i;
	word rxSize;
	word timeout;
	byte *d;
	byte b;
	#ifdef BB_NO_CHECKSUM
		// No data needed
	#else
		byte checkGiven;
	#endif
	byte checkActual;
	
	// Receive txData
	
	// Wait for start / sync
	timeout = 0xF000;
	
	// Flush
	do {
		b = bb_getByte();
		
		// Timeout
		timeout--;
		if (timeout <= 0) {
			bb_debugError("TO0");
			return -1;
		}
	} while (b != BB_PAD);
	
	// Wait for start
	do {
		b = bb_getByte();
		
		// Timeout
		timeout--;
		if (timeout <= 0) {
			bb_debugError("TO1");
			return -1;
		}
	} while (b != BB_START);
	
	
	rxSize = bb_getWord();
	
	debugTraffic("RX %04X: ", rxSize);
	
	if (rxSize > rxMaxSize) {
		bb_debugError2("SM %d>%d", rxSize, rxMaxSize);
		return -2;
	}
	
	// Receive data and build checksum
	checkActual = 0x55;
	d = rxData;
	for (i = 0; i < rxSize; i++) {
		b = bb_getByte();
		debugTraffic("%02X ", b);
		*d++ = b;
		checkActual ^= b;
		
	}
	
	#ifdef BB_NO_CHECKSUM
		// Do not care
	#else
		// Compare checksums
		checkGiven = bb_getByte();
		
		if (checkGiven != checkActual) {
			bb_debugError2("CS %02X!=%02X", checkGiven, checkActual);
			return -3;
		} else {
			debugTraffic("cs=%02X", checkGiven);
		}
	#endif
	
	/*
	// Check for end character
	b = bb_getByte();
	if (b != BB_END) {
		bb_debugError("NE");
		//printf("No BB_END! (%02X)\n", b);
		return -4;
	}
	*/
	return rxSize;
}

// BusBuddy Functions
void bb_cmd(byte cmd) {
	// Send a command frame without arguments
	bb_frame[0] = cmd;
	bb_putFrame(&bb_frame[0], 1);
}

void bb_cmd_byte(byte cmd, byte v) {
	// Send a command frame with 1-byte argument
	bb_frame[0] = cmd;
	bb_frame[1] = v;
	bb_putFrame(&bb_frame[0], 2);
}
void bb_cmd_word(byte cmd, word v) {
	// Send a command frame with two 1-byte arguments
	bb_frame[0] = cmd;
	bb_frame[1] = v >> 8;
	bb_frame[2] = v & 0xff;
	bb_putFrame(&bb_frame[0], 3);
}
void bb_cmd_asciiz(byte cmd, const char *s) {
	// Send a command frame with zero-terminated string argument
	byte b;
	byte *bs;
	byte *f;
	byte l;
	
	f = &bb_frame[0];
	*f++ = cmd;
	
	//@TODO: memcpy!
	bs = (byte *)s;
	l = 0;
	while(1) {
		b = *bs++;
		if (b == 0) break;	// Do not include zero byte in payload
		*f++ = b;
		l ++;
	}
	
	bb_putFrame(&bb_frame[0], 1+l);
}

void bb_cmd_data(byte cmd, byte *data, const byte dataLen) {
	// Send a command frame with variable length argument
	byte *f;
	byte l;
	
	f = &bb_frame[0];
	*f++ = cmd;
	
	//@TODO: memcpy/strcpy!
	l = dataLen;
	while (l > 0) {
		*f++ = *data++;
		l--;
	}
	bb_putFrame(&bb_frame[0], 1+dataLen);
}
void bb_cmd_byte_data(byte cmd, byte v, byte *data, const byte dataLen) {
	// Send a command frame with a byte plus a variable length argument
	byte *f;
	byte l;
	
	f = &bb_frame[0];
	*f++ = cmd;
	*f++ = v;
	
	//@TODO: memcpy/strcpy!
	l = dataLen;
	while (l > 0) {
		*f++ = *data++;
		l--;
	}
	bb_putFrame(&bb_frame[0], 1+1+dataLen);
}

void bb_cmd_resend() {
	#ifdef BB_PAUSE_ON_PROTOCOL_ERRORS
		getchar();
	#endif
	
	#ifdef BB_IGNORE_PROTOCOL_ERRORS
		// Do nothing
		return;
	#else
		//getchar();
		//#ifdef BB_DEBUG_TRAFFIC
		//	bb_debugTraffic = 1;
		//#endif
		
		// Drain all incoming bytes until empty
		bb_flush();
		
		// Now ask Arduino to resend last frame
		bb_cmd(BB_CMD_RESEND);
	#endif
}


byte bb_result_byte() {
	// Receive a 1-byte answer. Handles re-sending on error. Returns that byte.
	byte r;
	while (bb_getFrame(&r, 1) < 0) {
		bb_cmd_resend();
	}
	return r;
}
word bb_result_word() {
	// Receive a 16 bit word answer (MSB first). Handles re-sending on error. Returns that word.
	byte r[2];
	while (bb_getFrame(&r[0], 2) < 0) {
		bb_cmd_resend();
	}
	return (((word)r[0]) << 8) | r[1];
}

byte bb_result_data(byte *buf, byte lMax) {
	// Receive a variable byte answer. Handles re-sending on error. Returns the actual number of bytes.
	int l;
	
	do {
		l = bb_getFrame(buf, lMax);
		if (l < 0) bb_cmd_resend();
	} while (l < 0);
	return l;
}
byte bb_result_asciiz(byte *s, byte lMax) {
	byte l;
	l = bb_result_data(s, lMax-1);
	s[l] = 0;	// Zero-terminate
	return l;
}

byte bb_ping(byte v) {
	byte r;
	
	//printf("Ping %02X...", v);
	bb_cmd_byte(BB_CMD_PING, v);
	
	r = bb_result_byte();
	//printf("Pong %02X!\n", r);
	
	return r;
}

#ifdef BB_USE_HOST

byte bb_host_bytesAvailable() {
	int i;
	byte r;
	
	bb_cmd(BB_CMD_HOST_BYTES_AVAILABLE);
	
	// Blocking
	//return bb_result_byte();
	
	// Non-blocking
	i = bb_getFrame(&r, 1);
	if (i < 0) return 0;	// Error means zero bytes available
	
	return r;
}

byte bb_host_getchar() {
	bb_cmd(BB_CMD_HOST_GETCHAR);
	return bb_result_byte();
}

void bb_host_putchar(byte v) {
	bb_cmd_byte(BB_CMD_HOST_PUTCHAR, v);
}

byte bb_host_gets(byte *s, byte maxL) {
	int l;
	//byte buf[MAX_LINE];
	
	bb_cmd(BB_CMD_HOST_GETS);
	
	#ifdef BB_HOST_GETS_NONBLOCKING
		// Non-blocking on error
		l = bb_getFrame(s, maxL);
		if (l < 0) l = 0;	// return NULL;
		s[l] = 0;	// Zero-terminate
	#else
		// Blocking / error correction
		//l = bb_result_data(s, MAX_LINE);
		//s[l] = 0;	// Zero-terminate
		l = bb_result_asciiz(s, maxL);
	#endif
	
	//return s;
	return l;
	
}

void bb_host_puts(byte *s) {
	//int l;
	//byte ok;
	
	bb_cmd_asciiz(BB_CMD_HOST_PUTS, s);
	//ok = bb_result_byte();
}
#endif

#ifdef BB_USE_FILES
void bb_cmd_byte2(byte cmd, byte v1, byte v2) {
	// Send a command frame with two 1-byte arguments
	bb_frame[0] = cmd;
	bb_frame[1] = v1;
	bb_frame[2] = v2;
	bb_putFrame(&bb_frame[0], 3);
}


byte sd_init() {
	byte ok;
	bb_cmd(BB_CMD_SD_INIT);
	ok = bb_result_byte();
	return ok;
}

BB_HANDLE sd_open(const char *filename, byte mode) {
	byte b;
	byte *bs;
	byte *f;
	byte l;
	BB_HANDLE handle;
	
	f = &bb_frame[0];
	*f++ = BB_CMD_SD_OPEN;
	*f++ = mode;
	
	//@TODO: memcpy(dest, src, size) / strcpy(dest, src)!
	bs = (byte *)filename;
	l = 0;
	while(1) {
		b = *bs++;
		if (b == 0) break;
		*f++ = b;
		l ++;
	}
	bb_putFrame(&bb_frame[0], 2+l);
	
	//@TODO: What happens if the file does not exist? Negative handle?
	handle = bb_result_byte();
	
	return handle;
}

byte sd_exists(const char *filename) {
	byte ex;
	
	bb_cmd_asciiz(BB_CMD_SD_EXISTS, (byte *)filename);
	ex = bb_result_byte();
	return ex;
}

BB_HANDLE sd_openNext(BB_HANDLE handle) {
	bb_cmd_byte(BB_CMD_SD_OPEN_NEXT, handle);
	
	//@TODO: What happens if the file does not exist (BB_NO_HANDLE)? Negative handle?
	handle = bb_result_byte();
	
	return handle;
}
void file_info(BB_HANDLE handle, byte *isDir, word *size, char *name) {
	byte data[1 + 2 + BB_MAX_FILENAME];
	byte l;
	byte *f;
	
	bb_cmd_byte(BB_CMD_FILE_INFO, handle);
	
	// Result is a struct
	l = bb_result_data(&data[0], 1+2+BB_MAX_FILENAME);
	
	*isDir = data[0];
	*size = (((word)data[1]) << 8) | data[2];
	
	// Rest is Name
	l -= 3;
	f = &data[3];
	while(l > 0) {
		*name++ = *f++;
		l--;
	}
	*name = 0;	// Zero terminate
	
}
byte file_bytesAvailable(BB_HANDLE handle) {
	bb_cmd_byte(BB_FILE_BYTES_AVAILABLE, handle);
	
	return bb_result_byte();
}

void file_close(BB_HANDLE handle) {
	bb_cmd_byte(BB_CMD_FILE_CLOSE, handle);
	// Has no return value
}

byte file_read(BB_HANDLE handle, byte *buf, byte l) {
	
	bb_cmd_byte2(BB_CMD_FILE_READ, handle, l);
	
	l = bb_result_data(buf, l);
	// Number of bytes read is equal to the size of the payload
	
	return l;
}
byte file_write(BB_HANDLE handle, byte *buf, byte l) {
	
	bb_cmd_byte_data(BB_CMD_FILE_WRITE, handle, buf, l);
	
	l = bb_result_byte();
	return l;
}
#endif	// BB_USE_FILES

#endif //__BUSBUDDY_H