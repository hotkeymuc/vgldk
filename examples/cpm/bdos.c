#ifndef __BDOS_C
#define __BDOS_C

/*
	BDOS - Basic Disk Operating System
	==================================
	
	The BDOS sits "on top" of BIOS and takes care of file access
	
*/

// First bytes of _CODE area should be the BDOS entry

void bdos() __naked {
	__asm
		jp _bdos_entry
		.asciz '<- bdos_entry jump'
	__endasm;
}

#include "fcb.h"

#include "bdos.h"


// @TODO: De-couple from BIOS at compile time
//#include <stdio.h>	// for puts() putchar() gets() getchar()
//#include <stdiomin.h>	// for puts() putchar() gets() getchar()
#include "bios.h"


// Make puts / gets etc. talk directly to BIOS
void bdos_putchar(char c) {
	bios_conout(c);
}
byte bdos_getchar() {
	return bios_conin();
}

void bdos_puts(const char *str) {
	while(*str) bios_conout(*str++);
}
void bdos_gets(char *pc) {
	char *pcs;
	char c;
	pcs = pc;
	
	while(1) {
		c = bios_conin();
		if ( (c == 8) || (c == 127) ) {
			// Backspace/DEL
			if (pc > pcs) {
				pc--;
				//if (lcd_x > 0) {
				//	lcd_x--;
				//	vgl_lcd_set_cursor();
				//}
			}
			continue;
		}
		
		bios_conout(c);
		
		if ((c == '\n') || (c == '\r') || (c == 0)) {
			// End of string
			
			// Terminate string
			*pc = 0;
			return;
		}
		
		// Add char
		*pc++ = c;
	}
}

// Helpers
void bdos_printf(char *pc) {
	char c;
	c = *pc;
	while(c != 0) {
		bios_conout(c);
		pc++;
	}
}

void bdos_printf_d(char *pc, byte d) {
	byte i;
	
	bdos_printf(pc);
	i = 100;
	while(i > 0) {
		bios_conout('0' + ((d / i) % 10));
		i /= 10;
	}
	//bios_conout('\n');
}


byte bdos_strlen(const char *c) {
	byte l;
	l = 0;
	while (*c++ != 0)  {
		l++;
	}
	return l;
}
void bdos_memset(byte *addr, byte b, word count) {
	while(count > 0) {
		*addr++ = b;
		count--;
	}
}

// Host communication
#ifdef BDOS_USE_HOST
	//void host_send(const byte *data, byte ldata);
	//int host_receive(byte *data);
	
	#ifdef BDOS_USE_HOST_SOFTSERIAL
		#include "host_softserial.h"
	#endif
	
	#ifdef BDOS_USE_HOST_MAME
		#include "host_mame.h"
	#endif
	
#endif



#ifdef BDOS_USE_HOST
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
			memcpy((byte *)&data[1], (byte *)fcb, 36);
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
				memcpy(data, p, l);
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
	
	
	//#ifdef BDOS_TRACE_CALLS
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
		memcpy(pstack, pdata, stack_count * 2);
		
		host_send(data, 1 + 7 + 2 + 1 + stack_count * 2);
	
	}
	//#endif
	
#endif



#ifdef BDOS_TRAP
	#define TRAP_SIZE 10
	
	byte *trap_addr;
	byte trap_backup[TRAP_SIZE];
	
	void trap_set(byte *a) {
		// Install a "trap" at specified address
		
		trap_addr = a;	// Store trap address
		memcpy(a, &trap_backup[0], TRAP_SIZE);
		
		// PUSH all
		*a++ = 0xf5;	// PUSH AF
		*a++ = 0xc5;	// PUSH BC
		*a++ = 0xd5;	// PUSH DE
		*a++ = 0xe5;	// PUSH HL
		
		// LD C, #0xff
		*a++ = 0x0e;	// LD C, #..
		*a++ = 0xff;	// 0xff
		
		// CALL 0x0005
		*a++ = 0xcd;	// CALL ....
		*a++ = 0x05;	// low
		*a++ = 0x00;	// high
	}
	
	void trap_unset() {
		memcpy(&trap_backup[0], trap_addr, TRAP_SIZE);
	}
	
	void trap_trigger() __naked {
		__asm
			pop af	; Initial POP (has been pushed on call)
			
			; Store all registers
			push af
			push bc
			push de
			push hl
			
			
			; Dump BEFORE fixing
			;call _host_sendstack
			
			; Fix return address to point at trap origin
			ld hl,#0x0008
			add hl,sp
			ld d, h
			ld e, l
			
			ld hl, (_trap_addr)
			ld a, l
			ld (de), a
			inc de
			ld a,h
			ld (de), a
			
		__endasm;
		
		bdos_puts("** TRAP **");
		
		#ifdef BDOS_USE_HOST
			host_sendstack();
		#endif
		
		getchar();
		
		trap_unset();
		
		bdos_puts("cont...");
		__asm
			
			pop hl
			pop de
			pop bc
			pop af
			
			ret
			
			
		__endasm;
		
	}
#endif



byte bdos_f_open(struct FCB *fcb) {
	byte r;
	
	#ifdef BDOS_USE_HOST
		// Send request to host
		host_sendfcb(BDOS_FUNC_F_OPEN, fcb);
		r = host_receivefcb(fcb);
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_open n/a");
		r = 0xff;
	#endif
	
	if (r != 0xff) {
		fcb->s2 |= 0x80;	// Make it "0x80" even if not found
		
		/*
		//bdos_file_ofs = 0;
		fcb->ex = 0;	// Current extent
		fcb->s1 = 2;	//@TODO: ?
		fcb->s2 |= 0x80;	// mark "open"?
		fcb->rc = 0x80;	// @TODO: Number of records in extend
		//fcb->d = .... whatever
		fcb->cr = 0;
		fcb->r0 = 0;
		fcb->r1 = 0;
		fcb->r2 = 0;
		// Rest is zero
		*/
	}
	
	return r;
}

byte bdos_f_close(struct FCB *fcb) {
	byte r;
	
	#ifdef BDOS_USE_HOST
		host_sendfcb(BDOS_FUNC_F_CLOSE, fcb);
		r = 0x00;
	#else
		//@TODO: Implement bare-metal version
		(void)fcb;	// Quiet the compiler
		bdos_puts("bdos_f_close n/a!");
		r = 0xff;
	#endif
	
	if (r != 0xff) {
		fcb->s2 &= 0x7f;	// remove "open" flag?
	}
	return r;
}

byte bdos_f_read(struct FCB *fcb) {
	word l;
	word rn;
	word ex;
	
	#ifdef BDOS_USE_HOST
		//host_sendfcb(BDOS_FUNC_F_READ, fcb);
		host_sendfcb(bdos_param_c, fcb);
		
		// Receive data
		l = host_receivedma();
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_read n/a!");
		l = 0;
	#endif
	
	// Return 1 on EOF
	if (l == 0) {
		return 1;	// 1 = EOF
	}
	
	if (l < 128) {
		// Fill with EOFs
		bdos_memset(bios_dma + l, 0x1a, 128 - l);
	}
	
	/*
	bdos_file_ofs += l;
	fcb->cr = SEQ_CR(bdos_file_ofs);
	fcb->ex = SEQ_EX(bdos_file_ofs);
	fcb->s2 = (0x80 | SEQ_S2(bdos_file_ofs));
	*/
	
	rn = fcb->cr + (fcb->ex * 128) + ((fcb->s2 & 1) * 16384);
	rn++;
	ex = rn / 128;
	fcb->cr = rn % 128;
	fcb->ex = ex % 32;
	fcb->s2 = (0x80 | (ex / 32));
	
	//bdos_puts("readOK");
	return 0x00;
}

byte bdos_f_readrand(struct FCB *fcb) {
	byte r;
	word rn;
	word ex;
	//word l;
	
	//bdos_puts("ReadRand");
	
	//host_sendfcb(BDOS_FUNC_F_READRAND, fcb);
	//l = host_receivedma();
	// Return 1 on EOF
	//if (l == 0) return 1;
	//return 0x00;	// Fake "OK"
	
	/*
	// Calculate absolute offset (in bytes)
	bdos_file_ofs = ((long)fcb->r0 + ((long)fcb->r1 * 256)) * 128L;
	fcb->cr = SEQ_CR(bdos_file_ofs);
	fcb->ex = SEQ_EX(bdos_file_ofs);
	fcb->s2 = (0x80 | SEQ_S2(bdos_file_ofs));
	*/
	// Calculate absolute offset (in records)
	rn = (word)fcb->r0 + ((word)fcb->r1 * 256);
	ex = rn / 128;
	fcb->cr = rn % 128;
	fcb->ex = ex % 32;
	fcb->s2 = (0x80 | (ex / 32));
	
	// Proceed with sequencial read
	r = bdos_f_read(fcb);
	return r;
}

byte bdos_f_write(struct FCB *fcb) {
	#ifdef BDOS_USE_HOST
		host_sendfcb(BDOS_FUNC_F_WRITE, fcb);
		//return 0xff;
		return 0x00;	// Fake "OK"
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_write n/a!");
		(void)fcb;	// Quiet the compiler
		return 0xff;
	#endif
	
}

byte bdos_f_writerand(struct FCB *fcb) {
	#ifdef BDOS_USE_HOST
		host_sendfcb(BDOS_FUNC_F_WRITERAND, fcb);
		//return 0xff;
		return 0x00;	// Fake "OK"
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_writerand n/a!");
		(void)fcb;	// Quiet the compiler
		return 0xff;
	#endif
}

/*
byte bdos_f_writezf(struct FCB *fcb) {
	#ifdef BDOS_USE_HOST
		host_sendfcb(BDOS_FUNC_F_WRITEZF, fcb);
		
		//return 0xff;
		return 0x00;	// Fake "OK"
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_writezf n/a!");
		(void)fcb;	// Quiet the compiler
		return 0xff;
	#endif
}
*/

byte bdos_f_sfirst(struct FCB *fcb) {
	#ifdef BDOS_USE_HOST
		host_sendfcb(BDOS_FUNC_F_SFIRST, fcb);
		return host_receivefcb(fcb);
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_sfirst n/a!");
		(void)fcb;	// Quiet the compiler
		return 0xff;
	#endif
}
byte bdos_f_snext(struct FCB *fcb) {
	#ifdef BDOS_USE_HOST
		host_sendfcb(BDOS_FUNC_F_SNEXT, fcb);
		return host_receivefcb(fcb);
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_snext n/a!");
		(void)fcb;	// Quiet the compiler
		return 0xff;
	#endif
}



void bdos_init() __naked {
	//bdos_dma = (byte *)0x0080;
	bdos_delimiter = '$';
	bdos_user = 1;
	
	bdos_memset((byte *)bdos_fcb, 0x00, 36);	//sizeof(FCB));
	
	#ifdef BDOS_PATCH_JUMP_TO_TOP
		// At the moment, the BODS entry point "bdos()" is not at the top of RAM / bottom of code segment.
		// We need to patch the JP instruction at 0x0005 to point to the end of RAM.
		// Reason: ZORK looks at the address at 0x0006 to determine where to put its own stuff below.
		// So 0x0006 must point to the top of RAM, below the BDOS/BIOS/CCP code segment.
		
		// Clone the original jump instruction from 0x0005 to the end of RAM
		// It consists of three bytes: 0xC3 [lo] [hi]
		memcpy((byte *)0x0005, (byte *)(BDOS_TOP_OF_RAM - 2), 3);	// TOP_OF_RAM - 2 = 0x7ffd
		
		// Alter the pointer at 0x0006 to point to the cloned jump
		// We use 0x0006, since the byte at 0x0005 is 0xc3 (opcode for "JP") and the actual address is at 0x0006 and 0x0007
		*(word *)0x0006 = (BDOS_TOP_OF_RAM - 3);
	#endif
	
	
	//@TODO: Load CCP to transient area and run it!
	bdos_puts("Must load CCP now (n/a)");
	bdos_getchar();
	
}

// Main BDOS entry point (0x0005 should jumps here)
void bdos_entry() __naked {
	char *pc;
	char c;
	byte s;
	word bdos_param_de;
	byte bdos_func;
	//FCB *fcb;
	
	// BDOS
	
	// Ideally this whole procedure should be done in pure ASM
	// But while developing, it is much easier to debug in C
	
	__asm
		//#ifdef BDOS_TRACE_CALLS
		#ifdef BDOS_SAVE_ALL_REGISTERS
			; Save EVERYTHING
			push af
			push hl
			
			; Save register "A" first, we need that register
			ld (_bdos_param_a), a
			
			
			; Store register "B"
			ld a, b
			ld (_bdos_param_b), a
			
			; Store register "C"
			ld a, c
			ld (_bdos_param_c), a
			
			
			; Store register "D"
			ld a, d
			ld (_bdos_param_d), a
			
			; Store register "E"
			ld a, e
			ld (_bdos_param_e), a
			
			
			; Store register "H"
			ld a, h
			ld (_bdos_param_h), a
			
			; Store register "L"
			ld a, l
			ld (_bdos_param_l), a
			
			
			; Now get the stack pointer
			ld hl, #0x04	; Skip the items we just pushed ourselves
			add hl, sp
			
			; Write SP to global variable(s)
			ld a, h
			ld (_bdos_s), a
			ld a, l
			ld (_bdos_p), a
			
			pop hl
			pop af
		#else
			; Make registers available in C context
			push af
			
			; Store BDOS function number "C"
			ld a, c
			ld (_bdos_param_c), a
			
			; Store 8-bit argument "D"
			ld a, d
			ld (_bdos_param_d), a
			
			; Store 8-bit argument "E"
			ld a, e
			ld (_bdos_param_e), a
			
			pop af
		#endif
		
		
		#ifdef BDOS_TRAP
			push af
			ld a, c
			
			;cp #0xff
			.db #0xfe
			.db #0xff
			
			jp z, _trap_trigger
			pop af
		#endif
		
	__endasm;
	
	
	//@TODO: Add a custom BDOS command for "breakpoint" / "single step"
	// => Put a "ld c, BDOS_FUNC_BREAK, call 0x0005" into memory to stop execution
	// stop compilation here!
	
	
	// For debugging
	#ifdef BDOS_TRACE_CALLS
	// Send stack frame to host for analysis
	host_sendstack();
	#endif
	
	bdos_func = bdos_param_c;
	bdos_param_de = (word)bdos_param_d * 256 + (word)bdos_param_e;
	
	
	//printf_d("BDOS", bdos_func);
	//printf("FCB="); printf_x4(bdos_param_de);
	//printf("\n");
	
	// BDOS function selection
	switch(bdos_func) {
		
		case BDOS_FUNC_P_TERMCPM:	// 0: System reset
			// We call this from BIOS on wboot
			
			bdos_puts("TERMCPM");
			bdos_getchar();
		
			//@TODO: Warm boot / go to CCP
			//ccp();
			/*
			__asm
				// Reset
				jp 0x0000
			__endasm;
			*/
			__asm
				jp _bdos_init
			__endasm;
			break;
		
		case BDOS_FUNC_C_READ:	// 1: Console input
			s = bios_conin();	// Get a char from bios
			bios_conout(s);	// Echo it
			bdos_return1(s);	// Return it
			break;
		
		case BDOS_FUNC_C_WRITE:	// 2: Console output
			bios_conout(bdos_param_e);
			break;
		
		case BDOS_FUNC_A_READ:	// 3: Reader input
			s = bios_reader();	// Get a char from bios
			// Return ^Z on EOF
			//bios_conout(s);	// Echo it
			bdos_return1(s);	// Return it
			break;
		
		case BDOS_FUNC_A_WRITE:	// 4: Punch output
			bios_punch(bdos_param_e);
			break;
		
		case BDOS_FUNC_L_WRITE:	// 5: List output
			bios_list(bdos_param_e);
			break;
		
		case BDOS_FUNC_C_RAWIO:	// 6: Direct console I/O
			// Entered with C=6, E=code. Returned values (in A) vary.
			switch(bdos_param_e) {
				case 0xff:
					// Return a character without echoing if one is waiting; zero if none is available. In MP/M 1, this works like E=0FDh below and waits for a character.
					//bdos_return1(inkey());	// Non-blocking! Do NOT echo!
					
					c = bios_const();
					if (c == 0xff) {	// Key is pressed
						c = bios_conin();	// Get that key
						//if (c == getchar_last) bdos_return1(0);	// No new key
					} else {
						//getchar_last = 0;
						bdos_return1(0);	// No new key
					}
					
					// New key
					//getchar_last = c;
					bdos_return1(c);
					break;
					
				case 0xfe:
					// [CP/M3, NovaDOS, Z80DOS, DOS+] Return console input status. Zero if no character is waiting, nonzero otherwise.
					//bdos_return1(checkkey());
					bdos_return1(bios_const());
					break;
				case 0xfd:
					// [CP/M3, DOS+] Wait until a character is ready, return it without echoing.
					//bdos_return1(getchar());	// Do NOT echo!
					bdos_return1(bios_conin());	// Do NOT echo!
				case 0xfc:
					// [DOS+] One-character lookahead - return the next character waiting but leave it in the buffer.
					bdos_return1(0);
				default:
					bios_conout(bdos_param_e);
			}
			break;
		
		case BDOS_FUNC_GET_IOBYTE:	// 7: Get I/O Byte
			bdos_return1(bios_iobyte);
			break;
		
		case BDOS_FUNC_SET_IOBYTE:	// 8: Set I/O Byte
			bios_iobyte = bdos_param_e;
			break;
		
		case BDOS_FUNC_C_WRITESTR:	// 9: Print string (until delimiter "$")
			pc = (char *)bdos_param_de;
			while(*pc != bdos_delimiter) {
				bios_conout(*pc++);
			}
			break;
		
		case BDOS_FUNC_C_READSTR:	// 10: Read console buffer
			pc = (char *)bdos_param_de;
			s = *pc;	// First byte contains maximum buffer size
			bdos_gets(pc);
			
			// Add delimiter character
			s = bdos_strlen(pc);
			pc = (char *)(bdos_param_de + s);
			*pc = bdos_delimiter;
			break;
		
		case BDOS_FUNC_C_STAT:	// 11: Get console status
			// Returns A=0 if no characters are waiting, nonzero if a character is waiting.
			//bdos_return1(checkkey());
			bdos_return1(bios_const());
			break;
		
		case BDOS_FUNC_S_BDOSVER:	// 12: Return version number
			// Returns B=H=system type, A=L=version number.
			// type = 0=8080, 1=8086, 2=68000/Z8000; bit 0 = MP/M, bit 1 = CP/Net, bit 2 = 16-Bit multi-user
			// Version: Hex representation of major and minor number, e.g. 0x22 = v2.2
			bdos_return2(0, 0x22);
			break;
		
		case BDOS_FUNC_DRV_ALLRESET:	// 13: Reset disk system
			//@TODO: Implement
			// Close disks, reset DMA, forget directory
			break;
		
		case BDOS_FUNC_DRV_SET:	// 14: Select disk
			//cpm_drive = bdos_param_e;	// 0 = A, 1 = B
			
			//@TODO: If disk not available: return A=0xff, H=1
			bios_seldsk(bdos_param_e);
			
			bdos_return1(0);	// 0 = OK, 0xff = error
			break;
		
		case BDOS_FUNC_F_OPEN:	// 15: Open file
			// DE = FCB address
			// A = Result (0xff = error, 0-3 = OK)
			// Some versions (including CP/M 3) always return zero; others return 0-3 to indicate that an image of the directory entry is to be found at (80h+20h*A).
			
			//fcbp = (FCB *)bdos_param_de;
			//bdos_return1(0xff);	// (0xff = error, 0-3 = OK)
			
			s = bdos_f_open((struct FCB *)bdos_param_de);
			//printf_d("OPEN:A=", s);
			bdos_return2(s, 0);
			break;
		
		case BDOS_FUNC_F_CLOSE:	// 16: Close file
			// DE = FCB address
			// A = Result (0xff = error, 0-3 = OK)
			// This function closes a file, and writes any pending data. This function should always be used when a file has been written to.
			// Some versions (including CP/M 3) always return zero; others return 0-3 to indicate that an image of the directory entry is to be found at (80h+20h*A).
			s = bdos_f_close((struct FCB *)bdos_param_de);
			bdos_return2(s, 0);
			break;
		
		case BDOS_FUNC_F_SFIRST:	// 17: Search for first
			// DE=address of FCB. Returns error codes in BA and HL.
			// Search for the first occurrence of the specified file; the filename should be stored in the supplied FCB.
			// The filename can include ? marks
			s = bdos_f_sfirst((struct FCB *)bdos_param_de);
			//bdos_return1(s);
			bdos_return2(s, 0);
			break;
		
		case BDOS_FUNC_F_SNEXT:	// 18: Search for next
			// DE=(address of FCB)? Returns error codes in BA and HL.
			s = bdos_f_snext((struct FCB *)bdos_param_de);
			bdos_return1(s);
			break;
		
		case BDOS_FUNC_F_DELETE:	// 19: Delete file
			// DE=address of FCB. Returns error codes in BA and HL.
			// Returns A=0FFh if error, otherwise 0-3
			
			//@TODO: Implement!
			bdos_return1(0xff);
			break;
		
		case BDOS_FUNC_F_READ:	// 20: Read sequential
			// DE=address of FCB. Returns error codes in BA and HL.
			/*
			0	OK,
			1	end of file,
			9	invalid FCB,
			10	(CP/M) media changed; (MP/M) FCB checksum error,
			11	(MP/M) unlocked file verification error,
			0FFh	hardware error.
			*/
			s = bdos_f_read((struct FCB *)bdos_param_de);
			bdos_return2(s, 0);
			break;
		
		case BDOS_FUNC_F_WRITE:	// 21: Write sequential
			// DE=address of FCB. Returns error codes in BA and HL.
			/*
			0	OK,
			1	directory full,
			2	disc full,
			8	(MP/M) record locked by another process,
			9	invalid FCB,
			10	(CP/M) media changed; (MP/M) FCB checksum error,
			11	(MP/M) unlocked file verification error,
			0FFh	hardware error.
			*/
			bdos_return1(bdos_f_write((struct FCB *)bdos_param_de));
			break;
		
		case BDOS_FUNC_F_MAKE:	// 22: Make file
			// DE=address of FCB. Returns error codes in BA and HL.
			
			//@TODO: Implement!
			bdos_return1(0xff);	// 0xff if directory is full
			break;
		
		case BDOS_FUNC_F_RENAME:	// 23: Rename file
			// DE=address of FCB. Returns error codes in BA and HL.
			
			//@TODO: Implement!
			bdos_return1(0xff);	// Returns A=0-3 if successful; A=0FFh if error. Under CP/M 3, if H is zero then the file could not be found
			break;
		
		case BDOS_FUNC_DRV_LOGINVEC:	// 24: Return login vector
			// Bit 7 of H corresponds to P: while bit 0 of L corresponds to A:. A bit is set if the corresponding drive is logged in.
			bdos_return2(0x00, 0x01);	// 0x00 0x01 = Only drive A
			break;
		
		case BDOS_FUNC_DRV_GET:	// 25: Return current disk
			// Entered with C=19h. Returns drive in A. Returns currently selected drive. 0 => A:, 1 => B: etc.
			bdos_return1(bios_curdsk);
			break;
		
		case BDOS_FUNC_F_DMAOFF:	// 26: Set DMA address
			// Entered with C=1Ah, DE=address.
			// Set the Direct Memory Access address; a pointer to where CP/M should read or write data.
			// Initially used for the transfer of 128-byte records between memory and disc, but over the years has gained many more functions.
			bios_setdma((byte *)bdos_param_de);
			break;
		
		case BDOS_FUNC_DRV_ALLOCVEC:	// 27: Get addr (alloc)
			// BDOS function 27 (DRV_ALLOCVEC) - Return address of allocation map
			// Supported by: All versions, but differs in banked versions.
			// Entered with C=1Bh. Returns address in HL (16-bit versions use ES:BX).
			// Return the address of the allocation bitmap (which blocks are used and which are free) in HL. Under banked CP/M 3 and MP/M, this will be an address in bank 0 (the system bank) and not easily accessible.
			// Under previous versions, the format of the bitmap is a sequence of bytes, with bit 7 of the byte representing the lowest-numbered block on disc, and counting starting at block 0 (the directory). A bit is set if the corresponding block is in use.
			// Under CP/M 3, the allocation vector may be of this form (single-bit) or allocate two bits to each block (double-bit). This information is stored in the SCB.
			
			//@TODO: Implement!
			//bdos_puts("ALLOCVEC");
			break;
		
		// 28: Write protect disk
		// 29: Get R/O vector
		// 30: Set file attributes
		// 31: Get addr (disk parms)
		
		case BDOS_FUNC_F_USERNUM:	// 32: Set/Get user code
			// BDOS function 32 (F_USERNUM) - get/set user number
			// Supported by: CP/M 2 and later.
			// Entered with C=20h, E=number. If E=0FFh, returns number in A.
			// Set current user number. E should be 0-15, or 255 to retrieve the current user number into A.
			// Some versions can use user areas 16-31, but these should be avoided for compatibility reasons.
			// DOS+ returns the number set in A.
			
			if (bdos_param_e == 0xff) {
				bdos_return1(bdos_user);
			} else {
				bdos_user = bdos_param_e;
				bdos_return1(bdos_user);
			}
			break;
		
		case BDOS_FUNC_F_READRAND:	// 33: Read random
			s = bdos_f_readrand((struct FCB *)bdos_param_de);
			bdos_return2(s, 0);
			break;
		
		case BDOS_FUNC_F_WRITERAND:	// 34: Write random
			//bdos_puts("WRITERAND");
			s = bdos_f_writerand((struct FCB *)bdos_param_de);
			bdos_return2(s, 0);
			break;
		
		case BDOS_FUNC_F_SIZE:	// 35: Compute file size
			//@TODO: Return "0", but update FCB's R0, R1, R2 records:
			/*
			s.st_size >>= 7;
			FCB_R0(fcb) = s.st_size & 0xff;
			s.st_size >>= 8;
			FCB_R1(fcb) = s.st_size & 0xff;
			s.st_size >>= 8;
			FCB_R2(fcb) = s.st_size & 1;
			*/
			bdos_puts("SIZE");
			break;
		
		case BDOS_FUNC_F_RANDREC:	// 36: Set random record
			// Update FB.R0/R1/R2 to reflect current pos
			bdos_puts("RANDREC");
			/*
			fcb->r0 = (bdos_file_ofs >> 7) & 0xff;
			fcb->r1 = (bdos_file_ofs >> 7) >> 8;
			fcb->r2 = (bdos_file_ofs >> 7) >> 16;
			fcb->cr = SEQ_CR(bdos_file_ofs);
			fcb->ex = SEQ_EX(bdos_file_ofs);
			fcb->s2 = SEQ_S2(bdos_file_ofs);
			*/
			break;
		
		case BDOS_FUNC_DRV_RESET:	// 37: Reset drive
			//bdos_puts("DRV_RST");
			break;
		
		// 38: Undefined - go back
		// 39: Undefined - go back
		
		case BDOS_FUNC_F_WRITEZF:	// 40: Fill random file w/ zeros
			bdos_puts("WRITEZF");
			//bdos_return1(bdos_f_writezf((FCB *)bdos_param_de));
			break;
		
		/*
		#ifdef BDOS_TRAP
		case 0xff:	// My own custom function to TRAP
			//bdos_puts("** TRAP **");
			trap_trigger();
			//host_sendstack();
			//getchar();
			break;
		#endif
		*/
		
		default:
			bdos_printf_d("BDOS#", bdos_func);
			bdos_getchar();
	}
	
	__asm
		ret
	__endasm;
}

#endif	// __BDOS_C