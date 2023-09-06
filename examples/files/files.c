/*
	Testing the fileio functions
	
*/

#include <vgldk.h>

#include <stdiomin.h>
#include <hex.h>

/*
// Use SDCC's stdio
//#define VGLDK_STDOUT_PUTCHAR lcd_putchar
//#define VGLDK_STDIN_GETCHAR keyboard_getchar
//#define putchar VGLDK_STDOUT_PUTCHAR
//#define getchar VGLDK_STDIN_GETCHAR
int getchar(void) {
	return VGLDK_STDIN_GETCHAR();
}
int putchar(int c) {
	VGLDK_STDOUT_PUTCHAR(c);
	return 1;
}
#include <stdio.h>
*/


// File I/O setup
#define FILES_BUF_SIZE 32

/*
// FS:null
#include <fs_null.h>
*/

/*
// FS: internal
//#include "../app/out/app_hello.app.0xc800.bin.h"
#define APP_HELLO_DATA "hellorld!"	//{'h','e','l','l','o','r','l','d','!'}
//#define FS_INTERNAL_CASE_SENSITIVE
#define FS_INTERNAL_NAME "hello"
#define FS_INTERNAL_DATA APP_HELLO_DATA
#include <fs_internal.h>
*/

/*
// FS: parabuddy
#define PB_USE_MAME	// For testing in MAME
//#define PB_USE_SOFTSERIAL	// For running on real hardware
//#define PB_USE_SOFTUART	// For running on real hardware
//#define PB_DEBUG_FRAMES
//#define PB_DEBUG_PROTOCOL_ERRORS
#include <driver/parabuddy.h>
#include <fs_parabuddy.h>
*/

// FS: host
#define HOST_DRIVER_MAME
#define HOST_PROTOCOL_BINARY
#include <fs_host.h>


/*
// FS: root
// Define mounts for the root filesystem (automatically, as macros)
#ifdef __FS_NULL_H
  #define FS_ROOT_MOUNT__NUL {"nul", &fs_null},
#else
  #define FS_ROOT_MOUNT__NUL
#endif
#ifdef __FS_INTERNAL_H
  #define FS_ROOT_MOUNT__INT  {"int", &fs_internal},
#else
  #define FS_ROOT_MOUNT__INT
#endif
#ifdef __FS_PARABUDDY_H
  #define FS_ROOT_MOUNT__PB  {"pb", &fs_parabuddy},
#else
  #define FS_ROOT_MOUNT__PB
#endif


// Create the full list of available file system mounts
#define FS_ROOT_MOUNTS {\
	FS_ROOT_MOUNT__NUL \
	FS_ROOT_MOUNT__INT \
	FS_ROOT_MOUNT__PB  \
}
#include <fs_root.h>
*/

// Specify which FS to use as the root (usually fs_root, but can be fs_int to save space)
//#define FILEIO_ROOT_FS fs_internal
//#define FILEIO_ROOT_FS fs_root
#define FILEIO_ROOT_FS fs_host
#include <fileio.h>

volatile char buf[FILES_BUF_SIZE];

void main() __naked {
	file_FILE *f;
	size_t l;
	
	
	//printf("Hello World!");
	
	// DIR
	puts("DIR:");
	//absPath("", cwd);
	file_DIR *dir;
	dirent *de;
	
	dir = opendir(cwd);
	if (dir == NULL) puts("dir not found!");	//return errno;
	
	while (de = readdir(dir)) {
		puts((char *)de->name);
		//putchar(' ');
		//printf_d(de->size);
		//putchar('\n');
	}
	//putchar('\n');
	closedir(dir);
	
	// CAT
	puts("CAT:");
	// Open file
	f = fopen("hello", "r");
	if (f == NULL) puts("not found!");	//return errno;
	
	// Read file buffer-by-buffer
	//do {
	while(feof(f) == 0) {
		l = fread(&buf[0], 1, FILES_BUF_SIZE-1, f);
		if (l <= 0) break;
		/*
		printf("Read "); printf_x2(l); printf(" bytes: ");
		for(byte i = 0; i < l; i++) {
			if (i > 0) printf(", ");
			printf_x2(i);
			putchar('/');
			printf_x2(l);
			putchar(':');
			printf_x2(buf[i]);
			getchar();
		}
		*/
		
		if (l > 0) {
			buf[l] = 0;	// Terminate string
			printf(buf);
		}
	};
	//} while (l > 0);
	fclose(f);
	
	printf("EOF");
	while(1) { }
}

