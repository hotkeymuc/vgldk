#ifndef __ARCH_PLAIN
#define __ARCH_PLAIN

/*

Plain/App architecture

System header for a plain loadable programs.
Programs compiled against this architecture will not have their own hardware drivers.
Instead, they are strictly using VGLDK_VARIABLE_STDIO and get pointers to stdio
from the caller.

2020-06-03 Bernhard "HotKey" Slawik
*/

// Try having this function be the first bytes in the whole code segment.
// So the caller/loader can just jump to the first byte of data and does not have to search for the entry point.
void vgldk_entry() __naked {
	// By keeping this "__naked" and just using "jp", we do not lose the stack frame created by the caller.
	// This will effectively become 3 bytes (C3 ll hh)
	__asm
		jp	_vgldk_init
	__endasm;
}
// Now we can have arbitrary code and data


// Let's chose some sensefull values
//#define DISPLAY_COLS 40
//#define DISPLAY_ROWS 10
//#include "lcd.h"
//#include "keyboard.h"


// Publish callbacks for STDIO
// Plain architecture defaults to variable stdio
#define VGLDK_VARIABLE_STDIO
// Stdio must be included here as we need to set the stdio-pointers
#include "stdiomin.h"

//#define VGLDK_STDOUT_PUTCHAR plain_putchar
//#define VGLDK_STDIN_GETCHAR plain_getchar

//#define VGLDK_STDIN_GETS stdio_gets
//#define VGLDK_STDIN_INKEY plain_inkey


// Main entry point for callers
//void vgldk_init(void *v_stdout_putchar, void *v_stdin_getchar) __naked {
//void vgldk_init() __naked {
//int vgldk_init(void *v_stdout_putchar, void *v_stdin_getchar) {
//int vgldk_init(t_putchar *v_stdout_putchar, t_getchar *v_stdin_getchar) {

//int vgldk_argc;
//char **vgldk_argv;
int main(int argc, char *argv[]);	// Forward declaration

int vgldk_init(t_putchar *v_stdout_putchar, t_getchar *v_stdin_getchar, int argc, char *argv[]) {
	
	// Obtain pointers to putchar/getchar from the caller arguments
	p_stdout_putchar = v_stdout_putchar;
	p_stdin_getchar = v_stdin_getchar;
	
	/*
	vgldk_argc = argc;
	vgldk_argv = &argv[0];
	__asm
		pop bc
		
		ld hl, (_vgldk_argv)
		push hl
		
		ld hl, (_vgldk_argc)
		push hl
		
		push bc
	__endasm;
	*/
	
	/*
	// Get pointers to putchar/getchar from the caller stack
	__asm
		push hl
		push bc
		
		;; Param 1: putchar
		ld	hl, #2+4
		add	hl, sp
		;ld	bc, (hl)
		ld	b, (hl)
		inc	hl
		ld	c, (hl)
		ld	(_p_stdout_putchar), bc
		
		; Param 2: getchar
		ld	hl, #4+4
		add	hl, sp
		ld	b, (hl)
		inc	hl
		ld	c, (hl)
		ld	(_p_stdin_getchar), bc
		
		pop bc
		pop hl
	__endasm;
	*/
	
	/*
	// Continue with main program (don't CALL, but JP in order to RET to the original caller)
	//main();
	__asm
		jp _main
	__endasm;
	
	return 0;
	*/
	
	return main(argc, argv);
}

#endif //__ARCH_PLAIN