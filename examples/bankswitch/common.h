/*

Defines that all segments need

*/

/*
//extern char getchar(void);
extern char keyboard_getchar(void);
#define getchar keyboard_getchar

extern void printf(char *);


// externals from common segment
typedef void (t_bank_call)(void);
extern void bank1_call(unsigned char seg, t_bank_call *call, void *args);
*/

#include "common_addr.h"

typedef void (t_printf)(char *);
#define printf(s) ((t_printf)common_printf_addr)(s)

typedef char (t_getchar)(void);
#define getchar() ((t_getchar)common_keyboard_getchar_addr)()

/*
typedef void (t_bank_call)(void);
typedef void (t_bank_caller)(unsigned char, t_bank_call *, void *);
#define bank1_call(s,c,a) ((t_bank_caller)common_bank1_call_addr)(s,c,a)
*/
typedef void (t_bank_caller)(unsigned char, unsigned int, void *);
#define bank1_call(s,c,a) ((t_bank_caller)common_bank1_call_addr)(s,c,a)