/*
VGLDK (VTech Genius Leader) CP/M

2019-11-09 - 2023-08-15 Bernhard "HotKey" Slawik
*/

#include "cpm.h"

// BDOS should be at the bottom of the memory area, so load it first!
// CRT0 at 0x0005 contains a jump to bdos() (as per CP/M spec)
// The entry jump address of this jump is used by user programs to determine usable memory size (e.g. ZORK).
#include "bdos.c"

#include "bios.c"

#include "bint.c"

//@FIXME: CCP is a user program that should be loaded by BDOS to transient area
//#include "ccp.c"

// VGLDK entry point (system.h/vgldk_init)
void main() __naked {
	// Go straight to BIOS cold boot
	__asm
		jp _bios_boot
	__endasm;
}
