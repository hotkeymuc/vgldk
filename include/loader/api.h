#ifndef __API_H
#define __API_H
/*
	
	Common definitions used in "loader" and "apps"
	
	Both, the "loader" and the "apps" include this file to have common defines.
	A "loader" implements this API and runtime-links "apps" to use its implementation.
	
	This acts as an interface between a relocatable app and run-time loaded stdio of the loader
	
TODO:
	* Re-create usr/share/sdcc/stdio.h:
		?	extern void printf_small (char *,...) _REENTRANT;
		OK	extern int printf (const char *,...);
		*	extern int vprintf (const char *, va_list);
		ok	extern int sprintf (char *, const char *, ...);
		*	extern int vsprintf (char *, const char *, va_list);
		*	extern int puts(const char *);
		OK	extern char *gets(char *);
		OK	extern char getchar(void);
		OK	extern void putchar(char);
		?	extern void printf_fast(__code const char *fmt, ...) _REENTRANT;
		?	extern void printf_fast_f(__code const char *fmt, ...) _REENTRANT;
		?	extern void printf_tiny(__code const char *fmt, ...) _REENTRANT;
	
	OK Add custom "DIR" / dirent.h system browsing functions:
		[ https://en.wikibooks.org/wiki/C_Programming/POSIX_Reference/dirent.h ]
		* DIR struct (DIR *directory)
		* dirent struct: struct dirent *entry: char ->d_name[], ino_t ->d_ino, uint ->d_type
		* opendir, readdir, closedir; seekdir, telldir, rewinddir
	WIP Add custom "FILE" functions:
		* FILE struct
		* fopen, fclose
		* fprintf, fputs, fgets, fseek, ftell
	
	* Add string.h (strcmp, strcopy, strcat, strchr, memset, ...)
	* Add math.h
	* Add float.h
	* Add stdlib.h (atof, atoi, ...)
	+ Add stdarg.h (va_list)
	* Add serial (vgl_softserial)
	
*/

// stdint (kind of...)
#define byte unsigned char
#define word unsigned short

// stdbool
#ifndef true
	#define true 1
	#define false 0
#endif

// stddef
#ifndef NULL
	#define NULL (void *)0
	// size_t, ...
#endif

// Declaration of the start() function inside the app
typedef void (*t_start)(void);
typedef int (*t_main)(int argc, char *argv[]);

// Declaration of commonly used functions
// Note: These are NOT pointers, because these calls will be replaced by the linker!
typedef void (t_putchar)(char);
typedef char (t_getchar)(void);
typedef int (t_printf)(const char*, ...);
typedef int (t_sprintf)(char *, const char*, ...);
typedef char *(t_gets)(char *);
typedef void (t_clear)(void);
typedef void (t_beep)(void);

// Define the whole API as a struct
typedef struct {
	word version;
	void (*putchar)(char);
	char (*getchar)(void);
	int (*printf)(const char*, ...);
	int (*sprintf)(char *, const char*, ...);
	char *(*gets)(char *);
	void (*clear)(void);
	void (*beep)(void);
} t_api;
#endif // __API_H