#ifndef __VAGI_RES_H__
#define __VAGI_RES_H__

/*
Abstraction layer for opening AGI resources.
It re-directs accesses to ROM FS, which stores the raw files.

2024-09-16 Bernhard "HotKey" Slawik
*/

// ROM FS:
#define ROMFS_USE_FILENAMES	// Actually use filenames in ROMFS index data; do not open using baked-in header file
//#define ROMFS_DEBUG	// Verbose ROM access output to stdout
//#define R_MEM_OFFSET 0x4000	// Where to find the banked memory area
//#define R_BANK_SIZE 0x4000	// How big one bank is
#define ROMFS_SHOW_ROM_NAME	// Shows the ROM name briefly on startup

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


#ifdef ROMFS_USE_FILENAMES
	// Do not include any knowledge of the ROMFS data, use filename look ups instead.
	#define R_MEM_OFFSET 0x8000
	#define R_BANK_SIZE 0x4000
#else
	// The ROMFS index is baked into the source for maximum performance (no string comparisons)
	#include "romfs_data.h"
#endif

#include "romfs.c"	//@FIXME: Only the main C file gets passed to the VGLDK compiler pass...

#define VAGI_RES_MAX_HANDLES 6

#define VAGI_RES_ERROR_OUT_OF_HANDLES -11
#define VAGI_RES_ERROR_SIGNATURE_ERROR -12
#define VAGI_RES_ERROR_VOLUME_ERROR -13
#define VAGI_RES_ERROR_VOLUME_MISS -14
#define VAGI_RES_ERROR_UNKNOWN_KIND -15


//#define VAGI_RES_IGNORE_SIGNATURE_ERROR	// Not recommended, just for debugging quirks among different games
//#define VAGI_RES_IGNORE_VOLUME_MISS	// Not recommended, just for debugging quirks among different games
//#define VAGI_RES_IGNORE_COMPRESSED

#define AGI_RES_MAX_VOL 6	// Keep it at 6 (i.e. VOL.0-VOL.5)

enum AGI_RES_KIND {
	AGI_RES_KIND_LOG,
	AGI_RES_KIND_PIC,
	AGI_RES_KIND_SND,
	AGI_RES_KIND_VIEW,
	AGI_RES_KIND_WORDS,
	
	AGI_RES_KIND_COUNT
};

//const char * AGI_RES_KIND_ID[AGI_RES_KIND_COUNT] = {
//	"LOG",
//	"PIC",
//	"SND",
//	"VIEW",
//	"WORDS",	// Special!
//};
static const char AGI_RES_KIND_ID[AGI_RES_KIND_COUNT] = {
	'L',
	'P',
	'S',
	'V',
	'W'	// words
};

#ifdef ROMFS_USE_FILENAMES
	// Use filenames
	/*
	const char *AGI_DIR_FILENAMES[AGI_RES_KIND_COUNT] = {
		"LOGDIR",
		"PICDIR",
		"SNDDIR",
		"VIEWDIR",
		"WORDS.TOK",	// words!
	};
	const char *AGI_VOL_FILENAMES[AGI_RES_MAX_VOL] = {
		"VOL.0",
		"VOL.1",
		"VOL.2",
		"VOL.3",
		"VOL.4",
		"VOL.5",
	};
	*/
	// Dynamic indices, get resolved on startup.
	extern byte AGI_DIR_INDICES[AGI_RES_KIND_COUNT];
	extern byte AGI_VOL_INDICES[AGI_RES_MAX_VOL];
	byte AGI_DIR_INDICES[AGI_RES_KIND_COUNT];
	byte AGI_VOL_INDICES[AGI_RES_MAX_VOL];
#else
	// Use baked-in data from romfs_data.h
	static const byte AGI_DIR_INDICES[AGI_RES_KIND_COUNT] = {
		R_LOGDIR,
		R_PICDIR,
		R_SNDDIR,
		R_VIEWDIR,
		R_WORDS_TOK,
	};
	static const byte AGI_VOL_INDICES[] = {
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
#endif


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
	byte i;
	//int idx;
	
	romfs_init();
	
	// Mark all states as inactive
	for (i = 0; i < VAGI_RES_MAX_HANDLES; i++) {
		vagi_res_states[i].active = false;
	}
	
	#ifdef ROMFS_USE_FILENAMES
		// Resolve filenames to ROMFS indices
		/*
		for (i = 0; i < AGI_RES_KIND_COUNT; i++) {
			printf("Resolving DIR \""); printf(AGI_DIR_FILENAMES[i]); printf("\"...");
			idx = romfs_find(AGI_DIR_FILENAMES[i]);
			AGI_DIR_INDICES[i] = idx;
		}
		*/
		AGI_DIR_INDICES[AGI_RES_KIND_LOG] = romfs_find("LOGDIR");
		AGI_DIR_INDICES[AGI_RES_KIND_PIC] = romfs_find("PICDIR");
		AGI_DIR_INDICES[AGI_RES_KIND_SND] = romfs_find("SNDDIR");
		AGI_DIR_INDICES[AGI_RES_KIND_VIEW] = romfs_find("VIEWDIR");
		AGI_DIR_INDICES[AGI_RES_KIND_WORDS] = romfs_find("WORDS.TOK");
		
		/*
		for (i = 0; i < AGI_RES_MAX_VOL; i++) {
			printf("Resolving VOL \""); printf(AGI_VOL_FILENAMES[i]); printf("\"...");
			idx = romfs_find(AGI_VOL_FILENAMES[i]);
			AGI_VOL_INDICES[i] = idx;
		}
		*/
		AGI_VOL_INDICES[0] = romfs_find("VOL.0");
		AGI_VOL_INDICES[1] = romfs_find("VOL.1");
		AGI_VOL_INDICES[2] = romfs_find("VOL.2");
		AGI_VOL_INDICES[3] = romfs_find("VOL.3");
		AGI_VOL_INDICES[4] = romfs_find("VOL.4");
		AGI_VOL_INDICES[5] = romfs_find("VOL.5");
		//printf("resolved"); getchar();
	#endif
}

vagi_res_handle_t vagi_res_open(byte kind, word num) {
	byte b1, b2, b3;
	byte vi;
	word declen;
	word enclen;
	vagi_res_handle_t h;
	romfs_handle_t rh;
	byte ri;
	
	// Find a free handle/state
	
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
	
	// Initialize a new vagi_res_state
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
	
	ri = AGI_DIR_INDICES[kind];
	rh = romfs_fopen(ri);
	if (rh < 0) {
		vagi_res_printf_res(kind, num);
		printf("DIR err=-"); printf_d(-rh); getchar();
		//return 0;
		return rh;
	}
	
	if (kind == AGI_RES_KIND_WORDS) {
		// WORDS.TOK is not a dir, so: Return the first handle right away!
		// Activate state
		state->active = true;
		state->romfs_handle = rh;
		state->offset = 0;
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
	ri = AGI_VOL_INDICES[state->res_vol];
	rh = romfs_fopen(ri);
	if (rh < 0) {
		vagi_res_printf_res(kind, num);
		printf("VOL."); printf_d(state->res_vol);
		printf(" err=-"); printf_d(-rh);
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
		printf("VOL miss:");
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
		#ifdef VAGI_RES_IGNORE_COMPRESSED
		// Ignore
		#else
		// Show info
		vagi_res_printf_res(kind, num);
		printf("Compr:");
		printf_x2(enclen >> 8); printf_x2(enclen & 0xff);
		printf(" -> ");
		printf_x2(declen >> 8); printf_x2(declen & 0xff);
		//printf("...");
		#endif
		if (vi & 0x80) {
			#ifdef VAGI_RES_IGNORE_COMPRESSED
			// Ignore
			#else
			vagi_res_printf_res(kind, num);
			printf("Comp.PIC not supp.!");
			getchar();
			#endif
		} else {
			#ifdef VAGI_RES_IGNORE_COMPRESSED
			// Ignore
			#else
			vagi_res_printf_res(kind, num);
			printf("LZW not supp.!");
			getchar();
			#endif
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