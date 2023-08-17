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

def human_readable(s, rom_info=True):
	
	#return '%d Bytes %d Kbit / 0x%04X / %.1f KB' % (s, (s*8) // 1024, s, s/1024.0 )
	
	
	#sep = ' / '
	sep = '\t'
	stats = [
		'0x%06X' % s,
		'% 6d Bytes' % s,
		'% 7.3f KByte' % (s/1024.0),
		'% 7.3f Kbit' % ((s*8) / 1024.0)
	]
	
	if rom_info:
		ROM_MIN_SIZE_KB = 1
		s_ceil = math.pow(2, math.ceil(math.log(s, 2))) if s > 0 else 0
		s_ceil = max(1024*ROM_MIN_SIZE_KB, s_ceil)
		stats += [
			'fits in',
			'% 4d KByte' % (s_ceil // 1024),
			'% 4d Kbit' % ((s_ceil*8) // 1024),
			#'ROM'
		]
	r = sep.join(stats)
	return r


def analyze(filename):
	put('Filename  : "%s"' % filename)
	
	with open(filename, 'rb') as h:
		data = h.read()
	
	l_full = len(data)
	put('Filesize  : %s' % human_readable(l_full))
	
	l = l_full-1
	while (l >= 0) and (data[l] == 0xff):
		l -= 1
	l += 1
	put('Used space: %s' % human_readable(l))
	
	put('Free space: %s' % human_readable(l_full - l, rom_info=False))
	#put('Free space: %d Bytes' % (l_full - l))
	
	# Check if file is an "aligned" binary (power of two)
	l_log = int(math.log(l_full, 2))
	l_power = math.pow(2, l_log)
	if (l_full == l_power):
		if (l >= l_full):
			put('WARNING: Binary image is full! (%d > %d)' % (l, l_full))
			sys.exit(1)	# Throw error return code
	else:
		# Unaligned
		pass


if __name__ == '__main__':
	if len(sys.argv) < 2:
		put('Specify a .bin file!')
		sys.exit(1)
	
	filename = sys.argv[1]
	analyze(filename)
	sys.exit(0)
	