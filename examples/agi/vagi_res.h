#ifndef __VAGI_RES_H__
#define __VAGI_RES_H__

/*
Abstraction layer for opening AGI resources.
It re-directs accesses to ROM FS, which stores the raw files.

2024-09-16 Bernhard "HotKey" Slawik
*/

// ROM FS:
//#define ROMFS_DEBUG	// Verbose ROM access output to stdout
//#define R_MEM_OFFSET 0x4000	// Where to find the banked memory area
//#define R_BANK_SIZE 0x4000	// How big one bank is

// At 0x4000:
//#define romfs_switch_bank(bank) bank_0x4000_port = (0x20 | bank)	// and: bank_type_port = bank_type_port | 0x02
// At 0x8000:
//#define romfs_switch_bank(bank) bank_0x8000_port = (0x20 | bank)	// and: bank_type_port = bank_type_port | 0x04
void inline romfs_switch_bank(byte bank) {
	#ifdef ROMFS_DEBUG
		printf("romfs_switch_bank("); printf_d(bank); printf(")\n");
	#endif
	
	// if R_MEM_OFFSET = 0x4000:
	//bank_type_port = bank_type_port | 0x02;	// Enable external cart on segment 0x4000
	//bank_0x4000_port = (0x20 | bank);	// Map desired chip segment to memory segment 0x4000
	
	// if R_MEM_OFFSET = 0x8000:
	//bank_type_port = bank_type_port | 0x04;	// Enable external cart on segment 0x8000
	bank_0x8000_port = (0x20 | bank);	// Map desired chip segment to memory segment 0x8000
}

#include "romfs.h"
#include "romfs_data.h"
#include "romfs.c"	//@FIXME: Only the main C file gets passed to the VGLDK compiler pass...

#define VAGI_RES_MAX_HANDLES 6

#define VAGI_RES_ERROR_OUT_OF_HANDLES -11
#define VAGI_RES_ERROR_SIGNATURE_ERROR -12
#define VAGI_RES_ERROR_VOLUME_ERROR -13
#define VAGI_RES_ERROR_VOLUME_MISS -14
#define VAGI_RES_ERROR_UNKNOWN_KIND -15


#define VAGI_RES_IGNORE_SIGNATURE_ERROR	// Not recommended, just for debugging quirks among different games
//#define VAGI_RES_IGNORE_VOLUME_MISS	// Not recommended, just for debugging quirks among different games

enum AGI_RES_KIND {
	AGI_RES_KIND_LOG,
	AGI_RES_KIND_PIC,
	AGI_RES_KIND_SND,
	AGI_RES_KIND_VIEW,
	
	AGI_RES_KIND_COUNT
};

//const char * AGI_RES_KIND_ID[AGI_RES_KIND_COUNT] = {
//	"LOG",
//	"PIC",
//	"SND",
//	"VIEW",
//};
static const char AGI_RES_KIND_ID[AGI_RES_KIND_COUNT] = {
	'L',
	'P',
	'S',
	'V'
};

static const byte AGI_DIR_FILES[AGI_RES_KIND_COUNT] = {
	R_LOGDIR,
	R_PICDIR,
	R_SNDDIR,
	R_VIEWDIR,
};

static const byte AGI_VOL_FILES[] = {
	R_VOL_0,
	#ifdef R_VOL_1
	R_VOL_1,
	#endif
	#ifdef R_VOL_2
	R_VOL_2,
	#endif
	#ifdef R_VOL_3
	R_VOL_3,
	#endif
	#ifdef R_VOL_4
	R_VOL_4,
	#endif
	#ifdef R_VOL_5
	R_VOL_5,
	#endif
};


// State

typedef struct {
	bool active;	// Is something opened?
	//byte kind;	// Which kind of resource is opened
	//word num;	// Which resource number
	
	romfs_handle_t romfs_handle;
	
	byte res_vol;	// Volume number
	byte res_ofs_hi;	// Offset of resource inside VOL (high 8 bits)
	word res_ofs;	// Offset inside VOL (low 16 bits)
	
	word offset;	// Current offset inside resource
	word res_size;	// Size of resource data
} vagi_res_state_t;

typedef int vagi_res_handle_t;	// Those are processed by user programs


extern vagi_res_state_t vagi_res_states[VAGI_RES_MAX_HANDLES];
vagi_res_state_t vagi_res_states[VAGI_RES_MAX_HANDLES];

// Helper
void vagi_res_printf_res(byte kind, word num) {
	putchar(AGI_RES_KIND_ID[kind]); printf_d(num);
	putchar(':');
}

void vagi_res_init() {
	romfs_init();
	
	// Mark all states as inactive
	for (byte h = 0; h < VAGI_RES_MAX_HANDLES; h++) {
		vagi_res_states[h].active = false;
	}
}

vagi_res_handle_t vagi_res_open(byte kind, word num) {
	byte b1, b2, b3;
	byte vi;
	word declen;
	word enclen;
	
	// Find a free handle/state
	vagi_res_handle_t h;
	for (h = 0; h < VAGI_RES_MAX_HANDLES; h++) {
		if (vagi_res_states[h].active == false) break;
	}
	if (h >= VAGI_RES_MAX_HANDLES) {
		// No free handle available!
		return VAGI_RES_ERROR_OUT_OF_HANDLES;
	}
	if (kind >= AGI_RES_KIND_COUNT) {
		return VAGI_RES_ERROR_UNKNOWN_KIND;
	}
	
	vagi_res_state_t *state = &vagi_res_states[h];
	state->active = false;	// Set to true later on, if everthing works out
	//state->kind = kind;	// We actually don't need to access that after open
	//state->num = num;	// We actually don't need to access that after open
	state->romfs_handle = -1;
	state->res_size = 0;
	state->offset = 0;
	
	// Mount our cartridge ROM to address 0x4000 (data must be inside the ROM binary at position 0x4000 * n)
	//bank_type_port = bank_type_port | 0x02;	// Switch address region 0x4000-0x7FFF to use cartridge ROM (instead of internal ROM)
	// bank_0x4000_port = 0x20 | 1;	// Mount ROM segment n=1 (offset 0x4000 * n) to address 0x4000
	
	// Mount cartridge at cartridge port 0x8000
	//bank_type_port = bank_type_port | 0x04;	// Switch address region 0x8000-0xBFFF to use cartridge ROM
	//bank_0x8000_port = 0x20 | 0;	// Mount ROM segment n=0 (offset 0x4000 * n) to address 0x8000
	
	// Open directory for the given kind (LOG, PIC, SND, VIEW)
	//printf("Opening DIR...");
	romfs_handle_t rh;
	rh = romfs_fopen(AGI_DIR_FILES[kind]);
	if (rh < 0) {
		vagi_res_printf_res(kind, num);
		printf("DIR err=-"); printf_d(-rh); getchar();
		//return 0;
		return rh;
	}
	
	if (num*3 > romfs_fsize(rh)) {
		return ROMFS_ERROR_EOF;
	}
	
	// Seek to given entry number (3 bytes each)
	romfs_fskip(rh, num*3);
	
	// Get the 3 offset bytes from DIR
	b1 = romfs_fread(rh);	// vol / HSB+
	b2 = romfs_fread(rh);	// ofs HSB
	b3 = romfs_fread(rh);	// ofs LSB
	
	// Extract volume and offset
	vi = (b1 & 0xf0) >> 4;
	state->res_vol = vi;
	state->res_ofs_hi = (b1 & 0x0f);	// << 16
	state->res_ofs = ((word)b2 << 8) | (word)b3;
	//printf("vol="); printf_d(res_vol); printf(", ofs="); printf_x2(res_ofs_hi); printf_x2(res_ofs >> 8); printf_x2(res_ofs & 0xff); printf("\n");
	romfs_fclose(rh);	// We don't need to access the DIR after that and can re-use that romfs state
	
	
	// Open volume file (assume the VOL.* files are located sequencially)
	//printf("Opening VOL...");
	rh = romfs_fopen(AGI_VOL_FILES[state->res_vol]);
	if (rh < 0) {
		vagi_res_printf_res(kind, num);
		printf("VOL."); printf_d(state->res_vol);
		printf(": err=-"); printf_d(-rh);
		getchar();
		
		return rh;
	}
	
	// Seek to address (Up to 24 bit = far!)
	romfs_fskip_far(rh, state->res_ofs_hi, state->res_ofs);
	
	// Check signature (0x1234)
	b1 = romfs_fread(rh);	// LO: 0x34
	b2 = romfs_fread(rh);	// HI: 0x12
	if ((b1 != 0x12) || (b2 != 0x34)) {
		// Signature mismatch!
		vagi_res_printf_res(kind, num);
		printf("SIG err:");
		printf_x2(b1); printf_x2(b2);	//printf(" != 1234\n");
		getchar();
		#ifdef VAGI_RES_IGNORE_SIGNATURE_ERROR
		// Ignore!
		#else
		romfs_fclose(rh);
		return VAGI_RES_ERROR_SIGNATURE_ERROR;
		#endif
	}
	
	// Check resource volume value
	b1 = romfs_fread(rh);	// Volume number
	vi = b1;
	if ((vi & 0x7f) != state->res_vol) {
		// Volume mismatch (volume file number VS stored volume number of resource)
		vagi_res_printf_res(kind, num);
		printf("VOL mis:");
		printf_d(vi & 0x7f); printf(" != "); printf_d(state->res_vol);
		getchar();
		#ifdef VAGI_RES_IGNORE_VOLUME_MISS
		// Ignore!
		#else
		romfs_fclose(rh);
		return VAGI_RES_ERROR_VOLUME_MISS;
		#endif
	}
	
	// Read (decompressed) size (LO-HI)
	b1 = romfs_fread(rh);	// LO: decompressed size
	b2 = romfs_fread(rh);	// HI: decompressed size
	declen = (word)b1 | ((word)b2 << 8);
	state->res_size = declen;	//b1 | (b2 << 8);
	
	// Handle "PACKED_DIRS"!
	//if (PACKED_DIRS)
	#ifdef PACKED_DIRS
		//enclen 	= (gi->version->flags&PACKED_DIRS)?fgetw(f):declen;
		b1 = romfs_fread(rh);	// LO: decompressed size
		b2 = romfs_fread(rh);	// HI: decompressed size
		enclen = (word)b1 | ((word)b2 << 8);
		// Rewind!
		romfs_frewind(rh);
		romfs_fskip_far(rh, state->res_ofs_hi, state->res_ofs);
		romfs_fskip(rh, 5);	// Skip 5 byte header (SIG,SIG,VI,SIZE,SIZE)
	#else
		enclen = declen;
	#endif
	
	if (enclen == declen) {
		// not compressed
	} else {
		vagi_res_printf_res(kind, num);
		printf("Compressed:");
		printf_x2(enclen >> 8); printf_x2(enclen & 0xff);
		printf(" -> ");
		printf_x2(declen >> 8); printf_x2(declen & 0xff);
		printf("...");
		if (vi & 0x80) {
			//vagi_res_printf_res(kind, num);
			printf("Comp.PIC not supp.!");getchar();
		} else {
			//vagi_res_printf_res(kind, num);
			printf("LZW not supp.!");getchar();
		}
	}
	
	// Activate state
	state->active = true;
	state->romfs_handle = rh;
	state->offset = 0;
	
	return h;
}

// Abstract away
//#define vagi_res_close() romfs_fclose(agi_res_h)
//#define vagi_res_peek() romfs_fpeek(agi_res_h)
//#define vagi_res_skip(n) romfs_fskip(agi_res_h, n)

void vagi_res_close(vagi_res_handle_t h) {
	// Close underlying romfs handle
	romfs_fclose(vagi_res_states[h].romfs_handle);
	// Mark handle as free
	vagi_res_states[h].active = false;
}
byte *vagi_res_point(vagi_res_handle_t h) {
	// Return a pointer (not banking-safe!)
	return romfs_fpoint(vagi_res_states[h].romfs_handle);
}
byte inline vagi_res_peek(vagi_res_handle_t h) {
	return romfs_fpeek(vagi_res_states[h].romfs_handle);
}
void inline vagi_res_skip(vagi_res_handle_t h, word n) {
	vagi_res_state_t *state = &vagi_res_states[h];
	romfs_fskip(state->romfs_handle, n);
	state->offset += n;
}
void vagi_res_seek_to(vagi_res_handle_t h, word n) {
	vagi_res_state_t *state = &vagi_res_states[h];
	
	romfs_frewind(state->romfs_handle);
	romfs_fskip_far(state->romfs_handle, state->res_ofs_hi, state->res_ofs);
	romfs_fskip(state->romfs_handle, 5);	// Skip resource header (0x1234, vol, sizeLO, sizeHI)
	romfs_fskip(state->romfs_handle, n);	// Skip into data
	state->offset = n;
}
word inline vagi_res_tell(vagi_res_handle_t h) {
	return vagi_res_states[h].offset;
}
word inline vagi_res_size(vagi_res_handle_t h) {
	return vagi_res_states[h].res_size;
}

int vagi_res_read(vagi_res_handle_t h) {
	vagi_res_states[h].offset++;	// agi_res_size != R_FILES[].size, because a RES is just a part of a big VOL file
	//@TODO: Check for End-of-resource!
	return romfs_fread(vagi_res_states[h].romfs_handle);
}
word vagi_res_read_word(vagi_res_handle_t h) {
	//vagi_res_states[h].offset += 2;	// agi_res_size != R_FILES[].size, because a RES is just a part of a big VOL file
	//return romfs_fread_word(vagi_res_states[h].romfs_handle);
	word r;
	r = vagi_res_read(h);	// LO
	r |= vagi_res_read(h) << 8;	// HI
	return r;
}

bool vagi_res_eof(vagi_res_handle_t h) {
	// End-of-resource (agi_res_size) != R_FILES[].size, because a RES is just a part of a big VOL file
	//return romfs_feof(agi_res_h);
	if (!vagi_res_states[h].active) return true;
	return vagi_res_states[h].offset >= vagi_res_states[h].res_size;
}

#endif