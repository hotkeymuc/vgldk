/*
	Trying to create a multi-bank project
	
	This is the "common" file which is known to every segment.
	It is the "glue" to make the most important functions available without bank switching.
	
	
	2020-07-28 Bernhard "HotKey" Slawik
*/

#include <vgldk.h>
#include <stdiomin.h>

void main() {
	printf("common main");
	
	// Just return
}


//@FIXME: This port changes with architecture! (4000: 0x01, 6000: 0x51?)
//@TODO: Make it a MACRO inside the arch's system.h!
__sfr __at 0x01 bank1_port;

typedef void (t_bank_call)(void);
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
