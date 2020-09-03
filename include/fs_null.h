#ifndef __FS_NULL_H
#define __FS_NULL_H

#include "fs.h"

// Forwards
/*
void fs_null_mount(const char *options);
file_DIR *fs_null_opendir(const char *path);
int fs_null_closedir(file_DIR * dir);
dirent *fs_null_readdir(file_DIR *dir);
file_FILE *fs_null_fopen(const char *path, const char *openMode);
int fs_null_fclose(file_FILE *f);
byte fs_null_feof(file_FILE *f);
int fs_null_fgetc(file_FILE *f);
size_t fs_null_fread(void *ptr, size_t size, size_t nmemb, file_FILE *f);
size_t fs_null_fwrite(void *ptr, size_t size, size_t nmemb, file_FILE *f);
*/

// Publish a FS struct
const FS fs_null = {	// Keep in sync with fs.h:FS!
	NULL,	//fs_null_mount,
	
	NULL,	//fs_null_opendir,
	NULL,	//fs_null_closedir,
	NULL,	//fs_null_readdir,
	
	NULL,	//fs_null_fopen,
	NULL,	//fs_null_fclose,
	NULL,	//fs_null_feof,
	//NULL,	//fs_null_fgetc,
	NULL,	//fs_null_fread,
	NULL,	//fs_null_fwrite
};


// Implementation
/*
file_DIR fs_null_tmpDir;

void fs_null_mount(const char *options) {
	(void)options;
}
file_DIR *fs_null_opendir(const char *path) {
	file_DIR * dir;
	
	(void)path;
	dir = &fs_null_tmpDir;
	dir->fs = &fs_null;
	dir->count = 0;	// Empty!
	dir->currentPos = 0;	// Rewind
	
	return dir;
}
int fs_null_closedir(file_DIR * dir) {
	(void)dir;
	
	return 0;
}
dirent *fs_null_readdir(file_DIR *dir) {
	dirent *de;
	
	(void)dir;
	de = NULL;
	
	return de;
}
file_FILE *fs_null_fopen(const char *path, const char *openMode) {
	file_FILE *f;	// file_FILE to be returned
	
	(void)path;
	(void)openMode;
	f = NULL;
	
	return f;
}
int fs_null_fclose(file_FILE *f) {
	(void)f;
	return 0;
}
byte fs_null_feof(file_FILE *f) {
	(void)f;
	return 1;
}
int fs_null_fgetc(file_FILE *f) {
	(void)f;
	return -1;	// EOF
}
size_t fs_null_fread(void *ptr, size_t size, size_t nmemb, file_FILE *f) {
	(void)ptr;
	(void)size;
	(void)nmemb;
	(void)f;
	
	return 0;
}
size_t fs_null_fwrite(void *ptr, size_t size, size_t nmemb, file_FILE *f) {
	(void)ptr;
	(void)size;
	(void)nmemb;
	(void)f;
	
	return 0;
}
*/

#endif
