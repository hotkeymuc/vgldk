/*
	Trying softserial using pure C
	
	TODO:
	OK	Re-create TX 9600 baud: L=23, 9569 baud
	OK	Determine max. TX baud: L=1, 21739 baud
	OK	Try TX at 19200 baud
	*	Re-create RX 9600 baud
	*	Determine max. RX baud
	
*/

#include <vgldk.h>
#include <stdiomin.h>

__sfr __at 0x10 cs_port_data;
__sfr __at 0x11 cs_port_latch;
__sfr __at 0x12 cs_port_control;

// Macros are faster than calling a function, because there is no stack housekeeping necessary
#define cs_set(d) { cs_port_data = d; cs_port_latch = 0xff; cs_port_control |= 0x04; cs_port_control &= 0xfb; }
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

#define cs_set_high() cs_set(0xff)
/*
void cs_set_high() {
	cs_set(0xff);
}
*/
#define cs_set_low() cs_set(0x00)
/*
void cs_set_low() {
	cs_set(0x00);
}
*/

void cs_delay() {
	__asm
		push hl
		
		;ld	l, #25	; softserial.c on GL4000: For 9600 baud: L=25 (decimal)
		
		;ld	l, #1	; cserial.c on GL4000: L=1	=> 368us/8 => 21739 baud
		;ld	l, #2	; cserial.c on GL4000: L=2	=> 388us/8 => 20618 baud
		;ld	l, #3	; cserial.c on GL4000: L=3	=> 408us/8 => 19607 baud (works as 19200 baud!)
		;ld	l, #4	; cserial.c on GL4000: L=4	=> 436us/8 => 18348 baud
		;ld	l, #13	; cserial.c on GL4000: L=13	=> 624us/8 => 12820 baud
		;ld	l, #20	; cserial.c on GL4000: L=20	=> 772us/8 => 10362 baud
		;ld	l, #22	; cserial.c on GL4000: L=22	=> 816us/8 =>  9803 baud
		ld	l, #23	; cserial.c on GL4000: L=23	=> 836us/8 =>  9569 baud (works as 9600 baud!)
		;ld	l, #25	; cserial.c on GL4000: L=25	=> 880us/8 =>  9090 baud
		
		_cs_tx_delay_loop:
			dec	l
			jr	nz, _cs_tx_delay_loop
		
		pop hl
	__endasm;
}

void cs_sendByte(byte d) {
	byte b;
	
	// Start bit (HIGH)
	cs_set_high();
	cs_delay();
	
	// Data bits
	for(b = 0; b < 8; b++) {
		if (d & 1) {
			cs_set_high();
		} else {
			cs_set_low();
		}
		
		d >>= 1;
		cs_delay();
	}
	
	// Stop bit
	cs_set_low();
	cs_delay();
}

void cs_test(byte d) {
	byte i;
	
	cs_sendByte(d);
	
	for(i = 0; i < 0x10; i++) {
		cs_delay();
	}
}

//void main() __naked {
//void main() {
int main() {
	char c;
	
	printf("Hello World!\n");
	c = getchar();
	
	
	printf("Sending...");
	while(1) {
		
		//cs_test(0x00);
		//cs_test(0xff);
		cs_test(0x55);
		cs_test(0xaa);
		
		//printf("Key to end");
		//c = getchar();
	}
	
	//while(1) { }
	//return;
	//return c;
	return 0x43;
}
