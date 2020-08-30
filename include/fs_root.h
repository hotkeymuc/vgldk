#ifndef __FS_ROOT_H
#define __FS_ROOT_H

#include "fs.h"

#ifndef FS_ROOT_MOUNTS
	#warning "No mounts specified for fs_root.h (FS_ROOT_MOUNTS). Using empty list."
	#define FS_ROOT_MOUNTS {}
#endif

//#define FS_ROOT_CASE_SENSITIVE

typedef struct {
	const char *name;
	const FS *fs;
} fs_root_mount_t;

// Statically define the known FS's
const fs_root_mount_t fs_root_mounts[] = FS_ROOT_MOUNTS;
#define FS_ROOT_MOUNTS_COUNT (sizeof(fs_root_mounts) / sizeof(fs_root_mount_t))


// Forwards
void fs_root_mount(const char *options);
file_DIR *fs_root_opendir(const char *path);
int fs_root_closedir(file_DIR * dir);
dirent *fs_root_readdir(file_DIR *dir);
file_FILE *fs_root_fopen(const char *path, const char *openMode);
int fs_root_fclose(file_FILE *f);
byte fs_root_feof(file_FILE *f);
int fs_root_fgetc(file_FILE *f);
size_t fs_root_fread(void *ptr, size_t size, size_t nmemb, file_FILE *f);
size_t fs_root_fwrite(void *ptr, size_t size, size_t nmemb, file_FILE *f);


// Publish a FS struct
const FS fs_root = {	// Keep in sync with fs.h:FS!
	fs_root_mount,
	
	fs_root_opendir,
	fs_root_closedir,
	fs_root_readdir,
	
	fs_root_fopen,
	fs_root_fclose,
	fs_root_feof,
	//fs_root_fgetc,
	fs_root_fread,
	fs_root_fwrite
};


// Implementation
file_DIR fs_root_tmpDir;
dirent fs_root_tmpDirent;

int fs_root_indexOf(const char *path, const char **pp) {
	byte i;
	const char *cp;
	const char *cn;
	
	
	// Search for mount name and redirect to that FS
	for(i = 0; i < FS_ROOT_MOUNTS_COUNT; i++) {
		// compare fs_root_mounts[i].name against path
		cp = path;
		//if (*cp == FILE_PATH_DELIMITER) cp++;
		
		cn = fs_root_mounts[i].name;
		// Compare byte-by-byte
		#ifdef FS_ROOT_CASE_SENSITIVE
		while (*cp == *cn) {
		#else
		while (stricmp1(*cp, *cn) == 0) {
		#endif
			cp++;
			cn++;
			if ((*cp == FILE_PATH_DELIMITER) || (*cp == 0)) {
				// Path element ended
				if (*cn == 0) {
					// Name also ended: We have a match!
					
					// Set pp to rest of path
					if (*cp == FILE_PATH_DELIMITER) *pp = cp+1;
					else *pp = cp;
					
					return i;
				}
				// ..., but name goes on: Mismatch!
				break;
			}
		}
	}
	
	// Not found
	return -1;
}


void fs_root_mount(const char *options) {
	(void)options;
  int i;

  // Mount all file systems
  for(i = 0; i < FS_ROOT_MOUNTS_COUNT; i++) {
    fs_root_mounts[i].fs->mount(options);
  }
}

file_DIR *fs_root_opendir(const char *path) {
	file_DIR * dir;
	int i;
	const char *pp;
	
	// Skip initial slash
	pp = path;
	if (*pp == FILE_PATH_DELIMITER) pp++;
	
	if (*pp == 0) {
		// Root path: List mounts
		
		// Prepare a file_DIR handle
		dir = &fs_root_tmpDir;	//@FIXME: Using the ONE temp. dir...
		dir->fs = &fs_root;
		//dir->path = path;	// This can lead to weird behaviour!
		
		dir->count = FS_ROOT_MOUNTS_COUNT;
		dir->currentPos = 0;	// Rewind
		
		return dir;
		
	} else {
		// Deep path
		
		// Search for mount name and redirect to that FS
		i = fs_root_indexOf(pp, &pp);
		if (i < 0) {
			errno = ERR_FILE_NOT_FOUND;
			return NULL;
		}
		
		// Re-direct to FS
		return fs_root_mounts[i].fs->opendir(pp);
	}
	
}

int fs_root_closedir(file_DIR * dir) {
	dir->count = 0;
	return 0;
}

dirent *fs_root_readdir(file_DIR *dir) {
	dirent *de;
	
	if (dir->currentPos >= dir->count) {
		// Done.
		return NULL;
	}
	
	// Create a dirent
	de = &fs_root_tmpDirent;		//@FIXME: Using the ONE temp. dir.ent..
	de->pos = dir->currentPos;
	de->name = fs_root_mounts[dir->currentPos].name;
	
	dir->currentPos++;
	
	return de;
	
}

file_FILE *fs_root_fopen(const char *path, const char *openMode) {
	
	int i;
	const char *pp;
	
	pp = path;
	if (*pp == FILE_PATH_DELIMITER) pp++;	// Skip initial slash
	
	if (*pp == 0) {
		// Root path?
		//puts("No files in fs_root!\n");
		return NULL;
		
	} else {
		// Deep path: Search for mount name and redirect to that FS
		i = fs_root_indexOf(pp, &pp);
		if (i < 0) {
			errno = ERR_FILE_NOT_FOUND;
			return NULL;
		}
		// Re-direct to FS
		return fs_root_mounts[i].fs->open(pp, openMode);
	}
	
}

int fs_root_fclose(file_FILE *f) {
	(void)f;
	return 0;
}

byte fs_root_feof(file_FILE *f) {
	(void)f;
	return 1;
}

int fs_root_fgetc(file_FILE *f) {
	(void)f;
	return -1;	// EOF
}

size_t fs_root_fread(void *ptr, size_t size, size_t nmemb, file_FILE *f) {
	(void)ptr;
	(void)size;
	(void)nmemb;
	(void)f;
	
	return 0;
}

size_t fs_root_fwrite(void *ptr, size_t size, size_t nmemb, file_FILE *f) {
	(void)ptr;
	(void)size;
	(void)nmemb;
	(void)f;
	
	return 0;
}

#endif
