#ifndef __VGL_LED_H
#define __VGL_LED_H
/*

	LED(s) for the VTech Genius Leader 4000

	There is actually just one (Scroll Lock).
	It is bound to bit 5 of port 0x12.
	
	TODO:
	This could also be moved to include/drivers/led.h

*/

#define LED_PORT 0x12
#define LED_MASK 0x20

__sfr __at LED_PORT led_port;

void led_on() {
	led_port |= LED_MASK;
}
void led_off() {
	led_port &= ~LED_MASK;
}

#endif	// __VGL_LED_H