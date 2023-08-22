#ifndef __BIOS_H
#define __BIOS_H
/*
	BIOS - Basic Input/Output System
	================================
	
	This contains individual procedures that input and output data to/from keyboard, screen, printer etc.
	
*/

//#define MAX_DATA 128
#define BIOS_STACK 0xDFF0	// BIOS (re)sets stack pointer there


//#define BIOS_USE_PRINTER	// Include printer driver and redirect printer "list" to printer.
//#define BIOS_PAPER_TAPE_TO_SOFTUART	// Redirect paper tape functions "punch" and "reader" to SoftUART
//#define BIOS_PAPER_TAPE_TO_MAME	// Redirect paper tape functions "punch" and "reader" to MAME-Host

// BIOS function numbers
// https://www.seasip.info/Cpm/bios.html

// CP/M 1:
#define BIOS_FUNC_BOOT	0	// This function is completely implementation-dependent and should never be called from user code.
#define BIOS_FUNC_WBOOT	1	// Reloads the command processor and (on some systems) the BDOS as well. How it does this is implementation-dependent; it may use the reserved tracks of a floppy disc or extra memory.
#define BIOS_FUNC_CONST	2	// Returns its status in A; 0 if no character is ready, 0FFh if one is.
#define BIOS_FUNC_CONIN	3	// Wait until the keyboard is ready to provide a character, and return it in A.
#define BIOS_FUNC_CONOUT	4	// Write the character in C to the screen.
#define BIOS_FUNC_LIST	5	// Write the character in C to the printer. If the printer isn't ready, wait until it is.
#define BIOS_FUNC_PUNCH	6	// Write the character in C to the "paper tape punch" - or whatever the current auxiliary device is. If the device isn't ready, wait until it is.
#define BIOS_FUNC_READER	7	// Read a character from the "paper tape reader" - or whatever the current auxiliary device is. If the device isn't ready, wait until it is. The character will be returned in A. If this device isn't implemented, return character 26 (^Z).
#define BIOS_FUNC_HOME	8	// Move the current drive to track 0.
#define BIOS_FUNC_SELDSK	9	// Select the disc drive in register C (0=A:, 1=B: ...). Called with E=0 or 0FFFFh. SELDSK returns the address of a Disc Parameter Header in HL. The exact format of a DPH varies between CP/M versions; note that under CP/M 3, the DPH is in memory bank 0 and probably not visible to programs. If the disc could not be selected it returns HL=0.
#define BIOS_FUNC_SETTRK	10	// Set the track in BC - 0 based.
#define BIOS_FUNC_SETSEC	11	// Set the sector in BC. Under CP/M 1 and 2 a sector is 128 bytes.
#define BIOS_FUNC_SETDMA	12	// The next disc operation will read its data from (or write its data to) the address given in BC.
#define BIOS_FUNC_READ	13	// Read the currently set track and sector at the current DMA address. Returns A=0 for OK, 1 for unrecoverable error, 0FFh if media changed.
#define BIOS_FUNC_WRITE	14	// Write the currently set track and sector. C contains a deblocking code:
							// C=0 - Write can be deferred / C=1 - Write must be immediate / C=2 - Write can be deferred, no pre-read is necessary.
							// Returns A=0 for OK, 1 for unrecoverable error, 2 if disc is readonly, 0FFh if media changed.
// CP/M 2:
#define BIOS_FUNC_LISTST	15	// Return status of current printer device: Returns A=0 (not ready) or A=0FFh (ready).
#define BIOS_FUNC_SECTRAN	16	// ranslate sector numbers to take account of skewing. On entry, BC=logical sector number (zero based) and DE=address of translation table. On exit, HL contains physical sector number. On a system with hardware skewing, this would normally ignore DE and return either BC or BC+1.
/*
// CP/M 3:
#define BIOS_FUNC_CONOST	17	// Return status of current screen output device: A=0 (not ready) or A=0FFh (ready).
#define BIOS_FUNC_AUXIST	18	// Return status of current auxiliary input device: A=0 (not ready) or A=0FFh (ready).
#define BIOS_FUNC_AUXOST	19	// Return status of current auxiliary output device: A=0 (not ready) or A=0FFh (ready).
#define BIOS_FUNC_DEVTBL	20	// Return in HL the address of the devices table, or 0 if the devices table isn't implemented.
								// Each entry is formed:
								//	DEFB	'NAME  '	;Name, 6 bytes. If the first byte is zero, this is the end of the table.
								//	DEFB	mode		;Bitmapped value:
								//				;Bit 0 set => can input from this device
								//				;Bit 1 set => can output to this device
								//				;Bit 2 set => can change the baud rate
								//				;Bit 3 set => supports XON/XOFF
								//				;Bit 4 set => is using XON/XOFF
								//				;Bits 5,6,7 set to 0.
								//				; Amstrad extension: If bit 7 is set, output to the device does not time out. 
								//	DEFB	baudrate	;Coded speed, 1-15 or 0 if speed can't be changed.
								//				;Rates are 50,75,110,134.5,150,300,600,1200,1800,2400,3600,4800,7200,9600,19200.
#define BIOS_FUNC_DEVINI	21	// Reinitialise character device number C - called when the device's settings (baud rate, mode etc.) are changed.
#define BIOS_FUNC_DRVTBL	22	// Return in HL the address of the drive table, or 0 (or 0FFFFh, or 0FFFEh) if the drive table isn't implemented. The drive table contains 16 pointers to the Disc Parameter Headers of the 16 disc drives A-P; if a pointer is 0 it means that the corresponding drive does not exist.
#define BIOS_FUNC_MULTIO	23	// Notify the BIOS that the BDOS is intending to transfer a number of consecutive disc sectors with READ or WRITE. Entered with C = number of calls that will be made; up to 16k of data will be transferred. The idea is that after the MULTIO call, the BIOS can choose to transfer all the data in the first READ/WRITE operation, and then not to do anything on the subsequent (n-1) operations.
#define BIOS_FUNC_FLUSH	24	// Write any pending data to disc. Returns A=0 if successful, 1 for physical disc error, 2 for drive R/O. This function is only useful when the BIOS is doing the deblocking - ie, the physical sector size is not the size that the BIOS reports to the BDOS.
#define BIOS_FUNC_MOVE	25	// Move BC bytes of memory, from the address in DE to the address in HL (the other way round from the Z80's LDIR instruction). Should return HL and DE pointing to the first addresses not copied. If XMOVE is used before this function, data are moved between two memory banks.
#define BIOS_FUNC_TIME	26	// Get the current date and time into the SCB (at BOOT-0Ch). HL and DE must be preserved. If C=0FFh, then set the time from the SCB instead.
							// The format of the 5-byte buffer is:
							// 	DW	day	;Day 1 is 1 Jan 1978
							// 	DB	hour	;packed BCD
							// 	DB	minute	;packed BCD
							// 	DB	second	;packed BCD
#define BIOS_FUNC_SELMEM	27	// Set the current bank to the number in A. Bank 1 is the bank in which user programs run (the TPA); Bank 0 and any other banks are used by CP/M for disc buffers or as a RAMdisc. According to the DRI documentation, this function must preserve all registers except A.
#define BIOS_FUNC_SETBNK	28	// Set the bank to be used for the next read/write sector operation. The bank number is passed in A. Note that the BDOS will call SETBNK after calling SETDMA; some BIOSes insist on this order, so it's safest if your programs do the same.
#define BIOS_FUNC_XMOVE	29	// After XMOVE, the next call to MOVE will move data between different memory banks. Call XMOVE with C=source bank and B=destination bank. According to the CP/M Plus System Guide, the BDOS will only ever use this function to move 128 or fewer bytes in one go; some BIOSes may not support bigger moves between banks.
#define BIOS_FUNC_USERF	30	// This function is reserved for the author of the BIOS to add any extra features. On Amstrad computers, for example, this call accesses the extended BIOS functions. See: XBIOS
#define BIOS_FUNC_RESERV1	31	// These calls are reserved and contain JMP 0 instructions.
#define BIOS_FUNC_RESERV2	21	// These calls are reserved and contain JMP 0 instructions.
*/

#include <basictypes.h>	// byte, word, true, false, NULL, ...

// Disk parameter header
typedef struct {
	byte dummy;
} DPH;


// State variables (highly volatile)

/*
IO Byte at 0x003:
	Bits      Bits 6,7    Bits 4,5    Bits 2,3    Bits 0,1
	Device    LIST        PUNCH       READER      CONSOLE
	
	Value
	00      TTY:        TTY:        TTY:        TTY:
	01      CRT:        PTP:        PTR:        CRT:
	10      LPT:        UP1:        UR1:        BAT:
	11      UL1:        UP2:        UR2:        UC1:
	
	BAT = batch mode. Use the current Reader for console input, and he current List (printer) device as the console output.
	CRT = Standard console (keyboard and terminal screen).
	LPT = Standard line printer.
	PTP = Standard Paper Tape Punch.
	PTR = Standard Paper Tape Reader.
	TTY = Teletype device, eg a serial port.
	UC1 = User defined (ie implementation dependent) console device.
	UL1 = User defined (ie implementation dependent) printer device.
	UPn = User defined (ie implementation dependent) output device.
	URn = User defined (ie implementation dependent) input device.
*/
byte __at(0x0003) bios_iobyte;

byte __at(0x0004) bios_curdsk;
//byte __at(0x0040) bios_workarea[0x100];
//FCB __at(0x005c) bdos_fcb;	// Default FCB at 0x005c

byte *bios_dma;	// Defaults to 0x0080
word bios_trk;
word bios_sec;


DPH bios_dummy_dph;	// Dummy DPH


void bios_boot();
void bios_wboot();
byte bios_const();
byte bios_conin();
void bios_conout(byte c);
void bios_list(byte c);
void bios_punch(byte c);
byte bios_reader();

void bios_home();	// Move the current drive to track 0.
DPH *bios_seldsk(byte n);	// Select the disc drive in register C (0=A:, 1=B: ...). Called with E=0 or 0FFFFh. SELDSK returns the address of a Disc Parameter Header in HL. The exact format of a DPH varies between CP/M versions; note that under CP/M 3, the DPH is in memory bank 0 and probably not visible to programs. If the disc could not be selected it returns HL=0.
void bios_settrk(word t);	// Set the track in BC - 0 based.
void bios_setsec(word s);	// Set the sector in BC. Under CP/M 1 and 2 a sector is 128 bytes.
void bios_setdma(byte *a);	// The next disc operation will read its data from (or write its data to) the address given in BC.
byte bios_read();	// Read the currently set track and sector at the current DMA address. Returns A=0 for OK, 1 for unrecoverable error, 0FFh if media changed.
byte bios_write(byte c);	// Write the currently set track and sector. C contains a deblocking code:
// C=0 - Write can be deferred / C=1 - Write must be immediate / C=2 - Write can be deferred, no pre-read is necessary.
// Returns A=0 for OK, 1 for unrecoverable error, 2 if disc is readonly, 0FFh if media changed.

byte bios_listst(); // Return status of current printer device: Returns A=0 (not ready) or A=0FFh (ready).
word bios_sectran(word n, byte *a);	// ranslate sector numbers to take account of skewing. On entry, BC=logical sector number (zero based) and DE=address of translation table. On exit, HL contains physical sector number. On a system with hardware skewing, this would normally ignore DE and return either BC or BC+1.

#endif	// __BIOS_H