#ifndef __VAGI_RES_H__
#define __VAGI_RES_H__

// ROM FS:
#define R_MEM_OFFSET 0x4000	// Where to find the banked memory area
#define R_BANK_SIZE 0x4000	// How big one bank is
#define romfs_switch_bank(bank) bank_0x4000_port = (0x20 | bank)	// and: bank_type_port = bank_type_port | 0x02

#include "romfs.h"
#include "romfs_data.h"
#include "romfs.c"	//@FIXME: Only the main C file gets passed to the VGLDK compiler pass...
enum AGI_RES_KIND {
	AGI_RES_KIND_LOG,
	AGI_RES_KIND_PIC,
	AGI_RES_KIND_SND,
	AGI_RES_KIND_VIEW
};

static const byte AGI_DIR_FILES[4] = {
	R_LOGDIR,
	R_PICDIR,
	R_SNDDIR,
	R_VIEWDIR,
};

static const byte AGI_VOL_FILES[4] = {
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
};


// We only use one handle
romfs_handle_t agi_res_h;
word agi_res_ofs;
word agi_res_size;

word agi_res_open(byte kind, word num) {
	romfs_handle_t h;
	byte b1, b2, b3;
	byte res_vol, res_ofs_hi;
	word res_ofs;
	
	// Close previous
	if (romfs_factive(agi_res_h)) {
		romfs_fclose(agi_res_h);
	}
	
	// Mount our cartridge ROM to address 0x4000 (data must be inside the ROM binary at position 0x4000 * n)
	bank_type_port = bank_type_port | 0x02;	// Switch address region 0x4000-0x7FFF to use cartridge ROM (instead of internal ROM)
	//bank_0x4000_port = 0x20 | 1;	// Mount ROM segment n=1 (offset 0x4000 * n) to address 0x4000
	
	
	// Open directory for the given kind (LOG, PIC, SND, VIEW)
	//printf("Opening DIR...");
	h = romfs_fopen(AGI_DIR_FILES[kind]);
	if (h < 0) {
		printf("DIR err=-"); printf_d(-h); putchar('\n');
		return 0;
	}
	
	// Seek to given entry number (3 bytes each)
	romfs_fseek(h, num*3);
	
	// Get the 3 data bytes
	b1 = romfs_fread(h);
	b2 = romfs_fread(h);
	b3 = romfs_fread(h);
	romfs_fclose(h);
	
	// Extract volume and offset
	res_vol = (b1 & 0xf0) >> 4;
	res_ofs_hi = (b1 & 0x0f);	// << 16
	res_ofs = (b2 << 8) | b3;
	
	//printf("vol="); printf_d(res_vol); printf(", ofs="); printf_x2(res_ofs_hi); printf_x2(res_ofs >> 8); printf_x2(res_ofs & 0xff); printf("\n");
	
	// Open volume file (assume the VOL.* files are located sequencially)
	//printf("Opening VOL...");
	//h = romfs_fopen(R_VOL_0 + res_vol);
	h = romfs_fopen(AGI_VOL_FILES[res_vol]);
	if (h < 0) {
		printf("VOL."); printf_d(res_vol); printf(": err=-"); printf_d(-h); putchar('\n');
		return 0;
	}
	
	// Seek to address (Up to 24 bit = far!)
	romfs_fseek_far(h, res_ofs_hi, res_ofs);
	
	// Check signature (0x1234)
	b1 = romfs_fread(h);
	b2 = romfs_fread(h);
	if ((b1 != 0x12) || (b2 != 0x34)) {
		// Signature mismatch!
		printf("SIG err!\n");
		//printf("SIG err: "); printf_x2(b1); printf_x2(b2); putchar('\n');	//printf(" != 1234\n");
		romfs_fclose(h);
		return 0;
	}
	// Check resource volume value
	b1 = romfs_fread(h);
	if (b1 != res_vol) {
		// Volume mismatch (volume file number VS stored volume number of resource)
		printf("VOL err!\n");
		//printf("VOL err: "); printf_d(b1); putchar('\n');	//printf(" != "); printf_d(res_vol); printf("\n");
		romfs_fclose(h);
		return 0;
	}
	
	// Read size (LO-HI)
	b1 = romfs_fread(h);
	b2 = romfs_fread(h);
	agi_res_size = b1 | (b2 << 8);
	agi_res_ofs = 0;
	
	// Leave open!
	//romfs_fclose(h);
	agi_res_h = h;
	return agi_res_size;
}

#define agi_res_close() romfs_fclose(agi_res_h)
#define agi_res_peek() romfs_fpeek(agi_res_h)

int agi_res_read() {
	agi_res_ofs++;	// agi_res_size != R_FILES[].size, because a RES is just a part of a big VOL file
	return romfs_fread(agi_res_h);
}

bool agi_res_eof() {
	//return romfs_feof(agi_res_h);
	return agi_res_ofs >= agi_res_size;
}

#endif