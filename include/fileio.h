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

#define DRIVE_INDICATOR_CHAR ':'
#define PATH_DELIMITER '/'

#define MAX_PATH 32

//@TODO: Move this somehwere else, so it is easier to configure for different uses
#define MAX_DRIVES 2	// Keep in sync with implementations

//const FS *drives[] = { (FS *)&fs_internal };
FS *drives[MAX_DRIVES];	// Keep in sync with implementations

//FS *cfs;	// Current file system driver
char cwd[MAX_PATH];	// Current working directory


void absPath(const char *relPath, char *ret) {
	const char *bRelPath;
	char *bRet;
	char *t;
	
	bRelPath = relPath;
	bRet = ret;
	
	if ((relPath == NULL) || (*relPath == 0)) {
		// Handle "empty string"
		// Just use CWD
		relPath = &cwd[0];
		
	} else
	if (*(relPath+1) == DRIVE_INDICATOR_CHAR) {
		// Handle absolute path
		// nothing to do, relative = absolute
		
	} else {
		// Handle relative path
		
		// Start with CWD
		t = &cwd[0];
		while (*t != 0) {
			*bRet++ = *t++;
		}
		*bRet++ = PATH_DELIMITER;
		// Continue with relative path
	}
	
	// Copy the rest of relative path
	while (*bRelPath != 0) {
		if ((bRelPath > relPath) && (*(bRelPath-1) == '.') && (*bRelPath == '.')) {
			// Handle ".."
			while (*bRelPath == '.') bRelPath++;	// Skip the second "."
			
			// Go back to last delimiter
			bRet--;	// Back to last char
			bRet--;	// Back off first "."
			if (*bRet == PATH_DELIMITER) bRet--;	// Back off last delimiter
			if (*bRet == DRIVE_INDICATOR_CHAR) {bRet++; continue;}	// Too far!
			while((*bRet != PATH_DELIMITER) && (bRet > ret)) bRet--;	// Back to last delimiter
			
			continue;
		}
		
		*bRet++ = *bRelPath++;
	}
	*bRet = 0; // Terminate string
	
	//printf("\"%s\" -> \"%s\"\n", relPath, ret);
}

FS *fsByAbsPath(const char *path) {
	// Chose a file system instance based on the given ABSOLUTE path
	byte driveNum;
	FS *fs;
	
	// Handle DOS style drives
	if (path[1] != DRIVE_INDICATOR_CHAR) {
		printf("No drive spec!\n");
		errno = ERR_DRIVE_INVALID;
		return NULL;
	}
	
	// Get drive number from path
	driveNum = path[0] - 'A';
	
	if (driveNum >= 32) driveNum -= 32;	// handle lower case letters
	
	if (driveNum >= MAX_DRIVES) {
		printf("Drive %d > %d!\n", driveNum, MAX_DRIVES);
		errno = ERR_DRIVE_INVALID;
		return NULL;
	}
	
	// Return FS instance for drive
	fs = drives[driveNum];
	
	return fs;

}
FS *fsByPath(const char *path) {
	// Chose a file system instance based on the given path
	
	char aPath[MAX_PATH];
	
	// Resolve path
	absPath(path, &aPath[0]);
	
	return fsByAbsPath(aPath);
}


// Generalized functions (redirecting to actual fs driver)
DIR *opendir(const char *path) {
	char aPath[MAX_PATH];
	DIR *dir;
	FS *fs;
	
	// Resolve path
	absPath(path, &aPath[0]);
	//if (aPath[0] == 0) return NULL;
	//if (aPath[1] != DRIVE_INDICATOR_CHAR) return NULL;
	
	// Resolve file system
	fs = fsByAbsPath(aPath);
	if (fs == NULL) return NULL;
	
	// Call fs.opendir()
	if (aPath[2] == PATH_DELIMITER) {
		dir = fs->opendir(aPath+3);	// Start after drive spec and delimiter
	} else {
		dir = fs->opendir(aPath+2);
	}
	if (dir == NULL) return NULL;
	
	dir->fs = fs;
	return dir;
}

int closedir(DIR * dir) {
	FS *fs = (FS *)(dir->fs);
	return fs->closedir(dir);
}

//struct dirent *readdir(DIR *dir) {
dirent *readdir(DIR *dir) {
	FS *fs = (FS *)(dir->fs);
	return fs->readdir(dir);
}

FILE *fopen(const char *path, const char *openMode) {
	char aPath[MAX_PATH];
	FILE *f;
	FS *fs;
	
	// Resolve path
	absPath(path, &aPath[0]);
	//if (aPath[0] == 0) return NULL;
	//if (aPath[1] != DRIVE_INDICATOR_CHAR) return NULL;
	
	// Resolve file system
	fs = fsByAbsPath(aPath);
	if (fs == NULL) return NULL;
	
	// Call fs.fopen()
	//f = fs->fopen(path, openMode);
	f = fs->fopen(aPath+3, openMode);	// Start after drive spec and delimiter
	if (f == NULL) return NULL;
	
	//f->openMode = openMode;
	f->fs = fs;
	
	return f;
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



#endif