/*
	Trying softserial using pure C
	
	
	Measurements:
		9600baud = 104,1666us/bit
		
		delay	measurement	us/bit	baud
		L=25	1.12ms for 8	140us	7142
		L=21	1.04ms for 8	130us	7692
		L=10	804us for 8	100,5us	9950
		L=11	824us for 8	103us	9708	OK!
		L=12	844us for 8	105,5us	9478
*/

#include <vgldk.h>
#include <stdiomin.h>

__sfr __at 0x10 cs_port_data;
__sfr __at 0x11 cs_port_latch;
__sfr __at 0x12 cs_port_control;

void cs_set(byte data) {
	
	cs_port_data = data;
	cs_port_latch = 0xff;
	cs_port_control |= 0x04;	// STROBE low
	
	__asm
		// 5 NOPS for 0xff, 2 NOPS for 0x00
		nop
		nop
	__endasm;
	
	cs_port_control &= 0xfb;	// STROBE high
}
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
		
		;ld	l, #25	; softserial.c on GL4000: For 9600 baud: 25 (decimal)
		ld	l, #11
		
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
		d = d >> 1;
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
