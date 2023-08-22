//#ifndef __BDOS_C
//#define __BDOS_C

/*
	BDOS - Basic Disk Operating System
	==================================
	
	The BDOS talks to the BIOS and takes care of file access.
	
	Reference: https://www.seasip.info/Cpm/bdos.html
	
	TODO:
		* Use "#pragma callee-saves" / command line option "--callee-saves bdos, ..."
		
	
	2023-08-15 Bernhard "HotKey" Slawik
*/

#include "fcb.h"	// For files
#include "bdos.h"
#include "bios.h"	// For basic I/O


// First bytes of the _CODE area should be the BDOS entry function.
// The jump at CRT0 0x0005 should point here.
// Its address marks the end of the transient memory area (e.g. ZORK checks 0x0005 for that address to determine maximum RAM usage).
void bdos() __naked {
	/*
	// Dump call
	__asm
		; Skip lower calls
		ld a, c
		cp a, #5
		jr c, 0$
		
		; Dump
		push hl
		push de
		push bc
		call _bdos_printf_x2
		pop bc
		pop de
		pop hl
	0$:
	__endasm;
	*/
	
	__asm
		
		; Jump into table...
		ld hl, #_bdos_funcs	; BDOS function jump table
		
		; Function number is stored in C
		; Add it to the address (as BC, so just zero out B and we are fine)
		ld b, #0
		; Table consists of "JP xxxx" (3 bytes) entries. So add BC 3 times.
		add hl, bc
		add hl, bc
		add hl, bc
		
		jp (hl)	; Jump to BDOS function jump
		
	__endasm;
}



// Std. I/O Helpers
// Make puts / gets etc. talk directly to BIOS
void bdos_putchar(char c) {
	bios_conout(c);
}

byte bdos_getchar() {
	return bios_conin();
}

void bdos_puts(const char *str) {
//void bdos_puts(const char *str, char delimiter) {
	// Output zero terminated string
	while(*str) bios_conout(*str++);
	
	// Output variable terminated string
	//while(*str != delimiter) bios_conout(*str++);
	
	// New line after puts()
	//bios_conout('\r');
	bios_conout('\n');
}
//void bdos_gets(char *pc) {
//void bdos_gets(char *pc, char delimiter) {
void bdos_gets(char *pc, byte max_size) {
	char *pcs;
	char c;
	pcs = pc;
	byte l;
	
	l = 0;
	while(1) {
		c = bios_conin();
		
		// Echo
		bios_conout(c);
		
		if ( (c == 8) || (c == 127) ) {
			// Backspace/DEL
			//if (pcs > pc) {
			if (l > 0) {
				pcs--;
				l--;
				/*
				if (lcd_x > 0) {
					lcd_x--;
					vgl_lcd_set_cursor();
				}
				*/
			}
			continue;
		}
		
		// ENTER, EOF
		if ((c == '\n') || (c == '\r') || (c == 0x1a) || (c == 0)) {
			// End of string
			
			// Terminate string with zero
			*pcs = 0;
			
			// Terminate string with given delimiter
			//*pcs = delimiter;
			return;
		}
		
		if (l >= max_size) {
			//sound_beep();
			continue;
		}
		
		// Add char to buffer
		*pcs++ = c;
		l++;
	}
	
	//return pcs;
}

void bdos_printf(char *pc) {
	while(*pc) bios_conout(*pc++);
}


/*
#define HEX_USE_DUMP	// Include dump()
#include <hex.h>
#define bdos_printf_x2 printf_x2
#define bdos_printf_x4 printf_x4
*/
/*
//void bdos_printf_d(char *pc, byte d) {
void bdos_printf_d(byte d) {
	byte i;
	//bdos_printf(pc);
	i = 100;	// Maximum decimal digit (1/10/100/1000/...)
	while(i > 0) {
		bios_conout('0' + ((d / i) % 10));
		i /= 10;
	}
}
*/
byte bdos_hexDigit(byte c) {
	if (c < 10) return ('0'+c);
	return 'A' + (c-10);
}
void bdos_printf_x2(byte b) {
	bios_conout(bdos_hexDigit(b >> 4));
	bios_conout(bdos_hexDigit(b & 0x0f));
}
void bdos_printf_x4(word w) {
	bdos_printf_x2(w >> 8);
	bdos_printf_x2(w & 0x00ff);
}

void bdos_dump(word a, byte len) {
	byte i;
	byte b;
	byte *o;
	
	/*
	byte l;
	byte lLine;
	
	l = 0;
	while (l < len) {
		bdos_printf_x4(a); bdos_putchar('|');
		
		lLine = l;
		o = (byte *)a;
		for (i = 0; i < HEX_DUMP_WIDTH; i++) {
			if (l < len) {
				b = *o;
				//printf("%02X", b);
				bdos_printf_x2(b);
			} else {
				bdos_printf("  ");
			}
			#ifdef HEX_DUMP_INTRA_HEX
				bdos_printf(HEX_DUMP_INTRA_HEX);
			#endif
			l++;
			o++;
		}
		bdos_putchar('|');
		l = lLine;
		o = (byte *)a;
		for (i = 0; i < HEX_DUMP_WIDTH; i++) {
			if (l < len) {
				b = *o;
				if (b < 0x20)	bdos_putchar('.');
				else			bdos_putchar(b);
			} else bdos_putchar(' ');
			l++;
			o++;
		}
		a += HEX_DUMP_WIDTH;
		bdos_printf(HEX_DUMP_EOL);	//("\n");
	}
	*/
	
	// Simple
	// AAAAbbbbbbbbbbbbbbbb
	// hh hh hh hh hh hh hh
	//  hh hh hh hh hh hh h
	// h hh hh
	bdos_printf_x4(a);
	bdos_putchar('|');
	
	o = (byte *)a;
	for(i = 0; i < len; i++) {
		b = *o;
		if (b < 0x20)	bdos_putchar('.');
		else			bdos_putchar(b);
		o++;
	}
	
	bdos_putchar('|');
	//bdos_printf(" | ");
	//if (len != 16) bdos_printf("\n");
	
	o = (byte *)a;
	for(i = 0; i < len; i++) {
		b = *o;
		bdos_printf_x2(b);
		o++;
	}
	bdos_printf("\n");
	
}



// String Helpers
#include <stringmin.h>
#define bdos_strlen strlen
#define bdos_memcpy memcpy
#define bdos_memset memset
/*
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
*/

// Host communication (FCB)
#ifdef BDOS_USE_HOST
	#include "bdos_host.h"
#endif



#ifdef BDOS_TRAP
	#define TRAP_SIZE 10
	
	byte *trap_addr;
	byte trap_backup[TRAP_SIZE];
	
	void trap_set(byte *a) {
		// Install a "trap" at specified address
		
		trap_addr = a;	// Store trap address
		memcpy(&trap_backup[0], a, TRAP_SIZE);
		
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
		memcpy(trap_addr, &trap_backup[0], TRAP_SIZE);
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
			#ifdef BDOS_TRACE_CALLS
				host_sendstack();
			#endif
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


// BDOS functions

void bdos_init() __naked {	// BDOS_FUNC_P_TERMCPM:	// 0: System Reset
	
	// Called by bios_wboot()
	
	// Show BDOS banner (BIOS shows CP/M banner)
	//bdos_printf("BDOS");
	
	//bios_setdma(0x0080);	// Is already done at bios_wboot()
	
	// Restore BDOS entry (if corrupted)
	//*((word *)(0x0006)) = (word)&bdos_init;
	
	bdos_delimiter = '$';
	bdos_user = 1;	//@FIXME: This should be the upper bits of the bios_curdsk at 0x0004
	
	bdos_memset((byte *)bdos_fcb, 0x00, 36);	//sizeof(FCB));
	bdos_fcb_num = 0;	// currently used FCB in DMA area (0...3)
	
	#ifdef BDOS_PATCH_JUMP_TO_TOP
		// At the moment, the BODS entry point "bdos()" is not at the top of RAM / bottom of code segment.
		// We need to patch the JP instruction at 0x0005 to point to the end of RAM.
		// Reason: ZORK looks at the address at 0x0006 to determine where to put its own stuff below.
		// So 0x0006 must point to the top of RAM, below the BDOS/BIOS/CCP code segment.
		
		// Clone the original jump instruction from 0x0005 to the end of RAM
		// It consists of three bytes: 0xC3 [lo] [hi]
		memcpy((byte *)(BDOS_TOP_OF_RAM - 2), (byte *)0x0005, 3);	// TOP_OF_RAM - 2 = 0x7ffd
		
		// Alter the pointer at 0x0006 to point to the cloned jump
		// We use 0x0006, since the byte at 0x0005 is 0xc3 (opcode for "JP") and the actual address is at 0x0006 and 0x0007
		*(word *)0x0006 = (BDOS_TOP_OF_RAM - 3);
	#endif
	
	// Install scroll callback to pause after one page of text
	// ! Must be handled by BIOS - it "knows" the LCD driver.
	//bios_scroll_counter = LCD_ROWS;
	//lcd_scroll_cb = &bios_scroll_cb;
	
	
	// Load CCP to transient area and run it!
	//@TODO: Load from file using BDOS functions!
	while(1) {
		//bdos_printf("Load CCP [Y/N]?");
		bdos_printf("Load CCP?");
		char c = bdos_getchar();
		
		if (c == 'N') {
			bdos_printf("cart...");
			__asm
				jp 0x8000
			__endasm;
		}
		
		if (c == 'Y') {
			// Assuming CCP is included in current RAM image
			//word ccp_loc_code = 0x6000;
			//bdos_printf("0x6000...");
			bdos_printf_x4(CCP_LOC_CODE);
			bdos_printf("...");
			
			bdos_memset((byte *)0x0080, 0, 128);	// Clear DMA area (CCP command line arguments)
			//*((byte *)0x0080) = 0;	// Clear CCP argl (dma area)
			//*((byte *)0x0081) = 0;	// Clear CCP args (dma area+1)
			
			// Invoke CCP entry point (must match LOC_CODE of ccp compilation!)
			__asm
				;push af
				;push hl
				
				;jp 0x6000
				;call 0x6000
				jp CCP_LOC_CODE
				
				;pop hl
				;pop af
				
			__endasm;
		}
		
		// Else
		bdos_printf_x2(c);
		bdos_putchar('?');
	}
}

void bdos_c_read() __naked {	// BDOS_FUNC_C_READ:	// 1: Console input
	/*
	char c = bios_conin();	// Get a char from bios
	bios_conout(c);	// Echo it (as spec.)
	//bios_conout(')');	// for debugging
	bdos_return1(c);	// Return it
	*/
	__asm
		call _bios_conin
		; Result should be in L
		
		; Local echo (as per spec.)
		push hl	; Push parameter L (dont care about D). L will be the most recent stack argument, thats all that matters.
		call _bios_conout
		pop hl	; Pop parameter(s)
		
		; Result is still in L, great! So just return.
		ret
	__endasm;
}

void bdos_c_write(char c) __naked {	// BDOS_FUNC_C_WRITE:	// 2: Console output
	(void)c;	// Silence the  compiler about unused argument
	
	//bios_conout(bdos_param_e);
	__asm
		push de	; Push parameter E (dont care about D). E will be the most recent stack argument, thats all that matters.
		call _bios_conout
		pop de	; Pop the parameter(s). We pushed D and E, so we must pop both.
		ret
	__endasm;
}

void bdos_a_read() __naked {	// BDOS_FUNC_A_READ:	// 3: Reader input
	//s = bios_reader();	// Get a char from bios
	//// Return ^Z on EOF
	//bdos_return1(s);	// Return it
	__asm
		call _bios_reader
		; Result should be in L
		
		; ; Local echo (as per spec.)
		; push hl	; Push parameter L (dont care about D). L will be the most recent stack argument, thats all that matters.
		; call _bios_conout
		; pop hl	; Pop parameter(s)
		
		; Result is still in L, great! So just return.
		ret
	__endasm;
}

void bdos_a_write(char c) __naked {	//BDOS_FUNC_A_WRITE:	// 4: Punch output
	(void)c;	// Silence the  compiler about unused argument
	
	//bios_punch(bdos_param_e);
	__asm
		push de	; Push parameter E (dont care about D). E will be the most recent stack argument, thats all that matters.
		call _bios_punch
		pop de	; Pop the parameter(s). We pushed D and E, so we must pop both.
		ret
	__endasm;
}

void bdos_l_write(char c) __naked {	// BDOS_FUNC_L_WRITE:	// 5: List output
	(void)c;	// Silence the compiler about unused argument
	
	//bios_list(bdos_param_e);
	__asm
		push de	; Push parameter E (dont care about D). E will be the most recent stack argument, thats all that matters.
		call _bios_list
		pop de	; Pop the parameter(s). We pushed D and E, so we must pop both.
		ret
	__endasm;
}

void bdos_c_rawio(char c, char e) __naked {	//BDOS_FUNC_C_RAWIO:	// 6: Direct console I/O
	// Entered with C=6, E=code. Returned values (in A) vary.
	(void)c;(void)e;	// Silence the compiler about unused argument
	/*
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
	*/
	
	
	//bdos_puts("bdos_c_rawio n/a!");
	bdos_printf("rawio!"); bdos_getchar();
	
	//@TODO: Implement switch(e)
	__asm
		push de	; Push parameter E (dont care about D). E will be the most recent stack argument, thats all that matters.
		call _bios_conout
		pop de	; Pop the parameter(s). We pushed D and E, so we must pop both.
		ret
	__endasm;
}

void bdos_get_iobyte() __naked {	// BDOS_FUNC_GET_IOBYTE:	// 7: Get I/O Byte
	//bdos_return1(bios_iobyte);
	__asm
		push af
		ld a, (_bios_iobyte)
		ld l, a
		pop af
		ret
	__endasm;
}

void bdos_set_iobyte(char c) __naked {	// BDOS_FUNC_SET_IOBYTE:	// 8: Set I/O Byte
	(void)c;	// Silence the compiler about unused argument
	//bios_iobyte = bdos_param_e;
	__asm
		push af
		ld a, e
		ld (_bios_iobyte), a
		pop af
		ret
	__endasm;
}

void bdos_c_writestr(char *de) __naked {	// BDOS_FUNC_C_WRITESTR:	// 9: Print string (until delimiter "$")
	(void)de;	// Silence the compiler about unused argument
	
	// pc = (char *)bdos_param_de;
	// while(*pc != bdos_delimiter) {
	// 	bios_conout(*pc++);
	// }
	__asm
		push af
		push bc
		
		ld a, (_bdos_delimiter)
		ld b, a
		
	0$:
		ld a, (de)	; get byte
		cp a, b	; check for delimiter
		jr z, 1$	; delimiter? break!
		
		; Invoke bios_conout with A on stack
		push af	; Push AF...
		inc sp	; ...but neglect the F
		call _bios_conout
		inc sp	; Clean up rest of AF off the stack
		
		; increment DE and loop
		inc de
		jp 0$
		
	1$:
		pop bc
		pop af
	__endasm;
	
}


//@FIXME: Do it differently
volatile byte bdos_readstr_param_d;
volatile byte bdos_readstr_param_e;
void bdos_c_readstr(char *de) __naked {	// BDOS_FUNC_C_READSTR:	// 10: Read console buffer
	word bdos_param_de;
	char *pc;
	char s;
	
	// DE points to buffer (first byte is max size). Special case: DE=0 = use DMA area
	(void)de;	// Silence the compiler about unused argument
	//bdos_printf("DE="); bdos_printf_x4(bdos_param_de); bdos_getchar();
	__asm
		; Store register "D"
		ld a, d
		ld (_bdos_readstr_param_d), a
		
		; Store register "E"
		ld a, e
		ld (_bdos_readstr_param_e), a
	__endasm;
	
	bdos_param_de = (word)bdos_readstr_param_d * 256 + (word)bdos_readstr_param_e;
	if (bdos_param_de == 0) bdos_param_de = (word)bios_dma;	// 0x0080
	
	pc = (char *)bdos_param_de;
	s = *pc;	// First byte contains maximum buffer size
	
	//bdos_putchar('?'); bdos_getchar();
	
	// Actually read str (as zero-terminated c string)
	bdos_gets(pc+2, s);	// +0=max_size, +1=len, +2=data (without CR/LF)
	
	//bdos_putchar('!'); bdos_getchar();
	
	// Determine length
	s = bdos_strlen(pc+2);
	
	// Write string length to second byte
	*(pc+1) = s;
	
	// Add delimiter character
	//pc = (char *)(bdos_param_de + s);
	// *pc = bdos_delimiter;
	
	__asm
		ret
	__endasm;
	
}

void bdos_c_stat() __naked {	// BDOS_FUNC_C_STAT:	// 11: Get console status
	// Returns A=0 if no characters are waiting, nonzero if a character is waiting.
	//bdos_return1(bios_const());
	__asm
		call _bios_const
		; Result should be in L now
		ld a, l
		ret
	__endasm;
}
void bdos_s_bdosver() __naked {	// BDOS_FUNC_S_BDOSVER:	// 12: Return version number
	// Returns B=H=system type, A=L=version number.
	// type = 0=8080, 1=8086, 2=68000/Z8000; bit 0 = MP/M, bit 1 = CP/Net, bit 2 = 16-Bit multi-user
	// Version: Hex representation of major and minor number, e.g. 0x22 = v2.2
	
	//bdos_return2(0, 0x22);
	__asm
		ld a, #0
		ld b, a
		ld h, a
		
		ld a, #0x22
		ld l, a
		ret
	__endasm;
}

void bdos_drv_allreset() __naked {	// BDOS_FUNC_DRV_ALLRESET:	// 13: Reset disk system
	//@TODO: Implement
	// Close disks, reset DMA, forget directory
	return;
}

void bdos_drv_set(byte e) __naked {	// BDOS_FUNC_DRV_SET:	// 14: Select disk
	(void)e;	// Silence the  compiler about unused argument
	// cpm_drive = bdos_param_e;	// 0 = A, 1 = B
	
	//@TODO: If disk not available: return A=0xff, H=1
	
	//bios_seldsk(bdos_param_e);
	//bdos_return1(0);	// 0 = OK, 0xff = error
	/*
	// Log which disk
	__asm
		push de
		call _bdos_printf_x2
		pop de
	__endasm;
	*/
	
	__asm
		push de
		call _bios_seldsk
		pop de
		
		ld a, #0	; 0 = OK, 0xff = error
		ld l, a
	
		ret
	__endasm;
}

byte bdos_f_open_(struct FCB *fcb) {
	byte r;
	
	#ifdef BDOS_USE_HOST
		// Send request to host
		host_sendfcb(BDOS_FUNC_F_OPEN, fcb);
		
		//@TODO: Receive to different location (fcb_result) inside DMA area and return its 32-byte index (0...3)
		r = host_receivefcb(fcb);
		
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
			
			// Mirror to DMA and return its 32-byte offset
			//bdos_memcpy((byte *)0x0080, (byte *)fcb, 32);
			return 0;
		}
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_open n/a");
		(void)fcb;
	#endif
	return 0xff;	// Error
}
byte bdos_f_open(struct FCB *fcb) __naked {	// BDOS_FUNC_F_OPEN:	// 15: Open file
	(void)fcb;
	
	// DE = FCB address
	// A = Result (0xff = error, 0-3 = OK)
	// Some versions (including CP/M 3) always return zero; others return 0-3 to indicate that an image of the directory entry is to be found at (80h+20h*A).
	__asm
		push de	; Push argument DE = FCB address
		call _bdos_f_open_
		pop de	; Pop argument DE back
		; Result is in L
		
		; Return B=H=0
		ld a, #0
		ld b, a
		ld h, a
		
		; Return A=L=status (0xff=error, 0-3=OK)
		ld a, l
		ret
	__endasm;
}

byte bdos_f_close_(struct FCB *fcb) {
	byte r;
	
	#ifdef BDOS_USE_HOST
		host_sendfcb(BDOS_FUNC_F_CLOSE, fcb);
		r = 0x00;
		
		// if (r != 0xff) {
			fcb->s2 &= 0x7f;	// remove "open" flag?
		// }
	#else
		//@TODO: Implement bare-metal version
		(void)fcb;	// Quiet the compiler
		bdos_puts("bdos_f_close n/a!");
		r = 0xff;
	#endif
	
	return r;
}
byte bdos_f_close(struct FCB *fcb) __naked {	// BDOS_FUNC_F_CLOSE:	// 16: Close file
	(void)fcb;
	
	// DE = FCB address
	// A = Result (0xff = error, 0-3 = OK)
	// This function closes a file, and writes any pending data. This function should always be used when a file has been written to.
	// Some versions (including CP/M 3) always return zero; others return 0-3 to indicate that an image of the directory entry is to be found at (80h+20h*A).
	__asm
		push de	; Push argument DE = FCB address
		call _bdos_f_close_
		pop de	; Pop argument DE back
		; Result is in L
		
		; Return B=H=0
		ld a, #0
		ld b, a
		ld h, a
		
		; Return A=L=status (0xff=error, 0-3=OK)
		ld a, l
		ret
	__endasm;
}


byte bdos_f_sfirst_(struct FCB *fcb) {
	byte r;
	struct FCB *fcb_result;
	
	//bdos_puts("bdos_f_sfirst_...");
	#ifdef BDOS_USE_HOST
		// Send given FCB
		host_sendfcb(BDOS_FUNC_F_SFIRST, fcb);
		
		// Chose free FCB in DMA area
		bdos_fcb_num = (bdos_fcb_num + 1) & 0x03;	// 0...3
		fcb_result = (struct FCB *)((word)bios_dma + bdos_fcb_num * 32);
		
		// Receive into this area
		r = host_receivefcb(fcb_result);
		
		// Result 0xff means error
		if (r == 0xff) return 0xff;
	
		// Return result fcb number (32-byte offset in DMA area, 0-3)
		return bdos_fcb_num;
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_sfirst n/a!");
		(void)fcb;	// Quiet the compiler
		return 0xff;
	#endif
}
byte bdos_f_sfirst(struct FCB *fcb) __naked {	// BDOS_FUNC_F_SFIRST:	// 17: Search for first
	(void)fcb;	// Silence the  compiler about unused argument
	// DE=address of FCB. Returns error codes in BA and HL.
	// Search for the first occurrence of the specified file; the filename should be stored in the supplied FCB.
	// The filename can include ? marks
	//s = bdos_f_sfirst((struct FCB *)bdos_param_de);
	////bdos_return1(s);
	//bdos_return2(s, 0);
	__asm
		push de	; Push argument DE = FCB address
		call _bdos_f_sfirst_
		pop de	; Pop argument DE back
		; Result is in L
		
		; Return B=H=0
		ld a, #0
		ld b, a
		ld h, a
		
		; Return A=L=status (0xff=error, 0-3=OK=result 32-byte offset in DMA area)
		ld a, l
		ret
	__endasm;
}

byte bdos_f_snext_(struct FCB *fcb) {
	byte r;
	struct FCB *fcb_result;
	
	#ifdef BDOS_USE_HOST
		// Send given FCB
		host_sendfcb(BDOS_FUNC_F_SNEXT, fcb);
		
		// Chose free FCB in DMA area
		bdos_fcb_num = (bdos_fcb_num + 1) & 0x03;	// 0...3
		fcb_result = (struct FCB *)((word)bios_dma + bdos_fcb_num * 32);
		
		// Receive into this area
		r = host_receivefcb(fcb_result);
		
		// Result 0xff means error
		if (r == 0xff) return 0xff;
	
		// Return result fcb number (32-byte offset in DMA area, 0-3)
		return bdos_fcb_num;
		
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_snext n/a!");
		(void)fcb;	// Quiet the compiler
		return 0xff;
	#endif
}
byte bdos_f_snext(struct FCB *fcb) __naked {	// BDOS_FUNC_F_SNEXT:	// 18: Search for next
	(void)fcb;	// Silence the  compiler about unused argument
	// DE=(address of FCB)? Returns error codes in BA and HL.
	//s = bdos_f_snext((struct FCB *)bdos_param_de);
	//bdos_return1(s);
	__asm
		push de	; Push argument DE = FCB address
		call _bdos_f_snext_
		pop de	; Pop argument DE back
		; Result is in L
		
		; Return B=H=0
		ld a, #0
		ld b, a
		ld h, a
		
		; Return A=L=status (0xff=error, 0-3=OK=result 32-byte offset in DMA area)
		ld a, l
		ret
	__endasm;
}

byte bdos_f_delete(struct FCB *fcb) __naked {	// BDOS_FUNC_F_DELETE:	// 19: Delete file
	(void)fcb;	// Silence the  compiler about unused argument
	// DE=address of FCB. Returns error codes in BA and HL.
	// Returns A=0FFh if error, otherwise 0-3
	
	//@TODO: Implement!
	__asm
		ld l, #0xff
		ret
	__endasm;
}


byte bdos_f_read_(struct FCB *fcb) {
	
	#ifdef BDOS_USE_HOST
		word l;
		word rn;
		word ex;
	
		host_sendfcb(BDOS_FUNC_F_READ, fcb);
		//host_sendfcb(bdos_param_c, fcb);
		
		// Receive data
		l = host_receivedma();
		
		// Dump DMA area
		//bdos_printf("L="); bdos_printf_x4(l);
		//bdos_dump((word)0x0080, 4);	bdos_dump((word)0x0100-4, 4);
		//bdos_getchar();
		
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
		
		rn = (word)fcb->cr + ((word)fcb->ex * 128) + ((word)(fcb->s2 & 1) * 16384);
		rn++;
		ex = rn >> 7;	//	/ 128;
		fcb->cr = rn % 128;
		fcb->ex = ex % 32;
		fcb->s2 = (0x80 | (ex >> 5));
		
		//bdos_puts("readOK");
		return 0x00;
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_read n/a!");
		(void)fcb;
		return 1;	// 1 = EOF
	#endif
}
byte bdos_f_read(struct FCB *fcb) __naked {	// BDOS_FUNC_F_READ:	// 20: Read sequential
	(void)fcb;	// Silence the  compiler about unused argument
	// DE=address of FCB. Returns error codes in BA and HL.; H = number of 128-byte records read before error
	//	0	OK,
	//	1	end of file,
	//	9	invalid FCB,
	//	10	(CP/M) media changed; (MP/M) FCB checksum error,
	//	11	(MP/M) unlocked file verification error,
	//	0FFh	hardware error.
	//s = bdos_f_read((struct FCB *)bdos_param_de);
	//bdos_return2(s, 0);
	__asm
		push de	; Push argument DE = FCB address
		call _bdos_f_read_
		pop de	; Pop argument DE back
		; Result is in L
		
		; Return A=L=status (0xff=error, 0-3=OK)
		ld a, l
		ret
	__endasm;
}

byte bdos_f_write_(struct FCB *fcb) {
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
byte bdos_f_write(struct FCB *fcb) __naked {	// BDOS_FUNC_F_WRITE:	// 21: Write sequential
	(void)fcb;	// Silence the  compiler about unused argument
	// DE=address of FCB. Returns error codes in BA and HL.
	//	0	OK,
	//	1	directory full,
	//	2	disc full,
	//	8	(MP/M) record locked by another process,
	//	9	invalid FCB,
	//	10	(CP/M) media changed; (MP/M) FCB checksum error,
	//	11	(MP/M) unlocked file verification error,
	//	0FFh	hardware error.
	//bdos_return1(bdos_f_write((struct FCB *)bdos_param_de));
	__asm
		push de	; Push argument DE = FCB address
		call _bdos_f_write_
		pop de	; Pop argument DE back
		; Result is in L
		
		; Return B=H=0
		;ld a, #0
		;ld b, a
		;ld h, a
		
		; Return A=L=status (0xff=error, 0-3=OK)
		ld a, l
		ret
	__endasm;
}

byte bdos_f_make(struct FCB *fcb) __naked {	// BDOS_FUNC_F_MAKE:	// 22: Make file
	(void)fcb;	// Silence the  compiler about unused argument
	// DE=address of FCB. Returns error codes in BA and HL.
	
	//@TODO: Implement!
	bdos_puts("bdos_f_make n/a!");
	__asm
		ld l, #0xff	; 0xff if directory is full
		ret
	__endasm;
}

byte bdos_f_rename(struct FCB *fcb) __naked {	// BDOS_FUNC_F_RENAME:	// 23: Rename file
	(void)fcb;	// Silence the  compiler about unused argument
	// DE=address of FCB. Returns error codes in BA and HL.
	
	bdos_puts("bdos_f_rename n/a!");
	
	//@TODO: Implement!
	__asm
		ld l, #0xff	; // Returns A=0-3 if successful; A=0FFh if error. Under CP/M 3, if H is zero then the file could not be found
		ret
	__endasm;
}

byte bdos_drv_loginvec() __naked {	// BDOS_FUNC_DRV_LOGINVEC:	// 24: Return login vector
	// Bit 7 of H corresponds to P: while bit 0 of L corresponds to A:. A bit is set if the corresponding drive is logged in.
	//bdos_return2(0x00, 0x01);	// 0x00 0x01 = Only drive A
	
	//@TODO: Create 16-bit map: L0=A: ... H7=P:
	__asm
		ld a, #0x00
		ld b, a
		ld h, a
		
		ld a, #0x0f
		ld l, a
		ret
	__endasm;
}

void bdos_drv_get() __naked {	// BDOS_FUNC_DRV_GET:	// 25: Return current disk
	// Entered with C=19h. Returns drive in A. Returns currently selected drive. 0 => A:, 1 => B: etc.
	//bdos_return1(bios_curdsk);
	__asm
		ld a, (_bios_curdsk)
		ret
	__endasm;
}

void bdos_f_dmaoff(char *de) __naked {	// BDOS_FUNC_F_DMAOFF:	// 26: Set DMA address
	(void)de;	// Silence the  compiler about unused argument
	// Entered with C=1Ah, DE=address.
	// Set the Direct Memory Access address; a pointer to where CP/M should read or write data.
	// Initially used for the transfer of 128-byte records between memory and disc, but over the years has gained many more functions.
	//bios_setdma((byte *)bdos_param_de);
	__asm
		push de
		call _bios_setdma
		pop de
		ret
	__endasm;
}

void bdos_drv_allocvec() __naked {	// BDOS_FUNC_DRV_ALLOCVEC:	// 27: Get addr (alloc)
	// BDOS function 27 (DRV_ALLOCVEC) - Return address of allocation map
	// Supported by: All versions, but differs in banked versions.
	// Entered with C=1Bh. Returns address in HL (16-bit versions use ES:BX).
	// Return the address of the allocation bitmap (which blocks are used and which are free) in HL. Under banked CP/M 3 and MP/M, this will be an address in bank 0 (the system bank) and not easily accessible.
	// Under previous versions, the format of the bitmap is a sequence of bytes, with bit 7 of the byte representing the lowest-numbered block on disc, and counting starting at block 0 (the directory). A bit is set if the corresponding block is in use.
	// Under CP/M 3, the allocation vector may be of this form (single-bit) or allocate two bits to each block (double-bit). This information is stored in the SCB.
	
	//@TODO: Implement!
	bdos_puts("bdos_drv_allocvec n/a!");
	return;
}

// 28: Write protect disk
// 29: Get R/O vector
// 30: Set file attributes
// 31: Get addr (disk parms)

byte bdos_f_usernum() __naked {	// BDOS_FUNC_F_USERNUM:	// 32: Set/Get user code
	// BDOS function 32 (F_USERNUM) - get/set user number
	// Supported by: CP/M 2 and later.
	// Entered with C=20h, E=number. If E=0FFh, returns number in A.
	// Set current user number. E should be 0-15, or 255 to retrieve the current user number into A.
	// Some versions can use user areas 16-31, but these should be avoided for compatibility reasons.
	// DOS+ returns the number set in A.
	
	//@FIXME: User is usually stored in the upper bits of bios_curdsk [0x0004]
	__asm
		; e == 0xff?
		ld a, e
		cp a, #0xff
		jr z, 0$
		; Set user (and return it)
		ld (_bdos_user), a
		
	0$:
		; Return user
		ld a, (_bdos_user)
		ld l, a
		ret
	__endasm;
}

byte bdos_f_readrand_(struct FCB *fcb) {
	//byte r;
	//word rn;
	//word ex;
	//word l;
	
	//bdos_printf("readrand"); bdos_printf_x2(fcb->r2); bdos_printf_x2(fcb->r1); bdos_printf_x2(fcb->r0); bdos_getchar();
	
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
	/*
	rn = (word)fcb->r0 + ((word)fcb->r1 * 256);	// + r2 * 65536
	ex = rn >> 7;	// / 128;
	fcb->cr = rn % 128;
	fcb->ex = ex % 32;
	fcb->s2 = (0x80 | (ex / 32));
	*/
	fcb->cr = fcb->r0 & 0x7f;	// bits 0...6 of r0
	fcb->ex = ((fcb->r0 & 0x80) >> 7) | ((fcb->r1 & 0x0f) << 1);	// bit 7 of r0 + bits 0...3 of r1
	fcb->s2 = 0x80 | ((fcb->r1 >> 4) & 0x0f);	// bits 4...7 of r1
	
	/*
	// Proceed with sequencial read
	r = bdos_f_read_(fcb);
	return r;
	*/
	
	#ifdef BDOS_USE_HOST
		word l;
		word rn;
		word ex;
	
		//host_sendfcb(bdos_param_c, fcb);
		//host_sendfcb(BDOS_FUNC_F_READ, fcb);
		host_sendfcb(BDOS_FUNC_F_READRAND, fcb);
		
		// Receive data
		l = host_receivedma();
		
		// Return 1 on EOF
		if (l == 0) {
			return 1;	// 1 = EOF
		}
		
		if (l < 128) {
			// Fill with EOFs
			bdos_memset(bios_dma + l, 0x1a, 128 - l);
		}
		
		rn = (word)fcb->cr + ((word)fcb->ex * 128) + ((word)(fcb->s2 & 1) * 16384);
		rn++;
		ex = rn >> 7;	//	/ 128;
		fcb->cr = rn % 128;
		fcb->ex = ex % 32;
		fcb->s2 = (0x80 | (ex / 32));
		
		//bdos_puts("readOK");
		return 0x00;
	#else
		//@TODO: Implement bare-metal version
		bdos_puts("bdos_f_readrand n/a!");
		(void)fcb;
		return 6;	// 1 = EOF
	#endif
	
}
byte bdos_f_readrand(struct FCB *fcb) __naked {	// BDOS_FUNC_F_READRAND:	// 33: Read random
	(void)fcb;
	
	//s = bdos_f_readrand((struct FCB *)bdos_param_de);
	//bdos_return2(s, 0);
	__asm
		push de	; Push argument DE = FCB address
		call _bdos_f_readrand_
		pop de	; Pop argument DE back
	
		; Result is in L
		
		; Return B=H=0
		ld a, #0
		ld b, a
		ld h, a
		
		; Return A=L=status (0xff=error, 0-3=OK)
		ld a, l
		ret
	__endasm;
}


byte bdos_f_writerand_(struct FCB *fcb) {
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
byte bdos_f_writerand(struct FCB *fcb) __naked {	// BDOS_FUNC_F_WRITERAND:	// 34: Write random
	(void)fcb;
	
	//s = bdos_f_writerand((struct FCB *)bdos_param_de);
	//bdos_return2(s, 0);
	__asm
		push de	; Push argument DE = FCB address
		call _bdos_f_writerand_
		pop de	; Pop argument DE back
		; Result is in L
		
		; Return B=H=0
		;ld a, #0
		;ld b, a
		;ld h, a
		
		; Return A=L=status (0xff=error, 0-3=OK)
		ld a, l
		ret
	__endasm;
}

void bdos_f_size(struct FCB *fcb) __naked {	// BDOS_FUNC_F_SIZE:	// 35: Compute file size
	(void)fcb;	// Silence the  compiler about unused argument
	
	bdos_puts("bdos_f_size n/a!");
	//@TODO: Return "0", but update FCB's R0, R1, R2 records:
	// s.st_size >>= 7;
	// FCB_R0(fcb) = s.st_size & 0xff;
	// s.st_size >>= 8;
	// FCB_R1(fcb) = s.st_size & 0xff;
	// s.st_size >>= 8;
	// FCB_R2(fcb) = s.st_size & 1;
	return;
}

void bdos_f_randrec(struct FCB *fcb) __naked {	// BDOS_FUNC_F_RANDREC:	// 36: Set random record
	(void)fcb;	// Silence the  compiler about unused argument
	// Update FB.R0/R1/R2 to reflect current pos
	
	bdos_puts("bdos_f_randrec n/a!");
	
	// fcb->r0 = (bdos_file_ofs >> 7) & 0xff;
	// fcb->r1 = (bdos_file_ofs >> 7) >> 8;
	// fcb->r2 = (bdos_file_ofs >> 7) >> 16;
	// fcb->cr = SEQ_CR(bdos_file_ofs);
	// fcb->ex = SEQ_EX(bdos_file_ofs);
	// fcb->s2 = SEQ_S2(bdos_file_ofs);
	return;
}

void bdos_drv_reset() __naked {	// BDOS_FUNC_DRV_RESET:	// 37: Reset drive
	//bdos_puts("DRV_RST");
	bdos_puts("bdos_drv_reset n/a!");
}

// 38: Undefined - go back

// 39: Undefined - go back

byte bdos_f_writezf_(struct FCB *fcb) {
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

byte bdos_f_writezf(struct FCB *fcb) __naked {	// BDOS_FUNC_F_WRITEZF:	// 40: Fill random file w/ zeros
	(void)fcb;	// Silence the  compiler about unused argument
	//bdos_return1(bdos_f_writezf((FCB *)bdos_param_de));
	__asm
		push de	; Push argument DE = FCB address
		call _bdos_f_writezf_
		pop de	; Pop argument DE back
		; Result is in L
		
		; Return A=L=status (0xff=error, 0-3=OK)
		ld a, l
		ret
	__endasm;
}


// Catch-all for unimplemented functions
void bdos_unimplemented() __naked {
	//@TODO: Also show function number (register C)
	bdos_puts("UNIMPL!");
	__asm
		; Return A=L=0xff
		ld a, #0xff
		ld l, #0xff
		ret
	__endasm;
}

// BDOS function jump table
void bdos_funcs() __naked {
	__asm
		jp _bdos_init	; BDOS_FUNC_P_TERMCPM:	// 0: System reset
		jp _bdos_c_read	; BDOS_FUNC_C_READ:	// 1: Console input
		jp _bdos_c_write	; BDOS_FUNC_C_WRITE:	// 2: Console output
		jp _bdos_a_read	; BDOS_FUNC_A_READ:	// 3: Reader input
		jp _bdos_a_write	; BDOS_FUNC_A_WRITE:	// 4: Punch output
		jp _bdos_l_write	; BDOS_FUNC_L_WRITE:	// 5: List output
		jp _bdos_c_rawio	; BDOS_FUNC_C_RAWIO:	// 6: Direct console I/O
		jp _bdos_get_iobyte	; BDOS_FUNC_GET_IOBYTE:	// 7: Get I/O Byte
		jp _bdos_set_iobyte	; BDOS_FUNC_SET_IOBYTE:	// 8: Set I/O Byte
		jp _bdos_c_writestr	; BDOS_FUNC_C_WRITESTR:	// 9: Print string (until delimiter "$")
		jp _bdos_c_readstr	; BDOS_FUNC_C_READSTR:	// 10: Read console buffer
		jp _bdos_c_stat	; BDOS_FUNC_C_STAT:	// 11: Get console status
		jp _bdos_s_bdosver	; BDOS_FUNC_S_BDOSVER:	// 12: Return version number
		
		jp _bdos_drv_allreset	; BDOS_FUNC_DRV_ALLRESET:	// 13: Reset disk system
		jp _bdos_drv_set	; BDOS_FUNC_DRV_SET:	// 14: Select disk
	
		jp _bdos_f_open	; BDOS_FUNC_F_OPEN:	// 15: Open file
		jp _bdos_f_close	; BDOS_FUNC_F_CLOSE:	// 16: Close file
		jp _bdos_f_sfirst	; BDOS_FUNC_F_SFIRST:	// 17: Search for first
		jp _bdos_f_snext	; BDOS_FUNC_F_SNEXT:	// 18: Search for next
		jp _bdos_f_delete	; BDOS_FUNC_F_DELETE:	// 19: Delete file
		jp _bdos_f_read	; BDOS_FUNC_F_READ:	// 20: Read sequential
		jp _bdos_f_write	; BDOS_FUNC_F_WRITE:	// 21: Write sequential
		jp _bdos_f_make	; BDOS_FUNC_F_MAKE:	// 22: Make file
		jp _bdos_f_rename	; BDOS_FUNC_F_RENAME:	// 23: Rename file
		
		jp _bdos_drv_loginvec	; BDOS_FUNC_DRV_LOGINVEC:	// 24: Return login vector
		jp _bdos_drv_get	; BDOS_FUNC_DRV_GET:	// 25: Return current disk
		jp _bdos_f_dmaoff	; BDOS_FUNC_F_DMAOFF:	// 26: Set DMA address
		jp _bdos_drv_allocvec	; BDOS_FUNC_DRV_ALLOCVEC:	// 27: Get addr (alloc)
		jp _bdos_unimplemented	; // 28: Write protect disk
		jp _bdos_unimplemented	; // 29: Get R/O vector
		jp _bdos_unimplemented	; // 30: Set file attributes
		jp _bdos_unimplemented	; // 31: Get addr (disk parms)
		jp _bdos_f_usernum	; BDOS_FUNC_F_USERNUM:	// 32: Set/Get user code
		jp _bdos_f_readrand	; BDOS_FUNC_F_READRAND:	// 33: Read random
		jp _bdos_f_writerand	; BDOS_FUNC_F_WRITERAND:	// 34: Write random
		jp _bdos_f_size	; BDOS_FUNC_F_SIZE:	// 35: Compute file size
		jp _bdos_f_randrec	; BDOS_FUNC_F_RANDREC:	// 36: Set random record
		jp _bdos_drv_reset	; BDOS_FUNC_DRV_RESET:	// 37: Reset drive
		jp _bdos_unimplemented	; // 38: Undefined - go back
		jp _bdos_unimplemented	; // 39: Undefined - go back
		jp _bdos_f_writezf	; BDOS_FUNC_F_WRITEZF:	// 40: Fill random file w/ zeros
		
		; Pad with "unimplemented" vectors
		jp _bdos_unimplemented
		jp _bdos_unimplemented
	__endasm;
}
//#endif	// __BDOS_C