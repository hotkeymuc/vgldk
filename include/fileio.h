#ifndef __FILEIO_H
#define __FILEIO_H
/*

File I/O

DOS specific file handling functions.
These are high level abstraction for file.h and fs.h

TODO:
	* fputs()
	* "fexists()"
	* "mkdir()"...

2019-07-13 Bernhard "HotKey" Slawik

*/

#include "fs.h"

#ifndef FILEIO_MAX_PATH
	#define FILEIO_MAX_PATH 32	// Maximum number of characters in path names
#endif

//FS *cfs;	// Current file system driver
char cwd[FILEIO_MAX_PATH];	// Current working directory

#ifndef FILEIO_ROOT_FS
	#warning "No root filesystem specified for fileio.h (FILEIO_ROOT_FS). Using fs_null."
	#include "fs_null.h"
	#define FILEIO_ROOT_FS fs_null
#endif

#define fileio_root_fs FILEIO_ROOT_FS


void absPath(const char *relPath, char *ret) {
	const char *bRelPath;
	char *bRet;
	char *t;
	
	bRelPath = relPath;
	bRet = ret;
	
	//@FIXME: This is a piece of s###t!
	
	if ((relPath == NULL) || (*bRelPath == 0)) {
		// Handle "empty string"
		// Just use CWD
		relPath = &cwd[0];
		
	} else
	if (*bRelPath == FILE_PATH_DELIMITER) {
		// Handle absolute path
		// nothing to do, relative => absolute
		
	} else {
		// Handle relative path
		
		// Start with CWD
		t = &cwd[0];
		while (*t != 0) {
			*bRet++ = *t++;
		}
		if (*(t-1) != FILE_PATH_DELIMITER) *bRet++ = FILE_PATH_DELIMITER;
		// Continue with relative path
	}
	
	// Copy the rest of relative path
	while (*bRelPath != 0) {
		if ((bRelPath > relPath) && (*(bRelPath-1) == '.') && (*bRelPath == '.')) {
			// Handle ".."
			while (*bRelPath == '.') bRelPath++;	// Skip the second "." in input
			
			// Go back to last delimiter
			bRet--;	// Back to previous written "."
			bRet--;	// Back to last valid char
			if (bRet > ret) {
				if (*bRet == FILE_PATH_DELIMITER) bRet--;	// Back off last delimiter
				while((*bRet != FILE_PATH_DELIMITER) && (bRet > ret)) bRet--;	// Back to last delimiter/start
			}
			*bRet = 0;	// Terminate new end
			continue;
		}
		
		*bRet++ = *bRelPath++;
	}
	
	// Empty path? Make it "/"
	if (bRet == ret) {
		*bRet++ = FILE_PATH_DELIMITER;
	}
	
	*bRet = 0; // Terminate string
}

// Generalized functions (redirecting to actual fs driver)
DIR *opendir(const char *path) {
	char aPath[FILEIO_MAX_PATH];
	
	// Resolve path
	absPath(path, aPath);
	
	// Re-direct to root_fs
	return fileio_root_fs.opendir(aPath);
}

int closedir(DIR * dir) {
	FS *fs = (FS *)(dir->fs);
	return fs->closedir(dir);
}

dirent *readdir(DIR *dir) {
	FS *fs = (FS *)(dir->fs);
	return fs->readdir(dir);
}

FILE *fopen(const char *path, const char *openMode) {
	char aPath[FILEIO_MAX_PATH];
	
	// Resolve path
	absPath(path, aPath);
	
	// Re-direct to root_fs
	return fileio_root_fs.fopen(aPath, openMode);
	
}

int fclose(FILE *f) {
	FS *fs = (FS *)(f->fs);
	return fs->fclose(f);
}

byte feof(FILE *f) {
	FS *fs = (FS *)(f->fs);
	return fs->feof(f);
}

int fgetc(FILE *f) {
	FS *fs = (FS *)(f->fs);
	return fs->fgetc(f);
}

size_t fread(void *ptr, size_t size, byte nmemb, FILE *f) {
	FS *fs = (FS *)(f->fs);
	return fs->fread(ptr, size, nmemb, f);
}

size_t fwrite(void *ptr, size_t size, byte nmemb, FILE *f) {
	FS *fs = (FS *)(f->fs);
	return fs->fwrite(ptr, size, nmemb, f);
}

#endif