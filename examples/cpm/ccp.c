#ifndef __CCP_C
#define __CCP_C

/*
	CCP - Console Command Processor
	===============================
	
	This is the "shell" of CP/M.
	It is loaded as a standard program into the transient area.
*/

/*
Command (Internal)	Function
DIR <filespec>
	Produces a list of files matching the <filespec>, if no filespec is entered, the result is the same as *.*, i.e., all files on the currently logged disk. The command accepts similar wildcards as DOS.
	Note, CP/M 2.2 has no concept of directories, all files are in the "root" area of the disk. Other target disks can be selected, e.g., DIR C:*.* will list all files on drive C:
	System files are not shown by the DIR command. (See STAT).
ERA <filespec>
	Deletes the file(s) specified by <filespec>. Wildcards are allowed.
REN <new> = <old>
	Renames the <old> filespec to the <new>. Wildcards are allowed.
SAVE <pages> <file>
	Saves a copy of memory to file. <pages> is the number of 256 byte pages starting at address 100(h) to save and <file> is the filename to save the data to.
TYPE <filename>
	Copies the named text file to the console (screen)
USER <area>
	The system has "user areas" numbered from 0 to 15. On start-up, the User area is set to "0". The DIR command displays files in the current user area and only programs in the current user area can be executed. The USER command is used to select another User area.
*/

// Entry point
void main() __naked {
	__asm
		jp _ccp
		;.asciz '[<- CCP entry]'	; Marker in binary to see if this is actually the first byte, jumping to CCP
	__endasm;
}

#include <basictypes.h>

//#define PROGRAM_GETS_LOCAL_ECHO	// Force local echo (even though getchar() and gets() output the typed character as per spec.!)

#include "program.c"

#include "ccp.h"

#include "fcb.h"

//#include <string.h>	// For strcmp
#include <stringmin.h>	// For memcpy

#include <strcmpmin.h>	// For strcmp


byte fcb_isspace(byte b) {
	if (b <= 0x20) return 1;
	return 0;
}

/*
char *fcb2name(struct FCB *fcb) {
	char temp[16];
	char *p;
	byte i;
	
	p = temp;
	
	// Prepend drive
	i = fcb->dr;
	if (i == 0) i = bios_curdsk;
	*p++ = ('A' + i);
	*p++ = '/';
	
	// Filename base
	for(i = 0; i < 8; i++) {
		if (!fcb_isspace(fcb->f[i]))
			*p++ = (char)fcb->f[i];
	}
	
	// Extension/type
	*p++ = '.';
	for(i = 0; i < 3; i++) {
		if (!fcb_isspace(fcb->t[i]))
			*p++ = (char)fcb->t[i];
	}
	
	// Terminate
	*p-- = 0;
	if (*p == '.')
		*p = 0;
	
	return &temp[0];
}
*/


#ifdef CCP_CMD_DUMP
	#define HEX_USE_DUMP	// Force hex.h to include dump()
	#define HEX_DUMP_WIDTH 16
	#define HEX_DUMP_INTRA_HEX " "
	#define HEX_DUMP_EOL "\r\n"
	
	//	void dump(byte *a) {
	//		word ai;
	//		byte i;
	//		byte j;
	//		
	//		ai = (word)a;
	//		printf_x4(ai);
	//		printf(":\n");
	//		
	//		for (i = 0; i < 4; i++) {
	//			printf_x2(ai & 0xff);
	//			//printf_x4(a);
	//			putchar('|');
	//			
	//			for (j = 0; j < 8; j++) {
	//				printf_x2(*a++);
	//				//putchar(' ');
	//				ai++;
	//			}
	//			printf("\n");
	//		}
	//	}
#endif
#include <hex.h>	// for printf_x2, printf_x4, ...

void ccp_print_error(byte a) {
	printf("ERR 0x");
	printf_x2(a);
	printf("\r\n");
}
void ccp_print_ok() {
	printf("OK\r\n");
}


#ifdef CCP_CMD_PORT
byte port_in(byte p) {
	(void)p;
	
	__asm
		;push af
		
		; Get parameter from stack into a
		ld hl,#0x0002
		add hl,sp
		ld c,(hl)
		
		in	a, (c)
		ld	l, a
		
		;pop af
		ret
	__endasm;
	return 0;
}

void port_out(byte p, byte v)  {
	(void)p;	// suppress warning "unreferenced function argument"
	(void)v;	// suppress warning "unreferenced function argument"
	__asm
		;push hl
		;push af
		
		; Get parameter v from stack into a
		ld hl,#0x0002
		add hl,sp
		ld a,(hl)
		
		; Get parameter p from stack into c
		ld hl,#0x0004
		add hl,sp
		ld c,(hl)
		
		out	(c), a
		
		;pop af
		;pop hl
		;ret
	__endasm;
}
#endif


#ifdef CCP_USE_HOST
	// Host communication
	
	//void host_send(const byte *data, byte ldata);
	//int host_receive(byte *data);
	
	#ifdef BDOS_USE_HOST_SOFTSERIAL
		#include "host_softserial.h"
	#endif
	
	#ifdef BDOS_USE_HOST_MAME
		#include "host_mame.h"
	#endif
	
	void host_load() {
		byte data[MAX_DATA];
		int l;
		word o;
		word a;
		
		l = MAX_DATA;
		a = 0x0100;
		o = 0x0000;
		while(l > 0) {
			data[0] = 'L';
			data[1] = (o >> 8);
			data[2] = (o     ) & 0xff;
			
			l = -1;
			while(l < 0) {
				host_send(data, 1 + 2);
				l = host_receive(data);
			}
			
			printf_x2(l);
			printf(" bytes @ ");
			printf_x4(a);
			printf("\n");
			if (l > 0) {
				memcpy(byte *)a, (&data[0], l);
				a += l;
				o += l;
			}
		}
		printf("done.\n");
		
	}
#endif


void ccp_run() __naked {
	// Try running a program
	//printf("Running...");
	__asm
		jp 0x0100
		;call 0x0100
	__endasm;
	
	// Will not exit, but rather invoke BIOS warm boot
	ccp_print_ok();
}

volatile byte ccp_ret_a;
volatile byte ccp_reg_e;

void ccp_dir() {
	byte i;
	char c;
	
	//def_fcb.dr = bios_curdsk;	// current drive or '?' for meta info (disc labels, date stamps)
	def_fcb.dr = '?';	// '?' for meta info (disc labels, date stamps)
	
	def_fcb.f[0] = '?';
	def_fcb.f[1] = '?';
	def_fcb.f[2] = '?';
	def_fcb.f[3] = '?';
	def_fcb.f[4] = '?';
	def_fcb.f[5] = '?';
	def_fcb.f[6] = '?';
	def_fcb.f[7] = '?';
	
	def_fcb.t[0] = '?';
	def_fcb.t[1] = '?';
	def_fcb.t[2] = '?';
	//memset(&def_fcb.f[0], 0x20, 8 + 3);	// Blank filename
	//def_fcb.f = "        ";
	//def_fcb.t = "   ";
	
	def_fcb.ex = 0;	// default
	//def_fcb.ex = '?';	// ? = All suitable extents
	
	__asm
		push af
		push bc
		push de
		push hl
		ld c, #17	; 17 = BDOS_FUNC_F_SFIRST
		ld d, #0x00	; Address of FCB
		ld e, #0x5c	; Address of FCB
		call 5
		
		ld (_ccp_ret_a), a
		
		pop hl
		pop de
		pop bc
		pop af
	__endasm;
	
	struct FCB *dir_fcb;
	while (ccp_ret_a != 0xff) {
		
		// Directory entry is now at memory DMA + a*32
		//dump((word)&ccp_dma + ccp_ret_a * 32, 48);
		dir_fcb = (struct FCB *)((word)&ccp_dma + ccp_ret_a * 32);
		
		if (dir_fcb->dr & 0x20) {
			// Deleted file
		} else 
		if (dir_fcb->rc == 0x80) {
			// Big file, more entries will follow. We only show the last entry
			
		} else  {
			// Show filename
			
			for(i = 0; i < 8; i++) {
				c = dir_fcb->f[i];
				if (fcb_isspace(c)) break;	//putchar(' ');
				putchar(c);
			}
			putchar('.');
			for(i = 0; i < 3; i++) {
				c = dir_fcb->t[i];
				if (fcb_isspace(c)) break;	//putchar(' ');
				putchar(c);
			}
			//printf(fcb2name(&def_fcb));
			printf("\r\n");
		}
		
		__asm
			push af
			push bc
			push de
			push hl
			ld c, #18	; BDOS_FUNC_F_SNEXT = 18
			ld d, #0x00
			ld e, #0x5c
			call 5
			
			ld (_ccp_ret_a), a
			
			pop hl
			pop de
			pop bc
			pop af
		__endasm;
	}
	
}

byte ccp_fopen(char *filename) {
	byte i;
	char c;
	char *pc;
	
	//@FIXME: Extract drive from filename!
	//def_fcb.dr = bios_curdsk;	// Default drive (setting it to bios_curdsk will throw "invalid drive" in yaze)
	def_fcb.dr = 0;	// this works in YAZE
	
	pc = filename;
	for(i = 0; i < 8; i++) {
		c = *pc;
		if (c == '.') break;
		if (c == 0) break;
		def_fcb.f[i] = c;
		pc++;
	}
	while (i < 8) def_fcb.f[i++] = 0x20;
	pc++;
	
	for(i = 0; i < 3; i++) {
		c = *pc;
		if (c == 0) break;
		def_fcb.t[i] = c;
		pc++;
	}
	while (i < 3) def_fcb.t[i++] = 0x20;
	
	// Prep fcb.ex
	def_fcb.ex = 0;
	def_fcb.s1 = 0;
	def_fcb.s2 = 0;
	
	def_fcb.cr = 0;
	def_fcb.r0 = 0;
	def_fcb.r1 = 0;
	def_fcb.r2 = 0;
	
	//printf("Opening...\r\n");
	//dump((word)(&def_fcb), 36);
	
	__asm
		push af
		push bc
		push de
		push hl
		
		ld c, #15	; BDOS_FUNC_F_OPEN = 15
		ld d, #0x00
		ld e, #0x5c
		call 5
		
		ld (_ccp_ret_a), a
		
		pop hl
		pop de
		pop bc
		pop af
	__endasm;
	
	// FCB shoud be at DMA + A*32
	
	return ccp_ret_a;
}
byte ccp_fclose() {
	__asm
		push af
		push bc
		push de
		push hl
		ld c, #16	; BDOS_FUNC_F_CLOSE = 16
		ld d, #0x00
		ld e, #0x5c
		call 5
		
		ld (_ccp_ret_a), a
		
		pop hl
		pop de
		pop bc
		pop af
	__endasm;
	
	return ccp_ret_a;
}

byte ccp_load(char *filename) {
	byte r;
	byte *a;
	
	printf("Open \"");
	printf(filename);
	printf("\"...");
	
	r = ccp_fopen(filename);
	
	if (ccp_ret_a != 0x00) {
		ccp_print_error(ccp_ret_a);
		return ccp_ret_a;
	}
	
	ccp_print_ok();
	
	//@TODO: Load data
	printf("Load...");
	a = (byte *)0x0100;
	do {
		
		//printf(".");	// Progress
		
		__asm
			push af
			push bc
			push de
			push hl
			
			ld c, #20	; BDOS_FUNC_F_READ = 20
			ld d, #0x00
			ld e, #0x5c
			call 5
			
			ld (_ccp_ret_a), a
			
			pop hl
			pop de
			pop bc
			pop af
		__endasm;
		
		//printf_x2(ccp_ret_a);
		
		// 0 = OK, 1 = EOF, 9 = invalid FCB, 10 = media changed/checksum error, 11 = unlocked/verification error, 0xff = hardware error
		if (ccp_ret_a != 0)
			break;
		
		memcpy(a, ccp_dma, 128);
		a += 128;
		
	} while(ccp_ret_a == 0x00);
	
	ccp_print_ok();
	
	//@TODO: Close file
	ccp_fclose();
	
	//printf("Run...");
	ccp_run();
	
	return 0x00;
	
}


void handle(char *input) {
	byte input_l;
	byte cmd_l;
	char *c;
	
	// Copy "command tail" (i.e. args) to bdos_dma area at 0x0080-0x00ff
	c = &input[0];
	input_l = strlen(input);
	if (input_l > CCP_MAX_INPUT) return;
	if (input_l == 0) return;
	
	// Get args
	//@FIXME: Regular CCP stores length of arg string in ccp_args[0], then zero-terminated args including first space
	// e.g. "CCP ARGS1234" -> 0x09 " ARGS1234" 0x00
	ccp_args[0] = 0;
	while(*c != 0) {
		if (*c == 0x20) {
			// Copy arg string over
			//memcpy((byte *)&ccp_args[0], (byte *)c+1, input_l);
			memcpy((byte *)&ccp_args[0], (byte *)c, input_l);	// Include the space!
			break;
		}
		c++;
	}
	ccp_argl = strlen(ccp_args);
	cmd_l = (word)c - (word)&input[0];	// Length of command part
	
	// Actually handle commands
	//printf("args=["); printf(&ccp_args[0]); printf("]\r\n");
	
	// Empty command
	if (strcmp(input, "") == 0) {
		// Ignore empty entries
		return;
	} else
	
	// Change drive (e.g. "A:")
	if ((input_l == 2) && (input[1] == ':')) {
		ccp_reg_e = input[0] - 'A';
		
		// Upper case
		if (ccp_reg_e >= 32) ccp_reg_e -= 32;
		if (ccp_reg_e > 25) {
			printf("INV\r\n");
			return;
		}
		
		//printf("Changing to drive ");
		//putchar(ccp_reg_e + 'A');
		//printf(":...\r\n");
		
		__asm
			push af
			push bc
			push de
			push hl
			ld c, #14	; BDOS_FUNC_DRV_SET = 14
			ld a, (_ccp_reg_e)
			ld e, a
			call 5
			
			ld (_ccp_ret_a), a
			
			pop hl
			pop de
			pop bc
			pop af
		__endasm;
		
		/*
		if (ccp_ret_a != 0) {
			ccp_print_error(ccp_ret_a);
		}
		*/
		// YAZE does not change bios_curdsk after that, so let's do it manually...
		bios_curdsk = ccp_reg_e;
		
		
	} else
	
	// DIR
	if (strncmp(input, "DIR",3) == 0) {
		//@TODO: Handle args
		ccp_dir();
		
	} else
	
	/*
	volatile byte globalA;
	if (strcmp(input, "TEST") == 0) {
		// Do a simple R/W test
		word a;
		byte b;
		a = 0x0300;
		b = 111;
		printf_d("Writing: ", b);
		*((byte *)a) = b;
		b = 0;
		b = *((byte *)a);
		printf_d("Read: ", b);
		
		__asm
			ld a, #0x55
			out(0x13), a
		__endasm;
		
		__asm
			in a, (0x13)
			ld (_globalA), a
		__endasm;
		printf_d("ReadP: ", globalA);
		
	} else
	
	if (strcmp(input, "BDOS") == 0) {
		// Test invoking BDOS functions with "call 5"
		//printf("CALL %d: ", test);
		__asm
			push af
			
			;ld	de, parameter
			;ld c, function
			
			ld c, #0x01	; 1 = C_READ
			
			;ld c, #0x02	; 2 = C_WRITE
			;ld e, #0x40
			
			
			call 5
			
			; returns A and BA, later also L/HL
			ld a, l
			ld (_test), a
			
			pop af
		__endasm;
		printf("L=%d\n", test);
		
	} else
	
	if (strcmp(input, "DI") == 0) {
		__asm
			di
		__endasm;
	} else
	if (strcmp(input, "EI") == 0) {
		__asm
			ei
		__endasm;
	} else
	*/
	
	#ifdef BIOS_USE_HOST
	if (strcmp(input, "HOSTLOAD") == 0) {
		
		vgl_lcd_scroll_cb = 0;	// No beeping
		host_load();
		vgl_lcd_scroll_cb = &myscroll;	// Restore scroll callback
	} else
	/*
	if (strcmp(input, "SP") == 0) {
		serial_sendSafe("P", 1);
		c = serial_receiveSafe((byte *)&arg[0]);
		arg[c] = 0;	// Zero-terminate
		printf("Received: \"");
		printf(arg);
		printf("\"");
	} else
	if (strcmp(input, "ST") == 0) {
		serial_sendSafe("Hello", 5);
	} else
	if (strcmp(input, "SX") == 0) {
		while(1) {
			serial_puts("ABCDEF");
			printf(".");
			
		}
	} else
	if (strcmp(input, "SR") == 0) {
		c = serial_receiveSafe(&arg[0]);
		printf("Okay, received ");
		printf_x2(c);
		printf(" bytes.\n");
	} else
	if (strcmp(input, "SERIALGET") == 0) {
		while(1) {
			c = serial_getchar();
			if (c >= 0) putchar((char)c);
			//else printf_x4((word)c);
		}
	} else
	if (strcmp(input, "SERIALGETS") == 0) {
		while(1) {
			serial_gets(arg);
			printf(arg);
		}
	} else
	if (strcmp(input, "SERIALPUTS") == 0) {
		serial_puts(arg);
	} else
	*/
	#endif
	
	#ifdef CCP_INCLUDE_TEST_PROGRAM
	if (strcmp(input, "LOAD") == 0) {
		// Try running a program
		printf("Loading ");
		printf(CCP_TEST_PROGRAM_FILENAME);
		printf("...");
		mem_copy(&CCP_TEST_PROGRAM_DATANAME[0], (byte *)0x0100, CCP_TEST_PROGRAM_SIZENAME);
		printf("OK\n");
		
		//printf("Running...");
		//__asm
		//	jp 0x0100
		//__endasm;
		// Will not exit, but rather invoke BIOS warm boot
		//printf("OK\n");
		
	} else
	#endif
	
	#ifdef CCP_CMD_DUMP
	//if (strcmp(input, "DUMP") == 0) {
	if (strncmp(input, "DUMP",4) == 0) {
		byte l = 32;
		if (ccp_argl > 5) l = hextow(&ccp_args[6]);
		dump(hextow(&ccp_args[1]), 32);
	} else
	#endif
	#ifdef CCP_CMD_PORT
	if (strncmp(input, "PORT",4) == 0) {
		//dump((byte *)hextow(&arg[5]));
		port_out(hextob(&ccp_args[1]), hextob(&ccp_args[4]));
	} else
	#endif
	if (strncmp(input, "POKE",4) == 0) {
		//dump((byte *)hextow(&arg[5]));
		*(byte *)hextow(&ccp_args[1]) = hextob(&ccp_args[6]);
	} else
	
	#ifdef CCP_CMD_TRAP
	if (strncmp(input, "TRAP",4) == 0) {
		//dump((byte *)hextow(&input[5]));
		trap_set((byte *)hextow(&ccp_args[1]));
	} else
	#endif
	
	if (strcmp(input, "RUN") == 0) {
		ccp_run();
	} else
	
	if (strcmp(input, "VER") == 0) {
		puts("VGLDK CCP");
	} else
	/*
	if (strcmp(input, "BEEP") == 0) {
		vgl_sound_note(12*4 + 9, 250);
	} else
	*/
	
	if (strcmp(input, "EXIT") == 0) {
		//break;
		ccp_running = 0;
	} else
	
	if (strcmp(input, "RESET") == 0) {
		
		//@TODO: Usually, cold boot is done by "jp [_bios - 3]"
		//booted = BOOTED_FALSE;	// Make sure to go through cold start procedure
		__asm
			di
			jp 0x0000
		__endasm;
		
	} else
	if (strcmp(input, "SHUTDOWN") == 0) {
		printf("Powering off...");
		//booted = BOOTED_FALSE;
		
		//@FIXME: Re-implement!
		//vgl_shutdown();
		/*
		__asm
			;; V-Tech power down // BIOS4000 0181
			ld	a, #1
			out	(0ah), a
			
			ld	a, #1
			out	(12h), a
			halt
		__endasm;
		*/
		//vgl_lcd_off();
		while(1) { }
		
	} else {
		
		// Try to load given file...
		//printf("\"%s\"?\n", arg);
		
		if (ccp_load(input) == 0xff) {
			//ccp_print_error(r);
			printf(input);
			printf("?\r\n");
		} else {
			ccp_run();
		}
	}
}



void ccp() __naked {
	
	//putchar('!');
	puts('CCP');
	//printf("CCP!"); getchar();
	memset(&ccp_input[0], 0, CCP_MAX_INPUT);	// Zero out the input buffer
	
	if (ccp_argl > 0) {
		// Handle command line input...
		// Copy args over to input (or they would be overwritten by handle()!)
		memcpy(&ccp_input[0], &ccp_args[1], CCP_MAX_INPUT);	// Skip initial space (which is included at ccp_args[0])
		
		//printf("args["); printf_x2(ccp_argl); printf("]: \""); printf(ccp_input); printf("\"?");
		//if (getchar() == 'Y')
		
		handle(ccp_input);
		
		//@TODO: Could also add "; EXIT"?
		// ...and exit
		ccp_running = 0;
	} else {
		ccp_running = 1;
	}
	
	
	//input[0] = 0;
	while(ccp_running) {
		
		/*
		// Restore scroll callback
		vgl_lcd_scroll_cb = &myscroll;
		*/
		
		// Prompt
		putchar('A' + bios_curdsk);
		putchar('>');
		
		// Zero out the buffer
		memset(&ccp_input[0], 0x00, CCP_MAX_INPUT);
	
		//arg[0] = CCP_MAX_INPUT;
		//arg[1] = 0;
		gets(&ccp_input[0]);
		
		//printf("You said: \"%s\"\n", input);
		//printf("You said: \""); printf(&ccp_input[0]); printf("\"...");
		//putchar('\n');	// Input ends in "\r"
		
		handle(ccp_input);
	}
}

#endif	// __CCP_C