#ifndef __VGL_SOFTSERIAL_H
#define __VGL_SOFTSERIAL_H
/*

VTech Genius Leader 2000p/400x Soft Serial
==========================================

Bit-banged serial functions for V-Tech Genius LEADER 4000 series.

The parallel port has 9 output pins (1=STROBE and 2..9=D0..D7), but only ONE input pin (11=BUSY).
The stock "link cable" uses D0..D7 to select a bit and BUSY/STROBE to receive data bit by bit from the PC.

This file, however, implements a standard UART using only 3 pins.
A Z80 at 4~6 MHz is fast enough to read and send data at 9600 baud pretty reliably.

Beware: There is no interrupt, so your program must poll the port as often as possible in order to not miss a byte!
I therefore strongly advise you to implement error checking and re-sending in your serial protocol.


Use the following wiring:

	FTDI 5V       dir  V-Tech Parallel Port
	------------  ---  ------------------------------------
	RXD (yellow)  <--    2-9 (D0..D7) - any pin will do, as I am banging ALL of them...
	TXD (orange)  -->     11 (BUSY) - with a 1k pull down resistor to GND
	GND (black)   <->  18-25 (GND) - any will do as they are all connected
	...........   <--      1 (STROBE) - not needed, but in case you want a CLK signal...


Here it is again, in order:

	FTDI	1    	2	3	4	5	6
	FTDI	black	brn	red	ora	yel	grn
	FTDI	GND  	CTS	+5V	TXD	RXD	DTR

	Para	18-25	---	---	11	3	---
	Para	GND  	---	---	BUS	D1	---

	extra	GND  	---	---	PDn	---	---


2017-01-08 Bernhard "HotKey" Slawik (Z88DK)
2019-07-11 Bernhard "HotKey" Slawik (SDCC)

*/


// Hardware helpers
/*
byte port_in_0x10() __naked {
__asm
	in	a, (0x10)	; Get printer DATA
	ld	l, a
	ret
__endasm;
}
byte port_in_0x11() __naked {
	// port 0x11: 0x7F=idle, 0x5f=connected
__asm
	in	a, (0x11)	; Get printer status
	ld	l, a
	ret
__endasm;
}
byte port_in_0x12() __naked {
__asm
	in	a, (0x12)	; Get VGL status (CAPS, STOBE, ...)
	ld	l, a
	ret
__endasm;
}
void inline port_out_0x10(byte c) {
(void)c;	// Suppress warning "unreferenced"
__asm
	out	(#0x10), a	; Set printer DATA
__endasm;
}
void inline port_out_0x11(byte c) {
(void)c;	// Suppress warning "unreferenced"
__asm
	out	(#0x11), a	; Set printer status
__endasm;
}
void inline port_out_0x12(byte c) {
(void)c;	// Suppress warning "unreferenced"
__asm
	out	(#0x12), a	; Set VGL status (CAPS, STOBE, ...)
__endasm;
}
*/

/*
void serial_init() {
	// Init the serial port
	__asm
		ld	a, #0x00				; Set all D0-D7 to LOW
		out	(0x10), a
		
		ld	a, #0xff				; Send all data bits to the pins
		out	(0x11), a
	__endasm;
}
*/

void serial_put(const byte *serial_put_buf, byte l) __naked {
	
	// Suppress "unreferenced" warnings
	(void)serial_put_buf;
	(void)l;
	
	
	__asm
		;di
		
		;exx	; Switch to secondary BC, DE and HL
		push af
		push bc
		push de
		;push hl
		
		
		; Param "l" into B
		ld	hl, #4+6
		add	hl, sp
		ld	b, (hl)
		
		;; Param "buffer" to HL
		;pop hl
		;push hl
		ld	hl, #2+6
		add	hl, sp
		ld	e, (hl)
		inc	hl
		ld	d, (hl)
		ex de, hl
		
		
		;push af	; AF is not exchanged by EXX
		
		; Start with initial LOW level (like in previous stop bit)
		; so that the start bit makes a distinct transition
		call _bbtx_set_LOW
		
		
		;;DEBUG: Show the length (max 9) and wait for keypress
		;push bc
		;push hl
		;push af
		;
		;ld a, b
		;add a, #0x30	; '0'+
		;push af
		;inc sp	; only 8 bit on stack
		;call _putchar
		;inc sp
		;
		;call _getchar
		;
		;pop af
		;pop hl
		;pop bc
		;;DEBUG end
		
		
		ld	a, b	; Length in B: check it...
		cp #0
		jp	z, _bbtx_end			; ...exit if it is zero (i.e. nothing to send)
		
		
		; Loop over data...
		_bbtx_loop:
			push bc						; B stores the remaining bytes
			
			ld	c, (hl)					; Get byte to send from HL into C
			
			;; Zero termination
			;ld a, c
			;cp	#0						; Check if byte is zero
			;jr	z, _bbtx_end			; ...exit if it is so (zero terminated string)
			
			
			;;DEBUG: Show each byte and wait for keypress
			;push bc
			;push hl
			;push af
			;
			;ld a, c
			;;add a, #0x30	; '0'+
			;push af
			;inc sp	; only 8 bit on stack
			;call _putchar
			;inc sp
			;
			;call _getchar
			;
			;pop af
			;pop hl
			;pop bc
			;;DEBUG end
			
			; Send all 8 data bits of C
			ld	b, #8					; 8 Bits to send
			
			; Send start bit (logical "0", HIGH level)
			call	_bbtx_set_HIGH
			call	_bbtx_delay
			
			; Calibrated NOP slide (12) for start bit at 9600 baud
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			
			; Send data bits
			_bbtx_bit_loop:
				; Send out current LSB of C
				call	_bbtx_set
				call	_bbtx_delay
				
				srl	c					; Shift down to put next bit to LSB; the bit that was lost is now in CARRY
			djnz	_bbtx_bit_loop		; Repeat B times
			
			; Send stop bit(s) (logical "1", LOW level)
			call	_bbtx_set_LOW
			call	_bbtx_delay
			
			; Normally some nops should be here. But I am pretty certain it takes enough clock cylcles to fetch the next byte
			;nop
			;nop
			
			inc	hl						; Next byte
			pop bc						; Get our length back in B
			dec	b						; decrease length
			jp	z, _bbtx_end			; ...exit if it is 0 (nothing more to send)
			
		jp	_bbtx_loop		; Go back
		;djnz	_bbtx_loop	; In B=length mode: djnz (decrements b)
		;jp	_bbtx_end
		
		
		_bbtx_set:
			; Set the output pin to the LSB of C (bit 0)
			; Try to keep the control flow of "1" and "0" about the same CPU cycle length!
			
			bit	0, c				; Check LSB of C (if it is 0, ZERO flag is set)
			jr	nz, _bbtx_set_LOW	; ...it is 1: Send LOW level
			jr	_bbtx_set_HIGH		; ...it is 0: Send HIGH level
		ret							; Unknown state - should never happen
		
		; For some reason setting LOW takes longer than HIGH.
		; So we need to calibrate both cases using "NOP slides"
		
		_bbtx_set_HIGH:					; Send HIGH level / logical "0"
			
			ld	a, #0xff				; Set all D0-D7 to HIGH
			out	(0x10), a
			
			ld	a, #0xff				; Send all data bits to the pins
			out	(0x11), a
			
			in	a, (0x12)
			or	#0x04					; STROBE LOW...
			out	(0x12), a
			
			; Calibrated NOP slide (5) for HIGH
			nop
			nop
			nop
			nop
			nop
			
			and	#0xfb					; STROBE HIGH... (takes >50us to reach HIGH)
			out	(0x12), a
		ret
		
		_bbtx_set_LOW:					; Send LOW level / logical "1"
			
			ld	a, #0x00				; Set all D0-D7 to LOW
			out	(0x10), a
			
			ld	a, #0xff				; Send all data bits to the pins
			out	(0x11), a
			
			in	a, (0x12)
			or	#0x04					; STROBE LOW...
			out	(0x12), a
			
			; Calibrated NOP slide (2) for LOW
			nop
			nop
			
			and	#0xfb					; STROBE HIGH... (takes >50us to reach HIGH)
			out	(0x12), a
		ret
		
		
		_bbtx_delay:
			push hl
			
			; Delay the length of one bit (depending on the desired baud rate and CPU speed)
			;
			; I did some measurements on my 4000 Quadro:
			;
			;ld	l, 0x40		; Measured: 8 bits in 1658880 ns, so 1 bit in 207360 E-9 s, so 4822,53 baud
			;ld	l, 0x20		; Measured: 8 bits in 983040 ns, so 1 bit in 122880 E-9 s, so 8138,02 baud
			;
			; Calculation: tpbN [ns] = tc + tpi * lN	(time per bit at setting N = constant time + time per iteration * iterations at setting N)
			; tc = tpbN - tpi * lN
			;
			; N=0x40
			;	l40 := 0x40 == 64	(given)
			;	tpb40 := 207360	(measured)
			;	tc = tpb40 - tpi * 0x40
			;
			; N=0x20
			;	l20 := 0x20 == 32	(given)
			;	tpb20 := 122880	(measured)
			;	tc = tpb20 - tpi * 0x20
			;
			; Now we have enough data to solve tpi and tc:
			;	0 = (tpb40 - tpi * 0x40) - (tpb20 - tpi * 0x20)
			;	0 = tpb40 - tpi * 0x40 - tpb20 + tpi * 0x20
			;	0 = tpb40-tpb20 - tpi * 0x20
			;	tpi = (tpb40-tpb20) / 0x20
			;	tpi = 84480 / 32
			;	tpi = 2640
			;
			;	2*tc = tpb20+tpb40 - tpi*96
			;	2*tc = 330240 - 253440
			;	2*tc = 76800
			;	tc = 38400
			;
			; For 9600 baud:
			;	tpbB9600 = 104166,67 ns
			;	tpb = tc + tpi * l
			;	tpb - tc = l * tpi
			;	l = (tpb - tc) / tpi
			;	l = 24,911
			;
			ld	l, #25	; For 9600 baud: 25 (decimal)
			
			_bbtx_delay_loop:
				dec	l
				jr	nz, _bbtx_delay_loop
			
			pop hl
		ret
		
		
		_bbtx_end:
			; Leave with level LOW (stop bit level)
			call _bbtx_set_LOW
			
			;exx		; Back to original BC, DE, HL
			;pop hl
			pop de
			pop bc
			pop af	; AF was not exchanged by EXX
			;ei		; @FIXME: Maybe they should not be enabled!
			
			ret		; Only needed if declared as "__naked"
			
	__endasm;
}


void serial_putchar(byte c) {
	serial_put(&c, 1);
}


void serial_puts(const char *s) {
	byte l;
	const char *c;
	
	// Determine length
	c = s;
	l = 0;
	while (*c != 0) {
		c++;
		l++;
	}
	
	serial_put((byte *)s, l);
}

byte serial_isReady() __naked {
__asm
	in	a, (0x11)	; Get printer status
	cp	#0x5f
	jr	nz, _serial_isReady_not

_serial_isReady_yes:
	ld	l, #1
	ret

_serial_isReady_not:
	ld	l, #0
	ret
__endasm;
}

int serial_getchar_nonblocking() __naked {
// Return byte
// If in "less blocking" mode, it returns "0" if no data was detected. In blocking mode all data is received
// Returns -1 for timeout
__asm
	;di				; Full attention!
	
	;push	hl
	push	af
	push	bc
	push	de
	
	; Get timings for inter-bit-delay
	; By trial and error: 9600 baud: D=0x03, E=0x12
	ld	d, #0x03
	ld	e, #0x12
	
	
	_brx_byte_start:
	
	ld	c, #0					; This holds the result
	
	; Wait for start bit (blocking)
	;_brx_wait_loop:
	;	in	a, (0x11)	; Get printer status
	;	cp	#0x5f
	;jr	z, _brx_wait_loop
	
	; Wait for start bit (less blocking)
	ld	b, #0x08		; ...with timeout
	;ld	b, #0x20		; ...with timeout
	_brx_wait_loop:
		in	a, (0x11)	; Get printer status
		cp	#0x7f
		jp z, _brx_bits_start	; if "0" go to byte start
		
		nop
		
		; Count down
		dec b
		ld a, b
		cp #0
		jp z, _brx_timeout	; Timeout!
		
	jr	_brx_wait_loop
	
	
	_brx_bits_start:
	; Wait half of the start bit...
	call	_brx_delayEdge
	
	
	ld	b, #8					; 8 bits is what we want
	_brx_loop:
		
		; Wait for next bit
		call	_brx_delay
		
		in	a, (0x11)			; Get port status
		
		; Extract the state of the BUSY line (parallel port pin 11, the other serial terminal is connected to that line)
		cp	#0x7f
		jp	z, _brx_got_0		; the received bit is 0
		jp	_brx_got_1
		
		
		; Both cases should be designed to use the same number of CPU cycles to complete
		_brx_got_1:				; The received bit is 1...
			ld	a, c			; Load the bits so far
			srl	a				; Move old bits down
			or	#0x80			; Set the MSB
			jp _brx_next
		
		_brx_got_0:				; The received bit is 0...
			ld	a, c
			srl	a				; Move old bits down
			and	#0x7f			; Clear the MSB
			jp _brx_next
		
		_brx_next:
			ld	c, a			; Put the temp result A back to C
		
	djnz	_brx_loop			; Loop over all bits
	; All data bits were handled
	
	; Epilogue
	; Wait 1 bit
	call _brx_delay
	
	; Wait for stop bit to actually occur
	ld	b, #0x08		; ...with timeout
	
	_brx_wait_stopBit_loop:
		dec b
		;ld a, b
		;cp a, #0
		jp z, _brx_timeout2	; Timeout!
		
		in	a, (0x11)
		cp	#0x5f
	jr	nz, _brx_wait_stopBit_loop
	
	
	; The stop bit should now be happening.
	; We are ready to receive next byte
	jp	_brx_byte_end				; Jump to end
	
	_brx_delayEdge:
		push hl
		; Delay a little to make sure the edge is over
		; This must be calibrated for the desired baud rate and the CPU speed used
		; By trial and error: H=0x01, L=0x0c works fine (0x03 >> 1, 0x18 >> 1)
		
		; Works, but could be more stable for late entry
		ld	h, d
		ld	l, e
		srl	h
		srl	h
		srl	l
		srl	l
		
		jp	_brx_delay_loop		; Skip right to the loop (of _brx_delay)
	
	_brx_delay:
		push hl
		; Set the delay. This must be calibrated for the desired baud rate and the CPU speed used
		; By trial and error: 9600 baud result in a delay of H=0x03, L=0x18
		ld	h, d
		ld	l, e
		
		_brx_delay_loop:
			nop
			nop
			nop
			
			dec	l
			jp	nz, _brx_delay_loop
			
			ld	a, h
			jp	z, _brx_delay_end
			
			dec	h
		jp	_brx_delay_loop
		
		_brx_delay_end:
			pop hl
			ret
		
	
	_brx_timeout:
		; Return -1
		ld	hl, #0xffff
		jp _brx_end
	
	_brx_timeout2:
		; Return -1
		ld	hl, #0xfffe
		jp _brx_end
	
	_brx_byte_end:
		; Set return value L to the character stored in C
		ld	h, #0
		ld	l, c
	
	_brx_end:
		pop	de
		pop	bc
		pop	af
		;pop	hl
	
	;ei
	ret
	
__endasm;
}

byte serial_getchar() {
	int c;
	
	// Wait until a byte is there
	while(1) {
		c = serial_getchar_nonblocking();
		if (c <= 0) continue;
		
		return (byte)(c & 0xff);
	}
	//return c;
}

byte *serial_gets(byte *serial_get_buf) {
	int c;
	byte *b;
	
	b = serial_get_buf;
	while(1) {
		//c = serial_getchar();
		c = serial_getchar_nonblocking();
		if (c <= 0) continue;	// < 0 means "no data"
		
		// Check for end-of-line character(s)
		if ((c == 0x0a) || (c == 0x0d)) break;
		
		// Store in given buffer
		*b++ = c;
	}
	
	// Terminate string
	*b = 0;
	
	// Return length
	//return (word)b - (word)serial_get_buf;
	
	// Return buf (like stdlib gets())
	return serial_get_buf;
}
#endif //__VGL_SOFTSERIAL_H