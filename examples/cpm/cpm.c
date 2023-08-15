/*
VGLDK (VTech Genius Leader) CP/M

2019-11-09 - 2023-08-15 Bernhard "HotKey" Slawik
*/

//#define VGLDK_SERIES 4000
//#define VGLDK_VARIABLE_STDIO
//#define VGLDK_STDOUT_PUTCHAR lcd_putchar
//#define VGLDK_STDIN_GETCHAR keyboard_getchar
//#include <vgldk.h>

// BDOS should be at the bottom of the top memory area
// The entry jump at 0x0005 is used by user programs to determine usable memory size
#include "bdos.c"

#include "bios.c"

#include "cpm.h"

#include "bint.c"

//@FIXME: CCP is a user program that should be loaded by BDOS to transient area
//#include "ccp.c"


// VGLDK and CPM CRT0 entry point (system.h/vgldk_init)
void main() __naked {
	// Go to BIOS cold boot
	__asm
		jp _bios_boot
	__endasm;
}
