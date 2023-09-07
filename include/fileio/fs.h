#ifndef __FS_H
#define __FS_H

/*

File System

"fs.h" sits below "fileio.h" and abstracts the file system away.
This allows for different file systems (internal, FAT, ...)


2019-07-11 Bernhard "HotKey" Slawik

*/

#include "file.h"	// Include definitions for file_FILE and file_DIR

// File system abstraction
typedef void (*t_mount)(const char *);

typedef file_DIR *(*t_opendir)(const char *);
typedef int (*t_closedir)(file_DIR *);
typedef dirent *(*t_readdir)(file_DIR *);
//@TODO: t_mkdir
//@TODO: t_rm/rmdir

typedef file_FILE *(*t_fopen)(const char *, const char *);
typedef int (*t_fclose)(file_FILE *);
typedef byte (*t_feof)(file_FILE *);
//typedef int (*t_fgetc)(file_FILE *);
typedef size_t (*t_fread)(void *, size_t, size_t, file_FILE *);
typedef size_t (*t_fwrite)(void *, size_t, size_t, file_FILE *);
//@TODO: t_ftell
//@TODO: t_fseek

typedef struct {
	t_mount mount;
	
	t_opendir opendir;
	t_closedir closedir;
	t_readdir readdir;
	
	t_fopen open;
	t_fclose close;
	t_feof eof;
	//t_fgetc getc;
	t_fread read;
	t_fwrite write;
} FS;


#endif
