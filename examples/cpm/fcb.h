#ifndef __FCB_H
#define __FCB_H

/*
	CP/M File Control Block
	
	2019-11-09 Bernhard "HotKey" Slawik
*/

/*
	Each file consist of:
		* segments: size=16KB ("logical extents")
		* records: size=128 bytes, count=65536
		
CR = current record,   ie (file pointer % 16384)  / 128
EX = current extent,   ie (file pointer % 524288) / 16384
S2 = extent high byte, ie (file pointer / 524288). The CP/M Plus source
    code refers to this use of the S2 byte as 'module number'.
*/

#include <vgldk.h>	// Basic types (byte, word, ...)

//typedef struct {
struct FCB {
	byte dr;	// 0=defaul, 1=A, 2=B, ... 16=P
	byte f[8];	// file name [f1...f8]; also [highest bit]: attributes
	byte t[3];	// file type [t1...t3]; also [highest bit]: RO, SYS, ARC attributes
	byte ex;	// Current Extent = (file pointer % 524288) / 16384
	byte s1;	// reserved
	byte s2;	// reserved / Extent high byte = (file pointer / 524288)
	byte rc;	// RC (Record Count for extent "ex")
	byte d[16];	// Allocation of current file
	byte cr;	// Current Record to r/w = (file pointer % 16384)  / 128
	byte r0;	// Random access record number, lowest byte
	byte r1;	// middle byte of rn
	byte r2;	// highest byte of rn (for CP/M 3)
};	// FCB;

#define recsize 128	// bytes per record


#endif // __FCB_H