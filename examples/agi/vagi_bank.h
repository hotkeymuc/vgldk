#ifndef __VAGI_BANK_H__
#define __VAGI_BANK_H__
/*

	Bank switching stuff

*/

// Define GL6000SL bank switching ports:
//	0x50 = 0x0000 - 0x3fff
//	0x51 = 0x4000 - 0x7fff
//	0x52 = 0x8000 - 0xbfff
//	0x53 = 0xc000 - 0xdfff
//	0x54 = 0xe000 - 0xffff (VRAM at 0xe000 - 0xebb7)
volatile __sfr __at 0x50 bank_0x0000_port;
volatile __sfr __at 0x51 bank_0x4000_port;
volatile __sfr __at 0x52 bank_0x8000_port;
volatile __sfr __at 0x53 bank_0xc000_port;
volatile __sfr __at 0x54 bank_0xe000_port;

// RAM segments (values to write to the bank ports):
//	bank 0	0x0000 - 0x1fff	8KB VRAM (3000 bytes) at 0xe000 - 0xebb7 and RAM (5192 bytes) at 0xebb8 - 0xffff
//	bank 1	0x2000 - 0x3fff	8KB default RAM at 0xc000 - 0xdfff
//	bank 2	0x4000 - 0x5fff	8KB extended RAM at 0xc000 - 0xdfff
//	bank 3	0x6000 - 0x7fff	8KB extended RAM at 0xc000 - 0xdfff

//	0x55: (usually 0x1C = 0b00011100)
//		lowest bit set: Map cartridge at 0x0000 (e.g. "out 55 1d" maps cart to 0x0000 AND 0x8000)
//		2nd    bit set: Map cartridge at 0x4000 (e.g. "out 55 1e" maps cart to 0x4000 AND 0x8000)
//		3rd    bit unset: Reboot loop (e.g. "out 55 18")
//		4th    bit unset: Reboot loop (e.g. "out 55 14")
//		5th    bit unset: nothing, doesn't even gets retained (turns back to 0x1c)
//		!! out 55 0c -> Crash sometimes
//		!! out 55 3c -> Crash with Capslock LED on
//		!! out 55 9c -> nothing, doesn't even gets retained (turns back to 0x1c)
//		!! out 55 fc -> System powers off!
volatile __sfr __at 0x55 bank_type_port;	// This controls whether a bank is mapped to internal ROM (0) or external cartridge ROM (1)


// Bank switched code
typedef void (t_code_segment_call)(void);
typedef void (t_code_segment_call_b)(byte a);

//static word bank_sp;

// Note: This function must stay at a static address, no matter which segment is active!
//void code_segment_call(word addr) {
void inline code_segment_call(word addr) {
	// Switch to other segment (let's hope the switch-a-roo works!)
	//byte b0x0000 = bank_0x0000_port;
	//byte b0x4000 = bank_0x4000_port;
	
	bank_0x0000_port = 2;
	bank_0x4000_port = 3;
	/*
	__asm
	push af
	push bc
	push de
	push hl
	__endasm;
	*/
	
	/*
	// Safe SP
	__asm
	; Store SP in HL
	ld	hl, #0
	add hl, sp
	; Store SP in RAM
	ld	(_bank_sp), hl
	__endasm;
	printf("Before: SP="); printf_x2(bank_sp >> 8); printf_x2(bank_sp & 0xff); putchar('\n');
	*/
	
	// Call the given address as a function...
	(*((t_code_segment_call *)addr))();
	
	/*
	// Safe SP
	__asm
	; Store SP in HL
	ld	hl, #0
	add hl, sp
	; Store SP in RAM
	ld	(_bank_sp), hl
	__endasm;
	printf("After: SP="); printf_x2(bank_sp >> 8); printf_x2(bank_sp & 0xff); putchar('\n');
	*/
	
	/*
	__asm
	pop hl
	pop de
	pop bc
	pop af
	__endasm;
	*/
	// Switch back to main segment
	bank_0x0000_port = 0;	//b0x0000;	//0;
	bank_0x4000_port = 1;	//b0x4000;	//1;
}

/*
void inline code_segment_dump() {
	#ifdef CODE_SEGMENT_0
		printf("seg0");
	#endif
	#ifdef CODE_SEGMENT_1
		printf("seg1");
	#endif
}
*/

//void code_segment_call_b(word addr, byte a) {
void inline code_segment_call_b(word addr, byte a) {
	// Switch to other segment (let's hope the switch-a-roo works!)
	
	//code_segment_dump();
	
	//printf("bank0000="); printf_d(bank_0x0000_port); putchar('\n');
	//printf("bank4000="); printf_d(bank_0x4000_port); putchar('\n');
	//byte b0x0000 = bank_0x0000_port;
	//byte b0x4000 = bank_0x4000_port;
	bank_0x0000_port = 2;
	bank_0x4000_port = 3;
	
	//code_segment_dump();
	
	/*
	__asm
	push af
	push bc
	push de
	push hl
	__endasm;
	*/
	
	/*
	// Safe SP
	__asm
	; Store SP in HL
	ld	hl, #0
	add hl, sp
	; Store SP in RAM
	ld	(_bank_sp), hl
	__endasm;
	printf("Before: SP="); printf_x2(bank_sp >> 8); printf_x2(bank_sp & 0xff); putchar('\n');
	*/
	
	// Call the given address as a function...
	(*((t_code_segment_call_b *)addr))(a);
	
	/*
	// Safe SP
	__asm
	; Store SP in HL
	ld	hl, #0
	add hl, sp
	; Store SP in RAM
	ld	(_bank_sp), hl
	__endasm;
	printf("After: SP="); printf_x2(bank_sp >> 8); printf_x2(bank_sp & 0xff); putchar('\n');
	*/
	
	//code_segment_dump();
	
	// Switch back to main segment
	//bank_0x4000_port = b0x4000;	// Usually 1;
	//bank_0x0000_port = b0x0000;	// Usually 0;
	bank_0x0000_port = 0;
	bank_0x4000_port = 1;
	
	//code_segment_dump();
	
	//printf("bank end!");getchar();
}

#endif	// __VAGI_BANK_H__