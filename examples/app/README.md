"Apps" are programs which are compiled in a lightweight fashion. They don't come with their own STDIO, but instead re-use the I/O from a host program.

Apps can be compiled using "make app" and uploaded via serial to the MONITOR program using "make upload".

This is intended as a quick way to test low-level hardware stuff (e.g. fine-tuning softuart NOP slides or poking at the GL6000 speech chip).