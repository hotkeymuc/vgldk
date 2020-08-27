#ifndef __FS_H
#define __FS_H

/*

File System

"fs.h" sits below "fileio.h" and abstracts the file system away.
This allows for different file systems (internal, FAT, ...)


2019-07-11 Bernhard "HotKey" Slawik

*/

#include "file.h"	// Include definitions for FILE and DIR

// File system abstraction
typedef void (*t_mount)(const char *);

typedef DIR *(*t_opendir)(const char *);
typedef int (*t_closedir)(DIR *);
typedef dirent *(*t_readdir)(DIR *);
//@TODO: t_mkdir
//@TODO: t_rm/rmdir

typedef FILE *(*t_fopen)(const char *, const char *);
typedef int (*t_fclose)(FILE *);
typedef byte (*t_feof)(FILE *);
typedef int (*t_fgetc)(FILE *);
typedef word (*t_fread)(void *, word, byte, FILE *);
typedef word (*t_fwrite)(void *, word, byte, FILE *);
//@TODO: t_ftell
//@TODO: t_fseek

typedef struct {
	t_mount mount;
	
	t_opendir opendir;
	t_closedir closedir;
	t_readdir readdir;
	
	t_fopen fopen;
	t_fclose fclose;
	t_feof feof;
	t_fgetc fgetc;
	t_fread fread;
	t_fwrite fwrite;
} FS;


#endif