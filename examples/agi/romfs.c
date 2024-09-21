#include "romfs.h"

static romfs_state_t romfs_states[ROMFS_MAX_HANDLES];

void romfs_init() {
	// Mark all states as inactive
	for (byte h = 0; h < ROMFS_MAX_HANDLES; h++) {
		romfs_states[h].active = false;
	}
}

romfs_handle_t romfs_fopen(byte index) {
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
	const romfs_entry_t *e = &R_FILES[index];
	
	// Initialize the state
	romfs_state_t *state = &romfs_states[h];
	state->active = true;
	state->index = index;
	state->mem_bank_start = e->bank;
	state->mem_ofs_start = e->addr;	//@FIXME: Addr or offset?!?!?
	state->mem_bank = state->mem_bank_start;
	state->mem_ofs = state->mem_ofs_start;	//@FIXME: Addr or offset?!?!?
	
	// For quicker EOF checks:
	state->mem_bank_end = e->bank + e->banks;
	state->mem_ofs_end = e->addr + e->size;
	// Stats:
	state->offset = 0;
	
	// Return the handle (which is just the index of the global state we used)
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

word romfs_fpos(romfs_handle_t h) {
	return romfs_states[h].offset;
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
	state->offset += skip;
	state->mem_bank += (skip / R_BANK_SIZE);
	state->mem_ofs += (skip % R_BANK_SIZE);
	// Handle memory bank roll-over
	while (state->mem_ofs >= R_BANK_SIZE) {
		state->mem_bank ++;
		state->mem_ofs -= R_BANK_SIZE;
	}
}
void romfs_fskip_far(romfs_handle_t h, word skip_hi, word skip_lo) {
	// One giant leap across banks...
	romfs_states[h].mem_bank += (skip_hi / (R_BANK_SIZE >> 8));
	
	// ...and then the rest.
	romfs_fskip(h, skip_lo);
}


int romfs_fpeek(romfs_handle_t h) {
	// Check if handle is active
	//if (!romfs_states[h].active) return ROMFS_ERROR_NOT_OPEN;
	if (!romfs_factive(h)) {
		//printf("NOT OPEN!");
		return ROMFS_ERROR_NOT_OPEN;
	}
	
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

int romfs_fread(romfs_handle_t h) {
	int r = romfs_fpeek(h);
	
	// Go to next byte
	romfs_fskip(h, 1);
	
	return r;
}
