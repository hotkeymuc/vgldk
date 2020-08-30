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

#ifndef FILEIO_CUSTOM_NAMES
  
  // If not specified otherwise: Map stdio file methods to fileio methods
  #define FILEIO_DEFINE_FUNCTIONS
#endif


//FS *cfs;	// Current file system driver
char cwd[FILEIO_MAX_PATH];	// Current working directory

#ifndef FILEIO_ROOT_FS
	#warning "No root filesystem specified for fileio.h (FILEIO_ROOT_FS). Using fs_null."
	#include "fs_null.h"
	#define FILEIO_ROOT_FS fs_null
#endif

#define fileio_root_fs FILEIO_ROOT_FS

void fileio_mount(const char *options) {
  fileio_root_fs.mount(options);
}

void fileio_absPath(const char *relPath, char *ret) {
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
file_DIR *fileio_opendir(const char *path) {
	char aPath[FILEIO_MAX_PATH];
	
	// Resolve path
	fileio_absPath(path, aPath);
	
	// Re-direct to root_fs
	return fileio_root_fs.opendir(aPath);
}

int fileio_closedir(file_DIR * dir) {
	FS *fs = (FS *)(dir->fs);
	return fs->closedir(dir);
}

dirent *fileio_readdir(file_DIR *dir) {
	FS *fs = (FS *)(dir->fs);
	return fs->readdir(dir);
}

file_FILE *fileio_open(const char *path, const char *openMode) {
	char aPath[FILEIO_MAX_PATH];
	
	// Resolve path
	fileio_absPath(path, aPath);
	
	// Re-direct to root_fs
	return fileio_root_fs.open(aPath, openMode);
	
}

int fileio_close(file_FILE *f) {
	FS *fs = (FS *)(f->fs);
	return fs->close(f);
}

byte fileio_eof(file_FILE *f) {
	FS *fs = (FS *)(f->fs);
 
	return fs->eof(f);
}

/*
int fileio_getc(file_FILE *f) {
	FS *fs = (FS *)(f->fs);
	return fs->getc(f);
}
*/
size_t fileio_read(void *ptr, size_t size, byte nmemb, file_FILE *f) {
	FS *fs = (FS *)(f->fs);
	return fs->read(ptr, size, nmemb, f);
}

size_t fileio_write(void *ptr, size_t size, byte nmemb, file_FILE *f) {
	FS *fs = (FS *)(f->fs);
	return fs->write(ptr, size, nmemb, f);
}


#ifdef FILEIO_DEFINE_FUNCTIONS
  #define absPath fileio_absPath
  #define opendir fileio_opendir
  #define closedir fileio_closedir
  #define readdir fileio_readdir
  
  #define fopen fileio_open
  #define fclose fileio_close
  #define feof fileio_eof
  //#define fgetc fileio_getc
  #define fread fileio_read
  #define fwrite fileio_write
#endif


#endif
