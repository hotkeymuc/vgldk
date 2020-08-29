#ifndef __FS_INTERNAL_H
#define __FS_INTERNAL_H
/*

Internal File System
====================

Implementation of fs.h using hard-coded data. Mainly used for testing.


2019-07-11 Bernhard "HotKey" Slawik
*/

#include "fs.h"

//#define FS_INTERNAL_INCLUDE_TEST_APPS	// For testing: Include external app(s) from "loader" folder as virtual FS
//#define FS_INTERNAL_CASE_SENSITIVE


// Internal fs disk
// Define some static test files. Only for testing low-level file access.
//const char TEST_CONTENTS[] = "This is contents of test.txt, hard-baked into the DOS code!";
const char TEST_CONTENTS[] = "TEST_CONTENTS!\nLine2\n";
const char AUTO_CMD_CONTENTS[] = "echo \"This is %TEST% auto.cmd!\"\n";

#ifdef FS_INTERNAL_INCLUDE_TEST_APPS
	#include "../loader/out/app_test.app.h"
	#include "../loader/out/app_hello.app.h"
#endif

const FILE FS_INTERNAL_FILES[] = {	// Keep in sync with file.h:FILE!
	{
		"auto.cmd",
		NULL,
		NULL,
		&AUTO_CMD_CONTENTS[0],
		sizeof(AUTO_CMD_CONTENTS),
		0
	},
	{
		"test.txt",	// name
		NULL,	// mode
		NULL,	// fs
		&TEST_CONTENTS[0],	// userData
		sizeof(TEST_CONTENTS),	// size
		0	// currentPos
	},
#ifdef FS_INTERNAL_INCLUDE_TEST_APPS
	{
		"test.app",
		NULL,
		NULL,
		&APP_TEST[0],
		sizeof(APP_TEST),
		0
	},
	{
		"hello.app",
		NULL,
		NULL,
		&APP_HELLO[0],
		sizeof(APP_HELLO),
		0
	}
#endif
};
#define FS_INTERNAL_FILES_COUNT (sizeof(FS_INTERNAL_FILES) / sizeof(FILE))


// Forwards
void fs_int_mount(const char *options);
DIR *fs_int_opendir(const char *path);
int fs_int_closedir(DIR * dir);
dirent *fs_int_readdir(DIR *dir);
FILE *fs_int_fopen(const char *path, const char *openMode);
int fs_int_fclose(FILE *f);
byte fs_int_feof(FILE *f);
int fs_int_fgetc(FILE *f);
size_t fs_int_fread(void *ptr, size_t size, size_t nmemb, FILE *f);
size_t fs_int_fwrite(void *ptr, size_t size, size_t nmemb, FILE *f);


// Publish a FS struct
const FS fs_internal = {	// Keep in sync with fs.h:FS!
	fs_int_mount,
	
	fs_int_opendir,
	fs_int_closedir,
	fs_int_readdir,
	
	fs_int_fopen,
	fs_int_fclose,
	fs_int_feof,
	fs_int_fgetc,
	fs_int_fread,
	fs_int_fwrite
};


// Implementation
DIR fs_int_tmpDir;
dirent fs_int_tmpDirent;
FILE fs_int_tmpFile;

void fs_int_mount(const char *options) {
	// Nothing to do here for now
	(void)options;
}

DIR *fs_int_opendir(const char *path) {
	DIR * dir;
	const char *pp;
	
	pp = path;
	if (*pp == FILE_PATH_DELIMITER) pp++;	// Skip initial slash
	
	dir = &fs_int_tmpDir;	//@FIXME: Using the ONE temp. dir...
	dir->fs = &fs_internal;
	//dir->path = path;	// this can have weird behaviour
	
	// Only root directory is allowed for now
	if (*pp != 0) {
		//printf("Only root dir!\n");
		errno = ERR_FILE_NOT_FOUND;
		return NULL;
	}
	
	dir->userData = (FILE *)&FS_INTERNAL_FILES[0];
	dir->count = FS_INTERNAL_FILES_COUNT;
	
	dir->currentPos = 0;	// Rewind
	return dir;
}

int fs_int_closedir(DIR * dir) {
	//dir->currentPos = 0;	// or something
	dir->count = 0;
	return 0;
}

dirent *fs_int_readdir(DIR *dir) {
	dirent *de;
	FILE *f;
	FILE *files;
	
	if (dir->currentPos >= dir->count) {
		// Done.
		return NULL;
	}
	
	// Create a dirent
	//de = &dir->entries[dir->currentPos];
	de = &fs_int_tmpDirent;		//@FIXME: Using the ONE temp. dir.ent..
	
	de->pos = dir->currentPos;
	
	files = (FILE *)dir->userData;
	f = &files[dir->currentPos];
	de->name = f->name;
	de->userData = f->userData;	// Only possible on internal fs, since data is already in memory
	
	dir->currentPos++;
	
	return de;
}

FILE *fs_int_fopen(const char *path, const char *openMode) {
	FILE *f;	// FILE to be returned
	
	// For searching
	const char *pp;
	const byte *parentDir;
	FILE *files;
	FILE *dirf;
	DIR *dir;
	dirent *de;
	
	// Scan directory for the given file
	pp = path;
	if (*pp == FILE_PATH_DELIMITER) pp++;	// Skip initial slash
	
	f = NULL;
	
	//@FIXME: Actually use the parent directory of that file (ask DOS)
	parentDir = "\0";	//path;
	
	dir = fs_int_opendir(parentDir);
	if (dir == NULL) return NULL;
	
	files = (FILE *)dir->userData;
	
	while(de = fs_int_readdir(dir)) {
		#ifdef FS_INTERNAL_CASE_SENSITIVE
		if (strcmp(path, de->name) == 0) {
		#else
		if (stricmp(pp, de->name) == 0) {
		#endif
			//@FIXME: Pointer madness...
			dirf = &files[de->pos];
			
			// Copy over
			f = &fs_int_tmpFile;		//@FIXME: Using the ONE temp. file...
			f->name = dirf->name;
			//f->fs = dirf->name;
			f->userData = dirf->userData;
			f->size = dirf->size;
			//f->currentPos = 0;
			break;
		}
	}
	fs_int_closedir(dir);
	
	if (f == NULL) {
		//printf("Not found!\n");
		errno = ERR_FILE_NOT_FOUND;
		return NULL;
	}
	
	f->fs = &fs_internal;
	f->mode = openMode;
	f->currentPos = 0;
	
	return f;
}

int fs_int_fclose(FILE *f) {
	// ...
	f->currentPos = 0;
	f->size = 0;
	return 0;
}

byte fs_int_feof(FILE *f) {
	if (f->currentPos >= f->size) return 1;
	return 0;
}

int fs_int_fgetc(FILE *f) {
	int b;
	
	if (fs_int_feof(f)) return -1;	//EOF;
	
	// This is only possible on internal fs
	b = ((byte *)f->userData)[f->currentPos];
	
	f->currentPos++;
	
	return b;
}

size_t fs_int_fread(void *ptr, size_t size, size_t nmemb, FILE *f) {
	// Read SIZE elements of size NMEMB and return how many elements were read
	byte b;
	word l;
	byte *d;
	
	size *= nmemb;	//@FIXME: This can/will overflow
	
	l = 0;
	d = (byte *)ptr;
	while ((size > 0) && (f->currentPos < f->size)) {
		b = ((byte *)f->userData)[f->currentPos++];
		*((byte *)d++) = b;
		l++;
		size--;
	}
	
	return l;
}

size_t fs_int_fwrite(void *ptr, size_t size, size_t nmemb, FILE *f) {
	(void)ptr;
	(void)size;
	(void)nmemb;
	(void)f;
	
	printf("fs_int is RO!\n");
	return 0;
}

#endif