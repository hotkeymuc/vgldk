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


#include "basictypes.h"	// byte, word, true, false, NULL, ...


#ifndef VGLDK_SERIES
	#error "VGLDK_SERIES must be defined"
#endif


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