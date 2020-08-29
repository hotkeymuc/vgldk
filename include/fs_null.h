#ifndef __FS_NULL_H
#define __FS_NULL_H

#include "fs.h"


// Forwards
void fs_null_mount(const char *options);
DIR *fs_null_opendir(const char *path);
int fs_null_closedir(DIR * dir);
dirent *fs_null_readdir(DIR *dir);
FILE *fs_null_fopen(const char *path, const char *openMode);
int fs_null_fclose(FILE *f);
byte fs_null_feof(FILE *f);
int fs_null_fgetc(FILE *f);
size_t fs_null_fread(void *ptr, size_t size, size_t nmemb, FILE *f);
size_t fs_null_fwrite(void *ptr, size_t size, size_t nmemb, FILE *f);


// Publish a FS struct
const FS fs_null = {	// Keep in sync with fs.h:FS!
	fs_null_mount,
	
	fs_null_opendir,
	fs_null_closedir,
	fs_null_readdir,
	
	fs_null_fopen,
	fs_null_fclose,
	fs_null_feof,
	fs_null_fgetc,
	fs_null_fread,
	fs_null_fwrite
};


// Implementation
DIR fs_null_tmpDir;

void fs_null_mount(const char *options) {
	(void)options;
}
DIR *fs_null_opendir(const char *path) {
	DIR * dir;
	
	(void)path;
	dir = &fs_null_tmpDir;
	dir->fs = &fs_null;
	dir->count = 0;	// Empty!
	dir->currentPos = 0;	// Rewind
	
	return dir;
}
int fs_null_closedir(DIR * dir) {
	(void)dir;
	
	return 0;
}
dirent *fs_null_readdir(DIR *dir) {
	dirent *de;
	
	(void)dir;
	de = NULL;
	
	return de;
}
FILE *fs_null_fopen(const char *path, const char *openMode) {
	FILE *f;	// FILE to be returned
	
	(void)path;
	(void)openMode;
	f = NULL;
	
	return f;
}
int fs_null_fclose(FILE *f) {
	(void)f;
	return 0;
}
byte fs_null_feof(FILE *f) {
	(void)f;
	return 1;
}
int fs_null_fgetc(FILE *f) {
	(void)f;
	return -1;	// EOF
}
size_t fs_null_fread(void *ptr, size_t size, size_t nmemb, FILE *f) {
	(void)ptr;
	(void)size;
	(void)nmemb;
	(void)f;
	
	return 0;
}
size_t fs_null_fwrite(void *ptr, size_t size, size_t nmemb, FILE *f) {
	(void)ptr;
	(void)size;
	(void)nmemb;
	(void)f;
	
	return 0;
}

#endif