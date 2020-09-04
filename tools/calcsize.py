#!/usr/bin/python
"""
Analyze a bin file and show the actual used size.
Useful for trimming down a binary file to fit a desired EEPROM.

2020-09-04 Bernhard "HotKey" Slawik
"""
import sys
import math

def put(t):
	print(str(t))

def human_readable(s):
	
	#return '%d Bytes %d Kbit / 0x%04X / %.1f KB' % (s, (s*8) // 1024, s, s/1024.0 )
	
	ROM_MIN_SIZE_KB = 1
	
	sCeil = math.pow(2, math.ceil(math.log(s, 2))) if s > 0 else 0
	sCeil = max(1024*ROM_MIN_SIZE_KB, sCeil)
	
	#sep = ' / '
	sep = '\t'
	stats = [
		'0x%06X' % s,
		'% 6d Bytes' % s,
		'% 8.3f KByte' % (s/1024.0),
		'% 8.3f Kbit' % ((s*8) / 1024.0),
		'fits in',
		'% 4d KByte' % (sCeil // 1024),
		'% 6d Kbit' % ((sCeil*8) // 1024),
		'ROM'
	]
	r = sep.join(stats)
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
	
	l -= 1
	while (l >= 0) and (data[l] == 0xff):
		l -= 1
	l += 1
	put('Used space: %s' % human_readable(l))
	
