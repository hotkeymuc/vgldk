//#include "romfs.h"

static romfs_state_t romfs_states[ROMFS_MAX_HANDLES];

void romfs_init() {
	// Mark all states as inactive
	for (byte h = 0; h < ROMFS_MAX_HANDLES; h++) {
		romfs_states[h].active = false;
	}
}

int romfs_fopen(byte index) {
	// Check if file is "found"
	if (index >= R_NUM_FILES) {
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
		if (!romfs_states[h].active) break;
	}
	if (h >= ROMFS_MAX_HANDLES) {
		// No free handle available!
		return ROMFS_ERROR_OUT_OF_HANDLES;
	}
	
	
	// Grab the file entry
	romfs_entry_t e = R_FILES[index];
	
	// Initialize the state
	romfs_states[h].active = true;
	romfs_states[h].index = index;
	romfs_states[h].mem_bank = e.bank;
	romfs_states[h].mem_ofs = e.addr;
	romfs_states[h].offset = 0;
	
	// Return the handle (which is just the index of the global state we used)
	return h;
}

int romfs_fclose(romfs_handle_t h) {
	if (!romfs_states[h].active) {
		return ROMFS_ERROR_NOT_OPEN;
	}
	romfs_states[h].active = false;
	return ROMFS_OK;
}

bool romfs_feof(romfs_handle_t h) {
	// Check for EOF
	if ((romfs_states[h].mem_bank == R_FILES[romfs_states[h].index].bank + R_FILES[romfs_states[h].index].banks) && (romfs_states[h].mem_ofs >= R_FILES[romfs_states[h].index].size)) {
		return true;
	}
	return false;
}

int romfs_fread(romfs_handle_t h) {
	// Check if handle is active
	if (!romfs_states[h].active) return ROMFS_ERROR_NOT_OPEN;
	
	// Check for EOF
	//if ((romfs_states[h].mem_bank == R_FILES[romfs_states[h].index].bank + R_FILES[romfs_states[h].index].banks) && (romfs_states[h].mem_ofs >= R_FILES[romfs_states[h].index].size)) {
	if (romfs_feof(h)) return ROMFS_ERROR_EOF;
	
	// Calculate memory address
	word a = R_MEM_OFFSET + romfs_states[h].mem_ofs;
	
	// Switch to bank
	romfs_switch_bank(romfs_states[h].mem_bank);
	
	// Get the byte
	byte r = *((byte *)a);
	
	// Go to next byte
	romfs_states[h].offset ++;
	romfs_states[h].mem_ofs ++;
	
	// Handle memory bank roll-over
	while (romfs_states[h].mem_ofs >= R_BANK_SIZE) {
		romfs_states[h].mem_bank ++;
		romfs_states[h].mem_ofs -= R_BANK_SIZE;
	}
	
	return r;
}
