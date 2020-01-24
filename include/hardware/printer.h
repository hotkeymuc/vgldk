#ifndef __VGL_PRINTER_H
#define __VGL_PRINTER_H
/*
V-Tech Genius Leader Printer

2019-07-10 Bernhard "HotKey" Slawik

===> UNTESTED!

VGL4000 connects 11 lines
	Pin	Signal	Access
	 1	STROBE	WRITE: port[0x12] 0x04=LOW 0x00=HIGH
	 2	D0    	WRITE: port[0x10]
	 3	D1    	WRITE: port[0x10]
	 4	D2    	WRITE: port[0x10]
	 5	D3    	WRITE: port[0x10]
	 6	D4    	WRITE: port[0x10]
	 7	D5    	WRITE: port[0x10]
	 8	D6    	WRITE: port[0x10]
	 9	D7    	WRITE: port[0x10]
	11	BUSY  	READ: port[0x11]: in a, (11h); cp 0x5f
	18-25	GND	
	
	also maybe used:
		port[0x12] 0x01	to turn something on/off?
*/

byte vgl_printer_isReady() __naked {
// Wait until BUSY line is cleared (1) or timeout (0)
__asm
	; Wait for port 0x11 bit 5 to go low (with time out)
	push	bc
	ld	bc, #0xff		; Retry counter
	ld	l, #0x01		; Initial return value l=1
	
	_printer_check_loop:
		
		in	a, (0x11)	; Read port 0x11...
		bit	5, a		; ...and test if bit 5 is set
		
		jr	nz, _printer_check_end	; If set: Return
	djnz	_printer_check_loop	; If not: retry 255 times
	
	; Timeout: l=0
	ld	l, #0x00
	
	_printer_check_end:
	pop	bc
	ret
__endasm;
}

void vgl_printer_putchar(byte c) __naked {
	(void)c;	// Suppress warning "not used"
	
	// Output a byte to the parallel port as it is done in MODEL4000 firmware
	/*
		Port		0x10	0x11	0x12
					...		...		...
		data		~X >	...		...
		TX?			...		FF 		...
					...		...		<<<
		strobe		...		...		+4 >
		strobe		...		...		-4 >
					...		...		...
	*/
	
	// First we should check if the printer is busy or not
	//print_check():
	
__asm
	
	; Get param
	ld	hl, #2
	add	hl, sp
	ld	l, (hl)		; Param into L
	
	; Get dirty
	di				; Put your gloves on!
	
	
	ld	a, l		; Get the byte into A...
	cpl				; ...invert it...
	out	(0x10),a	; ...and send it to the parallel port data pins

	nop
	nop
	nop
	nop
	nop
	
	ld	a, #0xff
	out	(0x11),a	; Send 0xff to port 0x11 (transmit it?)
	
	nop
	nop
	nop
	
	; Strobe: Set Parallel port pin 1 LOW
	in	a,(0x12)	; Get port 0x12...
	or	#0x04		; ...set bit 3...
	
	nop
	nop
	
	out	(0x12),a	; ...and send it back to port 0x12
	
	nop
	nop
	nop
	nop
	
	; Strobe end: Set Parallel port pin 1 HIGH again
	and	#0x0fb		; Clear bit 3...
	out	(0x12),a	; ...and send it back to port 0x12
	
	ei				; Back to normal
	ret
__endasm;
}

void printer_get(byte *printer_get_buf) __naked {
	(void)printer_get_buf;	// Suppress warning of "not used"
	
	// Get a character from parallel port?
	// Does this implement the PC-Link thing?
	// It seems to be requesting a byte by sending out a bit mask and receiving the result on the status register
	// This can be seen at 0x01a6 of MODEL4000 ROM
	/*
		Port		0x10	0x11	0x12
					...		...		...
					...		FF >	...
		FOR X = [0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00]
		mask		X  >	...		...
					...		<<<		...
					<<<		...		...
					00 >	...		...
		LOOP
		strobe		...		...		<<<
		strobe		...		...		+0x20 >
					...		...		...
	*/

__asm
	
	pop bc
	pop hl	; Pointer to buffer
	push hl
	push bc
	
	;ld	hl, _printer_get_buf		; Where to put the data
	
	
	ld	a, #0xff
	out	(0x11), a					; Send 0xff to port 0x11
	
	
	ld	c, #0x80					; Set mask to 0b10000000
	ld	b, #0x08					; 8 rounds of it (bit by bit)
	
	; Loop:
	_printer_get_loop:
		
		ld	a, c					; Get mask from C
		out	(0x10),a				; Send mask to port 0x10
		
		in	a, (0x11)				; Read port 0x11
		ld	(hl), a					; ...and store in buffer
		inc	hl
		
		in	a,(0x10)				; Read port 0x10
		ld	(hl), a					; ...and store in buffer
		inc	hl
		
		xor	a
		out	(0x10),a				; Send 0x00 to port 0x10
		
		srl	c						; Shift mask right
	djnz	_printer_get_loop		; Decrease B and check if all 8 rounds are done
	; Next round/bit
	
	
	;; Disable CAPS LOCK?
	;in	a,(0x12)					; Read port 0x12...
	;or	#0x20						; ...set bit 5...
	;nop
	;nop
	;out	(0x12),a					; ...and send it to port 0x12
	
	ret
__endasm;
	
}
#endif //__VGL_PRINTER_H