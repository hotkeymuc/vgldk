#ifndef __FS_NULL_H
#define __FS_NULL_H

#include "fs.h"

void fs_null_mount(const char *options) {
	(void)options;
}
DIR *fs_null_opendir(const char *path) {
	DIR * dir;
	
	(void)path;
	
	return dir;
}
int fs_null_closedir(DIR * dir) {
	return 0;
}
dirent *fs_null_readdir(DIR *dir) {
	dirent *de;
	
	(void)dir;
	
	return de;
}
FILE *fs_null_fopen(const char *path, const char *openMode) {
	FILE *f;	// FILE to be returned
	
	(void)path;
	(void)openMode;
	
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

#endif