/*
	Trying to create a multi-bank project
	
	2020-07-28 Bernhard "HotKey" Slawik
*/


//@TODO: Include the "CRT0 surrogate" to vgldk.h

// CRT0 surrogate: The following block does what is usually done by a CRT0.s file
//extern void vgldk_init();	// Forward declaration to suppress compiler warnings
void main0();	// Forward declaration to suppress compiler warnings

#include "common.h"
#include "segment1_addr.h"
#include "segment2_addr.h"


void CRT0_ROM() __naked {
	// This is the system ROM header
	__asm
		di
		ld	sp, #0xdff0	; Load StackPointer to 0xdff0
		
		; Some internal resets (see ROM4000, 0x0eda)
		ld a,#0xff		;0edd	3e ff 	> . 
		out (0x11),a		;0edf	d3 11 	. . 
		ld a,#0xde		;0ee1	3e de 	> . 
		out (0x12),a		;0ee3	d3 12 	. . 
	
		xor a			;0ee5	af 	. 
		out (0x01),a		;0ee6	d3 01 	. . 
		out (0x02),a		;0ee8	d3 02 	. . 
		out (0x03),a		;0eea	d3 03 	. . 
		out (0x04),a		;0eec	d3 04 	. . 
		out (0x05),a		;0eee	d3 05 	. . 
		
		; Activate Seg1
		ld a, #1
		out (0x01),a
		
		call	common_vgldk_init_addr
	__endasm;
	
	
	//vgldk_init();	// will call common.main() and return
	
	main0();
}

#ifdef __DISABLED__

// Make sure this is the very first function to be compiled, so it provides the very first bytes in the binary
void CRT0_CART() __naked {
	// This is a cartridge ROM header

	// ROM header first bytes
	__asm
		.db	#0x55
		.db	#0xaa
	__endasm;
	
	
	/*
	// Normal signature (i.e. non-autostart program cartridge)
	__asm
		.db #0x47 ; "G"
		.db #0x41 ; "A"
	__endasm;
	*/
	
	#if VGLDK_SERIES == 1000
		// PreComputer1000 auto-start signature
		__asm
			.db #0x33  ; 0x33 = autostart jump to 0x8010
			.db #0x00  ; Dont care
			
			; fill up until 0x8010
			.db #0x00
			.db #0x00
			.db #0x00
			.db #0x00
			.db #0x00
			.db #0x00
			.db #0x00
			.db #0x00
			.db #0x00
			.db #0x00
			.db #0x00
			.db #0x00
		__endasm;
	#else
		// GL2000/4000/6000 auto-start signature
		__asm
			.db	#0x59	; "Y"
			.db	#0x45	; "E"
		__endasm;
	#endif
	
	
	// Actually run some code
	
	/*
	__asm
		ld	sp, #0xff00
	__endasm;
	*/
	
	/*
	__asm
		; First executed instruction (usually a jump to actual code)
		;di
		;im 1
		jp	_vgldk_init
	__endasm;
	*/
	vgldk_init();
}
#endif
// End of CRT0 surrogate


/*
#include <vgldk.h>
#include <stdiomin.h>
*/
/*
//extern char getchar(void);
extern char keyboard_getchar(void);
#define getchar keyboard_getchar
extern void printf(char *);
*/


// externals from different segments
//extern void test_segment1(void);
//extern void test_segment2(void);


/*
// Bank switching stuff

//@FIXME: This port changes with architecture! (4000: 0x01, 6000: 0x51?)
//@TODO: Make it a MACRO inside the arch's system.h!
__sfr __at 0x01 bank1_port;

void bank1_call(byte seg, t_bank_call *call, void *args) {
	
	//@TODO: If args are in banked memory, it has to be copied to RAM prior to bank switch
	
	//@TODO: Store old bank value on stack
	
	//@TODO: Do the bank switch
	bank1_port = seg;
	
	// Call the banked function
	//(*call)(args);
	(*call)();
	(void)args;
	
	//@TODO: Restore old bank value from stack
	
}
*/

void main0() __naked {
	char c;
	
	
	printf("This is main main!\n");
	c = getchar();
	
	
	printf("Call 1:");
	//bank1_call(1, (t_bank_call *)0x4000, NULL);
	//bank1_call(1, (t_bank_call *)(0x4000 + 0x05de), NULL);
	//bank1_call(1, (t_bank_call *)(main1), NULL);
	//bank1_call(1, (t_bank_call *)(test_segment1), (void *)0);
	bank1_call(1, segment1_test_segment1_addr, (void *)0);
	printf("OK\n");
	
	c = getchar();
	
	printf("Call 2:");
	//bank1_call(2, (t_bank_call *)0x4000, NULL);
	//bank1_call(2, (t_bank_call *)(0x4000 + 0x5ce), NULL);
	//bank1_call(2, (t_bank_call *)(main2), NULL);
	bank1_call(2, segment2_test_segment2_addr, (void *)0);
	printf("OK\n");
	
	printf("End.");
	c = getchar();
	
	while(1) { }
	//return;
	//return c;
}
