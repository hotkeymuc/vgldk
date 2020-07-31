
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

void main1() {
	char c;
	
	//while(1) { }
	printf("This is segment1 main\n");
	c = getchar();
}

void test_segment1() {
	char c;
	printf("test_segment1!\n");
	c = getchar();
}
