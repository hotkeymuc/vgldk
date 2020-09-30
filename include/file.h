#ifndef __FILE_H
#define __FILE_H
/*
File definitions

The SDCC does not come with FILE headers.
So I implemented my own minimal one to be used in my "DOS".

2019-07-10 Bernhard "HotKey" Slawik
*/


#define ERR_OK 0
#define ERR_GENERAL -1
#define ERR_FILE_NOT_FOUND -2
#define ERR_DRIVE_INVALID -3
#define ERR_TIMEOUT -4
#define ERR_UNSUPPORTED -5
#define ERR_NOT_READY -6

#define FILE_PATH_DELIMITER '/'

typedef word size_t;

int errno;

//#define EOF -1
typedef struct {
	//const char *path;
	const char *name;
	const char *mode;
	
	// Link to file system
	//FS *fs;
	const void *fs;	// Link to file system
	
	void *userData;	// Remote handles etc.
	size_t size;
	size_t currentPos;
} file_FILE; // FILE

typedef struct {
	//ino_t d_ino;
	//char d_name[];
	size_t pos;
	const char *name;
	
	// Mainly for testing
	void *userData;
} dirent;

typedef struct {
	const char* path;
	
	// Link to ressource
	//FS *fs;
	const void *fs;	// Link to file system
	
	//dirent *entries;
	void *userData;	// Remote handles etc.
	
	size_t count;
	size_t currentPos;
} file_DIR;  // DIR



#endif
