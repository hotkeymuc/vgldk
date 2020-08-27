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

#include <parabuddy.h>

// Implementation

byte fs_pb_mounted = 0;

void fs_pb_mount(const char *options) {
	byte ok;
	
	(void)options;	// Do not need them for now
	
	fs_pb_mounted = 0;
	
	// Init ParallelBuddy
	//printf("bf_init()...");
	//bf_init();	// This might crash the emulation
	//printf("OK\n");
	
	// Init SD card via BusBuddy
	printf("sd_init()...");
	ok = pb_sd_init();
	if (ok == 1) {
		printf("OK\n");
		fs_pb_mounted = 1;
		
	} else {
		printf("failed %d.\n", ok);
		fs_pb_mounted = 0;
	}
}


DIR fs_pb_tmpDir;
DIR *fs_pb_opendir(const char *path) {
	
	BB_HANDLE dirHandle;
	DIR * dir;
	
	dir = &fs_pb_tmpDir;
	dir->path = path;
	
	//@TODO: call bf_init / sd_init! Or do that automatically in Arduino sketch?
	
	//@TODO: call bb_cmd_...()
	printf("fs_pb_opendir NOT IMPL!\n");
	
	// SD library handles opendir() like fopen()!
	dirHandle = sd_open(path, BB_FILE_MODE_DIRECTORY);
	
	//@TODO: Handle "dir not found"
	
	/*
    // Arduino side:
    dir = SD.open("/");
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
	*/
	
	dir->userData = (void *)dirHandle;	// Store remote handle
	dir->count = 0;	//(sizeof(FS_INTERNAL_FILES) / sizeof(FILE));
	//rewinddir(dir);
	dir->currentPos = 0;
	
	return dir;
}

int fs_pb_closedir(DIR * dir) {
	BB_HANDLE dirHandle;
	
	// SD open directories are handled like files
	dirHandle = (BB_HANDLE)dir->userData;
	file_close(dirHandle);
	
	// Invalidate
	dir->userData = NULL;
	//dir->currentPos = 0;
	dir->count = 0;
	return 0;
}


dirent fs_pb_tmpDirent;
//struct dirent *readdir(DIR *dir) {
dirent *fs_pb_readdir(DIR *dir) {
	//struct dirent *de;
	dirent *de;
	BB_HANDLE dirHandle;
	BB_HANDLE entryHandle;
	byte isDir;
	size_t size;
	char name[BB_MAX_FILENAME];
	
	// Fetch next file info from driver...
	
	// Get remote handle for that directory
	dirHandle = (BB_HANDLE)dir->userData;
	
	// Get next file in that directory
	entryHandle = sd_openNext(dirHandle);
	
	// End of directory?
	if (entryHandle == BB_NO_HANDLE) {
		// End of dir
		return NULL;
	}
	
	// Get entry info
	file_info(entryHandle, &isDir, &size, &name[0]);
	
	// Create a dirent
	//de = &dir->entries[dir->currentPos];
	de = &fs_pb_tmpDirent;
	de->name = &name[0];
	de->pos = dir->currentPos;
	//de->userData = (void *)entryHandle;
	
	file_close(entryHandle);
	
	dir->currentPos++;
	
	return de;
}



FILE fs_pb_tmpFile;
FILE *fs_pb_fopen(const char *path, const char *openMode) {
	FILE *f;	// FILE to be returned
	
	byte mode;	// for converting openMode string to mode byte
	BB_HANDLE handle;	// Remote handle
	
	
	//@FIXME: Actually use the parent directory of that file (ask DOS)
	//parentDir = "\0";	//path;
	
	mode = BB_FILE_MODE_READ;
	if (strcmp(openMode, "r") == 0) mode = BB_FILE_MODE_READ;
	if (strcmp(openMode, "w") == 0) mode = BB_FILE_MODE_WRITE;	//@TODO: Truncate to empty if it already exists!
	if (strcmp(openMode, "a") == 0) mode = BB_FILE_MODE_WRITE;	//@TODO: Open/create and move to end of file, not seekable
	if (strcmp(openMode, "r+") == 0) mode = BB_FILE_MODE_READ;	//@TODO: Open exsiting file for read and write
	if (strcmp(openMode, "w+") == 0) mode = BB_FILE_MODE_WRITE;	//@TODO: Create empty file and open for read and write
	if (strcmp(openMode, "a+") == 0) mode = BB_FILE_MODE_WRITE;	//@TODO: Seekable, but writes always seek to end and append
	
	//@TODO: If read: Check if file exists!
	// if (sd_exists(path)) {
		printf("File \"%s\" exists=%d\n", sd_exists(path));
	// }
	
	
	// Actually do the call to BusBuddy
	handle = pb_sd_open(path, mode);
	
	// Invalid handle? File not found?
	if (handle == BB_NO_HANDLE) {
		printf("No handle opened!\n");
		errno = ERR_FILE_NOT_FOUND;	// Maybe a different error
		return NULL;
	}
	
	// Copy over some data
	f = &fs_pb_tmpFile;
	
	f->userData = (void *)handle;	// Store remote handle
	
	//f->name = dirf->name;
	f->name = path;
	
	//f->fs = dirf->name;
	//f->userData = dirf->userData;
	//f->size = dirf->size;
	//f->currentPos = 0;
	
	f->mode = openMode;
	f->currentPos = 0;
	
	return f;
}

int fs_pb_fclose(FILE *f) {
	BB_HANDLE handle;
	
	// Close remote file handle
	handle = (BB_HANDLE)f->userData;
	pb_file_close(handle);
	
	// Invalidate
	f->userData = NULL;
	f->currentPos = 0;
	f->size = 0;
	return 0;
}

byte fs_pb_feof(FILE *f) {
	BB_HANDLE handle;
	byte b;
	
	// Get "bytes available for reading" from remote
	handle = (BB_HANDLE)f->userData;
	b = pb_file_bytesAvailable(handle);
	
	if (b <= 0) return 1;
	
	return 0;
}



int fs_pb_fgetc(FILE *f) {
	BB_HANDLE handle;
	byte b;
	byte l;
	
	if (fs_pb_feof(f)) return EOF;
	
	// It is really a bad idea to read each byte individually from SD over the BB protocol!
	// Read SIZE elements of size NMEMB and return how many elements were read
	
	// Read from remote file handle
	handle = (BB_HANDLE)f->userData;
	l = pb_file_read(handle, &b, 1);
	
	if (l <= 0) return EOF;
	
	f->currentPos++;
	
	return b;
}

size_t fs_pb_fread(void *ptr, size_t size, size_t nmemb, FILE *f) {
	// Read SIZE elements of size NMEMB and return how many elements were read
	BB_HANDLE handle;
	byte l;
	
	size *= nmemb;	//@FIXME: This can/will overflow
	
	//@TODO: Check max frame length! Fragment if neccessary!
	
	// Read from remote file handle
	handle = (BB_HANDLE)f->userData;
	l = pb_file_read(handle, (byte *)ptr, size);
	
	return l;
}

size_t fs_pb_fwrite(void *ptr, size_t size, size_t nmemb, FILE *f) {
	BB_HANDLE handle;
	byte l;
	byte *b;
	
	size *= nmemb;	//@FIXME: This can/will overflow
	
	//@TODO: Check max frame length! Fragment if neccessary!
	
	// Write to remote file handle
	handle = (BB_HANDLE)f->userData;
	
	b = (byte *)ptr;
	l = pb_file_write(handle, b, size);
	
	return l;
}


// Publish a FS struct
const FS fs_parabuddy = {	// Keep in sync with fs.h:FS!
	fs_pb_mount,
	
	fs_pb_opendir,
	fs_pb_closedir,
	fs_pb_readdir,
	
	fs_pb_fopen,
	fs_pb_fclose,
	fs_pb_feof,
	fs_pb_fgetc,
	fs_pb_fread,
	fs_pb_fwrite
};

#endif