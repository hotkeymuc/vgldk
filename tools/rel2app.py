#!/usr/bin/python3
"""
rel2app
=======

This script analyzes a SDCC .rel file and extracts relocation information.
Goal is to get a "loadable app" which can be loaded and re-located at run-time.

Made for my "V-Tech Genius Leader" projects using SDCC

2019-07-03 Bernhard "HotKey" Slawik

"""

WRITE_APP_BIN = True	# Write the relocatable binary to a .bin file
WRITE_APP_H = False	# Write out the APP_BIN as an includable .h header file
WRITE_RELOCATED = False	# Attempt a re-location here in python

# Which symbol names should be included in the .app file?
WRITE_ALL_SYMBOL_NAMES = False	# If True: all (!) symbol names will be written to app (can be a lot)
WRITE_SYMBOL_NAMES = ['_main']	# Those symbols will always be written
WRITE_SYMBOLS_IN_AREA = ['ext']	# Symbols in this area are always written


#LOC_CODE = 0x0000	# Should be same as compiler option --code-loc
#LOC_DATA = 0xc000	# Should be same as compiler option --data-loc

AREA_HEADER_BYTE_CODE = 0xC0
AREA_HEADER_BYTE_DATA = 0xDA
AREA_HEADER_BYTE_EXT = 0xE0

APP_SIGNATURE = b'HAPP'	# Maybe use some valid Z80 code?

import os	# For basename
import sys	# For failing
from rel_file import *

def put(txt):
	print(str(txt))


def write_app(filename_base, areas, linked_bin_data):
	
	filename_app_bin = filename_base + '.app.bin'
	filename_app_h = filename_base + '.app.h'
	
	### Create .app structure
	app_data = []
	def write_byte(b):
		app_data.append(b)
	def write_word(w):
		app_data.append(w % 0x100)
		app_data.append(w >> 8)
	def write_str(s):
		write_byte(len(s))
		for c in s:
			write_byte(ord(c))
	
	### Write app_data header file
	#put('Binary size: %d / 0x%04X' % (len(rel_bin_data), len(rel_bin_data)))
	#write_word(len(rel_bin_data))	# Length of binary data
	
	# Write signature
	for c in APP_SIGNATURE:
		#write_byte(ord(c))
		write_byte(c)
	
	put('Binary size: %d / 0x%04X' % (len(linked_bin_data), len(linked_bin_data)))
	write_word(len(linked_bin_data))	# Length of binary data
	
	# Write binary data
	#for b in rel_bin_data:
	for b in linked_bin_data:
		write_byte(b)
	
	
	### Write areas/symbols to app_data
	put('Writing relocation information...')
	# Count actually used symbols
	areas_used = 0
	#for area_name, a in areas.iteritems():	# Python2
	for area_name, a in areas.items():	# Python3
		# Count syms used in this area
		syms_used = 0
		for sym in a.symbols:
			if len(sym.usages) > 0: syms_used += 1
		if syms_used > 0:
			areas_used += 1
	
	write_word(areas_used)	# Number of (actual used) areas
	
	# Write out all used areas
	#for area_name, a in areas.iteritems():	# Python2
	for area_name, a in areas.items():	# Python3
		
		# Count syms used in this area (again...)
		syms_used = 0
		for sym in a.symbols:
			if len(sym.usages) > 0: syms_used += 1
		if syms_used == 0: continue
		
		# Write area header
		put('	* Area "%s"' % (a.name))
		#write_str(a.name)	# Write full area name for better debugging
		
		self.header_byte = 0x00
		if   (a.name == AREA_NAME_CODE): header_byte = AREA_HEADER_BYTE_CODE
		elif (a.name == AREA_NAME_DATA): header_byte = AREA_HEADER_BYTE_DATA
		elif (a.name == AREA_NAME_EXT ): header_byte = AREA_HEADER_BYTE_EXT
		
		write_byte(header_byte)	# Save space by just writing out an identification byte
		
		# Write areas's (used) symbols
		write_word(syms_used)
		for sym in a.symbols:
			if len(sym.usages) == 0: continue
			
			# Write symbol header
			put('		* Symbol "%s" (used %d times)' % (sym.name, len(sym.usages)))
			
			# To safe space: Only include important symbols
			if WRITE_ALL_SYMBOL_NAMES or (sym.name in WRITE_SYMBOL_NAMES) or (a.name in WRITE_SYMBOLS_IN_AREA):
				# Include full name of symbol (for symbolic linking)
				write_str(sym.name)
			else:
				# Unimportant symbol, do not include a name (safe space)
				write_str('')	# ...or just "0x00" for empty string
			
			#write_word(sym.addr)
			write_word(sym.linked_addr)
			
			# Write symbol's usages
			write_word(len(sym.usages))	# Number of usages of this symbol
			for u in sym.usages:
				write_word(u.bin_ofs)
				#write_word(u.sym_ofs)	# This can be looked up at bin_ofs! The value at bin_ofs = sym.linked_addr + u.sym_ofs
				
			
		
	
	put('Relocatable .app size=%d bytes' % (len(app_data)))
	### End of .app structure
	
	
	### Write files
	l = len(app_data)
	
	if WRITE_APP_BIN:
		put('Writing app .bin file "%s"...' % filename_app_bin)
		with open(filename_app_bin, 'wb') as h:
			h.write(bytearray(app_data))
	
	if WRITE_APP_H:
		
		data_name = os.path.basename(filename_base).upper()
		
		put('Writing app .h file "%s" with data named "%s[]"...' % (filename_app_h, data_name))
		with open(filename_app_h, 'w') as h:
			h.write('const unsigned char %s[%d] = {\n\t' % (data_name, l))
			col = 0
			i = 0
			for b in app_data:
				h.write('0x%02X' % b)
				if (i < l-1): h.write(', ')
				
				i += 1
				col += 1
				if (col >= 16):
					h.write('\n\t')
					col = 0
			
			if (col > 0): h.write('\n')
			h.write('};\n')
	
	
	"""
	#@FIXME: Not working atm.!
	if WRITE_RELOCATED:
		### Relocate binary (just for testing - will be done on runtime later on)
		put('Relocating binary...')
		
		new_offsets = {
			'_CODE': 0xc800,
			'_DATA': 0xc400,
			
			'_EXT': 0x8000
		}
		
		for id,sym in symbols.iteritems():
			if len(sym.uses) == 0:
				continue
			
			new_addr = sym.addr
			
			sym_area_name = areas[sym.area]
			new_addr = sym.addr + new_offsets[sym_area_name]
			put('Relocating symbol "%s" (%s) from 0x%04X to 0x%04X in %d places...' % (sym.name, sym_area_name, sym.addr, new_addr, len(sym.uses)))
			
			for u in sym.uses:
				# Replace all uses of this symbol by the new address
				#put('\t* Used at offset %d...' % (u))
				
				# Check if binary confirms that
				org_addr = data[u] + 0x100 * data[u+1]
				if (org_addr != sym.addr):
					put('!! Patch differs! Expected symbol address 0x%04X, but binary says 0x%04X !!' % (sym.addr, org_addr))
					return False
				
				# Apply patch
				data[u] = new_addr % 0x100
				data[u+1] = new_addr >> 8
				
			#
		#
		put('Relocation finished.')
		
		# Write out
		put('Writing relocated bin file "%s"...' % filename_bin_relocated)
		with open(filename_bin_relocated, 'wb') as h:
			h.write(bytearray(data))
	#
	"""
	
	put('Done processing "%s".' % (filename_base))


if __name__ == '__main__':
	#put(__doc__)
	import sys
	
	if (len(sys.argv) > 1):
		filename_rel = sys.argv[1]
		if (len(sys.argv) > 2):
			filename_bin_linked = sys.argv[2]
	else:
		filename_rel = 'out/app_test.rel'
		filename_bin_linked = None
	
	areas, rel_bin_data, linked_bin_data = process_rel_file(filename_rel, filename_bin_linked)
	
	write_app(filename_base=filename_rel, areas=areas, linked_bin_data=linked_bin_data)
	
	sys.exit(0)
