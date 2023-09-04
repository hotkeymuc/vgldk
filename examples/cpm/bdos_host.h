#ifndef __BDOS_HOST_H
#define __BDOS_HOST_H

/*
BDOS Host
=========

This file contains functions to allow sending and receiving files from/to an external host machine.
The host might be a PC connected via serial (SoftUART / SoftSerial) or a (modified) MAME emulator
running inside bdos_host.py

(this might migrate into the BIOS instead of the BDOS)

2023-08-22 Bernhard "HotKey" Slawik
*/

//#define BDOS_HOST_ACTIVITY_LED	// Light up the LED on disk access (if available on current architecture)
//#define HOST_DRIVER_PAPER_TAPE	// Re-direct to BIOS paper tape routines (and let BIOS decide what to do)
// ...also check include/driver/host.h for more options!

#include "bdos.h"	// Need some numbers and functions
#include "fcb.h"	// FCB struct
#include <stringmin.h>	// memcpy, memset, strlen

#define BDOS_HOST_DMA_MAX_DATA 128	// Buffer size for DMA data
#define BDOS_HOST_STATUS_DMA_EOF 0x1A
#define BDOS_HOST_STATUS_DMA_DATA 0x20


// CP/M also offers paper tape
#ifdef HOST_DRIVER_PAPER_TAPE
	// Use BIOS paper tape functions
	#define host_send_byte	bios_punch
	#define host_receive_byte	bios_reader	// bios_reader is blocking by spec.
#endif
#include <driver/host.h>	// Abstract host functionality



// Error trace for protocols
//#define host_error(s)	bdos_printf(s)
#define bdos_host_error(s)	;	// Be quiet!


#ifdef BDOS_HOST_ACTIVITY_LED
	//  Light up LED during disk access
	//@FIXME: Should go to bios.c:bios_read and bios.c:bios_write
	#if VGLDK_SERIES == 4000
		#include <arch/gl4000/led.h>
		#define bdos_host_activity_on	led_on
		#define bdos_host_activity_off	led_off
	#else
		#error LED is currently not available on this architecture, but HOST_ACTIVITY_LED is set.
	#endif
#else
	// Disable activity LED
	#define bdos_host_activity_on()	;
	#define bdos_host_activity_off()	;
#endif

// For debugging: Trace calls
#ifdef BDOS_TRACE_CALLS
	volatile byte bdos_s;
	volatile byte bdos_p;
	void host_sendstack() {
		byte data[MAX_DATA];
		byte *pdata;
		byte *pstack;
		byte stack_count;
		
		/*
		//word bdos_sp;
		
		// Get stack pointer
		__asm
			; Write SP to bdos_param_s
			push af
			push hl
			
			ld hl, #0x04	; Skip the items we just pushed ourselves
			add hl, sp
			
			; Write to a global
			ld a, h
			ld (_bdos_s), a
			ld a, l
			ld (_bdos_p), a
			
			pop hl
			pop af
		__endasm;
		*/
		
		// Dump to serial
		pdata = &data[0];
		
		*pdata++ = 'D';	// D for Debug
		
		// Add registers
		#ifdef BDOS_SAVE_ALL_REGISTERS
			*pdata++ = bdos_param_a;
			*pdata++ = bdos_param_b;
		#else
			*pdata++ = 0;
			*pdata++ = 0;
		#endif
		
		*pdata++ = bdos_param_c;
		*pdata++ = bdos_param_d;
		*pdata++ = bdos_param_e;
		
		#ifdef BDOS_SAVE_ALL_REGISTERS
			*pdata++ = bdos_param_h;
			*pdata++ = bdos_param_l;
		#else
			*pdata++ = 0;
			*pdata++ = 0;
		#endif
		
		// Add stack pointer
		//*pdata++ = (word)bdos_sp >> 8;
		//*pdata++ = (word)bdos_sp & 0xff;
		*pdata++ = bdos_s;
		*pdata++ = bdos_p;
		
		// Add stack dump
		stack_count = 16;
		*pdata++ = stack_count;
		pstack = (byte *)(((word)bdos_s * 256) + (word)bdos_p);
		memcpy(pdata, pstack, stack_count * 2);
		
		host_send(data, 1 + 7 + 2 + 1 + stack_count * 2);
	
	}
#endif



// Protocol agnostic functions

void bdos_host_sendfcb(byte num, struct FCB *fcb) {
	byte data[1+1+2+36 + 4];	// +4 extra
	byte *pdata;
	byte *pfcb;
	byte i;
	
	// Show on screen
	//printf_d("F", num); printf(fcb2name(fcb));
	//bdos_printf("F"); bdos_printf_x2(num);
	
	// Dump to serial
	pdata = &data[0];
	
	*pdata++ = 'F';
	*pdata++ = num;
	
	*pdata++ = (word)fcb >> 8;
	*pdata++ = (word)fcb & 0xff;
	
	pfcb = (char *)fcb;
	for (i = 0; i < 36; i++) {
		*pdata++ = *pfcb++;
	}
	
	bdos_host_activity_on();
	host_send_frame(&data[0], 1+1+2+36);
	bdos_host_activity_off();
	
	// Use host_receivefcb() to receive result and altered FCB
}


byte bdos_host_receive_byte() {
	byte data[4];
	byte l;
	l = host_receive_frame(&data[0]);
	if (l != 1) {
		bdos_host_error("Rb!");
		return 0xff;
	}
	
	return data[0];
}

byte bdos_host_receivefcb(struct FCB *fcb) {
	//byte data[1 + 36 + 16];	// Leave some extra to be sure
	byte data[HOST_MAX_DATA + 4];
	byte l;
	byte r;
	
	// Light up LED during disk access! :-D
	bdos_host_activity_on();
	
	// Receive result and FCB as one data frame
	l = host_receive_frame(&data[0]);
	
	// L must be 32 or 36 for a proper FCB (32 for dir listing) plus 1 byte for the return value. Return error if not.
	if (l > (1+36)) {
		bdos_host_error("FCB>36!");
		//bdos_printf("FCB"); bdos_printf_x2(l); bdos_puts(">0x25!");
		return 0xff;
	}
	
	// Get return value
	r = data[0];	// First byte = result value
	
	if (r == 0xff) {
		//@TODO: Error codes!?
		//printf("FCB FUN ERR!\n");
	} else {
		// Copy the received data over to the FCB pointer
		memcpy((byte *)fcb, (byte *)&data[1], l);	// data[0] = result value, data[1...] = actual FCB data
	}
	
	bdos_host_activity_off();
	
	return r;
}

word bdos_host_receivedma() {
	byte data[BDOS_HOST_DMA_MAX_DATA + 4];	// 128 + headers
	byte l;
	byte *p;
	word l_total;
	byte r;
	
	p = bios_dma;
	l_total = 0;
	
	// Light up LED during disk access! :-D
	bdos_host_activity_on();
	
	while(l_total < BDOS_HOST_DMA_MAX_DATA) {
		
		// Receive one DMA chunk
		l = host_receive_frame(&data[0]);
		
		if (l == 0) {
			// Empty DMA frame! Where is the status byte? Error!
			//bdos_puts("DMA0!");
			return 0;	 // Return total length = 0
		}
		
		// Handle first byte (DMA status)
		r = data[0];
		
		if (r == BDOS_HOST_STATUS_DMA_DATA) {
			// 0x20 = DMA_DATA
			l -= 1;	// Skip first byte (status), rest is DMA data
			
			// Copy from receive buffer to dma area
			memcpy(p, &data[1], l);
			
			p += l;
			l_total += l;
			continue;
		}
		
		if (r == BDOS_HOST_STATUS_DMA_EOF) {
			// 0x1A = DMA_EOF (end of file)
			//bdos_puts("EOF");
			break;	// Return total length so far
		}
		
		// Unknown status!
		//bdos_puts("DMA-Err!");
		bdos_printf("DMAe:"); bdos_printf_x2(r);
		bdos_getchar();
		return 0;	 // Return total length = 0
		
	}
	//bdos_printf("DMA L="); bdos_printf_x2(l_total);
	
	bdos_host_activity_off();
	
	return l_total;
}

#endif