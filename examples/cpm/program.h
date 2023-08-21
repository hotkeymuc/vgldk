#ifndef __PROGRAM_H
#define __PROGRAM_H

/*

Include this to create a simple CP/M program.
It contains wrappers for BDOS function calls.

*/

//#define PROGRAM_GETS_LOCAL_ECHO	// Should gets() have local echo?
#define PROGRAM_GETS_MAX_SIZE 127

void putchar(char b);
char getchar();

void puts(const char *s);
//char *gets(char *pc);
//void gets(char *pc, byte max_size);
void gets(char *pc);

// Helpers
void printf(const char *s);
//void printf_d(char *pc, byte d);
//void printf_d(byte d);


void exit();

#endif	// __PROGRAM_H