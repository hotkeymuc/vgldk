#ifndef __SOFTUART_H
#define __SOFTUART_H
/*
	Soft-Serial re-implementation using pure C
	(Using the printer port to bit-bang valid UART serial)
	
	Tested and working on GL4000 with 9600 and 19200 baud
	
	TODO:
	OK	Re-create TX 9600 baud: L=23, 9569 baud
	OK	Determine max. TX baud: L=1, 21739 baud
	OK	Try TX at 19200 baud
	
	OK	Re-create RX 9600 baud
	OK	Determine max. RX baud: ca. 38400 baud
	OK	Try RX at 19200 baud
	
	* GL6000SL support
	
	2020-08-10 Bernhard "HotKey" Slawik
*/


//#define SOFTUART_BAUD 9600
//#define SOFTUART_BAUD 19200

// GL4000
#define SOFTUART_PORT_DATA 0x10
#define SOFTUART_PORT_LATCH 0x11
#define SOFTUART_PORT_CONTROL 0x12

#define SOFTUART_BITS_LOW 0x00	// Bits to set for serial "HIGH"
#define SOFTUART_BITS_HIGH 0xff	// Bits to set for serial "LOW"
#define SOFTUART_BITS_MASK 0xff	// Bitmask on parallel port

#define SOFTUART_STROBE_MASK 0x04	// Bitmask (OR) on control port to strobe LOW (inverse for HIGH)


/*
// GL6000SL
//#define SOFTUART_PORT_DATA 0x20
//#define SOFTUART_PORT_LATCH 0x22
//#define SOFTUART_PORT_CONTROL 0x23

		_bbtx_set_LOW:					; Send LOW level / logical "0"
			
			ld	a, #0x20				; Disable latch?
			out	(0x23), a
			
			ld	a, #0xff				; Select all bits to send?
			out	(0x22), a
			
			ld	a, #0x00				; Set all data pins D0-D7 to LOW
			out	(0x20), a
			
			ld	a, #0x60				; Send all data bits to the pins
			out	(0x23), a
			
			;@FIXME: I think this can be removed (but pad with NOPs!)
			in	a, (0x21)
			and	#0xbf					; STROBE LOW...
			out	(0x21), a
			
			; Calibrated NOP slide for LOW
			; 3 nops: 92us
			; 6 nops: 98us
			; 9 nops: 104us OK!
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			
			;@FIXME: I think this can be removed (but pad with NOPs!)
			or	#0x40					; STROBE HIGH... (takes >50us to reach HIGH)
			out	(0x21), a
			
		ret
		
		_bbtx_set_HIGH:					; Send HIGH level / logical "1"
			
			ld	a, #0x20				; Disable latch?
			out	(0x23), a
			
			ld	a, #0xff				; Select all bits to send?
			out	(0x22), a
			
			ld	a, #0xff				; Set all data pins D0-D7 to HIGH
			out	(0x20), a
			
			;@FIXME: I think this can be removed (but pad with NOPs!)
			ld	a, #0x60				; Send all data bits to the pins
			out	(0x23), a
			
			;@FIXME: I think this can be removed (but pad with NOPs!)
			in	a, (0x21)
			and	#0xbf					; STROBE LOW...
			out	(0x21), a
			
			; Calibrated NOP slide for HIGH
			; 0 nops: 92us
			; 3 nops: 98us
			; 6 nops: 104us OK!
			nop
			nop
			nop
			nop
			nop
			nop
			
			;@FIXME: I think this can be removed (but pad with NOPs!)
			or	#0x40					; STROBE HIGH... (takes >50us to reach HIGH)
			out	(0x21), a
			
		ret
*/


// Timing configuration
#if SOFTUART_BAUD == 9600
	// 9600 baud:
	#define SOFTUART_DELAY_RX 16	// GL4000 at 9600 baud: 16-17
	#define SOFTUART_DELAY_TX 22	// GL4000 at 9600 baud: L=23	=> 836us/8 =>  9569 baud
#elif SOFTUART_BAUD == 19200
	// 19200 baud:
	#define SOFTUART_DELAY_RX 6	// GL4000 at 19200 baud: 5-6
	#define SOFTUART_DELAY_TX 3	// GL4000 at 19200 baud: L=3	=> 408us/8 => 19607 baud
#else
	#error "SOFTUART_BAUD not defined!"
#endif

#define SOFTUART_TIMEOUT_START 250	// Timeout while waiting for start bit
#define SOFTUART_TIMEOUT_STOP 100	// Timeout while waiting for stop bit

// For debugging: Send a "mark" pulse for gathering timings using an oscilloscope
//#define SOFTUART_MARK softuart_set_low(); softuart_set_high();
#define SOFTUART_MARK ;

// Define port accesses
__sfr __at SOFTUART_PORT_DATA softuart_port_data;
__sfr __at SOFTUART_PORT_LATCH softuart_port_latch;
__sfr __at SOFTUART_PORT_CONTROL softuart_port_control;


// Macros are faster than calling a function, because there is no stack management necessary
#define softuart_set(d) {\
	softuart_port_data = d;\
	softuart_port_latch = SOFTUART_BITS_MASK;\
	softuart_port_control |= SOFTUART_STROBE_MASK;\
	softuart_port_control &= (0xff ^ SOFTUART_STROBE_MASK);\
}

/*
void softuart_set(byte data) {
	
	softuart_port_data = data;
	softuart_port_latch = 0xff;
	softuart_port_control |= 0x04;	// STROBE low
	
	/ *
	__asm
		// 5 NOPS for 0xff, 2 NOPS for 0x00
		nop
		nop
		nop
		nop
		nop
	__endasm;
	* /
	softuart_port_control &= 0xfb;	// STROBE high
}
*/

#define softuart_set_high() softuart_set(SOFTUART_BITS_HIGH)
/*
void softuart_set_high() {
	softuart_set(0xff);
}
*/
#define softuart_set_low() softuart_set(SOFTUART_BITS_LOW)
/*
void softuart_set_low() {
	softuart_set(0x00);
}
*/


#define softuart_get() (softuart_port_latch & 0x20)
/*
byte softuart_get() {
	if ((softuart_port_latch & 0x20) == 0) return 1;	// HIGH
	else return 0;	// LOW
}
*/



// Reception
void softuart_rx_delay() {
	
	byte i;
	
	// cserial.c on GL4000 at 9600 baud: 16-17
	// cserial.c on GL4000 at 9600 baud with MARK at each bit: 9
	// cserial.c on GL4000 at ~38400 baud: 1
	// cserial.c on GL4000 at 19200 baud: 5
	for(i = 0; i < SOFTUART_DELAY_RX; i++) {
		__asm
			nop
		__endasm;
	}
	/*
	__asm
		nop
		nop
	__endasm;
	*/
	
	/*
	__asm
		push hl
		ld	l, #23	; cserial.c on GL4000: L=23	=> 2200us/8 =>  3630 baud
		_softuart_rx_delay_loop:
			dec	l
			jr	nz, _softuart_rx_delay_loop
		pop hl
	__endasm;
	*/
}
void softuart_rx_delayEdge() {
	byte i;
	for(i = 0; i < 1; i++) {
		__asm
			nop
		__endasm;
	}
}

int softuart_receiveByte() {
	byte i;
	byte b;
	
	// Wait for start bit (low) to happen
	i = 0;
	while(softuart_get() == 0x00) {
		// Handle timeout
		i++;
		if (i > SOFTUART_TIMEOUT_START) return -1;
	}
	
	// Start bit is happening
	// Delay so we hit the center of it
	softuart_rx_delayEdge();
	
	// Mark
	SOFTUART_MARK;
	
	// Receive bits
	b = 0;
	for(i = 0; i < 8; i++) {
		
		b >>= 1;
		
		softuart_rx_delay();
		
		// Mark bit
		//softuart_set_low();
		//softuart_set_high();
		
		if (softuart_get() == 0x00) {
			// "1"
			b |= 0x80;
		} else {
			// "0"
			// Caution: Keep this part of the "if" clause roughly the same number of CPU cycles!
			//b &= 0xfe;
			__asm
				nop
				nop
				nop
				nop
				nop
				nop
			__endasm;
		}
	}
	
	// Stop bit should be happening
	
	
	// Mark
	SOFTUART_MARK;
	
	//softuart_rx_delay();
	
	// Wait for stop bit (high) to happen
	i = 0;
	while(softuart_get() != 0x00) {
		// Handle timeout
		i++;
		if (i > SOFTUART_TIMEOUT_STOP) return -2;
	}
	
	return b;
}


// Transmission
void softuart_tx_delay() {
	__asm
		push hl
		
		;	ld	l, #1	; cserial.c on GL4000: L=1	=> 368us/8 => 21739 baud
		;	ld	l, #2	; cserial.c on GL4000: L=2	=> 388us/8 => 20618 baud
		;	ld	l, #3	; cserial.c on GL4000: L=3	=> 408us/8 => 19607 baud (works as 19200 baud!)
		;	ld	l, #4	; cserial.c on GL4000: L=4	=> 436us/8 => 18348 baud
		;	ld	l, #13	; cserial.c on GL4000: L=13	=> 624us/8 => 12820 baud
		;	ld	l, #20	; cserial.c on GL4000: L=20	=> 772us/8 => 10362 baud
		;	ld	l, #22	; cserial.c on GL4000: L=22	=> 816us/8 =>  9803 baud
		;	ld	l, #23	; cserial.c on GL4000: L=23	=> 836us/8 =>  9569 baud (works as 9600 baud!)
		;	ld	l, #25	; cserial.c on GL4000: L=25	=> 880us/8 =>  9090 baud
		ld	l, #SOFTUART_DELAY_TX
		
		_softuart_tx_delay_loop:
			dec	l
			jr	nz, _softuart_tx_delay_loop
		
		nop
		
		pop hl
	__endasm;
}

void softuart_sendByte(byte d) {
	byte b;
	
	// Start bit (HIGH)
	softuart_set_high();
	softuart_tx_delay();
	
	// Data bits
	for(b = 0; b < 8; b++) {
		if (d & 0x01) {
			softuart_set_low();
		} else {
			softuart_set_high();
		}
		
		d >>= 1;
		softuart_tx_delay();
	}
	
	// Stop bit
	softuart_set_low();
	softuart_tx_delay();
}

#endif