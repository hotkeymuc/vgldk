#ifndef __SOFTUART_H
#define __SOFTUART_H
/*
	Soft-Serial re-implementation using pure C
	(Using the printer port to bit-bang valid UART serial)
	
	Tested and working on GL4000 with 9600 and 19200 baud
	Tested and working on GL6000SL with 9600 and 19200 baud
	
	TODO:
	* Use "OUTI" for fast port output? (Outputs (HL) to port C, decrements B, increments HL)
	OK	Re-create TX 9600 baud: L=23, 9569 baud
	OK	Determine max. TX baud: L=1, 21739 baud
	OK	Try TX at 19200 baud
	OK	Re-create RX 9600 baud
	OK	Determine max. RX baud: ca. 38400 baud
	OK	Try RX at 19200 baud
	OK GL6000SL support
	
	
	2020-08-10 Bernhard "HotKey" Slawik
*/


// Please define these values outside:
//#define SOFTUART_SERIES 4000
//#define SOFTUART_SERIES 6000
//#define SOFTUART_BAUD 9600
//#define SOFTUART_BAUD 19200


// Timeouts (for "semi-blocking" reads)
#define SOFTUART_TIMEOUT_START 250	// Timeout while waiting for start bit
#define SOFTUART_TIMEOUT_STOP 100	// Timeout while waiting for stop bit

// For debugging: Send a "mark" pulse for gathering timings using an oscilloscope
//#define SOFTUART_MARK softuart_set_HIGH(); softuart_set_LOW();
#define SOFTUART_MARK ;

// For the special case of compiling an app (VGLDK_SERIES=0) you can manually specify the hardware series for softuart
#ifndef SOFTUART_SERIES
	#define SOFTUART_SERIES VGLDK_SERIES
#endif


//@TODO: Put this into their respective "arch/XXX/system.h" once this is clean across all platforms
#if SOFTUART_SERIES == 4000
	// GL4000
	#define SOFTUART_PORT_DATA 0x10
	#define SOFTUART_PORT_LATCH 0x11
	#define SOFTUART_PORT_CONTROL 0x12
	
	#define SOFTUART_BITS_MASK 0xff	// Bitmask on parallel port
	
	#define SOFTUART_STROBE_MASK 0x04	// Bitmask (OR) on control port to strobe LOW (inverse for HIGH)
	#define SOFTUART_BITS_HIGH 0x00	// Bits (on printer port) to set for serial data = 1 / LOW
	#define SOFTUART_BITS_LOW 0xff	// Bits (on printer port) to set for serial data = 0 / HIGH
	
	// Timing configuration
	#if SOFTUART_BAUD == 9600
		// 9600 baud:
		#define SOFTUART_RX_DELAY 16	// GL4000 at 9600 baud: 16-17
		#define SOFTUART_RX_DELAY_EDGE 1	// Usually 1
		
		#define SOFTUART_DELAY_TX 22	// GL4000 at 9600 baud: L=22-23	=> 836us/8 =>  9569 baud
		#define SOFTUART_TX_OPS nop
	
	#elif SOFTUART_BAUD == 19200
		// 19200 baud:
		//#warning "SOFTUART_BAUD at 19200 has timing problems. Use with caution!"
		
		#define SOFTUART_RX_DELAY 5	// GL4000 at 19200 baud: 5-6
		#define SOFTUART_RX_OPS nop\
			nop\
			nop\
			nop\
			nop
		#define SOFTUART_RX_DELAY_EDGE 0	// Usually 1, but in this case: 0, to not miss the highest bit. Might give a warning for "unreachable code" (empty loop)
		
		
		#define SOFTUART_DELAY_TX 1	// GL4000 at 19200 baud: 1-3?
		// Not reconstructable: L=3	=> 408us/8 => 19607 baud: Need some (4-6?) DELAY_POST nops
		// L=3 with 5 nops: 58,8us => 17006 baud
		// L=2 with 4 nops: 56,0us => 17857 baud
		// L=3 with 2 nops: 56,8us => 17605 baud
		// L=2 with 2 nops: 53,0us => 18867 baud
		// L=2 with 1 nops: 52-53us => 18866-19230 baud
		// L=1 with 3 nops: 55,25us => 18433 baud
		// L=1 with 0 nops: 53,25us => 18779 baud
		//#define SOFTUART_TX_OPS nop
		
		//#define SOFTUART_TX_OPS_PRE_START nop
		#define SOFTUART_TX_OPS_POST_START nop\
			nop\
			nop\
			nop\
			nop\
			nop\
			nop\
			nop
		
		#define SOFTUART_TX_OPS_PRE_STOP nop\
			nop\
			nop\
			nop\
			nop\
			nop\
			nop\
			nop
		
		//#define SOFTUART_TX_OPS_POST_STOP nop
	#else
		#error "SOFTUART_BAUD not defined/supported on GL4000!"
	#endif
	
	// Define port accesses
	__sfr __at SOFTUART_PORT_DATA softuart_port_data;
	__sfr __at SOFTUART_PORT_LATCH softuart_port_latch;
	__sfr __at SOFTUART_PORT_CONTROL softuart_port_control;
	
	// Macros are faster than calling a function, because there is no stack management necessary
	#define softuart_set_LOW() softuart_set(SOFTUART_BITS_LOW)
	#define softuart_set_HIGH() softuart_set(SOFTUART_BITS_HIGH)
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
			// 2 NOPS for 0x00, 5 NOPS for 0xff
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
	
	#define softuart_get_LOW() ((softuart_port_latch & 0x20) > 0)
	#define softuart_get_HIGH() ((softuart_port_latch & 0x20) == 0)
	/*
	byte softuart_get() {
		if ((softuart_port_latch & 0x20) == 0) return 1;	// HIGH
		else return 0;	// LOW
	}
	*/
	
	
#elif SOFTUART_SERIES == 6000
	// GL6000SL
	// Those port names are chosen rather arbitrarily, as I can only guess...
	#define SOFTUART_PORT_DATA 0x20
	#define SOFTUART_PORT_STATUS 0x21
	#define SOFTUART_PORT_LATCH 0x22
	#define SOFTUART_PORT_CONTROL 0x23
	
	#define SOFTUART_BITS_MASK 0xff	// Bitmask on parallel port
	#define SOFTUART_BITS_HIGH 0xff	// Bits (on printer port) to set for serial "HIGH"
	#define SOFTUART_BITS_LOW 0x00	// Bits (on printer port) to set for serial "LOW"
	
	#if SOFTUART_BAUD == 9600
		// 9600 baud:
		#define SOFTUART_RX_DELAY 17	// GL6000SL at 9600 baud: 17 works
		#define SOFTUART_RX_DELAY_EDGE 1	// Usually 1
		
		#define SOFTUART_DELAY_TX 21	// GL6000SL at 9600 baud: 20-22 works
		#define SOFTUART_TX_OPS nop
	#elif SOFTUART_BAUD == 19200
		// 19200 baud:
		#define SOFTUART_RX_DELAY 7	// GL6000SL at 19200 baud: 7 works
		#define SOFTUART_RX_DELAY_EDGE 1	// Usually 1
		
		#define SOFTUART_DELAY_TX 0	// GL6000SL at 19200 baud: 0 with 0-3 post-nops works;  1 with no post-nops ALMOST works, but is too slow on last MSB
		#define SOFTUART_TX_OPS 
	#else
		#error "SOFTUART_BAUD not defined/supported on GL6000SL!"
	#endif
	
	// Define port accesses
	__sfr __at SOFTUART_PORT_DATA softuart_port_data;
	__sfr __at SOFTUART_PORT_STATUS softuart_port_status;
	__sfr __at SOFTUART_PORT_LATCH softuart_port_latch;
	__sfr __at SOFTUART_PORT_CONTROL softuart_port_control;
	
	// Macros are faster than calling a function, because there is no stack management necessary
	#define softuart_set_LOW() {\
		softuart_set(SOFTUART_BITS_LOW)\
		__asm\
			; Usually writing 0x00 requires 3 extra NOPS, but empirically thats not needed any more?\
			;nop\
			;nop\
			;nop\
		__endasm;\
	}
	#define softuart_set_HIGH() {\
		softuart_set(SOFTUART_BITS_HIGH)\
		__asm\
		__endasm;\
	}
	
	#define softuart_set(d) {\
		softuart_port_control = 0x20;\
		softuart_port_latch = SOFTUART_BITS_MASK;\
		softuart_port_data = d;\
		softuart_port_control = 0x60;\
		softuart_port_status &= 0xbf;\
		softuart_port_status |= 0x40;\
	}
	
	/*
	void softuart_set_LOW() {
		softuart_port_control = 0x20;
		softuart_port_latch = 0xff;
		softuart_port_data = 0xff;
		softuart_port_control = 0x60;
		
		softuart_port_status &= 0xbf;
		softuart_port_status |= 0x40;
		
		__asm
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
		__endasm
	}
	
	
	void softuart_set_HIGH() {
		
		__asm
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
		__endasm;
		
	}
	*/
	
	#define softuart_get_LOW() ((softuart_port_status & 0x80) == 0)
	/*
	void softserial_get_high() {
		__asm
			in	a, (0x21)			; Get port status
			; Extract the state of the BUSY line (parallel port pin 11, the other serial terminal is connected to that line)
			cp	#0x80	; when transmitting a LOW, the 7th bit (0x80) goes low
			jr c, _brx_got_HIGH	; HIGH/"0" received!
		__endasm;
	}
	*/
#else
	#error "No/unsupported VGLDK_SERIES/SOFTUART_SERIES!"
#endif



void softuart_tx_delay() {
	// Delay between bits for sending
	
	__asm
		push hl
		
		; Trials on GL4000:
		;	ld	l, #1	; => 368us/8 => 21739 baud
		;	ld	l, #2	; => 388us/8 => 20618 baud
		;	ld	l, #3	; => 408us/8 => 19607 baud (works as 19200 baud!)
		;	ld	l, #4	; => 436us/8 => 18348 baud
		;	ld	l, #13	; => 624us/8 => 12820 baud
		;	ld	l, #20	; => 772us/8 => 10362 baud
		;	ld	l, #22	; => 816us/8 =>  9803 baud
		;	ld	l, #23	; => 836us/8 =>  9569 baud (works as 9600 baud!)
		;	ld	l, #25	; => 880us/8 =>  9090 baud
		#if SOFTUART_DELAY_TX > 0
		ld	l, #SOFTUART_DELAY_TX
		
		_softuart_tx_delay_loop:
			dec	l
			jr	nz, _softuart_tx_delay_loop
		#endif
		
		#ifdef SOFTUART_TX_OPS
		; Allow some custom NOP slides
		SOFTUART_TX_OPS
		#endif
		
		pop hl
	__endasm;
}

void softuart_sendByte(byte d) {
	// Transmit one byte
	
	byte b;
	
	#ifdef SOFTUART_TX_OPS_PRE_START
	__asm
		SOFTUART_TX_OPS_PRE_START
	__endasm;
	#endif
	
	// Start bit (LOW)
	softuart_set_LOW();
	softuart_tx_delay();
	
	#ifdef SOFTUART_TX_OPS_POST_START
	__asm
		SOFTUART_TX_OPS_POST_START
	__endasm;
	#endif
	
	// Data bits
	for(b = 0; b < 8; b++) {
		if (d & 0x01) {
			softuart_set_HIGH();
		} else {
			softuart_set_LOW();
		}
		
		d >>= 1;
		softuart_tx_delay();
	}
	
	#ifdef SOFTUART_TX_OPS_PRE_STOP
	__asm
		SOFTUART_TX_OPS_PRE_STOP
	__endasm;
	#endif
	
	// Stop bit (HIGH)
	softuart_set_HIGH();
	softuart_tx_delay();
	
	#ifdef SOFTUART_TX_OPS_POST_STOP
	__asm
		SOFTUART_TX_OPS_POST_STOP
	__endasm;
	#endif
}


void softuart_rx_delay() {
	// Delay between bits for receiving
	
	byte i;
	
	// GL4000 at 9600 baud: 16-17
	// GL4000 at 9600 baud with MARK at each bit: 9
	// GL4000 at 19200 baud: 5
	// GL4000 at ~38400 baud: 1
	for(i = 0; i < SOFTUART_RX_DELAY; i++) {
		__asm
			nop
		__endasm;
	}
	
	#ifdef SOFTUART_RX_OPS
	__asm
		; Allow some custom NOP slides
		SOFTUART_RX_OPS
	__endasm;
	#endif
	
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
	// Delay between edge and middle of bit while receiving (usually a single nop)
	
	byte i;
	// Ignore compiler warnings about "comparison is always false" / "unreachable code"
	for(i = 0; i < SOFTUART_RX_DELAY_EDGE; i++) {
		__asm
			nop
		__endasm;
	}
}

int softuart_receiveByte() {
	// Receive one byte
	
	byte i;
	byte b;
	
	// Wait for start bit (LOW/0) to happen
	i = 0;
	while(softuart_get_HIGH()) {
		// Still LOW - Handle timeout
		i++;
		if (i > SOFTUART_TIMEOUT_START) return -1;
	}
	
	// Start bit is happening!
	
	// Delay so we hit the center of it
	softuart_rx_delayEdge();
	
	// Mark the start of data bits
	SOFTUART_MARK;
	
	// Receive all bits
	b = 0x00;
	
	for(i = 0; i < 8; i++) {
		
		b >>= 1;
		
		softuart_rx_delay();
		
		// Mark bit
		//SOFTUART_MARK;
		
		if (softuart_get_HIGH()) {
			// HIGH/"1"
			b |= 0x80;
		} else {
			// LOW/"0"
			
			// Caution: Keep this part of the "if" clause roughly the same number of CPU cycles!
			//b &= 0xfe;	// Not required as b is 0 from the beginning
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
	
	// Stop bit should be happening right now!
	
	// Mark the end of data bits
	SOFTUART_MARK;
	
	// Wait for stop bit (HIGH/1) to actually happen
	i = 0;
	while(softuart_get_LOW()) {
		// Still not HIGH - Handle timeout
		i++;
		if (i > SOFTUART_TIMEOUT_STOP) return -2;
	}
	
	return b;
}


#endif