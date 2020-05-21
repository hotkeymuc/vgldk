/*
Genius Leader 6000SL / Prestige printer

Write port 0x20:
	Data to be set to the pins

Write port 0x21
	0x40: STROBE on (0x40) or off (0x00)

Write port 0x22
	Select bits to latch?
	0xff is written here before setting a new output data byte

Write 0x23:
	0x40: Latch data from port 0x20 to pins

Read port 0x21:
	0x40: printer busy (0x00=yes, 0x40=no)
	0x80: cable connected? (0x00=yes, 0x80=no)



Dump of printing a character:

W 23 20	// Latch off (0x40 = 0x00)
R 21:7F	// Read old status
W 23 20	// latch off (&0x40 = 0x00)
W 22 FF	// Select all bits to output?
W 20 48	// Actual data byte "h"
W 23 60	// latch data onto pins (&0x40 = 0x40)
R 21:7F	// Read old status
W 21 3F	// STROBE on (0x40 = 0x00)
W 21 7F	// Strobe off (0x40 = 0x40)
W 23 20	// Latch off (0x40 = 0x00)

2020-05-21 Bernhard "HotKey" Slawik
*/