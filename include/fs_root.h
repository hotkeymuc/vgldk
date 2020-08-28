#ifndef __FS_ROOT_H
#define __FS_ROOT_H

#include "fs.h"

void fs_root_mount(const char *options) {
	(void)options;
}
DIR *fs_root_opendir(const char *path) {
	DIR * dir;
	
	(void)path;
	
	return dir;
}
int fs_root_closedir(DIR * dir) {
	return 0;
}
dirent *fs_root_readdir(DIR *dir) {
	dirent *de;
	
	(void)dir;
	
	return de;
}
FILE *fs_root_fopen(const char *path, const char *openMode) {
	FILE *f;	// FILE to be returned
	
	(void)path;
	(void)openMode;
	
	return f;
}
int fs_root_fclose(FILE *f) {
	(void)f;
	return 0;
}
byte fs_root_feof(FILE *f) {
	(void)f;
	return 1;
}
int fs_root_fgetc(FILE *f) {
	return -1;	// EOF
}
size_t fs_root_fread(void *ptr, size_t size, size_t nmemb, FILE *f) {
	(void)ptr;
	(void)size;
	(void)nmemb;
	(void)f;
	
	return 0;
}
size_t fs_root_fwrite(void *ptr, size_t size, size_t nmemb, FILE *f) {
	(void)ptr;
	(void)size;
	(void)nmemb;
	(void)f;
	
	return 0;
}



// Publish a FS struct
const FS fs_root = {	// Keep in sync with fs.h:FS!
	fs_root_mount,
	
	fs_root_opendir,
	fs_root_closedir,
	fs_root_readdir,
	
	fs_root_fopen,
	fs_root_fclose,
	fs_root_feof,
	fs_root_fgetc,
	fs_root_fread,
	fs_root_fwrite
};

#endif