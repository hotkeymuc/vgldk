#ifndef __LOADER_H
#define __LOADER_H
/*

App loader for V-Tech Genius Leader

These functions allow an "app" to be loaded somewhere in RAM.
It takes care of relocating all symbols and linking system calls.

TODO:
	* Use bank-switched SRAM (SUPERSPEICHER/SCHREIBAMSCHINENKURS) as app code RAM:
		* Copy an aux function to RAM (which runs while bank switching between ROM/SRAM at 0x8000)
		* Read/relocate app (chunk) from external drive to tempRAM
		* Jump to aux function in RAM:
			* bank-switch SRAM to 0x8000 (ROM vanishes)
			* copy data from tempRAM to SRAM
			* bank-switch ROM to 0x8000
			* return back to ROM code
		[repeat until the whole app is loaded]
		* Call app in SRAM (via aux function in RAM)

2019-07-03 Bernhard "HotKey" Slawik

*/

//#include <string.h>	// for strcmp() on the symbol names

// Establish a "common API" for loader and app
#include "api.h"

// Define the actual API implementation available to apps
// Here we link to the stdio implementation
const t_api api_impl = {	// Keep in sync with definition in api.h!
	0x0003,
	
	putchar,
	getchar,
	printf,
	sprintf,
	gets,
	
	clear,
	beep
};

// We need to define this, because it is needed for runtime-linking as it is usually added as a part of stdlib
void __sdcc_call_hl() __naked {
	__asm
		jp	(hl)
	__endasm;
}

// Loader variables
byte *p_appBin;	// Pointer to app binary
word app_o;	// Currrent offset in binary
word app_addr_start;	// Remembering the entry point of start()
word app_addr_main;	// Remembering the entry point of main()

// Addresses used during compilation of the apps (as found in app binary linked_addr)
const word app_srcCode = 0x0000;
const word app_srcData = 0xC000;

#define AREA_HEADER_BYTE_CODE 0xC0
#define AREA_HEADER_BYTE_DATA 0xDA
#define AREA_HEADER_BYTE_EXT 0xE0

const byte APP_SIGNATURE[] = "HAPP";	// Maybe use valid Z80 code?

#define ERR_SIGNATURE_FAILED -10
#define ERR_SYMBOL_UNKNOWN -11

// Helpers to read the source app binary
#define STR_MAX 32
byte app_readByte() {
	// Read one byte from the raw app binary
	return p_appBin[app_o++];
}
word app_readWord() {
	return app_readByte() + (app_readByte() << 8);
}
byte app_readStr(byte *s) {
	byte i;
	byte l;
	byte *t;
	
	t = s;
	l = app_readByte();
	for(i = 0; i < l; i++) {
		*t = app_readByte();
		t++;
	}
	*t = 0x00;	// Terminate
	return l;
}

// The actual loader
int app_load(byte *appBin, word destCode, word destData) {
	byte b;
	word i;
	word appSize;
	word areas;
	word areaI;
	byte areaHeaderByte;
	word syms;
	word symI;
	byte symNameLen;
	byte symName[STR_MAX];
	word symLinkedAddr;
	word usages;
	word usageI;
	word usageBinOfs;
	word usageSymOfs;
	word symRelocatedAddr;	// re-located address
	word finalAddr;	// re-located address including offset
	
	// Reset APP reader state
	p_appBin = appBin;	// Set read pointer to start of binary
	app_o = 0;
	
	// Read header
	for(i = 0; i < 4; i++) {
		b = app_readByte();
		if (b != APP_SIGNATURE[i]) {
			printf("Sign fail!\n");
			return ERR_SIGNATURE_FAILED;
		}
	}
	
	appSize = app_readWord();
	//printf("size=%d\n", appSize);
	
	// Read binary into RAM
	printf("Load.");
	for(i = 0; i < appSize; i++) {
		if ((i % 0x40) == 0) printf(".");
		
		// Load from APP structure
		b = app_readByte();
		
		// Write into memory
		VGL_MEM[destCode + i] = b;
	}
	//printf("OK\n");
	
	
	// Perform relocation of symbols
	//printf("Relocating...");
	printf("OK\nRelo...");
	
	app_addr_start = destCode;	// Use first code byte as default start address
	
	// Read all areas
	areas = app_readWord();
	for(areaI = 0; areaI < areas; areaI++) {
		// Read area header
		areaHeaderByte = app_readByte();
		
		// Read (used) symbols
		syms = app_readWord();
		for(symI = 0; symI < syms; symI++) {
			// Read symbol header
			symNameLen = app_readStr(&symName[0]);
			symLinkedAddr = app_readWord();
			
			//printf("Symbol \"%s\"\n", symName);
			
			// Translate address
			symRelocatedAddr = 0x0000;
			
			switch(areaHeaderByte) {
				case AREA_HEADER_BYTE_CODE:
					// Map to destination code area
					symRelocatedAddr = symLinkedAddr - app_srcCode + destCode;
					
					// Remember offset of function "start"
					//if (strcmp(symName, "_start") == 0) app_addr_start = symRelocatedAddr;
					if (strcmp(symName, "_main") == 0) app_addr_main = symRelocatedAddr;
					break;
				
				case AREA_HEADER_BYTE_DATA:
					// Map to destination data area
					symRelocatedAddr = symLinkedAddr - app_srcData + destData;
					break;
				
				case AREA_HEADER_BYTE_EXT:
					// Map externals
					
					//printf("Ext \"%s\" at 0x%04X", symName, symLinkedAddr);
					
					// Link external symbols (e.g. auto-generades code like "___sdcc_call_hl"
					if (symLinkedAddr > 0x0000) {
						// External was resolved by the linker
						
						//@FIXME: Assuming this symbol was CODE (and not DATA)
						symRelocatedAddr = symLinkedAddr - app_srcCode + destCode;	// Simply take what the linker told us and re-map it
						
					} else {
						
						// Link externals (i.e. no address from linker) by name
						
						
						if      (strcmp(symName, "_putchar") == 0) symRelocatedAddr = (word)(putchar);
						else if (strcmp(symName, "_getchar") == 0) symRelocatedAddr = (word)(getchar);
						else if (strcmp(symName, "_printf") == 0) symRelocatedAddr = (word)(printf);
						//else if (strcmp(symName, "_sprintf") == 0) symRelocatedAddr = (word)(sprintf);
						else if (strcmp(symName, "_gets") == 0) symRelocatedAddr = (word)(gets);
						
						else if (strcmp(symName, "_clear") == 0) symRelocatedAddr = (word)(clear);
						else if (strcmp(symName, "_beep") == 0) symRelocatedAddr = (word)(beep);
						
						else if (strcmp(symName, "___sdcc_call_hl") == 0) symRelocatedAddr = (word)(__sdcc_call_hl);
						
						else if (strcmp(symName, "_api") == 0) symRelocatedAddr = (word)(&api_impl);
						else {
							
							printf("Unknown \"%s\"!\n", symName);
							return ERR_SYMBOL_UNKNOWN;
						}
					}
					break;
				
				default:
					printf("Un-relo: 0x%04X!\n", symLinkedAddr);
					symRelocatedAddr = symLinkedAddr;
					return ERR_SYMBOL_UNKNOWN;
			}
			
			//printf("Symbol \"%s\" from 0x%04X to 0x%04X\n", symName, symLinkedAddr, symRelocatedAddr);
			
			// Read symbol's usages and re-locate
			usages = app_readWord();
			for(usageI = 0; usageI < usages; usageI++) {
				usageBinOfs = app_readWord();
				
				//usageSymOfs = app_readWord();	// The usageSymOfs = value at usageBinOfs - symLinkedAddr!
				usageSymOfs = ((word)VGL_MEM[destCode + usageBinOfs    ] + (((word)VGL_MEM[destCode + usageBinOfs + 1]) << 8)) - symLinkedAddr;
				
				// Perform re-location patch on memory
				finalAddr = symRelocatedAddr + usageSymOfs;	// Include the in-symbol offset (e.g. for structs)
				VGL_MEM[destCode + usageBinOfs    ] = (finalAddr & 0xff);
				VGL_MEM[destCode + usageBinOfs + 1] = (finalAddr >> 8);
				
			}
		}
	}
	printf("OK\n");
	return 0;
}

void app_callStart() {
	t_start p_start;
	
	//p_start = (t_start)app_destCode;	// Set the start address as the first byte in the relocated code area
	p_start = (t_start)app_addr_start;	// Set the start address as the addr of a symbol called "start"
	
	(*p_start)();	// Actually call the start() function of the app
	
	//printf("End of start()\n");
	//beep();
	//getchar();
}

int app_callMain(int argc, char *argv[]) {
	int r;
	t_main p_main;
	
	p_main = (t_main)app_addr_main;	// Set the start address as the addr of a symbol called "main"
	
	r = (*p_main)(argc, argv);	// Actually call the main() function of the app
	
	//printf("Exit code: %d\n", r);
	//beep();
	//getchar();
	
	return r;
}

#endif //__LOADER_H