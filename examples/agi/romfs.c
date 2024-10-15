#include "romfs.h"

romfs_state_t romfs_states[ROMFS_MAX_HANDLES];
#ifdef ROMFS_USE_FILENAMES
	word romfs_num_files;
	byte romfs_filename_len;
#endif

void romfs_init() {
	byte i;
	
	// Mark all states as inactive
	for (i = 0; i < ROMFS_MAX_HANDLES; i++) {
		romfs_states[i].active = false;
	}
	
	#ifdef ROMFS_USE_FILENAMES
		// Scan ROM index
		romfs_num_files = 1;	// Bust be made > 0
		romfs_handle_t h = romfs_fopen(0);	// Open index (first file / R__)
		
		// Header: 'ROMFS000'
		romfs_fskip(h, ROMFS_ID_LEN);	// Just skip it
		//printf("ID: \""); for(i = 0; i < ROMFS_ID_LEN; i++) { putchar(romfs_fread(h)); } printf("\"\n"); getchar();
		//@TODO: Check it against ROMFS_ID_STRING
		
		// Filename size
		romfs_filename_len = romfs_fread(h);
		//@TODO: Compare to ROMFS_FILENAME_LEN
		//printf("fs="); printf_x2(romfs_filename_len); getchar();
		
		// ROM name
		#ifdef ROMFS_SHOW_ROM_NAME
			printf("ROM: \""); for(i = 0; i < romfs_filename_len; i++) { putchar(romfs_fread(h)); } printf("\"");
			//getchar();
		#else
			romfs_fskip(h, romfs_filename_len);
		#endif
		
		// File count (LO, HI = Z80 word)
		romfs_num_files = romfs_fread(h);
		romfs_num_files = (((word)romfs_fread(h)) << 8) | romfs_num_files;
		//printf("num=0x"); printf_x2(romfs_num_files >> 8); printf_x2(romfs_num_files & 0xff); getchar();
		
		// Add virtual index
		romfs_num_files++;
		
		romfs_fclose(h);
	#endif
}

#ifdef ROMFS_USE_FILENAMES
	int romfs_find(const char *name) {
		// Return the index of the file given by name
		int r;
		byte j;
		word i;
		byte b1, b2;
		const char *p;
		bool match;
		
		r = -1;
		romfs_handle_t h = romfs_fopen(0);	// Open index (first file / R__)
		
		// Skip header
		romfs_fskip(h, ROMFS_HEADER_SIZE);
		
		// Check all entries
		for(i = 0; i < romfs_num_files; i++) {
			// Check filenames
			//putchar('\n'); printf_x2(i); putchar('@');printf_x2(romfs_fpos(h)); putchar(':');
			match = true;
			p = name;
			for(j = 0; j < romfs_filename_len; j++) {
				b1 = *p++;
				if (b1 == 0) break;
				b2 = romfs_fread(h);
				if (b1 != b2) {
					match = false;
					j++;
					break;
				}
			}
			if (match) {
				// Match!
				//r = i;
				r = i + 1;	// Must skip across index file
				break;
			}
			// Skip rest of name
			romfs_fskip(h, romfs_filename_len-j);
			
			// Skip 6 data bytes
			romfs_fskip(h, sizeof(romfs_entry_t));
		}
		
		#ifdef ROMFS_IGNORE_NOT_FOUND
			// Ignore
		#else
			if (r < 0) {
				printf("ROMFS:\""); printf(name); printf("\"?!");
				//getchar();
			}
		#endif
		
		romfs_fclose(h);
		return r;
	}
	romfs_entry_t *romfs_get_entry(byte index) {
		const romfs_entry_t romfs_index_entry = {
			0, 0, 1,0	// virtual entry
		};
		if (index == 0) {
			return &romfs_index_entry;
		}
		
		romfs_switch_bank(0);	//romfs_states[h].mem_bank);	// Switch to first bank
		
		//return (romfs_entry_t *)(R_MEM_OFFSET + ROMFS_ID_LEN + 1 + ROMFS_FILENAME_LEN + 2 + index*(ROMFS_INDEX_SIZE));
		return (romfs_entry_t *)(R_MEM_OFFSET + ROMFS_HEADER_SIZE + (index-1)*ROMFS_INDEX_SIZE + ROMFS_FILENAME_LEN);
	}
#else
	#define romfs_num_files R_NUM_FILES
	romfs_entry_t *romfs_get_entry(byte index) {
		return &R_FILES[index];
	}
#endif

romfs_handle_t romfs_fopen(byte index) {
	#ifdef ROMFS_DEBUG
		printf("romfs_open("); printf_d(index); printf(")...");
	#endif
	
	
	// Check if file is "found"
	//if (index >= R_NUM_FILES) {
	if (index >= romfs_num_files) {
		// Invalid file index!
		return ROMFS_ERROR_NOT_FOUND;
	}
	
	
	/*
	// Use a single handle/state
	romfs_state_t state = romfs_state;
	if (state.active) {
		// Out of handles / State is busy
		return ROMFS_ERROR_OUT_OF_HANDLES;
	}
	*/
	
	// Find a free handle/state
	romfs_handle_t h;
	for (h = 0; h < ROMFS_MAX_HANDLES; h++) {
		if (romfs_states[h].active == false) break;
	}
	if (h >= ROMFS_MAX_HANDLES) {
		// No free handle available!
		return ROMFS_ERROR_OUT_OF_HANDLES;
	}
	
	// Grab the file entry
	//const romfs_entry_t *e = &R_FILES[index];
	const romfs_entry_t *e = romfs_get_entry(index);	// Get romfs_entry_t either from built-in code or read from ROM
	
	// Initialize the state
	romfs_state_t *state = &romfs_states[h];
	state->active = true;
	state->index = index;
	state->mem_bank_start = e->bank;
	state->mem_ofs_start = e->addr;	// This is the OFFSET inside the bank
	
	state->mem_bank = state->mem_bank_start;
	state->mem_ofs = state->mem_ofs_start;	// This is the OFFSET inside the bank
	
	// For quicker EOF checks:
	state->mem_bank_end = e->bank + e->banks;
	state->mem_ofs_end = (e->addr + e->size) % R_BANK_SIZE;	// This is the OFFSET inside the bank
	state->offset = 0;	// Just for stats
	
	// Return the handle (which is just the index of the global state we used)
	#ifdef ROMFS_DEBUG
		printf("OK ("); printf_d(h); printf(")\n");
	#endif
	return h;
}

bool romfs_factive(romfs_handle_t h) {
	return (romfs_states[h].active);
}

int romfs_fclose(romfs_handle_t h) {
	//if (!romfs_states[h].active) return ROMFS_ERROR_NOT_OPEN;
	if (!romfs_factive(h)) return ROMFS_ERROR_NOT_OPEN;
	
	romfs_states[h].active = false;
	return ROMFS_OK;
}

bool romfs_feof(romfs_handle_t h) {
	// Check for EOF
	//if ((romfs_states[h].mem_bank == R_FILES[romfs_states[h].index].bank + R_FILES[romfs_states[h].index].banks) && (romfs_states[h].mem_ofs >= R_FILES[romfs_states[h].index].size)) {
	romfs_state_t *state = &romfs_states[h];
	//@FIXME: It is not working! Reports EOF right away!
	if ((state->mem_bank == state->mem_bank_end) && (state->mem_ofs >= state->mem_ofs_end)) {
		return true;
	}
	return false;
}

word inline romfs_fpos(romfs_handle_t h) {
	return romfs_states[h].offset;
}

word romfs_fsize(romfs_handle_t h) {
	romfs_state_t *state = &romfs_states[h];
	
	if (state->mem_bank_start == state->mem_bank_end)
		return (state->mem_ofs_end - state->mem_ofs_start);
	
	return (
		(R_BANK_SIZE - state->mem_ofs_start)
		+ (R_BANK_SIZE * (state->mem_bank_end - state->mem_bank_start - 1))
		+ state->mem_ofs_end
	);
}

void romfs_frewind(romfs_handle_t h) {
	// Go to beginning of file
	
	romfs_state_t *state = &romfs_states[h];
	state->offset = 0;
	state->mem_bank = state->mem_bank_start;
	state->mem_ofs = state->mem_ofs_start;
}

void romfs_fskip(romfs_handle_t h, word skip) {
	// Do a "little" skip forward (without overflowing the offset counters...)
	
	romfs_state_t *state = &romfs_states[h];
	
	state->mem_bank += (skip / R_BANK_SIZE);
	state->mem_ofs += (skip % R_BANK_SIZE);
	state->offset += skip;	// Just for stats!
	
	// Handle memory bank roll-over
	while (state->mem_ofs >= R_BANK_SIZE) {
		state->mem_bank ++;
		state->mem_ofs -= R_BANK_SIZE;
	}
}

void romfs_fskip_far(romfs_handle_t h, word skip_hi, word skip_lo) {
	/*
	#ifdef ROMFS_DEBUG
		printf("romfs_skip_far("); printf_x2(skip_hi); printf(","); printf_x2(skip_lo >> 8); printf_x2(skip_lo & 0xff); printf(")...\n");
		printf("currently at bank=");
		printf_x2(romfs_states[h].mem_bank);
		printf(", mem_ofs="); printf_x2(romfs_states[h].mem_ofs >> 8); printf_x2(romfs_states[h].mem_ofs & 0xff);
		printf(".\n");
	#endif
	*/
	
	unsigned long skip = (skip_hi * 0x10000) + skip_lo;
	romfs_states[h].mem_bank += (skip / R_BANK_SIZE);
	romfs_states[h].offset += skip;	// This will overflow sooner or later. Just for stats.
	
	// Skip over the rest
	romfs_fskip(h, skip % R_BANK_SIZE);
	
	/*
	// One giant leap across banks...
	romfs_states[h].mem_bank += (skip_hi / (R_BANK_SIZE >> 8));
	romfs_states[h].offset += skip_hi * R_BANK_SIZE;	// This will overflow sooner or later. Just for stats.
	
	// ...and then the rest.
	romfs_fskip(h, skip_lo + R_BANK_SIZE * (skip_hi % (R_BANK_SIZE >> 8)) );
	*/
	/*
	#ifdef ROMFS_DEBUG
		printf("now at bank=");
		printf_x2(romfs_states[h].mem_bank);
		printf(", mem_ofs="); printf_x2(romfs_states[h].mem_ofs >> 8); printf_x2(romfs_states[h].mem_ofs & 0xff);
		printf(".\n");
	#endif
	*/
}

byte *romfs_fpoint(romfs_handle_t h) {
	// Return current pointer
	return (byte *)(R_MEM_OFFSET + romfs_states[h].mem_ofs);
}

//int romfs_fpeek(romfs_handle_t h) {
byte inline romfs_fpeek(romfs_handle_t h) {
	// Check if handle is active
	//if (!romfs_states[h].active) return ROMFS_ERROR_NOT_OPEN;
	/*
	if (!romfs_factive(h)) {
		//printf("NOT OPEN!");
		return ROMFS_ERROR_NOT_OPEN;
	}
	*/
	
	// Check for EOF
	//if ((romfs_states[h].mem_bank == R_FILES[romfs_states[h].index].bank + R_FILES[romfs_states[h].index].banks) && (romfs_states[h].mem_ofs >= R_FILES[romfs_states[h].index].size)) {
	/*
	if (romfs_feof(h)) {
		printf("EOF!");
		return ROMFS_ERROR_EOF;
	}
	*/
	
	// Calculate memory address
	word a = R_MEM_OFFSET + romfs_states[h].mem_ofs;
	//word a = romfs_states[h].mem_ofs;
	
	// Switch to bank
	romfs_switch_bank(romfs_states[h].mem_bank);
	
	// Get the byte
	byte r = *((byte *)a);
	//printf("Reading from ofs=0x"); printf_x2(a >> 8); printf_x2(a & 0xff); printf(" = 0x"); printf_x2(r); printf("\n");
	
	return r;
}

//int romfs_fread(romfs_handle_t h) {
byte romfs_fread(romfs_handle_t h) {
	//int r = romfs_fpeek(h);
	
	//@TODO: Check EOF
	//if (romfs_feof(h)) return -1;
	
	byte r = romfs_fpeek(h);
	
	/*
	#ifdef ROMFS_DEBUG
		printf("romfs_fread(");
		printf_x2(romfs_states[h].mem_ofs >> 8);
		printf_x2(romfs_states[h].mem_ofs & 0xff);
		printf(") ="); printf_x2(r); printf("\n");
	#endif
	*/
	// Go to next byte
	romfs_fskip(h, 1);
	
	return r;
}
