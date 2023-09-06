#ifndef __FS_HOST_H
#define __FS_HOST_H

/*

fs.h compatible File System using Host.py
================================================

Implementation of fs.h communicating with host.py

2023-09-05 Bernhard "HotKey" Slawik (vgldk.git)

*/

#include "fs.h"

#include <driver/host.h>

#include <stringmin.h>
#define FS_HOST_MAX_PATH 32

#define FS_HOST_COMMAND_RETURN_OK 0x06	//ACK
#define FS_HOST_COMMAND_RETURN_BYTE 0x11
#define FS_HOST_COMMAND_RETURN_WORD 0x12
#define FS_HOST_COMMAND_RETURN_ASCIIZ 0x13
#define FS_HOST_COMMAND_RETURN_DATA 0x14
#define FS_HOST_COMMAND_RETURN_NACK 0x15	// NAK

// Functionality
#define FS_HOST_COMMAND_END_BOOTLOADER 0x1A

#define FS_HOST_COMMAND_PING 0xE0
//#define FS_HOST_COMMAND_PING_HOST 0xE1

//#define FS_HOST_COMMAND_SD_INIT 0x20
//#define FS_HOST_COMMAND_SD_EXISTS 0x21
//#define FS_HOST_COMMAND_SD_OPEN 0x22
//#define FS_HOST_COMMAND_SD_CLOSE 0x23
//#define FS_HOST_COMMAND_SD_REMOVE 0x4
//#define FS_HOST_COMMAND_SD_MKDIR 0x25
//#define FS_HOST_COMMAND_SD_RMDIR 0x26

#define FS_HOST_COMMAND_FILE_OPENDIR 0x30
#define FS_HOST_COMMAND_FILE_READDIR 0x31
#define FS_HOST_COMMAND_FILE_CLOSEDIR 0x32

#define FS_HOST_COMMAND_FILE_OPEN 0x40
#define FS_HOST_COMMAND_FILE_CLOSE 0x41
#define FS_HOST_COMMAND_FILE_EOF 0x42
#define FS_HOST_COMMAND_FILE_READ 0x43
#define FS_HOST_COMMAND_FILE_WRITE 0x44
//#define FS_HOST_COMMAND_FILE_SEEK 0x45
//#define FS_HOST_COMMAND_FILE_SIZE 0x46
#define FS_HOST_COMMAND_FILE_AVAILABLE 0x47


#define FS_HOST_ERROR_OK 0x00
#define FS_HOST_ERROR_UNKNOWN 0x01
#define FS_HOST_ERROR_LENGTH 0x02
#define FS_HOST_ERROR_TIMEOUT 0x03
#define FS_HOST_ERROR_CORRUPT 0x04

#define FS_HOST_FILE_READ 0
#define FS_HOST_FILE_WRITE 1

#define FS_HOST_NO_HANDLE 0xff
typedef byte fs_host_handle;	// Remote handle (used for identification inside host)


// Forwards
void fs_host_mount(const char *options);
file_DIR *fs_host_opendir(const char *path);
int fs_host_closedir(file_DIR * dir);
dirent *fs_host_readdir(file_DIR *dir);
file_FILE *fs_host_fopen(const char *path, const char *mode);
int fs_host_fclose(file_FILE *f);
byte fs_host_feof(file_FILE *f);
int fs_host_fgetc(file_FILE *f);
size_t fs_host_fread(void *ptr, size_t size, size_t nmemb, file_FILE *f);
size_t fs_host_fwrite(void *ptr, size_t size, size_t nmemb, file_FILE *f);

// Publish a FS struct
const FS fs_host = {	// Keep in sync with fs.h:FS!
	fs_host_mount,
	
	fs_host_opendir,
	fs_host_closedir,
	fs_host_readdir,
	
	fs_host_fopen,
	fs_host_fclose,
	fs_host_feof,
	//fs_host_fgetc,
	fs_host_fread,
	fs_host_fwrite
};


// Implementation
//byte fs_host_mounted;
file_DIR fs_host_tmpDir;
dirent fs_host_tmpDirent;
char fs_host_tmpName[FS_HOST_MAX_PATH];
file_FILE fs_host_tmpFile;


byte tmpFrame[HOST_MAX_DATA];

void fs_host_send_frame(byte cmd, byte *v, const byte l) {
	byte *f;
	
	f = &tmpFrame[0];
	*f++ = 1+l;	// 1=cmd, x=dataLen
	*f++ = cmd;
	
	// Copy from buffer v to mem f
	memcpy(f, v, l);
	
	host_send_frame(&tmpFrame[0], 1+1+l);	// 1=len, 1=cmd, x=dataLen
}
void fs_host_sendByte(byte cmd, byte v) {
	fs_host_send_frame(cmd, &v, 1);
}
void fs_host_sendWord(byte cmd, word v) {
	fs_host_send_frame(cmd, (byte *)&v, 2);
}
void fs_host_sendAsciiz(byte cmd, const char *v) {
	byte l;
	const char *c;
	
	// Get length of zero-terminated string
	c = v;
	l = 0;
	while(*c++ != 0) l++;
	
	// Send it
	fs_host_send_frame(cmd, (byte *)v, l);
}





// FS stuff

void fs_host_mount(const char *options) {
	//byte ok;
	
	(void)options;	// Do not need them for now
	
	//fs_host_mounted = 0;
	
}


file_DIR *fs_host_opendir(const char *path) {
	fs_host_handle h;
	file_DIR * dir;
	
	fs_host_sendAsciiz(FS_HOST_COMMAND_FILE_OPENDIR, (char *)path);
	h = (fs_host_receiveByte(&h) == FS_HOST_ERROR_OK) ? h : FS_HOST_NO_HANDLE;
	
	if (h == FS_HOST_NO_HANDLE) {
		errno = ERR_FILE_NOT_FOUND;	// Or something
		return NULL;
	}
	
	dir = &fs_host_tmpDir;	//@FIXME: Currently using only "the ONE" temp. dir...
	dir->fs = &fs_host;
	//dir->path = path;	// This can lead to weird consequences
	
	dir->userData = (void *)h;	// Store remote handle
	dir->count = 0;	// not known in advance...
	dir->currentPos = 0;	// Rewind
	
	return dir;
}

int fs_host_closedir(file_DIR * dir) {
	fs_host_handle h;
	
	h = (fs_host_handle)dir->userData;
	fs_host_file_closedir(h);
	
	// Invalidate
	dir->userData = NULL;
	//dir->currentPos = 0;
	dir->count = 0;
	return 0;
}

dirent *fs_host_readdir(file_DIR *dir) {
	dirent *de;
	fs_host_handle h;
	
	// Fetch next file info from driver...
	h = (fs_host_handle)dir->userData;
	
	// Request via ParallelBuddy
	fs_host_file_readdir(h, &fs_host_tmpName[0]);
	
	if (fs_host_tmpName[0] == 0)
		return NULL;
	
	// Create a dirent
	de = &fs_host_tmpDirent;	//@FIXME: Using the ONE temp. dir.ent..
	de->pos = dir->currentPos;
	de->name = &fs_host_tmpName[0];
	//de->userData = NULL;
	
	dir->currentPos++;
	
	return de;
}


file_FILE *fs_host_fopen(const char *path, const char *mode) {
	file_FILE *f;	// file_FILE to be returned
	fs_host_handle h;	// Remote handle
	
	// Actually do the call to BusBuddy
	h = fs_host_file_open(path, mode);
	
	// Invalid handle? File not found?
	if (h == PB_NO_HANDLE) {
		errno = ERR_FILE_NOT_FOUND;	// Maybe a different error
		return NULL;
	}
	
	// Copy over some data
	f = &fs_host_tmpFile;
	
	f->name = path;
	f->mode = mode;
	f->fs = &fs_parabuddy;
	f->userData = (void *)h;	// Store remote handle
	//f->size = dirf->size;
	f->currentPos = 0;
	
	return f;
}

int fs_host_fclose(file_FILE *f) {
	fs_host_handle h;
	
	// Close remote file handle
	h = (fs_host_handle)f->userData;
	fs_host_file_close(h);
	
	// Invalidate
	f->userData = NULL;
	f->currentPos = 0;
	f->size = 0;
	return 0;
}

byte fs_host_feof(file_FILE *f) {
	fs_host_handle h;
	//byte b;
	
	// Get "bytes available for reading" from remote
	h = (fs_host_handle)f->userData;
	//b = fs_host_file_bytesAvailable(h);
	//if (b <= 0) return 1;
	//return 0;
	return fs_host_file_eof(h);
}



int fs_host_fgetc(file_FILE *f) {
	fs_host_handle h;
	byte b;
	byte l;
	
	if (fs_host_feof(f)) return -1;	//EOF;
	
	// It is really a bad idea to read each byte individually from SD over the BB protocol!
	// Read SIZE elements of size NMEMB and return how many elements were read
	
	// Read from remote file handle
	h = (fs_host_handle)f->userData;
	l = fs_host_file_read(h, &b, 1);
	
	if (l <= 0) return -1;	//EOF;
	
	f->currentPos++;
	
	return b;
}

size_t fs_host_fread(void *ptr, size_t size, size_t nmemb, file_FILE *f) {
	// Read SIZE elements of size NMEMB and return how many elements were read
	fs_host_handle h;
	byte l;
	
	size *= nmemb;	//@FIXME: This can/will overflow
	
	//@TODO: Check max frame length! Fragment if neccessary!
	
	// Read from remote file handle
	h = (fs_host_handle)f->userData;
	l = fs_host_file_read(h, (byte *)ptr, size);
	
	return l;
}

size_t fs_host_fwrite(void *ptr, size_t size, size_t nmemb, file_FILE *f) {
	fs_host_handle h;
	byte l;
	byte *b;
	
	size *= nmemb;	//@FIXME: This can/will overflow
	
	//@TODO: Check max frame length! Fragment if neccessary!
	
	// Write to remote file handle
	h = (fs_host_handle)f->userData;
	
	b = (byte *)ptr;
	l = fs_host_file_write(h, b, size);
	
	return l;
}

#endif