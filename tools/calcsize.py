#!/usr/bin/python
"""
Analyze a bin file and show the actual used size.
Useful for trimming down a binary file to fit a desired EEPROM.

2020-09-04 Bernhard "HotKey" Slawik
"""
import sys

def put(t):
	print(str(t))

def human_readable(s):
	
	#return '%d Bytes %d Kbit / 0x%04X / %.1f KB' % (s, (s*8) // 1024, s, s/1024.0 )
	
	#sep = ' / '
	sep = '\t'
	r = sep.join(['% 6d Bytes' % s, '0x%06X Bytes' % s, '% 8.2f Kbit' % ((s*8) / 1024.0), '% 6.1f KB' % (s/1024.0)])
	return r

if __name__ == '__main__':#
	if len(sys.argv) < 2:
		put('Specify a .bin file!')
		sys.exit(1)
	
	filename = sys.argv[1]
	put('Filename  : "%s"' % filename)
	
	with open(filename, 'rb') as h:
		data = h.read()
	
	l = len(data)
	put('Filesize  : %s' % human_readable(l))
	
	while data[l-1] == 0xff:
		l -= 1
	
	put('Used space: %s' % human_readable(l))
	
