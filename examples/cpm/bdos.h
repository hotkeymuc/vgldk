#ifndef __BDOS_H
#define __BDOS_H

/*
	BDOS - Basic Disk Operating System
	==================================
	
	The BDOS uses the BIOS to allow "high-level" file and device access
	
*/

// BDOS Configuration
//#define BDOS_SHOW_BANNER	// Show "BDOS" on boot (helpful for debugging)
//#define BDOS_SAVE_ALL_REGISTERS	// Make a copy of all registers on BDOS invocation
//#define BDOS_TRACE_CALLS	// Send stack traces on each call to host for in-depth analysis
//#define BDOS_TRAP	// Add trap functions

//#define BODS_WAIT_FOR_RAM	// Wait until RAM is writable before proceeding
//#define BDOS_RESTORE_LOWSTORAGE	// Restore the lower memory addresses for next cold-start

// Compatibility patch: Make the BDOS vector at 0x0005 a jump to the top of user-usable RAM. This allows BDOS to live anywhere in address space, while still marking the end of usable transient area.
//#define BDOS_PATCHED_ENTRY_ADDRESS (0x7fff - 2)	// Address of the "virtual BDOS entry", i.e. the limit of usable transient area, after which BDOS/CPM/CCP/ROM etc. start.
//#define BDOS_AUTOSTART_CCP	// Start CCP at boot without asking

//#define BDOS_USE_HOST	// Re-direct file operations to a host computer (which acts like an external drive). See bdos_host.h for more config!

#ifndef CCP_LOC_CODE
	//#define CCP_LOC_CODE	0x4000	// Entry address of CCP binary
	#error Please set CCP_LOC_CODE so that BDOS knows where to start it.
#endif

#include <basictypes.h>	// byte, word, true, false, NULL, ...

/*
	BDOS Helpers
	============
*/
// Current extent number
#define SEQ_EXT  ((long)z80->mem[DE + FCB_EX] + 32L * (long)(0x3F & z80->mem[DE + FCB_S2]))

// Current byte offset
#define SEQ_ADDRESS  (16384L * SEQ_EXT + 128L * (long)z80->mem[DE + FCB_CR])

// Convert offset to CR
#define SEQ_CR(n) (((n) % 16384) / 128)

// Convert offset to extent number
#define SEQ_EXTENT(n) ((n) / 16384)

// Convert offset to low byte of extent number
#define SEQ_EX(n) (SEQ_EXTENT(n) % 32)

// Convert offset to high byte of extent number
#define SEQ_S2(n) (SEQ_EXTENT(n) / 32)


// BODS function numbers
// https://www.seasip.info/Cpm/bdos.html
#define BDOS_FUNC_P_TERMCPM	0	// System Reset
#define BDOS_FUNC_C_READ	1	// Console input
#define BDOS_FUNC_C_WRITE	2	// Console output
#define BDOS_FUNC_A_READ	3	// Aux/Punch input
#define BDOS_FUNC_A_WRITE	4	// Aux/Punch output
#define BDOS_FUNC_L_WRITE	5	// Printer output
//	#define BDOS_FUNC_DetectMemorySize	6	// Detect memory size; CP/M 1.3 only
#define BDOS_FUNC_C_RAWIO	6	// Direct console I/O; CP/M 1.4+
//#define BDOS_FUNC_A_STATIN	7	// Auxiliary Input status; Supported by: CP/M 3 and above. Not supported in MP/M.
#define BDOS_FUNC_GET_IOBYTE	7	// Returns I/O byte
//#define BDOS_FUNC_A_STATOUT	8	// Auxiliary Output status; Supported by: CP/M 3 and above. Not supported in MP/M.
#define BDOS_FUNC_SET_IOBYTE	8	// Set I/O byte
#define BDOS_FUNC_C_WRITESTR	9	// Output string (terminated by '$')
#define BDOS_FUNC_C_READSTR	10	// Buffered console input
#define BDOS_FUNC_C_STAT	11	// Console status
#define BDOS_FUNC_S_BDOSVER	12	// Return version number

#define BDOS_FUNC_DRV_ALLRESET	13	// Reset discs, go to A:
#define BDOS_FUNC_DRV_SET	14
#define BDOS_FUNC_F_OPEN	15
//	#define BDOS_FUNC_D_OPEN	15	// Open directory; CP/M-86 v4
#define BDOS_FUNC_F_CLOSE	16
//	#define BDOS_FUNC_D_CLOSE	16	// Close directory; CP/M-86 v4
#define BDOS_FUNC_F_SFIRST	17	// Search for first
#define BDOS_FUNC_F_SNEXT	18	// Search for next
#define BDOS_FUNC_F_DELETE	19	// Delete file
//	#define BDOS_FUNC_D_DELETE	19	// Remove directory; CP/M-86 v4
#define BDOS_FUNC_F_READ	20	// Read next record
#define BDOS_FUNC_F_WRITE	21	// Write next record
#define BDOS_FUNC_F_MAKE	22	// Create file
//	#define BDOS_FUNC_D_MAKE	22	// Create directory; CP/M-86 v4
#define BDOS_FUNC_F_RENAME	23	// Rename file

#define BDOS_FUNC_DRV_LOGINVEC	24	// Return bitmap of logged-in drives
#define BDOS_FUNC_DRV_GET	25	// Return current drive
#define BDOS_FUNC_F_DMAOFF	26	// Set DMA address
#define BDOS_FUNC_DRV_ALLOCVEC	27	// Return address of allocation map
#define BDOS_FUNC_DRV_SETRO	28	// Software write-protect current disc
#define BDOS_FUNC_DRV_ROVEC	29	// Return bitmap of read-only drives
//	30 - set echo mode for function 1; CP/M 1.3 only
#define BDOS_FUNC_F_ATTRIB	30	// set file attributes
#define BDOS_FUNC_DRV_DPB	31	// get DPB address
#define BDOS_FUNC_F_USERNUM	32	// get/set user number
#define BDOS_FUNC_F_READRAND	33	// Random access read record
#define BDOS_FUNC_F_WRITERAND	34	// Random access write record
#define BDOS_FUNC_F_SIZE	35	// Compute file size
#define BDOS_FUNC_F_RANDREC	36	// Update random access pointer
#define BDOS_FUNC_DRV_RESET	37	// Selectively reset disc drives
//	38 (DRV_ACCESS) - Access drives; Supported by: MP/M, Concurrent CP/M, CP/Net redirector.
//	39 (DRV_FREE) - Free drive; Supported by: MP/M, Concurrent CP/M, CP/Net redirector.
#define BDOS_FUNC_F_WRITEZF	40	// Write random with zero fill
//	41 - Test and write record; Supported by: MP/M, Concurrent CP/M.
//	42 (F_LOCK) - Lock record; Supported by: MP/M, Concurrent CP/M, CP/Net redirector.
//	43 (F_UNLOCK) - Unlock record; Supported by: MP/M, Concurrent CP/M, CP/Net redirector.
//	44 (F_MULTISEC) - Set number of records to read/write at once; Supported by: MP/M II and later.
//	45 (F_ERRMODE) - Set action on hardware error; Supported by: Personal CP/M, MP/M II and later, CP/Net redirector.
//	46 (DRV_SPACE) - Find free space on a drive; Supported by: MP/M II and later.
//	47 (P_CHAIN) - Chain to program; Supported by: MP/M II and later; CP/M-86 v1.1
//	48 (DRV_FLUSH) - Empty disc buffers; Supported by: Personal CP/M; MP/M II and later; CP/M-86 v1.1
//		49 - Access the System Control Block; Supported by: CP/M 3.
//	49 - Return address of system variables; Supported by: CP/M-86 v1.1
//	49 (S_SYSVAR) - Access the system variables; Supported by: CP/M-86 v4.
//		50 (S_BIOS) - Use the BIOS; Supported by: CP/M 3 and later; CP/M-86 v1.1
//	51 (F_DMASEG) - Set DMA segment; Supported by: CP/M-86 v1.1 and later.
//	52 (F_DMAGET) - Get DMA address; Supported by: CP/M-86 v3+, CCP/M-86.
//	53 (MC_MAX) - Allocate maximum memory; Supported by: CP/M-86 v1.1 and later.
//	54 (MC_ABSMAX) - Allocate absolute maximum memory; Supported by: CP/M-86 v1.1 and later.
//	54 - Get file time stamps; Supported by: Z80DOS, ZPM3
//	55 (MC_ALLOC) - Allocate memory; Supported by: CP/M-86 v1.1 and later.
//	55 - Use file time stamps; Supported by: Z80DOS, ZPM3
//	56 (MC_ABSALLOC) - Allocate absolute memory; Supported by: CP/M-86 v1.1 and later.
//	57 (MC_FREE) - Free memory; Supported by: CP/M-86 v1.1 and later.
//	58 (MC_ALLFREE) - Free all memory; Supported by: CP/M-86 v1.1 and later.
//		59 (P_LOAD) - Load overlay; Supported by: CP/M 3 and higher Loaders.
//		60 - Call to RSX; Supported by: CP/M 3 and later RSXs.
//	61 - Rename file; Supported by: DOS Plus v2.1
//	62 - Unknown; Supported by: DOS Plus v2.1
//	64 - Log in; Supported by: CP/Net.
//	65 - Log off; Supported by: CP/Net.
//	66 - Send message; Supported by: CP/Net.
//	67 - Receive message; Supported by: CP/Net.
//	68 - Get network status; Supported by: CP/Net.
//	69 - Get configuration table address; Supported by: CP/Net.
//	70 - Set compatibility attributes; Supported by: CP/Net-86, some 8-bit CP/Net versions.
//	71 - Get server configuration; Supported by: CP/Net-86, some 8-bit CP/Net versions.
//		98 - Clean up disc; Supported by: CP/M 3 (Internal?).
//		99 (F_TRUNCATE) - Truncate file; Supported by: CP/M 3 and later.
//	100 (DRV_SETLABEL) - Set directory label; Supported by: MP/M II and later.
//	101 (DRV_GETLABEL) - Get directory label byte; Supported by: MP/M II and later.
//	102 (F_TIMEDATE) - Get file date and time; Supported by: MP/M II and later.
//	103 (F_WRITEXFCB) - Set file password and protection; Supported by: MP/M II and later.
//	104 (T_SET) - Set date and time; Supported by: MP/M II and later; Z80DOS, DOS+.
//	105 (T_GET) - Get date and time; Supported by: MP/M II and later; Z80DOS, DOS+.
//	106 (F_PASSWD) - Set default password; Supported by: MP/M II and above, CP/Net redirector.
//	107 (S_SERIAL) - Get serial number; Supported by: MP/M II and above.
//		108 (P_CODE) - Get/put program return code; Supported by: CP/M 3 and above.
//		109 (C_MODE) - Set or get console mode; Supported by: Personal CP/M; CP/M 3 and above
//		110 (C_DELIMIT) - Get/set string delimiter; Supported by: Personal CP/M; CP/M 3 and above
//		111 (C_WRITEBLK) - Send block of text to console; Supported by: Personal CP/M; CP/M 3 and above
//		112 (L_WRITEBLK) - Send block of text to printer; Supported by: Personal CP/M; CP/M 3 and above
//	113 - Direct screen functions; Supported by: Personal CP/M.
//	115 - Reserved for GSX; Supported by: GSX (Graphics System Extension)
//	116 - Set file date & time; Supported by: CP/M-86 v4.
//	117 - BDOS v4.x internal; Supported by: CP/M-86 v4.
//	124 - Byte block copy; Supported by: Personal CP/M.
//	125 - Byte block alter; Supported by: Personal CP/M.
//	128 (M_ALLOC) - Absolute memory request; Supported by: MP/M, Concurrent CP/M
//	129 (M_ALLOC) - Relocatable memory request; Supported by: MP/M
//	130 (M_FREE) - Free memory; Supported by: MP/M, Concurrent CP/M
//	131 (DEV_POLL) - Poll I/O device; Supported by: MP/M, Concurrent CP/M
//	132 (DEV_WAITFLAG) - Wait on system flag; Supported by: MP/M, Concurrent CP/M
//	133 (DEV_SETFLAG) - Set system flag; Supported by: MP/M, Concurrent CP/M
//	134 (Q_MAKE) - Create message queue; Supported by: MP/M, Concurrent CP/M
//	135 (Q_OPEN) - Open message queue; Supported by: MP/M, Concurrent CP/M
//	136 (Q_DELETE) - Delete message queue; Supported by: MP/M, Concurrent CP/M
//	137 (Q_READ) - Read from message queue; Supported by: MP/M, Concurrent CP/M
//	138 (Q_CREAD) - Conditionally read from message queue; Supported by: MP/M, Concurrent CP/M
//	139 (Q_WRITE) - Write to message queue; Supported by: MP/M, Concurrent CP/M
//	140 (Q_CWRITE) - Conditionally write to message queue; Supported by: MP/M, Concurrent CP/M
//	141 (P_DELAY) - Delay; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
//	142 (P_DISPATCH) - Call dispatcher; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
//	143 (P_TERM) - Terminate process; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
//	144 (P_CREATE) - Create a subprocess; Supported by: MP/M, Concurrent CP/M
//	145 (P_PRIORITY) - Set process priority; Supported by: MP/M, Concurrent CP/M
//	146 (C_ATTACH) - Attach console; Supported by: MP/M, Concurrent CP/M
//	147 (C_DETACH) - Detach console; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
//	148 (C_SET) - Set console; Supported by: MP/M, Concurrent CP/M
//	149 (C_ASSIGN) - Assign console; Supported by: MP/M, Concurrent CP/M
//	150 (P_CLI) - Send CLI command; Supported by: MP/M, Concurrent CP/M
//	151 (P_RPL) - Call resident procedure library; Supported by: MP/M, Concurrent CP/M
//		152 (F_PARSE) - Parse filename; Supported by: MP/M, CP/M 3 and higher.
//	153 (C_GET) - Return console number; Supported by: MP/M, Concurrent CP/M
//	154 (S_SYSDAT) - System data address; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
//	155 (T_SECONDS) - Get date and time; Supported by: MP/M, Concurrent CP/M
//	156 (P_PDADR) - Return address of process descriptor; Supported by: MP/M 2, Concurrent CP/M
//	157 (P_ABORT) - Abort a process; Supported by: MP/M 2, Concurrent CP/M
//	158 (L_ATTACH) - Attach printer; Supported by: MP/M 2, Concurrent CP/M
//	159 (L_DETACH) - Detach printer; Supported by: MP/M 2, Concurrent CP/M
//	160 (L_SET) - Select printer; Supported by: MP/M 2, Concurrent CP/M
//	161 (L_CATTACH) - Conditionally attach printer; Supported by: MP/M 2, Concurrent CP/M
//	162 (C_CATTACH)- Conditionally attach console; Supported by: MP/M 2, Concurrent CP/M
//	163 (S_OSVER) - Return version number; Supported by: DOSPLUS, MP/M 2, Concurrent CP/M
//	164 (L_GET) - Return default printer device number; Supported by: MP/M 2, Concurrent CP/M
//	175 - Return real drive ID; Supported by: DOSPLUS v2.1
//	P2DOS function 201 - set time; This has the same functionality under P2DOS as function 104 does under Z80DOS and DOS+.
//	DOS+ function 210 - Return system information; Supported by: DOS+
//	DOS+ function 211 - Print decimal number; Supported by: DOS+


// BDOS valid filename delimiters:
//	nul	00
//	spc	20
//	ret	0D
//	tab	09
//	:	3A
//	.	2E
//	;	3B
//	=	3D
//	,	2C
//	[	5B
//	]	5D
//	<	3C
//	>	3E
//	|	7C

char bdos_delimiter;
byte bdos_user;	// This should be the upper bits of the bios_curdsk 0x0004

struct FCB __at(0x005c) bdos_fcb;	// Default FCB at 0x005c
//long bdos_file_ofs;

volatile byte bdos_fcb_num;	// Currently used fcb number (0...3) inside DMA area. Used for bdos_f_sfirst/next

/*
//@FIXME: This is super nasty and inefficient!
// In order to force values to registers, I am shuffling them into memory and back. Please fix!

#ifdef BDOS_SAVE_ALL_REGISTERS
	volatile byte bdos_param_a;	// Only for debugging
	volatile byte bdos_param_b;	// Only for debugging
#endif
volatile byte bdos_param_c;
volatile byte bdos_param_d;
volatile byte bdos_param_e;

#ifdef BDOS_SAVE_ALL_REGISTERS
	volatile byte bdos_param_h;	// Only for debugging
	volatile byte bdos_param_l;	// Only for debugging
#endif

//#define bdos_param_de (((word)bdos_param_d) << 8 + bdos_param_e)
volatile byte bdos_ret_a;
volatile byte bdos_ret_b;

#define bdos_return1(A) {\
	bdos_ret_a = (byte)A;\
__asm\
	push af\
	ld a, (_bdos_ret_a)\
	ld l, a\
	pop af\
	ret\
__endasm;}

#define bdos_returnL(A) {\
__asm\
	ld l, A\
	ret\
__endasm;}

#define bdos_return2(A, B) {\
	bdos_ret_a = A;\
	bdos_ret_b = B;\
__asm\
	ld a, (_bdos_ret_b)\
	ld b, a\
	ld h, a\
	ld a, (_bdos_ret_a)\
	ld l, a\
	ret\
__endasm;}
*/

void bdos();	// Main entry point

// I/O Helpers
void bdos_putchar(char c);
byte bdos_getchar();

void bdos_puts(const char *str);
//void bdos_puts(const char *str, char delimiter);
//void bdos_gets(char *pc);
//void bdos_gets(char *pc, char delimiter);
void bdos_gets(char *pc, byte max_size);

// STDIO Helpers / hex helpers
void bdos_printf(char *pc);
//void bdos_printf_d(char *pc, byte d);
void bdos_printf_d(byte d);
void bdos_printf_x2(byte d);
void bdos_printf_x4(word w);

// String Helpers
//byte bdos_strlen(const char *c);
//void bdos_memset(byte *addr, byte b, word count);



// Functions for jump table
void bdos_init();	// aka. 0 = p_termcpm

void bdos_c_read();	//BDOS_FUNC_C_READ:	// 1: Console input
void bdos_c_write(char c); // BDOS_FUNC_C_WRITE:	// 2: Console output
void bdos_a_read();	// BDOS_FUNC_A_READ:	// 3: Reader input
void bdos_a_write(char c);	//BDOS_FUNC_A_WRITE:	// 4: Punch output
void bdos_l_write(char c);	// BDOS_FUNC_L_WRITE:	// 5: List output
void bdos_c_rawio(char c, char e);	//BDOS_FUNC_C_RAWIO:	// 6: Direct console I/O
void bdos_get_iobyte();	// BDOS_FUNC_GET_IOBYTE:	// 7: Get I/O Byte
void bdos_set_iobyte(char c);	// BDOS_FUNC_SET_IOBYTE:	// 8: Set I/O Byte
void bdos_c_writestr(char *de);	// BDOS_FUNC_C_WRITESTR:	// 9: Print string (until delimiter "$")
void bdos_c_readstr(char *de);	// BDOS_FUNC_C_READSTR:	// 10: Read console buffer
void bdos_c_stat();	// BDOS_FUNC_C_STAT:	// 11: Get console status
void bdos_s_bdosver();	// BDOS_FUNC_S_BDOSVER:	// 12: Return version number

void bdos_drv_allreset();	// BDOS_FUNC_DRV_ALLRESET:	// 13: Reset disk system
void bdos_drv_set(byte e);// BDOS_FUNC_DRV_SET:	// 14: Select disk

byte bdos_f_open(struct FCB *fcb);	// BDOS_FUNC_F_OPEN:	// 15: Open file
byte bdos_f_close(struct FCB *fcb);	// BDOS_FUNC_F_CLOSE:	// 16: Close file
byte bdos_f_sfirst(struct FCB *fcb);	// BDOS_FUNC_F_SFIRST:	// 17: Search for first
byte bdos_f_snext(struct FCB *fcb);	// BDOS_FUNC_F_SNEXT:	// 18: Search for next
byte bdos_f_delete(struct FCB *fcb);	// BDOS_FUNC_F_DELETE:	// 19: Delete file
byte bdos_f_read(struct FCB *fcb);	// BDOS_FUNC_F_READ:	// 20: Read sequential
byte bdos_f_write(struct FCB *fcb);	// BDOS_FUNC_F_WRITE:	// 21: Write sequential
byte bdos_f_make(struct FCB *fcb);	// BDOS_FUNC_F_MAKE:	// 22: Make file
byte bdos_f_rename(struct FCB *fcb);	// BDOS_FUNC_F_RENAME:	// 23: Rename file
byte bdos_drv_loginvec();	// BDOS_FUNC_DRV_LOGINVEC:	// 24: Return login vector
void bdos_drv_get();	// BDOS_FUNC_DRV_GET:	// 25: Return current disk
void bdos_f_dmaoff(char *de);	// BDOS_FUNC_F_DMAOFF:	// 26: Set DMA address
void bdos_drv_allocvec();	// BDOS_FUNC_DRV_ALLOCVEC:	// 27: Get addr (alloc)
// 28: Write protect disk
// 29: Get R/O vector
// 30: Set file attributes
// 31: Get addr (disk parms)
byte bdos_f_usernum();	// BDOS_FUNC_F_USERNUM:	// 32: Set/Get user code
byte bdos_f_readrand(struct FCB *fcb);	// BDOS_FUNC_F_READRAND:	// 33: Read random
byte bdos_f_writerand(struct FCB *fcb);	// BDOS_FUNC_F_WRITERAND:	// 34: Write random
void bdos_f_size(struct FCB *fcb);	// BDOS_FUNC_F_SIZE:	// 35: Compute file size
void bdos_f_randrec(struct FCB *fcb);	// BDOS_FUNC_F_RANDREC:	// 36: Set random record
void bdos_drv_reset();	// BDOS_FUNC_DRV_RESET:	// 37: Reset drive
// 38: Undefined - go back
// 39: Undefined - go back
byte bdos_f_writezf(struct FCB *fcb);	// BDOS_FUNC_F_WRITEZF:	// 40: Fill random file w/ zeros

void bdos_unimplemented();	// Catch-all for unimplemented functions

void bdos_funcs();	// BDOS function jump table

#endif	// __BDOS_H