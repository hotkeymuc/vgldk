#ifndef __APP_H
#define __APP_H
/*
	
	Include this file in your "app" to have all the link-time benefits of the common API
	
*/

#include "api.h"

#define NULL (void *)0

// External linking:
// The loader uses the actual NAME of unresolved externals to link them to run-time symbols

// Those (unresolved external) symbols will be runtime-linked according to their names
extern const t_api api;	// "api" will be linked to the actual API implementation
extern const t_putchar putchar;
extern const t_getchar getchar;
extern const t_printf printf;
extern const t_sprintf sprintf;
extern const t_gets gets;
extern const t_clear clear;
extern const t_beep beep;


// Entry point
// The main() function must be referenced at least SOMEWHERE in code in order to create a symbol we can work with at load-time
int main(int argc, char *argv[]);	// Forward declaration to main(), so it can be defined later and start() is the first instruction in the binary
void start() {	// First function declaration
	
	// First instruction! This should be the first memory cell of the binary
	main(0, NULL);
	
}
#endif // __APP_H