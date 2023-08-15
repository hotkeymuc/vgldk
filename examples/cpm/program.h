#ifndef __PROGRAM_H
#define __PROGRAM_H

/*
Include this to create a simple CP/M program
*/

//#define PROGRAM_GETS_LOCAL_ECHO	// Should gets() have local echo?

void putchar(char b);
char getchar();

char *gets(char *pc);
void printf(const char *s);

void exit();

#endif	// __PROGRAM_H