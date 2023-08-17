#ifndef __BASICTYPES_H__
#define __BASICTYPES_H__

//#define byte unsigned char
//#define word unsigned short
typedef unsigned char byte;
typedef unsigned short word;

typedef byte * p_byte;
typedef char * p_char;

#define true 1
#define false 0

#define NULL ((void *)0)
//const void *NULL = (void *)0;

#endif // __BASICTYPES_H__