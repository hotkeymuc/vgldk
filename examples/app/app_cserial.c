/*
	Trying softserial on GL4000 using pure C
	
	TODO:
	OK	Re-create TX 9600 baud: L=23, 9569 baud
	OK	Determine max. TX baud: L=1, 21739 baud
	OK	Try TX at 19200 baud
	
	OK	Re-create RX 9600 baud
	OK	Determine max. RX baud: ca. 38400 baud
	OK	Try RX at 19200 baud
	
	2020-08-10 Bernhard "HotKey" Slawik
*/

#include <vgldk.h>
#include <stdiomin.h>


// GL4000
#define CSERIAL_PORT_DATA 0x10
#define CSERIAL_PORT_LATCH 0x11
#define CSERIAL_PORT_CONTROL 0x12

#define CSERIAL_BITS_MASK 0xff	// Bitmask on parallel port
#define CSERIAL_BITS_LOW 0x00	// Bits to set for serial "HIGH"
#define CSERIAL_BITS_HIGH 0xff	// Bits to set for serial "LOW"

#define CSERIAL_STROBE_MASK 0x04	// Bitmask (OR) on control port to strobe LOW (inverse for HIGH)

// Timing configuration
// 9600 baud:
//#define CSERIAL_DELAY_RX 16	// GL4000 at 9600 baud: 16-17
//#define CSERIAL_DELAY_TX 22	// GL4000 at 9600 baud: L=23	=> 836us/8 =>  9569 baud

// 19200 baud:
#define CSERIAL_DELAY_RX 6	// GL4000 at 19200 baud: 5-6
#define CSERIAL_DELAY_TX 3	// GL4000 at 19200 baud: L=3	=> 408us/8 => 19607 baud

#define CSERIAL_TIMEOUT_START 250	// Timeout while waiting for start bit
#define CSERIAL_TIMEOUT_STOP 100	// Timeout while waiting for stop bit



// Define port accesses
__sfr __at CSERIAL_PORT_DATA cs_port_data;
__sfr __at CSERIAL_PORT_LATCH cs_port_latch;
__sfr __at CSERIAL_PORT_CONTROL cs_port_control;


// Macros are faster than calling a function, because there is no stack housekeeping necessary
#define cs_set(d) { cs_port_data = d; cs_port_latch = CSERIAL_BITS_MASK; cs_port_control |= CSERIAL_STROBE_MASK; cs_port_control &= (0xff ^ CSERIAL_STROBE_MASK); }
/*
void cs_set(byte data) {
	
	cs_port_data = data;
	cs_port_latch = 0xff;
	cs_port_control |= 0x04;	// STROBE low
	
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
	cs_port_control &= 0xfb;	// STROBE high
}
*/

#define cs_set_high() cs_set(CSERIAL_BITS_HIGH)
/*
void cs_set_high() {
	cs_set(0xff);
}
*/
#define cs_set_low() cs_set(CSERIAL_BITS_LOW)
/*
void cs_set_low() {
	cs_set(0x00);
}
*/


#define cs_get() (cs_port_latch & 0x20)
/*
byte cs_get() {
	if ((cs_port_latch & 0x20) == 0) return 1;	// HIGH
	else return 0;	// LOW
}
*/



void cs_rx_delay() {
	
	byte i;
	
	// cserial.c on GL4000 at 9600 baud: 16-17
	// cserial.c on GL4000 at 9600 baud with MARK at each bit: 9
	// cserial.c on GL4000 at ~38400 baud: 1
	// cserial.c on GL4000 at 19200 baud: 5
	for(i = 0; i < CSERIAL_DELAY_RX; i++) {
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
		_cs_rx_delay_loop:
			dec	l
			jr	nz, _cs_rx_delay_loop
		pop hl
	__endasm;
	*/
}
void cs_rx_delayEdge() {
	byte i;
	for(i = 0; i < 1; i++) {
		__asm
			nop
		__endasm;
	}
}

int cs_receiveByte() {
	byte i;
	byte b;
	
	// Wait for start bit (low) to happen
	i = 0;
	while(cs_get() == 0x00) {
		// Handle timeout
		i++;
		if (i > CSERIAL_TIMEOUT_START) return -1;
	}
	
	// Start bit is happening
	// Delay so we hit the center of it
	cs_rx_delayEdge();
	
	//MARK start
	//cs_set_low();
	//cs_set_high();
	
	// Receive bits
	b = 0;
	for(i = 0; i < 8; i++) {
		
		b >>= 1;
		
		cs_rx_delay();
		
		// Mark bit
		//cs_set_low();
		//cs_set_high();
		
		if (cs_get() == 0x00) {
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
	
	
	// MARK
	//cs_set_low();
	//cs_set_high();
	
	//cs_rx_delay();
	
	// Wait for stop bit (high) to happen
	i = 0;
	while(cs_get() != 0x00) {
		// Handle timeout
		i++;
		if (i > CSERIAL_TIMEOUT_STOP) return -2;
	}
	
	return b;
	
}




void cs_tx_delay() {
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
		ld	l, #CSERIAL_DELAY_TX
		
		_cs_tx_delay_loop:
			dec	l
			jr	nz, _cs_tx_delay_loop
		
		nop
		
		pop hl
	__endasm;
}

void cs_sendByte(byte d) {
	byte b;
	
	// Start bit (HIGH)
	cs_set_high();
	cs_tx_delay();
	
	// Data bits
	for(b = 0; b < 8; b++) {
		if (d & 0x01) {
			cs_set_low();
		} else {
			cs_set_high();
		}
		
		d >>= 1;
		cs_tx_delay();
	}
	
	// Stop bit
	cs_set_low();
	cs_tx_delay();
}

// Just for testing: Send a byte and wait
void cs_test(byte d) {
	byte i;
	
	cs_sendByte(d);
	
	for(i = 0; i < 0x10; i++) {
		cs_tx_delay();
	}
}

//void main() __naked {
//void main() {
int main() {
	//char c;
	int i;
	
	printf("cSerial\n");
	//c = getchar();
	
	/*
	printf("Sending...");
	while(1) {
		
		//cs_test(0x00);
		//cs_test(0xff);
		cs_test(0x55);
		cs_test(0xaa);
		
		//printf("Key to end");
		//c = getchar();
	}
	*/
	
	printf("RX...\n");
	//cs_set_low();
	
	while(1) {
		
		i = cs_receiveByte();
		if (i >= 0) {
			//printf_d(i);
			//printf(" ");
			putchar(i);
			
			cs_rx_delay();	// Wait for byte transmission to end
			cs_sendByte((char)i);
		}
		
	}
	
	
	//return 0x43;
}
