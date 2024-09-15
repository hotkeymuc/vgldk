#!/usr/bin/python3
"""
BIN MERGE
Merge binary files together. Used for creating bank-switched ROMs.

2024-09-15 Bernhard "HotKey" Slawik
"""

import sys
import os
from optparse import OptionParser

def put(t):
	print(str(t))

if __name__ == '__main__':
	
	parser = OptionParser(
		description='Merge two binary files together',
		usage='Usage: %prog [options] src_file dst_file [final_file]'
	)
	
	pad = 0xff
	
	parser.add_option('-s', '--src-ofs', dest='src_ofs', default=0, action='store', type='int', help='Source file offset (default: 0)')
	parser.add_option('-n', '--src-num', dest='src_num', default=-1, action='store', type='int', help='Source data count (default: all)')
	
	parser.add_option('-o', '--dst-ofs', dest='dst_ofs', default=-1, action='store', type='int', help='Destination offset (default: append)')
	
	parser.add_option('-p', '--pad', dest='pad', default=pad, action='store', type='int', help='Padding (default: %d / 0x%02X)' % (pad,pad))
	
	#parser.add_option('-t', '--keep-trailing', dest='keep_trailing', default=not skip_trailing, action='store_true', help='Keep trailing 0xFF and 0x00')
	#parser.add_option('-z', '--skip-zeros', dest='skip_zeros', default=skip_zeros, action='store_true', help='Skip zero bytes')
	#parser.add_option('-v', '--verbose', dest='verbose', default=verbose_traffic, action='store_true', help='Show verbose traffic')
	
	#parser.add_option('-f', '--file', action='store', type='string', dest='filename', help='Binary file to send')
	
	opt, args = parser.parse_args()
	
	src_ofs = opt.src_ofs
	src_num = opt.src_num
	dst_ofs = opt.dst_ofs
	
	# Positional
	if len(args) == 2:
		src_filename = args[0]
		dst_filename = args[1]
		fin_filename = dst_filename
	elif len(args) == 3:
		src_filename = args[0]
		dst_filename = args[1]
		fin_filename = args[2]
	else:
		parser.print_help()
		sys.exit(1)
	
	with open(src_filename, 'rb') as h:
		src_data = h.read()
	src_size = len(src_data)
	
	if src_num < 0:
		src_num = src_len - src_ofs
	
	# Extract source data
	src_data = src_data[src_ofs:src_ofs+src_num]
	
	with open(dst_filename, 'rb') as h:
		dst_data = h.read()
	dst_size = len(dst_data)
	if dst_ofs < 0:
		dst_ofs = dst_size
	
	put('Source file     : "%s" 0x%04X (%d bytes)' % (src_filename, src_size, src_size))
	put('Destination file: "%s" 0x%04X (%d bytes)' % (dst_filename, dst_size, dst_size))
	if (len(src_data) < src_num):
		d = src_num - len(src_data)
		put('Padding source data from 0x%04X (%d) to 0x%04X (%d)... (0x%04X / %d bytes)' % (len(src_data),len(src_data), src_num,src_num, d,d ))
		src_data += bytes([ pad ] * d)
	if (len(dst_data) < dst_ofs):
		d = dst_ofs - len(dst_data)
		put('Padding destination data from 0x%04X (%d) to 0x%04X (%d)... (0x%04X / %d bytes)' % (len(dst_data),len(dst_data), dst_ofs,dst_ofs, d,d ))
		dst_data += bytes([ pad ] * d)
	
	put('Copying from 0x%04X (%d) to 0x%04X (%d)... (0x%04X / %d bytes)' % (src_ofs,src_ofs, dst_ofs,dst_ofs, src_num,src_num))
	dst_data = dst_data[0:dst_ofs] + src_data + (dst_data[dst_ofs+len(src_data):] if (len(dst_data) > dst_ofs+len(src_data)) else b'')
	
	put('Final file      : "%s" 0x%04X (%d bytes)' % (fin_filename, len(dst_data), len(dst_data)))
	#put('Writing back result to "%s"...' % fin_filename)
	with open(fin_filename, 'wb') as h:
		h.write(dst_data)
	
