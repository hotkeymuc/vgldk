#ifndef __VGLDK_H
#define __VGLDK_H

/*
	
	VGLDK
	=====
	
	A Software Development Kit for the "VTech Genius LEADER" series of learning computers
	(aka "VTech PreComputer" aka "YENO MisterX")
	
	2017 Bernhard "HotKey" Slawik
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
	
*/

//#define byte unsigned char
//#define word unsigned short
typedef unsigned char byte;
typedef unsigned short word;
typedef byte * p_byte;
typedef char * p_char;
#define true 1
#define false 0
#define NULL ((void *)0)
//const void *NULL = (void *)0;

#ifndef VGLDK_SERIES
	#error "VGLDK_SERIES must be defined"
#endif

/*
// Try having this function be the first define in the whole code segment
// So one can can just jump to the first byte to get things rolling
void vgldk_init();	// Forward
void vgldk_entry() __naked {
	__asm
		jp	vgldk_init
	__endasm;
}
*/


// #include <arch/#VGLDK_ARCH/system.h>
// #include <system.h>
#if VGLDK_SERIES == 0
	#include "arch/plain/system.h"
#elif VGLDK_SERIES == 1000
	#include "arch/pc1000/system.h"
#elif VGLDK_SERIES == 2000
	#include "arch/gl2000/system.h"
#elif VGLDK_SERIES == 4000
	#include "arch/gl4000/system.h"
#elif VGLDK_SERIES == 6000
	#include "arch/gl6000sl/system.h"
#else
	#error "Specified VGLDK_SERIES is unknown"
#endif


#endif