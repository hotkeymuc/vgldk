#ifndef __FS_BUSBUDDY_H
#define __FS_BUSBUDDY_H

/*

fs.h compatible File System using Parallel Buddy
================================================

Implementation of fs.h using the "Parallel Buddy" Arduino connected to the VGL parallel port

----> WORK IN PROGRESS!

2020-08-02 Bernhard "HotKey" Slawik (vgldk.git)

*/

#include "file.h"

#define FS_PARABUDDY_MAX_PATH 32
#include <parabuddy.h>


// Forwards
void fs_pb_mount(const char *options);
file_DIR *fs_pb_opendir(const char *path);
int fs_pb_closedir(file_DIR * dir);
dirent *fs_pb_readdir(file_DIR *dir);
file_FILE *fs_pb_fopen(const char *path, const char *mode);
int fs_pb_fclose(file_FILE *f);
byte fs_pb_feof(file_FILE *f);
int fs_pb_fgetc(file_FILE *f);
size_t fs_pb_fread(void *ptr, size_t size, size_t nmemb, file_FILE *f);
size_t fs_pb_fwrite(void *ptr, size_t size, size_t nmemb, file_FILE *f);


// Publish a FS struct
const FS fs_parabuddy = {	// Keep in sync with fs.h:FS!
	fs_pb_mount,
	
	fs_pb_opendir,
	fs_pb_closedir,
	fs_pb_readdir,
	
	fs_pb_fopen,
	fs_pb_fclose,
	fs_pb_feof,
	//fs_pb_fgetc,
	fs_pb_fread,
	fs_pb_fwrite
};


// Implementation
byte fs_pb_mounted;
file_DIR fs_pb_tmpDir;
dirent fs_pb_tmpDirent;
char fs_pb_tmpName[FS_PARABUDDY_MAX_PATH];
file_FILE fs_pb_tmpFile;

void fs_pb_mount(const char *options) {
	//byte ok;
	
	(void)options;	// Do not need them for now
	
	fs_pb_mounted = 0;
	
	/*
	// Init ParallelBuddy
	//printf("bf_init()...");
	//bf_init();	// This might crash the emulation
	//printf("OK\n");
	
	// Init SD card via BusBuddy
	puts("sd_init()...");
	ok = pb_sd_init();
	if (ok == 1) {
		puts("OK\n");
		fs_pb_mounted = 1;
		
	} else {
		//printf("failed %d.\n", ok);
		puts("failed "); printf_d(ok); putchar('\n');
		fs_pb_mounted = 0;
	}
	*/
}


file_DIR *fs_pb_opendir(const char *path) {
	pb_handle h;
	file_DIR * dir;
	
	h = pb_file_opendir(path);
	if (h == PB_NO_HANDLE) {
		errno = ERR_FILE_NOT_FOUND;	// Or something
		return NULL;
	}
	
	dir = &fs_pb_tmpDir;	//@FIXME: Using the ONE temp. dir...
	dir->fs = &fs_parabuddy;
	//dir->path = path;	// This can lead to weird consequences
	
	dir->userData = (void *)h;	// Store remote handle
	dir->count = 0;	// not known in advance...
	dir->currentPos = 0;	// Rewind
	
	return dir;
}

int fs_pb_closedir(file_DIR * dir) {
	pb_handle h;
	
	h = (pb_handle)dir->userData;
	pb_file_closedir(h);
	
	// Invalidate
	dir->userData = NULL;
	//dir->currentPos = 0;
	dir->count = 0;
	return 0;
}

dirent *fs_pb_readdir(file_DIR *dir) {
	dirent *de;
	pb_handle h;
	
	// Fetch next file info from driver...
	h = (pb_handle)dir->userData;
	
	// Request via ParallelBuddy
	pb_file_readdir(h, &fs_pb_tmpName[0]);
	
	if (fs_pb_tmpName[0] == 0)
		return NULL;
	
	// Create a dirent
	de = &fs_pb_tmpDirent;	//@FIXME: Using the ONE temp. dir.ent..
	de->pos = dir->currentPos;
	de->name = &fs_pb_tmpName[0];
	//de->userData = NULL;
	
	dir->currentPos++;
	
	return de;
}


file_FILE *fs_pb_fopen(const char *path, const char *mode) {
	file_FILE *f;	// file_FILE to be returned
	pb_handle h;	// Remote handle
	
	// Actually do the call to BusBuddy
	h = pb_file_open(path, mode);
	
	// Invalid handle? File not found?
	if (h == PB_NO_HANDLE) {
		errno = ERR_FILE_NOT_FOUND;	// Maybe a different error
		return NULL;
	}
	
	// Copy over some data
	f = &fs_pb_tmpFile;
	
	f->name = path;
	f->mode = mode;
	f->fs = &fs_parabuddy;
	f->userData = (void *)h;	// Store remote handle
	//f->size = dirf->size;
	f->currentPos = 0;
	
	return f;
}

int fs_pb_fclose(file_FILE *f) {
	pb_handle h;
	
	// Close remote file handle
	h = (pb_handle)f->userData;
	pb_file_close(h);
	
	// Invalidate
	f->userData = NULL;
	f->currentPos = 0;
	f->size = 0;
	return 0;
}

byte fs_pb_feof(file_FILE *f) {
	pb_handle h;
	//byte b;
	
	// Get "bytes available for reading" from remote
	h = (pb_handle)f->userData;
	//b = pb_file_bytesAvailable(h);
	//if (b <= 0) return 1;
	//return 0;
	return pb_file_eof(h);
}



int fs_pb_fgetc(file_FILE *f) {
	pb_handle h;
	byte b;
	byte l;
	
	if (fs_pb_feof(f)) return -1;	//EOF;
	
	// It is really a bad idea to read each byte individually from SD over the BB protocol!
	// Read SIZE elements of size NMEMB and return how many elements were read
	
	// Read from remote file handle
	h = (pb_handle)f->userData;
	l = pb_file_read(h, &b, 1);
	
	if (l <= 0) return -1;	//EOF;
	
	f->currentPos++;
	
	return b;
}

size_t fs_pb_fread(void *ptr, size_t size, size_t nmemb, file_FILE *f) {
	// Read SIZE elements of size NMEMB and return how many elements were read
	pb_handle h;
	byte l;
	
	size *= nmemb;	//@FIXME: This can/will overflow
	
	//@TODO: Check max frame length! Fragment if neccessary!
	
	// Read from remote file handle
	h = (pb_handle)f->userData;
	l = pb_file_read(h, (byte *)ptr, size);
	
	return l;
}

size_t fs_pb_fwrite(void *ptr, size_t size, size_t nmemb, file_FILE *f) {
	pb_handle h;
	byte l;
	byte *b;
	
	size *= nmemb;	//@FIXME: This can/will overflow
	
	//@TODO: Check max frame length! Fragment if neccessary!
	
	// Write to remote file handle
	h = (pb_handle)f->userData;
	
	b = (byte *)ptr;
	l = pb_file_write(h, b, size);
	
	return l;
}

#endif