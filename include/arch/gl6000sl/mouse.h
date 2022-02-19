#ifndef __MOUSE_H
#define __MOUSE_H

__sfr __at 0x04 mouse_port_x;
__sfr __at 0x05 mouse_port_y;
__sfr __at 0x21 mouse_port_control;
/*
	bit 3 (0x08) = Left Mouse button
	bit 4 (0x10) = Right Mouse button
	bit 5 (0x20) = CAPS LOCK light?
	bit 6 (0x40) = busy?	(see ROM 0x7E2C)
	bit 7 (0x80) = cable connected? (0x00=yes, 0x80=no) 	(see ROM 0x7DAC)
*/

#define mouse_x mouse_port_x
#define mouse_y mouse_port_y
#define mouse_button ((mouse_port_control & 0x18) >> 3)

#endif	//__MOUSE_H