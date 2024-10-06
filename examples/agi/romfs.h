#ifndef __ROMFS_H__
#define __ROMFS_H__
/*
Helper to access files stored in ROM pages.

Use "romfs_gen.py" to generate a corresponding data header and prepare the ROM file.

These things must be defined outside:
	#define R_MEM_OFFSET 0x4000	// Where to find the banked memory area
	#define R_BANK_SIZE 0x4000	// How big one bank is
	void romfs_switch_bank(byte bank) { bank_port = bank; }	// How to switch banks

2024-09-16 Bernhard "HotKey" Slawik
*/

//#define ROMFS_DEBUG	// Verbose output to stdout

#define ROMFS_OK 0
#define ROMFS_ERROR_EOF -1
#define ROMFS_ERROR_NOT_FOUND -2
#define ROMFS_ERROR_OUT_OF_HANDLES -3
#define ROMFS_ERROR_NOT_OPEN -4

#define ROMFS_MAX_HANDLES 6	//4


// romfs_gen.py creates these entries
typedef struct {
	byte bank;
	word addr;
	//word size;	// Must be > 16 bit (32 bit?) for Sierra
	word banks;
	word size;	// Remainder of the actual size (banks * bank_size + size)
} romfs_entry_t;

typedef struct {
	bool active;	// Is something opened?
	byte index;	// Which file is opened
	
	byte mem_bank;	// Current bank
	word mem_ofs;	// Current offset inside bank
	
	byte mem_bank_start;	// For allowing rewind
	word mem_ofs_start;	// For allowing rewind
	
	byte mem_bank_end;	// For quicker EOF checking
	word mem_ofs_end;	// For quicker EOF checking
	
	word offset;	// Current linear offset @FIXME: 16 bit! Roll-over!
} romfs_state_t;


//static romfs_state_t romfs_state;	// One single state
typedef int romfs_handle_t;	// Those are processed by user programs

void romfs_init();

romfs_handle_t romfs_fopen(byte index);
bool romfs_factive(romfs_handle_t h);
int romfs_fclose(romfs_handle_t h);

bool romfs_feof(romfs_handle_t h);
word romfs_fpos(romfs_handle_t h);
word romfs_fsize(romfs_handle_t h);
void romfs_frewind(romfs_handle_t h);
void romfs_fskip(romfs_handle_t h, word skip);
void romfs_fskip_far(romfs_handle_t h, word skip_hi, word skip_lo);

byte *romfs_fpoint(romfs_handle_t h);

//int romfs_fpeek(romfs_handle_t h);
byte romfs_fpeek(romfs_handle_t h);

int romfs_fread(romfs_handle_t h);

#endif