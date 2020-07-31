
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
#include "common.h"

void main2() {
	char c;
	
	//while(1) { }
	printf("SEG2!\n");
	c = getchar();
}

void test_segment2() {
	char c;
	printf("test_segment2!\n");
	c = getchar();
}