#ifndef __MOUSE_H
#define __MOUSE_H


// Multiple peripherals at port 0x21: Mouse buttons, printer pins, ...
byte port_in_0x21() __naked {
__asm
	in	a, (0x21)
	ld	l, a
	ret
__endasm;
}

// Mouse at 0x04, 0x05
byte port_in_0x04() __naked {	// 0x04 = Mouse X
__asm
	in	a, (0x04)
	ld	l, a
	ret
__endasm;
}
byte port_in_0x05() __naked {	// 0x05 = Mouse Y
__asm
	in	a, (0x05)
	ld	l, a
	ret
__endasm;
}

#define mouse_x port_in_0x04
#define mouse_y port_in_0x05

#endif	//__MOUSE_H